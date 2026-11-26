#include "obt/signal.h"

#include <glib.h>
#include <signal.h>
#include <unistd.h>

typedef struct {
    GMainLoop *loop;
    gint received_sig;
    gint count;
} SignalTestState;

static void on_signal(gint sig, gpointer data)
{
    SignalTestState *state = data;
    state->received_sig = sig;
    state->count++;
    g_main_loop_quit(state->loop);
}

static gboolean raise_signal_cb(gpointer data)
{
    raise(GPOINTER_TO_INT(data));
    return G_SOURCE_REMOVE;
}

static gboolean timeout_fail_cb(gpointer data)
{
    g_main_loop_quit(data);
    g_assert_not_reached();
    return G_SOURCE_REMOVE;
}

static void run_signal_delivery_test(gint sig)
{
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    SignalTestState state = { .loop = loop, .received_sig = 0, .count = 0 };

    obt_signal_listen();
    obt_signal_add_callback(sig, on_signal, &state);

    guint timeout_id = g_timeout_add(2000, timeout_fail_cb, loop);
    g_idle_add(raise_signal_cb, GINT_TO_POINTER(sig));

    g_main_loop_run(loop);

    g_clear_handle_id(&timeout_id, g_source_remove);
    obt_signal_remove_callback(sig, on_signal);
    obt_signal_stop();

    g_assert_cmpint(state.count, ==, 1);
    g_assert_cmpint(state.received_sig, ==, sig);
}

static void test_glib_unix_signal(void)
{
    run_signal_delivery_test(SIGUSR1);
}

static void test_pipe_fallback_signal(void)
{
    run_signal_delivery_test(SIGPIPE);
}

int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/signal/glib-unix-sigusr1", test_glib_unix_signal);
    g_test_add_func("/signal/pipe-fallback-sigpipe", test_pipe_fallback_signal);

    return g_test_run();
}
