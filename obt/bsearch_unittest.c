#include "obt/bsearch.h"

#include <glib.h>

static void test_empty(void)
{
    BSEARCH_SETUP();
    int *array = NULL;
    guint array_size = 0;

    /* Search in an empty array. */
    BSEARCH(int, array, 0, array_size, 10);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);

    /* Search in an empty array with a non-zero starting position. */
    BSEARCH(int, array, 10, array_size, -10);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 10);
}

static void test_single_element(void)
{
    BSEARCH_SETUP();
    int array[1];
    guint array_size = 1;

    /* Search for something smaller than the only element. */
    array[0] = 20;
    BSEARCH(int, array, 0, array_size, -10);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    /* Search for something bigger than the only element. */
    array[0] = 20;
    BSEARCH(int, array, 0, array_size, 30);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    /* Search for something smaller than the only element. */
    array[0] = -20;
    BSEARCH(int, array, 0, array_size, -30);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    /* Search for something bigger than the only element. */
    array[0] = -20;
    BSEARCH(int, array, 0, array_size, 10);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    /* Search for the only element that exists. */
    array[0] = -20;
    BSEARCH(int, array, 0, array_size, -20);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());
}

static void test_single_element_nonzero_start(void)
{
    BSEARCH_SETUP();
    int array[10];
    guint array_start = 9;
    guint array_size = 1;

    /* Search for something smaller than the only element. */
    array[array_start] = 20;
    BSEARCH(int, array, array_start, array_size, -10);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    /* Search for something bigger than the only element. */
    array[array_start] = 20;
    BSEARCH(int, array, array_start, array_size, 30);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    /* Search for something smaller than the only element. */
    array[array_start] = -20;
    BSEARCH(int, array, array_start, array_size, -30);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    /* Search for something bigger than the only element. */
    array[array_start] = -20;
    BSEARCH(int, array, array_start, array_size, 10);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    /* Search for the only element that exists. */
    array[array_start] = -20;
    BSEARCH(int, array, array_start, array_size, -20);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());
}

static void test_present(void)
{
    BSEARCH_SETUP();
    int array[5] = { 10, 12, 14, 16, 18 };
    guint array_start = 0;
    guint array_size = 5;

    BSEARCH(int, array, array_start, array_size, 10);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 12);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 1);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 14);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 2);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 16);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 3);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 18);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 4);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());
}

static void test_present_nonzero_start(void)
{
    BSEARCH_SETUP();
    int array[5] = { 10, 12, 14, 16, 18 };
    guint array_start = 2;
    guint array_size = 3;

    BSEARCH(int, array, array_start, array_size, 10);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 12);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 14);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 2);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 16);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 3);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 18);
    g_assert_true(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 4);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());
}

static void test_missing(void)
{
    BSEARCH_SETUP();
    int array[5] = { 10, 12, 14, 16, 18 };
    guint array_start = 0;
    guint array_size = 5;

    BSEARCH(int, array, array_start, array_size, 9);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 11);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 0);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 13);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 1);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 15);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 2);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 17);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 3);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 19);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 4);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());
}

static void test_missing_nonzero_start(void)
{
    BSEARCH_SETUP();
    int array[5] = { 10, 12, 14, 16, 18 };
    guint array_start = 2;
    guint array_size = 3;

    BSEARCH(int, array, array_start, array_size, 9);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 11);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 13);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, array_start);
    g_assert_false(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 15);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 2);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 17);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 3);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());

    BSEARCH(int, array, array_start, array_size, 19);
    g_assert_false(BSEARCH_FOUND());
    g_assert_cmpuint(BSEARCH_AT(), ==, 4);
    g_assert_true(BSEARCH_FOUND_NEAREST_SMALLER());
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/bsearch/empty", test_empty);
    g_test_add_func("/bsearch/single-element", test_single_element);
    g_test_add_func("/bsearch/single-element-nonzero-start",
                    test_single_element_nonzero_start);
    g_test_add_func("/bsearch/present", test_present);
    g_test_add_func("/bsearch/present-nonzero-start",
                    test_present_nonzero_start);
    g_test_add_func("/bsearch/missing", test_missing);
    g_test_add_func("/bsearch/missing-nonzero-start",
                    test_missing_nonzero_start);

    return g_test_run();
}
