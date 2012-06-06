
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         gst-audiooutput.cpp  -  description
 *                         ------------------------------------
 *   begin                : Sat 27 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Gstreamer audio output code
 *
 */

#include <glib/gi18n.h>

#include "gst-audiooutput.h"

#include <gst/interfaces/propertyprobe.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappbuffer.h>

#include <string.h>

static gboolean
pipeline_cleaner (GstBus* /*bus*/,
		  GstMessage* message,
		  gpointer pipeline)
{
  bool result = TRUE;

  if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_EOS
      && GST_MESSAGE_SRC (message) == GST_OBJECT_CAST (pipeline)) {

    result = FALSE;
    gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
    g_object_unref (pipeline);
  }

  return result;
}


GST::AudioOutputManager::AudioOutputManager ()
{
  detect_devices ();
}

GST::AudioOutputManager::~AudioOutputManager ()
{
}

void
GST::AudioOutputManager::get_devices (std::vector<Ekiga::AudioOutputDevice>& devices)
{
  detect_devices ();

  for (std::map<std::pair<std::string, std::string>, std::string>::const_iterator iter
	 = devices_by_name.begin ();
       iter != devices_by_name.end ();
       ++iter) {

    Ekiga::AudioOutputDevice device;
    device.type = "GStreamer";
    device.source = iter->first.first;
    device.name = iter->first.second;
    devices.push_back (device);
  }
}

bool
GST::AudioOutputManager::set_device (Ekiga::AudioOutputPS ps,
				     const Ekiga::AudioOutputDevice& device)
{
  bool result = false;

  if (device.type == "GStreamer"
      && devices_by_name.find (std::pair<std::string,std::string>(device.source, device.name)) != devices_by_name.end ()) {

    unsigned ii = (ps == Ekiga::primary)?0:1;
    current_state[ii].opened = false;
    current_state[ii].device = device;
    result = true;
  }
  return result;
}

bool
GST::AudioOutputManager::open (Ekiga::AudioOutputPS ps,
			       unsigned channels,
			       unsigned samplerate,
			       unsigned bits_per_sample)
{
  bool result = false;
  unsigned ii = (ps == Ekiga::primary)?0:1;
  gchar* command = NULL;
  GError* error = NULL;
  GstState current;
  command = g_strdup_printf ("appsrc is-live=true name=ekiga_src"
			     " ! audio/x-raw-int"
			     ",rate=%d"
			     ",channels=%d"
			     ",width=%d"
			     ",depth=%d"
			     ",signed=true,endianness=1234"
			     " ! %s",
			     samplerate, channels, bits_per_sample, bits_per_sample,
			     devices_by_name[std::pair<std::string,std::string>(current_state[ii].device.source, current_state[ii].device.name)].c_str ());
  //g_print ("Pipeline: %s\n", command);
  pipeline[ii] = gst_parse_launch (command, &error);

  if (error == NULL) {

    (void)gst_element_set_state (pipeline[ii], GST_STATE_PLAYING);

    // this will make us wait so we can return the right value...
    (void)gst_element_get_state (pipeline[ii],
				 &current,
				 NULL,
				 GST_SECOND);

    if ( !(current == GST_STATE_PLAYING
	   || current == GST_STATE_PAUSED)) {

      gst_element_set_state (pipeline[ii], GST_STATE_NULL);
      gst_object_unref (GST_OBJECT (pipeline[ii]));
      pipeline[ii] = NULL;
      result = false;
    } else {

      Ekiga::AudioOutputSettings settings;
      GstElement* volume = NULL;
      gfloat val;

      volume = gst_bin_get_by_name (GST_BIN (pipeline[ii]), "ekiga_volume");
      if (volume != NULL) {

	g_object_get (G_OBJECT (volume),
		      "volume", &val,
		      NULL);
	settings.volume = (unsigned)(255*val);
	settings.modifyable = true;
	g_object_unref (volume);
      } else {

	settings.modifyable = false;
      }
      current_state[ii].channels = channels;
      current_state[ii].samplerate = samplerate;
      current_state[ii].bits_per_sample = bits_per_sample;
      device_opened.emit (ps, current_state[ii].device, settings);
      result = true;
    }

  } else {

    g_error_free (error);
    result = false;
  }

  g_free (command);

  current_state[ii].opened = result;

//   std::cout << __PRETTY_FUNCTION__
// 	    << " result=";
//   if (result)
//     std::cout << "TRUE";
//   else
//     std::cout << "FALSE";
//   std::cout << std::endl;

  return result;
}

void
GST::AudioOutputManager::close (Ekiga::AudioOutputPS ps)
{
  unsigned ii = (ps == Ekiga::primary)?0:1;
  if (pipeline[ii] != NULL) {

    GstElement* src = gst_bin_get_by_name (GST_BIN (pipeline[ii]), "ekiga_src");

    if (src != NULL) {

      gst_app_src_end_of_stream (GST_APP_SRC (src));
      GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline[ii]));
      gst_bus_add_watch (bus, pipeline_cleaner, pipeline[ii]);
      gst_object_unref (bus);
      pipeline[ii] = NULL;
      device_closed.emit (ps, current_state[ii].device);
    }
  }
  current_state[ii].opened = false;
}

void
GST::AudioOutputManager::set_buffer_size (Ekiga::AudioOutputPS ps,
					  unsigned buffer_size,
					  unsigned /*num_buffers*/)
{
  unsigned ii = (ps == Ekiga::primary)?0:1;
  GstElement* src = NULL;

  src = gst_bin_get_by_name (GST_BIN (pipeline[ii]), "ekiga_src");

  if (src != NULL) {

    g_object_set (G_OBJECT (src),
		  "blocksize", buffer_size,
		  NULL);
    g_object_unref (src);
  }
}

