/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/link.h for the Openbox window manager
   Copyright (c) 2009        Dana Jansens

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

#ifndef __obt_link_h
#define __obt_link_h

#include <glib.h>
#include "obt/version.h"

G_BEGIN_DECLS

typedef enum {
    OBT_LINK_TYPE_APPLICATION = 1,
    OBT_LINK_TYPE_URL         = 2,
    OBT_LINK_TYPE_DIRECTORY   = 3
} ObtLinkType;

typedef enum {
    OBT_LINK_APP_STARTUP_NO_SUPPORT,
    OBT_LINK_APP_STARTUP_PROTOCOL_SUPPORT,
    OBT_LINK_APP_STARTUP_LEGACY_SUPPORT
} ObtLinkAppStartup;

typedef enum {
    /*! The app can be launched with a single local file */
    OBT_LINK_APP_SINGLE_LOCAL = 1 << 0,
    /*! The app can be launched with multiple local files */
    OBT_LINK_APP_MULTI_LOCAL  = 1 << 1,
    /*! The app can be launched with a single URL */
    OBT_LINK_APP_SINGLE_URL   = 1 << 2,
    /*! The app can be launched with multiple URLs */
    OBT_LINK_APP_MULTI_URL    = 1 << 3
} ObtLinkAppOpen;

typedef struct _ObtLink     ObtLink;

/*! Loads a .desktop file. If @name is absolute or @paths is NULL, loads @name
    directly; otherwise searches @paths in order for @name. */
OBT_API ObtLink* obt_link_from_ddfile(const gchar *name, GSList *paths);

OBT_API void obt_link_ref(ObtLink *e);
OBT_API void obt_link_unref(ObtLink *e);

/*! Returns TRUE if the file exists but says it should be ignored, with
    the Hidden flag.  No other functions can be used for the ObtLink
    in this case. */
OBT_API gboolean obt_link_deleted(ObtLink *e);

/*! Returns the type of object referred to by the .desktop file. */
OBT_API ObtLinkType obt_link_type(ObtLink *e);

/*! Returns TRUE if the .desktop file should be displayed to users, given the
    current environment.  If FALSE, the .desktop file should not be shown.
    This also uses the TryExec option if it is present.
    @env A semicolon- or colon-delimited list of desktop environments (e.g.
         $XDG_CURRENT_DESKTOP), or NULL if none are specified. */
OBT_API gboolean obt_link_display(ObtLink *e, const gchar *env);

OBT_API const gchar* obt_link_name        (ObtLink *e);
OBT_API const gchar* obt_link_generic_name(ObtLink *e);
OBT_API const gchar* obt_link_comment     (ObtLink *e);
/*! Returns the icon for the object referred to by the .desktop file.
    Returns either an absolute path, or a string which can be used to find the
    icon using the freedesktop Icon Theme Specification. */
OBT_API const gchar* obt_link_icon        (ObtLink *e);

OBT_API const gchar* obt_link_url_path(ObtLink *e);

OBT_API const gchar*       obt_link_app_executable     (ObtLink *e);
/*! Returns the path in which the application should be run */
OBT_API const gchar*       obt_link_app_path           (ObtLink *e);
OBT_API gboolean           obt_link_app_run_in_terminal(ObtLink *e);
OBT_API const gchar*const* obt_link_app_mime_types     (ObtLink *e);
OBT_API const GQuark*      obt_link_app_categories     (ObtLink *e, gulong *n);
/*! Returns a combination of values in the ObtLinkAppOpen enum,
    specifying if the application can be launched to open one or more files
    and URLs. */
OBT_API ObtLinkAppOpen     obt_link_app_open           (ObtLink *e);

OBT_API ObtLinkAppStartup  obt_link_app_startup_notify (ObtLink *e);
OBT_API const gchar*       obt_link_app_startup_wmclass(ObtLink *e);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(ObtLink, obt_link_unref)

G_END_DECLS

#endif
