
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
 *                         robust-xml.h  -  description
 *                         ------------------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of helpers to work with XML docs
 *
 */

#ifndef __ROBUST_XML_H__
#define __ROBUST_XML_H__

#include <string>
#include <libxml/tree.h>

/* Returns a correctly escaped string for use as a node content
 * @param doc The doc for which the string is meant
 * @param value The raw string to escape
 */
std::string robust_xmlEscape (xmlDocPtr doc,
			      const std::string& value);

/* Sets the content of a node to a given value, creating the node if needed.
 * @param parent The parent node of the node whose value should be set
 * @param child The node whose value should be set, or NULL if needs creating
 * @param name The name of the node whose value should be set
 * @param value The value which should be set
 */
void robust_xmlNodeSetContent (xmlNodePtr parent,
			       xmlNodePtr* child,
			       const std::string& name,
			       const std::string& value);

#endif
