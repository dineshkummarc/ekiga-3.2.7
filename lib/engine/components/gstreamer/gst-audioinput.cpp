
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
 *                         gst-audioinput.cpp  -  description
 *                         ------------------------------------
 *   begin                : Tue 23 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Gstreamer audio input code
 *
 */

#include <glib/gi18n.h>

#include "gst-audioinput.h"

#include <gst/interfaces/propertyprobe.h>
#include <gst/app/gstappsink.h>

#include <string.h>

GST::AudioInputManager::AudioInputManager ()
{
  detect_devices ();
}

GST::AudioInputManager::~AudioInputManager ()
{
}

void
GST::AudioInputManager::get_devices (std::vector<Ekiga::AudioInputDevice>& devices)
{
  detect_devices ();

  for (std::map<std::pair<std::string, std::string>, std::string>::const_iterator iter
	 = devices_by_name.begin ();
       iter != devices_by_name.end ();
       ++iter) {

    Ekiga::AudioInputDevice device;
    device.type = "GStreamer";
    device.source = iter->first.first;
    device.name = iter->first.second;
    devices.push_back (device);
  }
}

bool
GST::AudioInputManager::set_device (const Ekiga::AudioInputDevice& device)
{
  bool result = false;

  if (device.type == "GStreamer"
      && devices_by_name.find (std::pair<std::string, std::string>(device.source, device.name)) != devices_by_name.end ()) {

    current_state.opened = false;
    current_state.device = device;
    result = true;
  }
  return result;
}

bool
GST::AudioInputManager::open (unsigned channels,
			      unsigned samplerate,
			      unsigned bits_per_sample)
{
  bool result = false;
  gchar* command = NULL;
  GError* error = NULL;
  GstState current;

  command = g_strdup_printf ("%s ! appsink max_buffers=2 drop=true"
			     " caps=audio/x-raw-int"
			     ",rate=%d"
			     ",channels=%d"
			     ",width=%d"
			     " name=ekiga_sink",
			     devices_by_name[std::pair<std::string,std::string>(current_state.device.source, current_state.device.name)].c_str (),
			     samplerate, channels, bits_per_sample);
  //g_print ("Pipeline: %s\n", command);
  pipeline = gst_parse_launch (command, &error);

  if (error == NULL) {

    (void)gst_element_set_state (pipeline, GST_STATE_PLAYING);

    // this will make us wait so we can return the right value...
    (void)gst_element_get_state (pipeline,
				 &current,
				 NULL,
				 GST_SECOND);

    if (current != GST_STATE_PLAYING) {

      gst_element_set_state (pipeline, GST_STATE_NULL);
      gst_object_unref (GST_OBJECT (pipeline));
      pipeline = NULL;
      result = false;
    } else {

      Ekiga::AudioInputSettings settings;
      GstElement* volume = NULL;
      gfloat val;

      volume = gst_bin_get_by_name (GST_BIN (pipeline), "ekiga_volume");
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
      current_state.channels = channels;
      current_state.samplerate = samplerate;
      current_state.bits_per_sample = bits_per_sample;
      device_opened.emit (current_state.device, settings);
      result = true;
    }

  } else {

    g_error_free (error);
    result = false;
  }

  g_free (command);

  current_state.opened = result;
  return result;
}

void
GST::AudioInputManager::close ()
{
  if (pipeline != NULL) {

    gst_element_set_state (pipeline, GST_STATE_NULL);
    g_object_unref (pipeline);
    pipeline = NULL;
    device_closed.emit (current_state.device);
  }
  current_state.opened = false;
}

void
GST::AudioInputManager::set_buffer_size (unsigned /*buffer_size*/,
					 unsigned /*num_buffers*/)
{
// FIXME: this is bug #554168 -- GstAppSink doesn't have "blocksize" yet!
//   GstElement* sink = NULL;

//   sink = gst_bin_get_by_name (GST_BIN (pipeline), "ekiga_sink");

//   if (sink != NULL) {

//     g_object_set (G_OBJECT (sink),
// 		  "blocksize", buffer_size,
// 		  NULL);
//     g_object_unref (sink);
//   }
}

bool
GST::AudioInputManager::get_frame_data (char* data,
					unsigned size,
					unsigned& read)
{
  bool result = false;
  GstBuffer* buffer = NULL;
  GstElement* sink = NULL;

  read = 0;

  g_return_val_if_fail (GST_IS_BIN (pipeline), false);

  sink = gst_bin_get_by_name (GST_BIN (pipeline), "ekiga_sink");

  if (sink != NULL) {

    buffer = gst_app_sink_pull_buffer (GST_APP_SINK (sink));

    if (buffer != NULL) {

      read = MIN (GST_BUFFER_SIZE (buffer), size);
      memcpy (data, GST_BUFFER_DATA (buffer), read);
      result = true;
      gst_buffer_unref (buffer);
    }
    g_object_unref (sink);
  }

  return result;
}

void
GST::AudioInputManager::set_volume (unsigned valu)
{
  GstElement* volume = NULL;
  gfloat valf;

  valf = valu / 255.0;

  volume = gst_bin_get_by_name (GST_BIN (pipeline), "ekiga_volume");
  if (volume != NULL) {

    g_object_set (G_OBJECT (volume),
		  "volume", valf,
		  NULL);
    g_object_unref (volume);
  }
}

bool
GST::AudioInputManager::has_device (const std::string& source,
				    const std::string& device_name,
				    Ekiga::AudioInputDevice& /*device*/)
{
  return (devices_by_name.find (std::pair<std::string,std::string>(source, device_name)) != devices_by_name.end ());
}

void
GST::AudioInputManager::detect_devices ()
{
  devices_by_name.clear ();
  detect_audiotestsrc_devices ();
  detect_alsasrc_devices ();
}

void
GST::AudioInputManager::detect_audiotestsrc_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("audiotestsrc", "audiotestsrcpresencetest");

  if (elt != NULL) {

    devices_by_name[std::pair<std::string,std::string>(_("Audio test"),_("Audio test"))] = "audiotestsrc name=ekiga_volume";
    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::AudioInputManager::detect_alsasrc_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("alsasrc", "alsasrcpresencetest");

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
	descr = g_strdup_printf ("alsasrc device=%s ! volume name=ekiga_volume",
				 g_value_get_string (device));

	if (name != 0) {

	  devices_by_name[std::pair<std::string,std::string>("ALSA", name)] = descr;
	  g_free (name);
	}
	g_free (descr);
      }
      g_value_array_free (array);
    }

    devices_by_name[std::pair<std::string,std::string>("ALSA","---")] = "volume name=ekiga_volume ! alsasrc";

    gst_element_set_state (elt, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::AudioInputManager::detect_pulsesrc_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("pulsesrc", "pulsesrcpresencetest");

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
	descr = g_strdup_printf ("pulsesrc device=%s ! volume name=ekiga_volume",
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
