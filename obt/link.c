/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/link.c for the Openbox window manager
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

#include "obt/link.h"
#include "obt/paths.h"

#include <glib.h>
#include <string.h>

struct _ObtLink {
    ObtLinkType type;
    gchar *name; /*!< Specific name for the object (eg Firefox) */
    gboolean display; /*!< When false, do not display this link in menus or
                           launchers, etc */
    gboolean deleted; /*!< When true, the Link exists but is deleted for the
                           current user */
    gchar *generic; /*!< Generic name for the object (eg Web Browser) */
    gchar *comment; /*!< Comment/description to display for the object */
    gchar *icon; /*!< Name/path for an icon for the object */
    gchar **only_show_in; /*!< Desktop environments where link is shown */
    gchar **not_show_in; /*!< Desktop environments where link is hidden */

    union _ObtLinkData {
        struct _ObtLinkApp {
            gchar *exec; /*!< Executable to run for the app */
            gchar *wdir; /*!< Working dir to run the app in */
            gboolean term; /*!< Run the app in a terminal or not */
            ObtLinkAppOpen open;

            gchar **mime; /*!< Mime types the app can open */

            GQuark *categories; /*!< Array of quarks representing the
                                  application's categories */
            gulong  n_categories; /*!< Number of categories for the app */

            ObtLinkAppStartup startup;
            gchar *startup_wmclass;
        } app;
        struct _ObtLinkLink {
            gchar *addr;
        } url;
        struct _ObtLinkDir {
        } dir;
    } d;
};

static void obt_link_free(ObtLink *dd)
{
    g_free(dd->name);
    g_free(dd->generic);
    g_free(dd->comment);
    g_free(dd->icon);
    g_strfreev(dd->only_show_in);
    g_strfreev(dd->not_show_in);
    if (dd->type == OBT_LINK_TYPE_APPLICATION) {
        g_free(dd->d.app.exec);
        g_free(dd->d.app.wdir);
        g_strfreev(dd->d.app.mime);
        g_free(dd->d.app.categories);
        g_free(dd->d.app.startup_wmclass);
    }
    else if (dd->type == OBT_LINK_TYPE_URL) {
        g_free(dd->d.url.addr);
    }
}

static gboolean parse_link_type(const gchar *type_str, ObtLinkType *out)
{
    if (!type_str)
        return FALSE;
    if (strcmp(type_str, G_KEY_FILE_DESKTOP_TYPE_APPLICATION) == 0) {
        *out = OBT_LINK_TYPE_APPLICATION;
        return TRUE;
    }
    if (strcmp(type_str, G_KEY_FILE_DESKTOP_TYPE_LINK) == 0) {
        *out = OBT_LINK_TYPE_URL;
        return TRUE;
    }
    if (strcmp(type_str, G_KEY_FILE_DESKTOP_TYPE_DIRECTORY) == 0) {
        *out = OBT_LINK_TYPE_DIRECTORY;
        return TRUE;
    }
    return FALSE;
}

static ObtLinkAppOpen parse_exec_open_flags(const gchar *exec)
{
    ObtLinkAppOpen open = 0;
    gboolean percent = FALSE;

    for (const gchar *c = exec; *c; ++c) {
        if (percent) {
            switch (*c) {
            case 'f': open |= OBT_LINK_APP_SINGLE_LOCAL; break;
            case 'F': open |= OBT_LINK_APP_MULTI_LOCAL; break;
            case 'u': open |= OBT_LINK_APP_SINGLE_URL; break;
            case 'U': open |= OBT_LINK_APP_MULTI_URL; break;
            default: break;
            }
            percent = FALSE;
        }
        else if (*c == '%') {
            percent = TRUE;
        }
    }
    return open;
}

