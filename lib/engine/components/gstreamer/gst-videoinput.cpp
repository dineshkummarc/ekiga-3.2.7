
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
 *                         gst-videoinput.cpp  -  description
 *                         ------------------------------------
 *   begin                : Wed 17 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Gstreamer video input code
 *
 */

#include <glib/gi18n.h>

#include "gst-videoinput.h"

#include <gst/interfaces/propertyprobe.h>
#include <gst/app/gstappsink.h>

#include <string.h>

GST::VideoInputManager::VideoInputManager (): pipeline(NULL)
{
  detect_devices (); // or we won't recognize the devices we'll be asked to use
}

GST::VideoInputManager::~VideoInputManager ()
{
  if (pipeline != NULL)
    g_object_unref (pipeline);
  pipeline = NULL;
}

void
GST::VideoInputManager::get_devices (std::vector<Ekiga::VideoInputDevice>& devices)
{
  detect_devices ();

  for (std::map<std::pair<std::string, std::string>, std::string>::const_iterator iter
	 = devices_by_name.begin ();
       iter != devices_by_name.end ();
       ++iter) {

    Ekiga::VideoInputDevice device;
    device.type = "GStreamer";
    device.source = iter->first.first;
    device.name = iter->first.second;
    devices.push_back (device);
  }
}

bool
GST::VideoInputManager::set_device (const Ekiga::VideoInputDevice& device,
				    int channel,
				    Ekiga::VideoInputFormat format)
{
  bool result = false;

  if (device.type == "GStreamer"
      && devices_by_name.find (std::pair<std::string, std::string>(device.source, device.name)) != devices_by_name.end ()) {

    current_state.opened = false;
    current_state.width = 320;
    current_state.height = 240;
    current_state.fps = 30;
    current_state.device = device;
    current_state.format = format;
    current_state.channel = channel;

    result = true;
  }

  return result;
}

bool
GST::VideoInputManager::open (unsigned width,
			      unsigned height,
			      unsigned fps)
{
  bool result;
  gchar* command = NULL;
  GError* error = NULL;
  GstState current;

  command = g_strdup_printf ("%s ! appsink max_buffers=2 drop=true"
			     " caps=video/x-raw-yuv"
			     ",format=(fourcc)I420"
			     ",width=%d,height=%d"
			     ",framerate=(fraction)%d/1"
			     " name=ekiga_sink",
			     devices_by_name[std::pair<std::string,std::string>(current_state.device.source, current_state.device.name)].c_str (),
			     width, height, fps);
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

      Ekiga::VideoInputSettings settings;
      settings.modifyable = false;
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
GST::VideoInputManager::close ()
{
  if (pipeline != NULL) {

    device_closed.emit (current_state.device);
    g_object_unref (pipeline);
    pipeline = NULL;
  }
  current_state.opened = false;
}

bool
GST::VideoInputManager::get_frame_data (char* data)
{
  bool result = false;
  GstBuffer* buffer = NULL;
  GstElement* sink = NULL;

  g_return_val_if_fail (GST_IS_BIN (pipeline), false);

  sink = gst_bin_get_by_name (GST_BIN (pipeline), "ekiga_sink");

  if (sink != NULL) {

    buffer = gst_app_sink_pull_buffer (GST_APP_SINK (sink));

    if (buffer != NULL) {

      uint size = MIN (GST_BUFFER_SIZE (buffer),
		       current_state.width * current_state.height * 3 / 2);
      memcpy (data, GST_BUFFER_DATA (buffer), size);
      result = true;
      gst_buffer_unref (buffer);
    }
    g_object_unref (sink);
  }

  return result;
}

bool
GST::VideoInputManager::has_device (const std::string& source,
				    const std::string& device_name,
				    G_GNUC_UNUSED unsigned capabilities,
				    G_GNUC_UNUSED Ekiga::VideoInputDevice& device)
{
  return (devices_by_name.find (std::pair<std::string,std::string> (source, device_name)) != devices_by_name.end ());
}

void
GST::VideoInputManager::detect_devices ()
{
  devices_by_name.clear ();
  detect_videotestsrc_devices ();
  detect_v4l2src_devices ();
  detect_dv1394src_devices ();
  detect_crazy_devices ();
}

void
GST::VideoInputManager::detect_videotestsrc_devices ()
{
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("videotestsrc", "videotestsrcpresencetest");

  if (elt != NULL) {

    devices_by_name[std::pair<std::string,std::string>(_("Video test"),_("Video test"))] = "videotestsrc";
    gst_object_unref (GST_OBJECT (elt));
  }
}

void
GST::VideoInputManager::detect_v4l2src_devices ()
{
  bool problem = false;
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("videoscale", "videoscalepresencetest");
  if (elt == NULL)
    problem = true;
  else
    gst_object_unref (elt);

  elt = gst_element_factory_make ("ffmpegcolorspace",
				  "ffmpegcolorspacepresencetest");
  if (elt == NULL)
    problem = true;
  else
    gst_object_unref (elt);

  elt = gst_element_factory_make ("v4l2src", "v4l2srcpresencetest");

  if (elt != NULL && problem == false) {

    GstPropertyProbe* probe = NULL;
    const GParamSpec* pspec = NULL;
    GValueArray* array = NULL;

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
	descr = g_strdup_printf ("v4l2src device=%s"
				 " ! videoscale ! ffmpegcolorspace",
				 g_value_get_string (device));
	if (name != 0) {

	  devices_by_name[std::pair<std::string,std::string>("V4L2",name)] = descr;
	  g_free (name);
	}
	g_free (descr);
      }

      g_value_array_free (array);
      gst_element_set_state (elt, GST_STATE_NULL);
    }
  }
  if (elt != NULL)
    gst_object_unref (GST_OBJECT (elt));
}

