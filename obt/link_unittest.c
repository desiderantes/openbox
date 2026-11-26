#include "obt/unittest_base.h"
#include "obt/link.h"

#include <glib.h>
#include <glib/gstdio.h>

static gchar *write_desktop_file(const gchar *dir, const gchar *name,
                                 const gchar *contents)
{
    gchar *path = g_build_filename(dir, name, NULL);
    EXPECT_BOOL_EQ(TRUE, g_file_set_contents(path, contents, -1, NULL));
    return path;
}

static void application_entry(void)
{
    TEST_START();

    g_autofree gchar *tmp = g_dir_make_tmp("obt-link-test-XXXXXX", NULL);
    g_autofree gchar *path = write_desktop_file(
        tmp, "app.desktop",
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=Test App\n"
        "GenericName=Tester\n"
        "Comment=Runs unit tests\n"
        "Icon=utilities-terminal\n"
        "Exec=test-app %%f %U\n"
        "TryExec=sh\n"
        "Path=/tmp\n"
        "Terminal=true\n"
        "StartupNotify=true\n"
        "StartupWMClass=TestApp\n"
        "Categories=Utility;Development;\n"
        "MimeType=text/plain;text/html;\n"
        "OnlyShowIn=OPENBOX;XFCE;\n");

    GSList *search = g_slist_append(NULL, tmp);
    g_autoptr(ObtLink) link = obt_link_from_ddfile("app.desktop", search);
    g_slist_free(search);

    EXPECT_BOOL_EQ(TRUE, link != NULL);
    if (link) {
        EXPECT_BOOL_EQ(FALSE, obt_link_deleted(link));
        EXPECT_INT_EQ(OBT_LINK_TYPE_APPLICATION, obt_link_type(link));
        EXPECT_STRING_EQ("Test App", obt_link_name(link));
        EXPECT_STRING_EQ("Tester", obt_link_generic_name(link));
        EXPECT_STRING_EQ("Runs unit tests", obt_link_comment(link));
        EXPECT_STRING_EQ("utilities-terminal", obt_link_icon(link));
        EXPECT_STRING_EQ("test-app %%f %U", obt_link_app_executable(link));
        EXPECT_STRING_EQ("/tmp", obt_link_app_path(link));
        EXPECT_BOOL_EQ(TRUE, obt_link_app_run_in_terminal(link));
        EXPECT_INT_EQ(OBT_LINK_APP_MULTI_URL, obt_link_app_open(link));
        EXPECT_INT_EQ(OBT_LINK_APP_STARTUP_PROTOCOL_SUPPORT,
                      obt_link_app_startup_notify(link));
        EXPECT_STRING_EQ("TestApp", obt_link_app_startup_wmclass(link));

        gulong n = 0;
        const GQuark *cats = obt_link_app_categories(link, &n);
        EXPECT_UINT_EQ(2, (guint)n);
        if (n == 2) {
            EXPECT_STRING_EQ("Utility", g_quark_to_string(cats[0]));
            EXPECT_STRING_EQ("Development", g_quark_to_string(cats[1]));
        }

        const gchar *const *mimes = obt_link_app_mime_types(link);
        EXPECT_BOOL_EQ(TRUE, mimes != NULL);
        if (mimes) {
            EXPECT_STRING_EQ("text/plain", mimes[0]);
            EXPECT_STRING_EQ("text/html", mimes[1]);
            EXPECT_BOOL_EQ(TRUE, mimes[2] == NULL);
        }

        EXPECT_BOOL_EQ(FALSE, obt_link_display(link, NULL));
        EXPECT_BOOL_EQ(FALSE, obt_link_display(link, "GNOME"));
        EXPECT_BOOL_EQ(TRUE, obt_link_display(link, "OPENBOX"));
        EXPECT_BOOL_EQ(TRUE, obt_link_display(link, "GNOME:XFCE"));
        EXPECT_BOOL_EQ(TRUE, obt_link_display(link, "KDE;OPENBOX"));
    }

    g_unlink(path);
    g_rmdir(tmp);

    TEST_END();
}