bool
GST::AudioOutputManager::set_frame_data (Ekiga::AudioOutputPS ps,
					 const char* data,
					 unsigned size,
					 unsigned& written)
{
  bool result = false;
  unsigned ii = (ps == Ekiga::primary)?0:1;
  gchar* tmp = NULL;
  GstBuffer* buffer = NULL;
  GstElement* src = NULL;

  written = 0;

  g_return_val_if_fail (GST_IS_BIN (pipeline[ii]), false);

  src = gst_bin_get_by_name (GST_BIN (pipeline[ii]), "ekiga_src");

  if (src != NULL) {

    tmp = (gchar*)g_malloc0 (size);
    memcpy (tmp, data, size);
    buffer = gst_app_buffer_new (tmp, size,
				 (GstAppBufferFinalizeFunc)g_free, tmp);
    gst_app_src_push_buffer (GST_APP_SRC (src), buffer);
    written = size;
    result = true;
    g_object_unref (src);
  }

  return result;
}

void
GST::AudioOutputManager::set_volume (Ekiga::AudioOutputPS ps,
				     unsigned valu)
{
  unsigned ii = (ps == Ekiga::primary)?0:1;
  GstElement* volume = NULL;
  gfloat valf;

  valf = valu / 255.0;

  volume = gst_bin_get_by_name (GST_BIN (pipeline[ii]), "ekiga_volume");
  if (volume != NULL) {

    g_object_set (G_OBJECT (volume),
		  "volume", valf,
		  NULL);
    g_object_unref (volume);
  }
}

bool
GST::AudioOutputManager::has_device (const std::string& source,
				     const std::string& device_name,
				     Ekiga::AudioOutputDevice& /*device*/)
{
  return (devices_by_name.find (std::pair<std::string,std::string>(source, device_name)) != devices_by_name.end ());
}

void
GST::AudioOutputManager::detect_devices ()
{
  devices_by_name.clear ();
  detect_fakesink_devices ();
  detect_alsasink_devices ();
  detect_pulsesink_devices ();
  detect_sdlsink_devices ();
devices_by_name[std::pair<std::string,std::string>("FILE","/tmp/sound.wav")] = "volume name=ekiga_volume ! filesink location=/tmp/sound.wav";
}

void
GST::AudioOutputManager::detect_fakesink_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("fakesink", "fakesinkpresencetest");

  if (elt != NULL) {

    devices_by_name[std::pair<std::string,std::string>(_("Silent"), _("Silent"))] = "fakesink";
    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::AudioOutputManager::detect_alsasink_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("alsasink", "alsasinkpresencetest");

  if (elt != NULL) {

    GstPropertyProbe* probe = NULL;
    const GParamSpec* pspec = NULL;
    GValueArray* array = NULL;

    gst_element_set_state (elt, GST_STATE_PAUSED);
    probe = GST_PROPERTY_PROBE (elt);
    pspec = gst_property_probe_get_property (probe, "device");

    array = gst_property_probe_probe_and_get_values (probe, pspec);
    if (array != NULL) {

      for (guint index = 0; index < array->n_values; index++) {

	GValue* device = NULL;
	gchar* name = NULL;
	gchar* descr = NULL;

	device = g_value_array_get_nth (array, index);
	g_object_set_property (G_OBJECT (elt), "device", device);
	g_object_get (G_OBJECT (elt), "device-name", &name, NULL);
	descr = g_strdup_printf ("volume name=ekiga_volume ! alsasink device=%s",
				 g_value_get_string (device));

	if (name != 0) {

	  devices_by_name[std::pair<std::string,std::string>("ALSA", name)] = descr;
	  g_free (name);
	}
	g_free (descr);
      }
      g_value_array_free (array);
    }

    devices_by_name[std::pair<std::string,std::string>("ALSA","---")] = "volume name=ekiga_volume ! alsasink";

    gst_element_set_state (elt, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::AudioOutputManager::detect_pulsesink_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("pulsesink", "pulsesinkpresencetest");

  if (elt != NULL) {

    GstPropertyProbe* probe = NULL;
    const GParamSpec* pspec = NULL;
    GValueArray* array = NULL;

    gst_element_set_state (elt, GST_STATE_PAUSED);
    probe = GST_PROPERTY_PROBE (elt);
    pspec = gst_property_probe_get_property (probe, "device");

    array = gst_property_probe_probe_and_get_values (probe, pspec);
    if (array != NULL) {

      for (guint index = 0; index < array->n_values; index++) {

	GValue* device = NULL;
	gchar* name = NULL;
	gchar* descr = NULL;

	device = g_value_array_get_nth (array, index);
	g_object_set_property (G_OBJECT (elt), "device", device);
	g_object_get (G_OBJECT (elt), "device-name", &name, NULL);
	descr = g_strdup_printf ("volume name=ekiga_volume ! pulsesink device=%s",
				 g_value_get_string (device));

	if (name != 0) {

	  devices_by_name[std::pair<std::string,std::string>("PULSEAUDIO", name)] = descr;

	  g_free (name);
	}
	g_free (descr);
      }
      g_value_array_free (array);
    }

    gst_element_set_state (elt, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::AudioOutputManager::detect_sdlsink_devices ()
{
  gchar* descr = NULL;
  descr = g_strdup_printf ("volume name=ekiga_volume ! sdlaudiosink");
  devices_by_name[std::pair<std::string,std::string>("SDL", "Default")] = descr;
  g_free (descr);
}
