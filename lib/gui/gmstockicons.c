
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
 *  Authors: Jorn Baayen <jorn@nl.linux.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "../../pixmaps/inlines.h"
#include "gmstockicons.h"

/**
 * gnomemeeting_stock_icons_init:
 *
 * Initializes the GnomeMeeting stock icons 
 *
 **/
void
gnomemeeting_stock_icons_init (void)
{
	GtkIconFactory *factory;
	int i;

        typedef struct 
        {
                char *id;
                const guint8 *data;
        } GmStockIcon;                

	static const GmStockIcon items[] =
	{
	        { GM_STOCK_COLOR_BRIGHTNESS_CONTRAST, gm_color_brightness_contrast_stock_data },

		{ GM_STOCK_STATUS_ONLINE, gm_status_online_stock_data },
		{ GM_STOCK_STATUS_OFFLINE, gm_status_offline_stock_data },
		{ GM_STOCK_STATUS_UNKNOWN, gm_status_unknown_stock_data },
		{ GM_STOCK_STATUS_AWAY, gm_status_away_stock_data },
		{ GM_STOCK_STATUS_DND, gm_status_dnd_stock_data },
		{ GM_STOCK_STATUS_INACALL, gm_status_inacall_stock_data_16 },
		{ GM_STOCK_STATUS_RINGING, gm_status_ringing_stock_data_16 },

		{ GM_STOCK_REMOTE_OBJECT, gm_remote_contact_stock_data},
		{ GM_STOCK_LOCAL_OBJECT, gm_local_contact_stock_data},
		{ GM_STOCK_MESSAGE, gm_message_stock_data},
		{ GM_STOCK_CALL_PLACED, gm_call_placed_stock_data},
		{ GM_STOCK_CALL_MISSED, gm_call_missed_stock_data},
		{ GM_STOCK_CALL_RECEIVED, gm_call_received_stock_data},
		{ GM_STOCK_CALL_TRANSFER, gm_call_transfer_stock_data},

		{ GM_STOCK_PHONE_HANG_UP_16, gm_phone_hang_up_stock_data_16},
		{ GM_STOCK_PHONE_PICK_UP_16, gm_phone_pick_up_stock_data_16},
		{ GM_STOCK_PHONE_HANG_UP_24, gm_phone_hang_up_stock_data_24},
		{ GM_STOCK_PHONE_PICK_UP_24, gm_phone_pick_up_stock_data_24},
	};

        typedef struct 
        {
                char *id;
                gint size;
                const guint8 *data;
        } GmThemeIcon;                

	static const GmThemeIcon theme_builtins[] =
	{
		{ GM_ICON_ADD_CONTACT, 24, gm_add_contact_24_stock_data},
	        { GM_ICON_AUDIO_VOLUME_HIGH, 16, gm_audio_volume_high_16_stock_data },
		{ GM_ICON_BRIGHTNESS, 16, gm_brightness_16_stock_data},
		{ GM_ICON_CAMERA_VIDEO, 16, gm_camera_video_16_stock_data },
		{ GM_ICON_COLOURNESS, 16, gm_colourness_16_stock_data},
		{ GM_ICON_CONTRAST, 16, gm_contrast_16_stock_data},
		{ GM_ICON_INTERNET_GROUP_CHAT, 24, gm_internet_group_chat_24_stock_data },
	        { GM_ICON_LOGO, 16, gm_logo_16_stock_data },
	        { GM_ICON_LOGO, 22, gm_logo_22_stock_data },
	        { GM_ICON_LOGO, 32, gm_logo_32_stock_data },
	        { GM_ICON_LOGO, 48, gm_logo_48_stock_data },
	        { GM_ICON_LOGO, 72, gm_logo_72_stock_data },
		{ GM_ICON_MEDIA_PLAYBACK_PAUSE, 16, gm_media_playback_pause_16_stock_data },
		{ GM_ICON_MICROPHONE, 24, gm_microphone_24_stock_data },
		{ GM_ICON_SYSTEM_SEARCH, 16, gm_system_search_16_stock_data},
		{ GM_ICON_WHITENESS, 16, gm_whiteness_16_stock_data},
	};

	/* First, register honest-to-goodness custom stock icons */
	factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (factory);

	for (i = 0; i < (int) G_N_ELEMENTS (items); i++)
	{
		GtkIconSet *icon_set;
		GdkPixbuf *pixbuf;

                pixbuf = gdk_pixbuf_new_from_inline (-1, items[i].data, FALSE, NULL);

		icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);

		gtk_icon_factory_add (factory, items[i].id, icon_set);
		gtk_icon_set_unref (icon_set);
		
		g_object_unref (G_OBJECT (pixbuf));
	}

	g_object_unref (G_OBJECT (factory));
	
	/* Now install theme builtins */
	for (i = 0; i < (int) G_N_ELEMENTS (theme_builtins); i++)
	{
		GdkPixbuf *pixbuf;

		pixbuf = gdk_pixbuf_new_from_inline (-1, theme_builtins[i].data,
						     FALSE, NULL);

		gtk_icon_theme_add_builtin_icon (theme_builtins[i].id,
						 theme_builtins[i].size, pixbuf);

		g_object_unref (G_OBJECT (pixbuf));
	}
}
