/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/paths.h for the Openbox window manager
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

#ifndef __obt_paths_h
#define __obt_paths_h

#include <glib.h>
#include "obt/version.h"

G_BEGIN_DECLS

typedef struct _ObtPaths ObtPaths;

OBT_API ObtPaths* obt_paths_new(void);
OBT_API void obt_paths_ref(ObtPaths *p);
OBT_API void obt_paths_unref(ObtPaths *p);

OBT_API const gchar* obt_paths_config_home(ObtPaths *p);
OBT_API const gchar* obt_paths_data_home(ObtPaths *p);
OBT_API const gchar* obt_paths_cache_home(ObtPaths *p);
OBT_API GSList* obt_paths_config_dirs(ObtPaths *p);
OBT_API GSList* obt_paths_data_dirs(ObtPaths *p);
OBT_API GSList* obt_paths_autostart_dirs(ObtPaths *p);

OBT_API gchar *obt_paths_expand_tilde(const gchar *f);
OBT_API gboolean obt_paths_mkdir(const gchar *path, gint mode);
OBT_API gboolean obt_paths_mkdir_path(const gchar *path, gint mode);

/*! Returns TRUE if the @path points to an executable file.
  If the @path is not an absolute path, then it is searched for in $PATH.
*/
OBT_API gboolean obt_paths_try_exec(ObtPaths *p, const gchar *path);

G_END_DECLS

#endif
