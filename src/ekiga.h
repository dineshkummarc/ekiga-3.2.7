
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
 *                         gnomemeeting.h  -  description
 *                         ------------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the main class
 *
 */


#ifndef _GNOMEMEETING_H_
#define _GNOMEMEETING_H_

#include "config.h"

#include "common.h"

#include "services.h"
#include "runtime.h"

#include "statusicon.h"

#include <ptlib/ipsock.h>


/**
 * COMMON NOTICE: The Application must be initialized with Init after its
 * creation.
 */
class GnomeMeeting : public PProcess
{
  PCLASSINFO(GnomeMeeting, PProcess);

 public:


  /* DESCRIPTION  :  Constructor.
   * BEHAVIOR     :  Init variables.
   * PRE          :  /
   */
  GnomeMeeting ();


  /* DESCRIPTION  :  Destructor.
   * BEHAVIOR     :  
   * PRE          :  /
   */
  ~GnomeMeeting ();

  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : Vroom.
   * PRE          : /
   */
  void InitEngine (int argc,
		   char* argv[]);

  
  /* DESCRIPTION  : / 
   * BEHAVIOR     : No more vroom.
   * PRE          : /
   */
  void StopEngine ();



  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Prepare the endpoint to exit by removing all
   * 		     associated threads and components.
   * PRE          :  /
   */
  void Exit ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the GmWindow structure
   *                 of widgets.
   * PRE          :  /
   */
  GtkWidget *GetMainWindow ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the preferences window GMObject.
   * PRE          :  /
   */
  GtkWidget *GetPrefsWindow (bool create = true);
 
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the assistant window.
   * PRE          :  /
   */
  GtkWidget *GetAssistantWindow (bool create = true);
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the accounts window.
   * PRE          :  /
   */
  GtkWidget *GetAccountsWindow ();
  
  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns a pointer to the tray.
   * PRE          :  /
   */
  StatusIcon *GetStatusicon ();


  /* Needed for PProcess */
  void Main();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return the ServiceCore.
   * PRE          :  /
   */
  Ekiga::ServiceCore *GetServiceCore ();
  

  static GnomeMeeting *Process ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Builds the GUI of GnomeMeeting. The config, GNOME
   *                 and GTK need to have been initialized before.
   *                 The GUI is built accordingly to the preferences
   *                 stored in config.
   * PRE          :  /
   */
  void BuildGUI ();

 private:
  
  PMutex ep_var_mutex;
  PMutex dev_access_mutex;
  int call_number;


  /* The different components of the GUI */
  GtkWidget *main_window;
  GtkWidget *history_window;
  GtkWidget *assistant_window;
  GtkWidget *prefs_window;
  GtkWidget *accounts_window;
  StatusIcon *statusicon;

  /* other things */
#ifdef HAVE_DBUS
  GObject *dbus_component;
#endif

  static GnomeMeeting *GM; 
};

#endif