static void visibility_and_types(void)
{
    TEST_START();

    g_autofree gchar *tmp = g_dir_make_tmp("obt-link-test-XXXXXX", NULL);

    /* Link (URL) entry with NotShowIn */
    g_autofree gchar *url_path = write_desktop_file(
        tmp, "url.desktop",
        "[Desktop Entry]\n"
        "Type=Link\n"
        "Name=Docs\n"
        "URL=https://openbox.org\n"
        "NotShowIn=GNOME;KDE;\n");
    g_autoptr(ObtLink) url = obt_link_from_ddfile(url_path, NULL);
    EXPECT_BOOL_EQ(TRUE, url != NULL);
    if (url) {
        EXPECT_INT_EQ(OBT_LINK_TYPE_URL, obt_link_type(url));
        EXPECT_STRING_EQ("https://openbox.org", obt_link_url_path(url));
        EXPECT_BOOL_EQ(TRUE, obt_link_display(url, NULL));
        EXPECT_BOOL_EQ(TRUE, obt_link_display(url, "OPENBOX"));
        EXPECT_BOOL_EQ(FALSE, obt_link_display(url, "GNOME"));
        EXPECT_BOOL_EQ(FALSE, obt_link_display(url, "OPENBOX:KDE"));
    }

    /* Directory entry with NoDisplay=true */
    g_autofree gchar *dir_path = write_desktop_file(
        tmp, "dir.desktop",
        "[Desktop Entry]\n"
        "Type=Directory\n"
        "Name=Games\n"
        "NoDisplay=true\n");
    g_autoptr(ObtLink) dir = obt_link_from_ddfile(dir_path, NULL);
    EXPECT_BOOL_EQ(TRUE, dir != NULL);
    if (dir) {
        EXPECT_INT_EQ(OBT_LINK_TYPE_DIRECTORY, obt_link_type(dir));
        EXPECT_BOOL_EQ(FALSE, obt_link_display(dir, "OPENBOX"));
    }

    /* Minimal Hidden=true override */
    g_autofree gchar *hidden_path = write_desktop_file(
        tmp, "hidden.desktop",
        "[Desktop Entry]\n"
        "Hidden=true\n");
    g_autoptr(ObtLink) hidden = obt_link_from_ddfile(hidden_path, NULL);
    EXPECT_BOOL_EQ(TRUE, hidden != NULL);
    if (hidden) {
        EXPECT_BOOL_EQ(TRUE, obt_link_deleted(hidden));
        EXPECT_BOOL_EQ(FALSE, obt_link_display(hidden, "OPENBOX"));
    }

    /* TryExec failing hides entry */
    g_autofree gchar *tryexec_path = write_desktop_file(
        tmp, "missing-bin.desktop",
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=Missing\n"
        "Exec=missing-bin\n"
        "TryExec=obt-definitely-nonexistent-bin\n");
    g_autoptr(ObtLink) missing = obt_link_from_ddfile(tryexec_path, NULL);
    EXPECT_BOOL_EQ(TRUE, missing != NULL);
    if (missing) {
        EXPECT_BOOL_EQ(FALSE, obt_link_display(missing, "OPENBOX"));
        EXPECT_INT_EQ(OBT_LINK_APP_STARTUP_LEGACY_SUPPORT,
                      obt_link_app_startup_notify(missing));
    }

    /* Invalid / nonexistent entries return NULL */
    g_autofree gchar *bad_path = write_desktop_file(
        tmp, "bad.desktop",
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=NoExec\n");
    g_autoptr(ObtLink) bad = obt_link_from_ddfile(bad_path, NULL);
    EXPECT_BOOL_EQ(TRUE, bad == NULL);

    g_autofree gchar *nope_path = g_build_filename(tmp, "nope.desktop", NULL);
    g_autoptr(ObtLink) nope = obt_link_from_ddfile(nope_path, NULL);
    EXPECT_BOOL_EQ(TRUE, nope == NULL);

    /* NULL unref is safe */
    obt_link_unref(NULL);

    g_unlink(url_path);
    g_unlink(dir_path);
    g_unlink(hidden_path);
    g_unlink(tryexec_path);
    g_unlink(bad_path);
    g_rmdir(tmp);

    TEST_END();
}

void run_link_unittest(void)
{
    unittest_start_suite("link");

    application_entry();
    visibility_and_types();

    unittest_end_suite();
}
