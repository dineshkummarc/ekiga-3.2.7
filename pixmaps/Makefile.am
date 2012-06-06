icondir = $(datadir)/icons/hicolor
pixmapdir = $(datadir)/pixmaps/@PACKAGE_NAME@

pixmap_DATA = @PACKAGE_NAME@-logo.png

nobase_dist_icon_DATA = \
	16x16/apps/@PACKAGE_NAME@.png \
	22x22/apps/@PACKAGE_NAME@.png \
	32x32/apps/@PACKAGE_NAME@.png \
	48x48/apps/@PACKAGE_NAME@.png \
	64x64/apps/@PACKAGE_NAME@.png \
	72x72/apps/@PACKAGE_NAME@.png \
	128x128/apps/@PACKAGE_NAME@.png


IMAGES_FILES= \
	48x48/multimedia-headset.png \
	22x22/multimedia-headset.png \
	22x22/multimedia-headset.svg \
	128x128/multimedia-headset.png \
	128x128/multimedia-headset.svg \
	16x16/multimedia-headset.png \
	16x16/multimedia-headset.svg \
	scalable/multimedia-headset.svg \
	32x32/multimedia-headset.png \
	32x32/multimedia-headset.svg \
	256x256/multimedia-headset.png \
	16x16/audio-volume-high.png \
	16x16/camera-video.png \
	16x16/media-playback-pause.png \
	16x16/stock_brightness.png \
	16x16/stock_channel-blue.png \
	16x16/stock_color.png \
	16x16/stock_contrast.png \
	16x16/system-search.png \
	24x24/audio-input-microphone.png \
	24x24/contact-new.png \
	24x24/internet-group-chat.png \
	color-brightness-contrast.png \
	local_contact.png \
	remote_contact.png \
	message.png \
	call-placed.png \
	call-missed.png \
	call-received.png \
	call-transfer.png \
	status-online.png \
	status-offline.png \
	status-unknown.png \
	status-away.png \
	status-freeforchat.png \
	status-dnd.png \
	status-inacall.png \
	status-ringing.png \
	ekiga-logo.png \
	24x24/phone-hang-up.png \
	24x24/phone-pick-up.png \
	16x16/phone-hang-up.png \
	16x16/phone-pick-up.png \
	gm_powermeter_default_00.xpm \
	gm_powermeter_default_01.xpm \
	gm_powermeter_default_02.xpm \
	gm_powermeter_default_03.xpm \
	gm_powermeter_default_04.xpm \
	icon.h


IMAGES= \
	gm_add_contact_24_stock_data $(srcdir)/24x24/contact-new.png \
	gm_audio_volume_high_16_stock_data $(srcdir)/16x16/audio-volume-high.png \
	gm_brightness_16_stock_data $(srcdir)/16x16/stock_brightness.png \
	gm_camera_video_16_stock_data $(srcdir)/16x16/camera-video.png \
	gm_colourness_16_stock_data $(srcdir)/16x16/stock_color.png \
	gm_contrast_16_stock_data $(srcdir)/16x16/stock_contrast.png \
	gm_internet_group_chat_24_stock_data $(srcdir)/24x24/internet-group-chat.png \
	gm_logo_16_stock_data $(srcdir)/16x16/apps/@PACKAGE_NAME@.png \
	gm_logo_22_stock_data $(srcdir)/22x22/apps/@PACKAGE_NAME@.png \
	gm_logo_32_stock_data $(srcdir)/32x32/apps/@PACKAGE_NAME@.png \
	gm_logo_48_stock_data $(srcdir)/48x48/apps/@PACKAGE_NAME@.png \
	gm_logo_72_stock_data $(srcdir)/72x72/apps/@PACKAGE_NAME@.png \
	gm_media_playback_pause_16_stock_data $(srcdir)/16x16/media-playback-pause.png \
	gm_microphone_24_stock_data $(srcdir)/24x24/audio-input-microphone.png \
	gm_system_search_16_stock_data $(srcdir)/16x16/system-search.png \
	gm_whiteness_16_stock_data $(srcdir)/16x16/stock_channel-blue.png \
	gm_color_brightness_contrast_stock_data $(srcdir)/color-brightness-contrast.png \
	gm_status_freeforchat_stock_data $(srcdir)/status-freeforchat.png \
	gm_remote_contact_stock_data $(srcdir)/remote_contact.png \
	gm_local_contact_stock_data $(srcdir)/local_contact.png \
	gm_message_stock_data $(srcdir)/message.png \
	gm_call_placed_stock_data $(srcdir)/call-placed.png \
	gm_call_received_stock_data $(srcdir)/call-received.png \
	gm_call_missed_stock_data $(srcdir)/call-missed.png \
	gm_call_transfer_stock_data $(srcdir)/call-transfer.png \
	gm_status_online_stock_data $(srcdir)/status-online.png \
	gm_status_offline_stock_data $(srcdir)/status-offline.png \
	gm_status_unknown_stock_data $(srcdir)/status-unknown.png \
	gm_status_dnd_stock_data $(srcdir)/status-dnd.png \
	gm_status_inacall_stock_data_16 $(srcdir)/16x16/multimedia-headset.png \
	gm_status_ringing_stock_data_16 $(srcdir)/status-ringing.png \
	gm_status_away_stock_data $(srcdir)/status-away.png \
	gm_phone_hang_up_stock_data_24 $(srcdir)/24x24/phone-hang-up.png \
	gm_phone_pick_up_stock_data_24 $(srcdir)/24x24/phone-pick-up.png \
	gm_phone_hang_up_stock_data_16 $(srcdir)/16x16/phone-hang-up.png \
	gm_phone_pick_up_stock_data_16 $(srcdir)/16x16/phone-pick-up.png 


noinst_DATA = inlines.h

DISTCLEANFILES = inlines.h

EXTRA_DIST= $(IMAGES_FILES)

inlines.h: $(IMAGE_FILES) $(nobase_dist_icon_DATA) Makefile.am
	gdk-pixbuf-csource --raw --build-list $(IMAGES) > inlines.h

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache

uninstall-hook: update-icon-cache

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