void
GST::VideoInputManager::detect_dv1394src_devices ()
{
  bool problem = false;
  GstElement* elt = NULL;

  elt = gst_element_factory_make ("videoscale", "videoscalepresencetest");
  if (elt == NULL)
    problem = true;
  else
    gst_object_unref (elt);

  elt = gst_element_factory_make ("decodebin", "decodebinpresencetest");
  if (elt == NULL)
    problem = true;
  else
    gst_object_unref (elt);

  elt = gst_element_factory_make ("ffmpegcolorspace",
				  "ffmpegcolorspacepresencetest");
  if (elt == NULL)
    problem = true;
  else
    gst_object_unref (elt);

  elt = gst_element_factory_make ("dv1394src", "dv1394srcpresencetest");

  if (elt != NULL && problem == false) {

    GstPropertyProbe* probe = NULL;
    const GParamSpec* pspec = NULL;
    GValueArray* array = NULL;

    probe = GST_PROPERTY_PROBE (elt);
    pspec = gst_property_probe_get_property (probe, "guid");
    array = gst_property_probe_probe_and_get_values (probe, pspec);

    if (array != NULL) {

      for (guint index = 0; index < array->n_values; index++) {

	GValue* guid = NULL;
	gchar* name = NULL;
	gchar* descr = NULL;
	guid = g_value_array_get_nth (array, index);
	g_object_set_property (G_OBJECT (elt), "guid", guid);

	g_object_get (G_OBJECT (elt), "device-name", &name, NULL);
	descr = g_strdup_printf ("dv1394src guid=%Ld"
				 " ! decodebin"
				 " ! videoscale"
				 " ! ffmpegcolorspace",
				 g_value_get_uint64 (guid));
	if (name != 0) {

	  devices_by_name[std::pair<std::string,std::string>("DV",name)] = descr;
	  g_free (name);
	}
	g_free (descr);
      }

      g_value_array_free (array);
      gst_element_set_state (elt, GST_STATE_NULL);
    }
  }
  if (elt != NULL)
    gst_object_unref (GST_OBJECT (elt));
}

void
GST::VideoInputManager::detect_crazy_devices ()
{
  GstElement* goom = NULL;
  GstElement* audiotest = NULL;
  GstElement* ffmpeg = NULL;
  GstElement* scale = NULL;
  GstElement* ximage = NULL;

  goom = gst_element_factory_make ("goom", "goompresencetest");
  audiotest = gst_element_factory_make ("audiotestsrc", "audiotestsrcpresencetest");
  ffmpeg = gst_element_factory_make ("ffmpegcolorspace", "ffmpegcolorspacepresencetest");
  scale = gst_element_factory_make ("videoscale", "videoscalepresencetest");
  ximage = gst_element_factory_make ("ximagesrc", "ximagesrcpresencetest");

  if (goom != NULL && audiotest != NULL && ffmpeg != NULL)
    devices_by_name[std::pair<std::string,std::string>(_("Crazy"), "Goom")] = "audiotestsrc ! goom ! ffmpegcolorspace";

  if (ximage != NULL && ffmpeg != NULL) {

    /* Translators: "Screencast" means the video input device will be your screen -- the other end will see your desktop */
    devices_by_name[std::pair<std::string,std::string>(_("Crazy"),_("Screencast"))] = "ximagesrc ! videoscale ! ffmpegcolorspace";
  }

  if (goom != NULL)
    gst_object_unref (goom);
  if (audiotest != NULL)
    gst_object_unref (audiotest);
  if (scale != NULL)
    gst_object_unref (scale);
  if (ffmpeg != NULL)
    gst_object_unref (ffmpeg);
  if (ximage != NULL)
    gst_object_unref (ximage);
}
