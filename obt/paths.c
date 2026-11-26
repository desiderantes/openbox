/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/paths.c for the Openbox window manager
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

#include "obt/paths.h"

#include <glib/gstdio.h>
#include <string.h>

struct _ObtPaths
{
    gchar  *config_home;
    gchar  *data_home;
    gchar  *cache_home;
    GSList *config_dirs;
    GSList *data_dirs;
    GSList *autostart_dirs;
};

static GSList* slist_path_add(GSList *list, gchar *path)
{
    if (!path || path[0] == '\0') {
        g_free(path);
        return list;
    }

    if (!g_slist_find_custom(list, path, (GCompareFunc)strcmp))
        list = g_slist_append(list, path);
    else
        g_free(path);

    return list;
}

static void obt_paths_free(ObtPaths *p)
{
    g_slist_free_full(p->config_dirs, g_free);
    g_slist_free_full(p->data_dirs, g_free);
    g_slist_free_full(p->autostart_dirs, g_free);
    g_free(p->config_home);
    g_free(p->data_home);
    g_free(p->cache_home);
}

ObtPaths* obt_paths_new(void)
{
    ObtPaths *p;
    const gchar *env;

    p = g_rc_box_new0(ObtPaths);

    p->config_home = g_strdup(g_get_user_config_dir());
    p->data_home = g_strdup(g_get_user_data_dir());
    p->cache_home = g_strdup(g_get_user_cache_dir());

    p->config_dirs = slist_path_add(p->config_dirs, g_strdup(p->config_home));
    env = g_getenv("XDG_CONFIG_DIRS");
    if (!env || env[0] == '\0')
        p->config_dirs = slist_path_add(p->config_dirs, g_strdup(CONFIGDIR));
    for (const gchar *const *it = g_get_system_config_dirs(); *it; ++it)
        p->config_dirs = slist_path_add(p->config_dirs, g_strdup(*it));

    for (GSList *it = p->config_dirs; it; it = g_slist_next(it)) {
        gchar *s = g_build_filename(it->data, "autostart", NULL);
        p->autostart_dirs = g_slist_append(p->autostart_dirs, s);
    }

    p->data_dirs = slist_path_add(p->data_dirs, g_strdup(p->data_home));
    env = g_getenv("XDG_DATA_DIRS");
    if (!env || env[0] == '\0')
        p->data_dirs = slist_path_add(p->data_dirs, g_strdup(DATADIR));
    for (const gchar *const *it = g_get_system_data_dirs(); *it; ++it)
        p->data_dirs = slist_path_add(p->data_dirs, g_strdup(*it));

    return p;
}

void obt_paths_ref(ObtPaths *p)
{
    g_return_if_fail(p != NULL);
    g_rc_box_acquire(p);
}

void obt_paths_unref(ObtPaths *p)
{
    if (p)
        g_rc_box_release_full(p, (GDestroyNotify)obt_paths_free);
}

gchar *obt_paths_expand_tilde(const gchar *f)
{
    if (!f)
        return NULL;

    g_autoptr(GRegex) regex = g_regex_new("(?:^|(?<=[ \\t]))~(?=[/ \\t]|$)",
                                          G_REGEX_MULTILINE | G_REGEX_RAW,
                                          0, NULL);
    return g_regex_replace_literal(regex, f, -1, 0, g_get_home_dir(), 0, NULL);
}

gboolean obt_paths_mkdir(const gchar *path, gint mode)
{
    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(path[0] != '\0', FALSE);

    if (g_file_test(path, G_FILE_TEST_IS_DIR))
        return TRUE;

    return g_mkdir(path, mode) == 0;
}

gboolean obt_paths_mkdir_path(const gchar *path, gint mode)
{
    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(path[0] == '/', FALSE);

    return g_mkdir_with_parents(path, mode) == 0;
}

const gchar* obt_paths_config_home(ObtPaths *p)
{
    g_return_val_if_fail(p != NULL, NULL);
    return p->config_home;
}

const gchar* obt_paths_data_home(ObtPaths *p)
{
    g_return_val_if_fail(p != NULL, NULL);
    return p->data_home;
}

const gchar* obt_paths_cache_home(ObtPaths *p)
{
    g_return_val_if_fail(p != NULL, NULL);
    return p->cache_home;
}

GSList* obt_paths_config_dirs(ObtPaths *p)
{
    g_return_val_if_fail(p != NULL, NULL);
    return p->config_dirs;
}

GSList* obt_paths_data_dirs(ObtPaths *p)
{
    g_return_val_if_fail(p != NULL, NULL);
    return p->data_dirs;
}

GSList* obt_paths_autostart_dirs(ObtPaths *p)
{
    g_return_val_if_fail(p != NULL, NULL);
    return p->autostart_dirs;
}

gboolean obt_paths_try_exec(const gchar *path)
{
    g_return_val_if_fail(path != NULL, FALSE);
    if (path[0] == '\0')
        return FALSE;

    if (g_path_is_absolute(path))
        return g_file_test(path, G_FILE_TEST_IS_REGULAR) &&
               g_file_test(path, G_FILE_TEST_IS_EXECUTABLE);

    g_autofree gchar *found = g_find_program_in_path(path);
    return found != NULL && g_file_test(found, G_FILE_TEST_IS_REGULAR);
}
