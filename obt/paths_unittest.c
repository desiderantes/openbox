#include "obt/paths.h"

#include <glib.h>
#include <glib/gstdio.h>

static void test_xdg_dirs(void)
{
    g_autoptr(ObtPaths) p = obt_paths_new();
    g_assert_nonnull(p);

    const gchar *config_home = obt_paths_config_home(p);
    const gchar *data_home = obt_paths_data_home(p);
    const gchar *cache_home = obt_paths_cache_home(p);

    g_assert_cmpstr(config_home, ==, g_get_user_config_dir());
    g_assert_cmpstr(data_home, ==, g_get_user_data_dir());
    g_assert_cmpstr(cache_home, ==, g_get_user_cache_dir());

    GSList *config_dirs = obt_paths_config_dirs(p);
    g_assert_nonnull(config_dirs);
    g_assert_cmpstr((const gchar *)config_dirs->data, ==, config_home);

    GSList *data_dirs = obt_paths_data_dirs(p);
    g_assert_nonnull(data_dirs);
    g_assert_cmpstr((const gchar *)data_dirs->data, ==, data_home);

    GSList *autostart_dirs = obt_paths_autostart_dirs(p);
    g_assert_cmpuint(g_slist_length(autostart_dirs), ==,
                     g_slist_length(config_dirs));
    g_autofree gchar *expected_autostart =
        g_build_filename(config_home, "autostart", NULL);
    g_assert_cmpstr((const gchar *)autostart_dirs->data, ==,
                    expected_autostart);

    /* Ref/unref balance */
    obt_paths_ref(p);
    obt_paths_unref(p);
    obt_paths_unref(NULL);
}

static void test_expand_tilde(void)
{
    const gchar *home = g_get_home_dir();
    g_autofree gchar *s = NULL;
    g_autofree gchar *expected = NULL;

    g_assert_null(obt_paths_expand_tilde(NULL));

    s = obt_paths_expand_tilde("~");
    g_assert_cmpstr(s, ==, home);
    g_clear_pointer(&s, g_free);

    s = obt_paths_expand_tilde("~/config/openbox");
    expected = g_strdup_printf("%s/config/openbox", home);
    g_assert_cmpstr(s, ==, expected);
    g_clear_pointer(&s, g_free);
    g_clear_pointer(&expected, g_free);

    s = obt_paths_expand_tilde("cmd ~/foo ~");
    expected = g_strdup_printf("cmd %s/foo %s", home, home);
    g_assert_cmpstr(s, ==, expected);
    g_clear_pointer(&s, g_free);
    g_clear_pointer(&expected, g_free);

    s = obt_paths_expand_tilde("foo~bar ~other/dir");
    g_assert_cmpstr(s, ==, "foo~bar ~other/dir");
}

static void test_mkdir_and_try_exec(void)
{
    g_autofree gchar *tmp = g_dir_make_tmp("obt-paths-test-XXXXXX", NULL);
    g_assert_nonnull(tmp);

    g_autofree gchar *sub = g_build_filename(tmp, "a", NULL);
    g_assert_true(obt_paths_mkdir(sub, 0700));
    g_assert_true(obt_paths_mkdir(sub, 0700)); /* idempotent */
    g_assert_true(g_file_test(sub, G_FILE_TEST_IS_DIR));

    g_autofree gchar *nested = g_build_filename(tmp, "b", "c", "d", NULL);
    g_assert_true(obt_paths_mkdir_path(nested, 0700));
    g_assert_true(obt_paths_mkdir_path(nested, 0700));
    g_assert_true(g_file_test(nested, G_FILE_TEST_IS_DIR));

    /* A regular file blocks mkdir and is checked by try_exec */
    g_autofree gchar *file = g_build_filename(tmp, "tool", NULL);
    g_assert_true(g_file_set_contents(file, "#!/bin/sh\n", -1, NULL));
    g_assert_false(obt_paths_mkdir(file, 0700));
    g_assert_false(obt_paths_mkdir_path(file, 0700));

    g_assert_cmpint(g_chmod(file, 0600), ==, 0);
    g_assert_false(obt_paths_try_exec(file));

    g_assert_cmpint(g_chmod(file, 0700), ==, 0);
    g_assert_true(obt_paths_try_exec(file));

    /* Directories are not executable files */
    g_assert_false(obt_paths_try_exec(tmp));

    /* $PATH lookup */
    g_assert_true(obt_paths_try_exec("sh"));
    g_assert_false(obt_paths_try_exec("obt-definitely-nonexistent-bin"));
    g_assert_false(obt_paths_try_exec(""));

    /* Cleanup */
    g_unlink(file);
    g_rmdir(nested);
    g_autofree gchar *bc = g_build_filename(tmp, "b", "c", NULL);
    g_autofree gchar *b = g_build_filename(tmp, "b", NULL);
    g_rmdir(bc);
    g_rmdir(b);
    g_rmdir(sub);
    g_rmdir(tmp);
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

    g_test_add_func("/paths/xdg-dirs", test_xdg_dirs);
    g_test_add_func("/paths/expand-tilde", test_expand_tilde);
    g_test_add_func("/paths/mkdir-and-try-exec", test_mkdir_and_try_exec);

    return g_test_run();
}
