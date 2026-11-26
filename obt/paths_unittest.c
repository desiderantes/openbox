#include "obt/unittest_base.h"
#include "obt/paths.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>

static void xdg_dirs_basic(void)
{
    TEST_START();

    g_autoptr(ObtPaths) p = obt_paths_new();
    EXPECT_BOOL_EQ(TRUE, p != NULL);

    const gchar *config_home = obt_paths_config_home(p);
    const gchar *data_home = obt_paths_data_home(p);
    const gchar *cache_home = obt_paths_cache_home(p);

    EXPECT_STRING_EQ(g_get_user_config_dir(), config_home);
    EXPECT_STRING_EQ(g_get_user_data_dir(), data_home);
    EXPECT_STRING_EQ(g_get_user_cache_dir(), cache_home);

    GSList *config_dirs = obt_paths_config_dirs(p);
    EXPECT_BOOL_EQ(TRUE, config_dirs != NULL);
    EXPECT_STRING_EQ(config_home, (const gchar *)config_dirs->data);

    GSList *data_dirs = obt_paths_data_dirs(p);
    EXPECT_BOOL_EQ(TRUE, data_dirs != NULL);
    EXPECT_STRING_EQ(data_home, (const gchar *)data_dirs->data);

    GSList *autostart_dirs = obt_paths_autostart_dirs(p);
    EXPECT_UINT_EQ(g_slist_length(config_dirs), g_slist_length(autostart_dirs));
    g_autofree gchar *expected_autostart =
        g_build_filename(config_home, "autostart", NULL);
    EXPECT_STRING_EQ(expected_autostart, (const gchar *)autostart_dirs->data);

    /* Ref/unref balance */
    obt_paths_ref(p);
    obt_paths_unref(p);
    obt_paths_unref(NULL);

    TEST_END();
}

static void expand_tilde(void)
{
    TEST_START();

    const gchar *home = g_get_home_dir();
    g_autofree gchar *s = NULL;
    g_autofree gchar *expected = NULL;

    EXPECT_BOOL_EQ(TRUE, obt_paths_expand_tilde(NULL) == NULL);

    s = obt_paths_expand_tilde("~");
    EXPECT_STRING_EQ(home, s);
    g_clear_pointer(&s, g_free);

    s = obt_paths_expand_tilde("~/config/openbox");
    expected = g_strdup_printf("%s/config/openbox", home);
    EXPECT_STRING_EQ(expected, s);
    g_clear_pointer(&s, g_free);
    g_clear_pointer(&expected, g_free);

    s = obt_paths_expand_tilde("cmd ~/foo ~");
    expected = g_strdup_printf("cmd %s/foo %s", home, home);
    EXPECT_STRING_EQ(expected, s);
    g_clear_pointer(&s, g_free);
    g_clear_pointer(&expected, g_free);

    s = obt_paths_expand_tilde("foo~bar ~other/dir");
    EXPECT_STRING_EQ("foo~bar ~other/dir", s);
    g_clear_pointer(&s, g_free);

    TEST_END();
}

static void mkdir_and_try_exec(void)
{
    TEST_START();

    g_autofree gchar *tmp = g_dir_make_tmp("obt-paths-test-XXXXXX", NULL);
    EXPECT_BOOL_EQ(TRUE, tmp != NULL);

    g_autofree gchar *sub = g_build_filename(tmp, "a", NULL);
    EXPECT_BOOL_EQ(TRUE, obt_paths_mkdir(sub, 0700));
    EXPECT_BOOL_EQ(TRUE, obt_paths_mkdir(sub, 0700)); /* idempotent */
    EXPECT_BOOL_EQ(TRUE, g_file_test(sub, G_FILE_TEST_IS_DIR));

    g_autofree gchar *nested = g_build_filename(tmp, "b", "c", "d", NULL);
    EXPECT_BOOL_EQ(TRUE, obt_paths_mkdir_path(nested, 0700));
    EXPECT_BOOL_EQ(TRUE, obt_paths_mkdir_path(nested, 0700));
    EXPECT_BOOL_EQ(TRUE, g_file_test(nested, G_FILE_TEST_IS_DIR));

    /* A regular file blocks mkdir and is checked by try_exec */
    g_autofree gchar *file = g_build_filename(tmp, "tool", NULL);
    EXPECT_BOOL_EQ(TRUE, g_file_set_contents(file, "#!/bin/sh\n", -1, NULL));
    EXPECT_BOOL_EQ(FALSE, obt_paths_mkdir(file, 0700));
    EXPECT_BOOL_EQ(FALSE, obt_paths_mkdir_path(file, 0700));

    EXPECT_INT_EQ(0, g_chmod(file, 0600));
    EXPECT_BOOL_EQ(FALSE, obt_paths_try_exec(file));

    EXPECT_INT_EQ(0, g_chmod(file, 0700));
    EXPECT_BOOL_EQ(TRUE, obt_paths_try_exec(file));

    /* Directories are not executable files */
    EXPECT_BOOL_EQ(FALSE, obt_paths_try_exec(tmp));

    /* $PATH lookup */
    EXPECT_BOOL_EQ(TRUE, obt_paths_try_exec("sh"));
    EXPECT_BOOL_EQ(FALSE, obt_paths_try_exec("obt-definitely-nonexistent-bin"));
    EXPECT_BOOL_EQ(FALSE, obt_paths_try_exec(""));

    /* Cleanup */
    g_unlink(file);
    g_rmdir(nested);
    g_autofree gchar *bc = g_build_filename(tmp, "b", "c", NULL);
    g_autofree gchar *b = g_build_filename(tmp, "b", NULL);
    g_rmdir(bc);
    g_rmdir(b);
    g_rmdir(sub);
    g_rmdir(tmp);

    TEST_END();
}

void run_paths_unittest(void)
{
    unittest_start_suite("paths");

    xdg_dirs_basic();
    expand_tilde();
    mkdir_and_try_exec();

    unittest_end_suite();
}