ObtLink* obt_link_from_ddfile(const gchar *ddname, GSList *paths)
{
    g_return_val_if_fail(ddname != NULL, NULL);

    g_autoptr(GKeyFile) kf = g_key_file_new();
    g_autoptr(GError) err = NULL;
    g_autofree gchar *full_path = NULL;
    gboolean loaded;

    if (g_path_is_absolute(ddname) || paths == NULL) {
        loaded = g_key_file_load_from_file(kf, ddname, G_KEY_FILE_NONE, &err);
        if (loaded)
            full_path = g_strdup(ddname);
    }
    else {
        g_autoptr(GPtrArray) dirs = g_ptr_array_new();
        for (GSList *it = paths; it; it = g_slist_next(it))
            g_ptr_array_add(dirs, it->data);
        g_ptr_array_add(dirs, NULL);
        loaded = g_key_file_load_from_dirs(kf, ddname,
                                           (const gchar **)dirs->pdata,
                                           &full_path, G_KEY_FILE_NONE, &err);
    }

    if (!loaded) {
        if (err &&
            !g_error_matches(err, G_FILE_ERROR, G_FILE_ERROR_NOENT) &&
            !g_error_matches(err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_NOT_FOUND))
        {
            g_warning("Failed to parse desktop entry '%s': %s",
                      ddname, err->message);
        }
        return NULL;
    }

    const gchar *display_path = full_path ? full_path : ddname;

    if (!g_key_file_has_group(kf, G_KEY_FILE_DESKTOP_GROUP)) {
        g_warning("Missing [%s] group in %s",
                  G_KEY_FILE_DESKTOP_GROUP, display_path);
        return NULL;
    }

    g_autoptr(ObtLink) link = g_rc_box_new0(ObtLink);
    link->display = TRUE;

    if (g_key_file_get_boolean(kf, G_KEY_FILE_DESKTOP_GROUP,
                               G_KEY_FILE_DESKTOP_KEY_HIDDEN, NULL))
    {
        link->deleted = TRUE;
        link->display = FALSE;
        return g_steal_pointer(&link);
    }

    g_autofree gchar *type_str = g_key_file_get_string(
        kf, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TYPE, NULL);
    if (!parse_link_type(type_str, &link->type)) {
        g_warning("Missing or unknown Type key in %s", display_path);
        return NULL;
    }

    link->name = g_key_file_get_locale_string(
        kf, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    if (!link->name || link->name[0] == '\0') {
        g_warning("Missing Name key in %s", display_path);
        return NULL;
    }

    if (g_key_file_get_boolean(kf, G_KEY_FILE_DESKTOP_GROUP,
                               G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, NULL))
    {
        link->display = FALSE;
    }

    link->generic = g_key_file_get_locale_string(
        kf, G_KEY_FILE_DESKTOP_GROUP,
        G_KEY_FILE_DESKTOP_KEY_GENERIC_NAME, NULL, NULL);
    link->comment = g_key_file_get_locale_string(
        kf, G_KEY_FILE_DESKTOP_GROUP,
        G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
    link->icon = g_key_file_get_locale_string(
        kf, G_KEY_FILE_DESKTOP_GROUP,
        G_KEY_FILE_DESKTOP_KEY_ICON, NULL, NULL);
    link->only_show_in = g_key_file_get_string_list(
        kf, G_KEY_FILE_DESKTOP_GROUP,
        G_KEY_FILE_DESKTOP_KEY_ONLY_SHOW_IN, NULL, NULL);
    link->not_show_in = g_key_file_get_string_list(
        kf, G_KEY_FILE_DESKTOP_GROUP,
        G_KEY_FILE_DESKTOP_KEY_NOT_SHOW_IN, NULL, NULL);

    if (link->type == OBT_LINK_TYPE_APPLICATION) {
        link->d.app.exec = g_key_file_get_string(
            kf, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
        if (!link->d.app.exec || link->d.app.exec[0] == '\0') {
            g_warning("Missing Exec key for Application in %s", display_path);
            return NULL;
        }

        link->d.app.open = parse_exec_open_flags(link->d.app.exec);

        g_autofree gchar *try_exec = g_key_file_get_string(
            kf, G_KEY_FILE_DESKTOP_GROUP,
            G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, NULL);
        if (try_exec)
            link->display = link->display && obt_paths_try_exec(try_exec);

        link->d.app.wdir = g_key_file_get_string(
            kf, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_PATH, NULL);
        link->d.app.term = g_key_file_get_boolean(
            kf, G_KEY_FILE_DESKTOP_GROUP,
            G_KEY_FILE_DESKTOP_KEY_TERMINAL, NULL);

        if (g_key_file_has_key(kf, G_KEY_FILE_DESKTOP_GROUP,
                               G_KEY_FILE_DESKTOP_KEY_STARTUP_NOTIFY, NULL))
        {
            gboolean notify = g_key_file_get_boolean(
                kf, G_KEY_FILE_DESKTOP_GROUP,
                G_KEY_FILE_DESKTOP_KEY_STARTUP_NOTIFY, NULL);
            link->d.app.startup = notify ?
                OBT_LINK_APP_STARTUP_PROTOCOL_SUPPORT :
                OBT_LINK_APP_STARTUP_NO_SUPPORT;
        }
        else {
            link->d.app.startup = OBT_LINK_APP_STARTUP_LEGACY_SUPPORT;
        }
        link->d.app.startup_wmclass = g_key_file_get_string(
            kf, G_KEY_FILE_DESKTOP_GROUP,
            G_KEY_FILE_DESKTOP_KEY_STARTUP_WM_CLASS, NULL);

        gsize n_cats = 0;
        g_auto(GStrv) cats = g_key_file_get_string_list(
            kf, G_KEY_FILE_DESKTOP_GROUP,
            G_KEY_FILE_DESKTOP_KEY_CATEGORIES, &n_cats, NULL);
        if (cats && n_cats > 0) {
            link->d.app.categories = g_new(GQuark, n_cats);
            for (gsize i = 0; i < n_cats; ++i) {
                if (cats[i][0] != '\0')
                    link->d.app.categories[link->d.app.n_categories++] =
                        g_quark_from_string(cats[i]);
            }
            if (link->d.app.n_categories == 0)
                g_clear_pointer(&link->d.app.categories, g_free);
        }

        link->d.app.mime = g_key_file_get_string_list(
            kf, G_KEY_FILE_DESKTOP_GROUP,
            G_KEY_FILE_DESKTOP_KEY_MIME_TYPE, NULL, NULL);
    }
    else if (link->type == OBT_LINK_TYPE_URL) {
        link->d.url.addr = g_key_file_get_string(
            kf, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_URL, NULL);
        if (!link->d.url.addr || link->d.url.addr[0] == '\0') {
            g_warning("Missing URL key for Link in %s", display_path);
            return NULL;
        }
    }

    return g_steal_pointer(&link);
}

void obt_link_ref(ObtLink *dd)
{
    g_return_if_fail(dd != NULL);
    g_rc_box_acquire(dd);
}

void obt_link_unref(ObtLink *dd)
{
    if (dd)
        g_rc_box_release_full(dd, (GDestroyNotify)obt_link_free);
}

gboolean obt_link_deleted(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, FALSE);
    return e->deleted;
}

ObtLinkType obt_link_type(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, 0);
    return e->type;
}

gboolean obt_link_display(ObtLink *e, const gchar *env)
{
    g_return_val_if_fail(e != NULL, FALSE);

    if (e->deleted || !e->display)
        return FALSE;

    g_auto(GStrv) envs = (env && env[0] != '\0') ?
        g_strsplit_set(env, ";:", -1) : NULL;

    if (e->only_show_in) {
        gboolean matched = FALSE;
        if (envs) {
            for (gchar **it = envs; *it; ++it) {
                if ((*it)[0] != '\0' &&
                    g_strv_contains((const gchar *const *)e->only_show_in, *it))
                {
                    matched = TRUE;
                    break;
                }
            }
        }
        if (!matched)
            return FALSE;
    }

    if (e->not_show_in && envs) {
        for (gchar **it = envs; *it; ++it) {
            if ((*it)[0] != '\0' &&
                g_strv_contains((const gchar *const *)e->not_show_in, *it))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

const gchar* obt_link_name(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    return e->name;
}

const gchar* obt_link_generic_name(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    return e->generic;
}

const gchar* obt_link_comment(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    return e->comment;
}

const gchar* obt_link_icon(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    return e->icon;
}

const gchar* obt_link_url_path(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_URL, NULL);
    return e->d.url.addr;
}

const gchar* obt_link_app_executable(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_APPLICATION, NULL);
    return e->d.app.exec;
}

const gchar* obt_link_app_path(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_APPLICATION, NULL);
    return e->d.app.wdir;
}

gboolean obt_link_app_run_in_terminal(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, FALSE);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_APPLICATION, FALSE);
    return e->d.app.term;
}

const gchar*const* obt_link_app_mime_types(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_APPLICATION, NULL);
    return (const gchar *const *)e->d.app.mime;
}

const GQuark* obt_link_app_categories(ObtLink *e, gulong *n)
{
    g_return_val_if_fail(e != NULL, NULL);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_APPLICATION, NULL);
    g_return_val_if_fail(n != NULL, NULL);

    *n = e->d.app.n_categories;
    return e->d.app.categories;
}

ObtLinkAppOpen obt_link_app_open(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, 0);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_APPLICATION, 0);
    return e->d.app.open;
}

ObtLinkAppStartup obt_link_app_startup_notify(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, OBT_LINK_APP_STARTUP_NO_SUPPORT);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_APPLICATION,
                         OBT_LINK_APP_STARTUP_NO_SUPPORT);
    return e->d.app.startup;
}

const gchar* obt_link_app_startup_wmclass(ObtLink *e)
{
    g_return_val_if_fail(e != NULL, NULL);
    g_return_val_if_fail(e->type == OBT_LINK_TYPE_APPLICATION, NULL);
    return e->d.app.startup_wmclass;
}
