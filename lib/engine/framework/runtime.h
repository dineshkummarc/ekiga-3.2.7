
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         runtime.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *                          (c) 2007 by Damien Sandras
 *   description          : Threading helper functions
 *
 */

#include <sigc++/sigc++.h>

#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include "services.h"

template<typename data_type>
static void emit_signal_in_main_helper1 (sigc::signal1<void, data_type> sign,
                                         data_type data)
{
  sign.emit (data);
}

static void emit_signal_in_main_helper (sigc::signal0<void> sign)

{
  sign.emit ();
}

namespace Ekiga
{

  /**
   * @addtogroup services
   * @{
   */

  namespace Runtime
  {
    void init (); // depends on the implementation

    void run (); // depends on the implementation

    void quit (); // depends on the implementation

    void run_in_main (sigc::slot0<void> action,
		      unsigned int seconds = 0); // depends on the implementation

    inline void emit_signal_in_main (sigc::signal0<void> sign)
    {
      run_in_main (sigc::bind (sigc::ptr_fun (emit_signal_in_main_helper), sign));
    }

    template<typename data_type>
    void emit_signal_in_main (sigc::signal1<void, data_type> sign,
                              data_type data)
    {
      run_in_main (sigc::bind (sigc::ptr_fun (emit_signal_in_main_helper1<data_type>), sign, data));
    }
  };

  /**
   * @}
   */

};

#endif
