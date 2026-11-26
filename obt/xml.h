/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/xml.h for the Openbox window manager
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#ifndef __obt_xml_h
#define __obt_xml_h

#include <libxml/parser.h>
#include <glib.h>
#include "obt/version.h"

G_BEGIN_DECLS

typedef struct _ObtXmlInst ObtXmlInst;

typedef void (*ObtXmlCallback)(xmlNodePtr node, gpointer data);

OBT_API ObtXmlInst* obt_xml_instance_new(void);
OBT_API void obt_xml_instance_ref(ObtXmlInst *inst);
OBT_API void obt_xml_instance_unref(ObtXmlInst *inst);

OBT_API gboolean obt_xml_load_file(ObtXmlInst *inst,
                                   const gchar *path,
                                   const gchar *root_node);
OBT_API gboolean obt_xml_load_config_file(ObtXmlInst *inst,
                                          const gchar *domain,
                                          const gchar *filename,
                                          const gchar *root_node);
OBT_API gboolean obt_xml_load_data_file(ObtXmlInst *inst,
                                        const gchar *domain,
                                        const gchar *filename,
                                        const gchar *root_node);
OBT_API gboolean obt_xml_load_theme_file(ObtXmlInst *inst,
                                         const gchar *theme,
                                         const gchar *domain,
                                         const gchar *filename,
                                         const gchar *root_node);
OBT_API gboolean obt_xml_load_mem(ObtXmlInst *inst,
                                  gpointer data, guint len, const gchar *root_node);

/* Returns true if an error is present. */
OBT_API gboolean obt_xml_last_error(ObtXmlInst *inst);
OBT_API gchar* obt_xml_last_error_file(ObtXmlInst *inst);
OBT_API gint obt_xml_last_error_line(ObtXmlInst *inst);
OBT_API gchar* obt_xml_last_error_message(ObtXmlInst *inst);

OBT_API gboolean obt_xml_save_file(ObtXmlInst *inst,
                                   const gchar *path,
                                   gboolean pretty);

OBT_API xmlDocPtr obt_xml_doc(ObtXmlInst *inst);
OBT_API xmlNodePtr obt_xml_root(ObtXmlInst *inst);

OBT_API void obt_xml_close(ObtXmlInst *inst);

OBT_API void obt_xml_register(ObtXmlInst *inst, const gchar *tag,
                              ObtXmlCallback func, gpointer data);
OBT_API void obt_xml_unregister(ObtXmlInst *inst, const gchar *tag);
OBT_API void obt_xml_tree(ObtXmlInst *i, xmlNodePtr node);
OBT_API void obt_xml_tree_from_root(ObtXmlInst *i);


/* helpers */

OBT_API xmlNodePtr obt_xml_find_node   (xmlNodePtr node, const gchar *name);

OBT_API gboolean obt_xml_node_contains (xmlNodePtr node, const gchar *val);
OBT_API gchar   *obt_xml_node_string_unstripped(xmlNodePtr node);
OBT_API gchar   *obt_xml_node_string   (xmlNodePtr node);
OBT_API gint     obt_xml_node_int      (xmlNodePtr node);
OBT_API gboolean obt_xml_node_bool     (xmlNodePtr node);

OBT_API gboolean obt_xml_attr_contains (xmlNodePtr node, const gchar *name,
                                        const gchar *val);
OBT_API gboolean obt_xml_attr_string_unstripped(xmlNodePtr node, const gchar *name,
                                                gchar **value);
OBT_API gboolean obt_xml_attr_string   (xmlNodePtr node, const gchar *name,
                                        gchar **value);
OBT_API gboolean obt_xml_attr_int      (xmlNodePtr node, const gchar *name,
                                        gint *value);
OBT_API gboolean obt_xml_attr_bool     (xmlNodePtr node, const gchar *name,
                                        gboolean *value);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(ObtXmlInst, obt_xml_instance_unref)

G_END_DECLS

#endif
