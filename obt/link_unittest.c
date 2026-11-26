#include "obt/link.h"

#include <glib.h>
#include <glib/gstdio.h>

static gchar *write_desktop_file(const gchar *dir, const gchar *name,
                                 const gchar *contents)
{
    gchar *path = g_build_filename(dir, name, NULL);
    g_assert_true(g_file_set_contents(path, contents, -1, NULL));
    return path;
}

static void test_application_entry(void)
{
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

    g_assert_nonnull(link);
    g_assert_false(obt_link_deleted(link));
    g_assert_cmpint(obt_link_type(link), ==, OBT_LINK_TYPE_APPLICATION);
    g_assert_cmpstr(obt_link_name(link), ==, "Test App");
    g_assert_cmpstr(obt_link_generic_name(link), ==, "Tester");
    g_assert_cmpstr(obt_link_comment(link), ==, "Runs unit tests");
    g_assert_cmpstr(obt_link_icon(link), ==, "utilities-terminal");
    g_assert_cmpstr(obt_link_app_executable(link), ==, "test-app %%f %U");
    g_assert_cmpstr(obt_link_app_path(link), ==, "/tmp");
    g_assert_true(obt_link_app_run_in_terminal(link));
    g_assert_cmpint(obt_link_app_open(link), ==, OBT_LINK_APP_MULTI_URL);
    g_assert_cmpint(obt_link_app_startup_notify(link), ==,
                    OBT_LINK_APP_STARTUP_PROTOCOL_SUPPORT);
    g_assert_cmpstr(obt_link_app_startup_wmclass(link), ==, "TestApp");

    gulong n = 0;
    const GQuark *cats = obt_link_app_categories(link, &n);
    g_assert_cmpuint(n, ==, 2);
    g_assert_cmpstr(g_quark_to_string(cats[0]), ==, "Utility");
    g_assert_cmpstr(g_quark_to_string(cats[1]), ==, "Development");

    const gchar *const *mimes = obt_link_app_mime_types(link);
    g_assert_nonnull(mimes);
    g_assert_cmpstr(mimes[0], ==, "text/plain");
    g_assert_cmpstr(mimes[1], ==, "text/html");
    g_assert_null(mimes[2]);

    g_assert_false(obt_link_display(link, NULL));
    g_assert_false(obt_link_display(link, "GNOME"));
    g_assert_true(obt_link_display(link, "OPENBOX"));
    g_assert_true(obt_link_display(link, "GNOME:XFCE"));
    g_assert_true(obt_link_display(link, "KDE;OPENBOX"));

    obt_link_ref(link);
    obt_link_unref(link);

    g_unlink(path);
    g_rmdir(tmp);
}

static void test_url_entry(void)
{
    g_autofree gchar *tmp = g_dir_make_tmp("obt-link-test-XXXXXX", NULL);
    g_autofree gchar *path = write_desktop_file(
        tmp, "url.desktop",
        "[Desktop Entry]\n"
        "Type=Link\n"
        "Name=Docs\n"
        "URL=https://openbox.org\n"
        "NotShowIn=GNOME;KDE;\n");

    g_autoptr(ObtLink) url = obt_link_from_ddfile(path, NULL);
    g_assert_nonnull(url);
    g_assert_cmpint(obt_link_type(url), ==, OBT_LINK_TYPE_URL);
    g_assert_cmpstr(obt_link_url_path(url), ==, "https://openbox.org");
    g_assert_true(obt_link_display(url, NULL));
    g_assert_true(obt_link_display(url, "OPENBOX"));
    g_assert_false(obt_link_display(url, "GNOME"));
    g_assert_false(obt_link_display(url, "OPENBOX:KDE"));

    g_unlink(path);
    g_rmdir(tmp);
}

static void test_directory_and_hidden(void)
{
    g_autofree gchar *tmp = g_dir_make_tmp("obt-link-test-XXXXXX", NULL);

    g_autofree gchar *dir_path = write_desktop_file(
        tmp, "dir.desktop",
        "[Desktop Entry]\n"
        "Type=Directory\n"
        "Name=Games\n"
        "NoDisplay=true\n");
    g_autoptr(ObtLink) dir = obt_link_from_ddfile(dir_path, NULL);
    g_assert_nonnull(dir);
    g_assert_cmpint(obt_link_type(dir), ==, OBT_LINK_TYPE_DIRECTORY);
    g_assert_false(obt_link_display(dir, "OPENBOX"));

    g_autofree gchar *hidden_path = write_desktop_file(
        tmp, "hidden.desktop",
        "[Desktop Entry]\n"
        "Hidden=true\n");
    g_autoptr(ObtLink) hidden = obt_link_from_ddfile(hidden_path, NULL);
    g_assert_nonnull(hidden);
    g_assert_true(obt_link_deleted(hidden));
    g_assert_false(obt_link_display(hidden, "OPENBOX"));

    g_unlink(dir_path);
    g_unlink(hidden_path);
    g_rmdir(tmp);
}

static void test_try_exec(void)
{
    g_autofree gchar *tmp = g_dir_make_tmp("obt-link-test-XXXXXX", NULL);
    g_autofree gchar *path = write_desktop_file(
        tmp, "missing-bin.desktop",
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=Missing\n"
        "Exec=missing-bin\n"
        "TryExec=obt-definitely-nonexistent-bin\n");

    g_autoptr(ObtLink) missing = obt_link_from_ddfile(path, NULL);
    g_assert_nonnull(missing);
    g_assert_false(obt_link_display(missing, "OPENBOX"));
    g_assert_cmpint(obt_link_app_startup_notify(missing), ==,
                    OBT_LINK_APP_STARTUP_LEGACY_SUPPORT);

    g_unlink(path);
    g_rmdir(tmp);
}

static void test_invalid_entries(void)
{
    g_autofree gchar *tmp = g_dir_make_tmp("obt-link-test-XXXXXX", NULL);

    g_autofree gchar *no_exec_path = write_desktop_file(
        tmp, "no-exec.desktop",
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=NoExec\n");
    g_test_expect_message("Obt", G_LOG_LEVEL_WARNING, "*Missing Exec key*");
    g_autoptr(ObtLink) no_exec = obt_link_from_ddfile(no_exec_path, NULL);
    g_test_assert_expected_messages();
    g_assert_null(no_exec);

    g_autofree gchar *no_url_path = write_desktop_file(
        tmp, "no-url.desktop",
        "[Desktop Entry]\n"
        "Type=Link\n"
        "Name=NoURL\n");
    g_test_expect_message("Obt", G_LOG_LEVEL_WARNING, "*Missing URL key*");
    g_autoptr(ObtLink) no_url = obt_link_from_ddfile(no_url_path, NULL);
    g_test_assert_expected_messages();
    g_assert_null(no_url);

    g_autofree gchar *nope_path = g_build_filename(tmp, "nope.desktop", NULL);
    g_autoptr(ObtLink) nope = obt_link_from_ddfile(nope_path, NULL);
    g_assert_null(nope);

    obt_link_unref(NULL);

    g_unlink(no_exec_path);
    g_unlink(no_url_path);
    g_rmdir(tmp);
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

    g_test_add_func("/link/application", test_application_entry);
    g_test_add_func("/link/url", test_url_entry);
    g_test_add_func("/link/directory-and-hidden", test_directory_and_hidden);
    g_test_add_func("/link/try-exec", test_try_exec);
    g_test_add_func("/link/invalid", test_invalid_entries);

    return g_test_run();
}
