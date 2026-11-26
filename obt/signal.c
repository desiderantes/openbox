/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/signal.c for the Openbox window manager
   Copyright (c) 2010        Dana Jansens

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

#include "obt/signal.h"

#include <fcntl.h>
#include <glib-unix.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct _ObtSignalCallback ObtSignalCallback;

struct _ObtSignalCallback
{
    ObtSignalHandler func;
    gpointer data;
};

/* this should be more than the number of possible signals on any
   architecture... */
#define NUM_SIGNALS 99

/* keep track of what signals have a signal handler installed, and remember
   the action we replaced when installing it for when we clean up */
static struct {
    guint installed; /* a ref count */
    guint glib_source_id; /* non-zero when handled via g_unix_signal_add_full */
    struct sigaction oldact;
} all_signals[NUM_SIGNALS];

/* signals which cause a core dump, these can't be used for callbacks */
static const gint core_signals[] =
{
    SIGABRT,
    SIGSEGV,
    SIGFPE,
    SIGILL,
    SIGQUIT,
    SIGTRAP,
    SIGSYS,
    SIGBUS,
    SIGXCPU,
    SIGXFSZ
};
#define NUM_CORE_SIGNALS (gint)(sizeof(core_signals) / sizeof(core_signals[0]))

static guint listeners = 0; /* a ref count for the signal listener */
static gint pipe_fds[2] = { -1, -1 };
static guint pipe_source_id = 0;
static volatile sig_atomic_t signals_fired[NUM_SIGNALS];
static GSList *callbacks[NUM_SIGNALS];

static gboolean is_core_signal(gint sig)
{
    for (gint i = 0; i < NUM_CORE_SIGNALS; ++i)
        if (sig == core_signals[i])
            return TRUE;
    return FALSE;
}

static gboolean is_glib_unix_signal(gint sig)
{
    switch (sig) {
    case SIGHUP:
    case SIGINT:
    case SIGTERM:
    case SIGUSR1:
    case SIGUSR2:
#ifdef SIGWINCH
    case SIGWINCH:
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

static void dispatch_callbacks(gint sig)
{
    for (GSList *it = callbacks[sig]; it; ) {
        GSList *next = g_slist_next(it);
        const ObtSignalCallback *cb = it->data;
        cb->func(sig, cb->data);
        it = next;
    }
}

static gboolean glib_signal_dispatch(gpointer data)
{
    dispatch_callbacks(GPOINTER_TO_INT(data));
    return G_SOURCE_CONTINUE;
}

static gboolean pipe_signal_dispatch(gint fd, GIOCondition condition,
                                     gpointer data)
{
    guchar buf[64];

    while (read(fd, buf, sizeof(buf)) > 0)
        ;

    for (gint i = 0; i < NUM_SIGNALS; ++i) {
        if (signals_fired[i]) {
            signals_fired[i] = 0;
            dispatch_callbacks(i);
        }
    }

    return G_SOURCE_CONTINUE;
}

static void sighandler(gint sig)
{
    if (sig < 0 || sig >= NUM_SIGNALS)
        return;

    if (is_core_signal(sig)) {
        const char msg[] =
            "How are you gentlemen? All your base are belong to us. "
            "(Openbox received a fatal signal)\n";
        ssize_t ignored = write(STDERR_FILENO, msg, sizeof(msg) - 1);
        (void)ignored;
        abort();
    }

    signals_fired[sig] = 1;
    if (pipe_fds[1] >= 0) {
        guchar b = (guchar)sig;
        ssize_t ignored = write(pipe_fds[1], &b, 1);
        (void)ignored;
    }
}

void obt_signal_listen(void)
{
    if (!listeners) {
        struct sigaction action;

        sigemptyset(&action.sa_mask);
        action.sa_handler = sighandler;
        action.sa_flags = SA_NOCLDSTOP;

        /* always grab all the signals that cause core dumps */
        for (gint i = 0; i < NUM_CORE_SIGNALS; ++i) {
            /* SIGABRT is not grabbed here because we call abort() to dump core
               when a core signal is caught. */
            if (core_signals[i] != SIGABRT) {
                sigaction(core_signals[i], &action,
                          &all_signals[core_signals[i]].oldact);
                all_signals[core_signals[i]].installed++;
            }
        }

        if (g_unix_open_pipe(pipe_fds, O_CLOEXEC, NULL)) {
            g_unix_set_fd_nonblocking(pipe_fds[0], TRUE, NULL);
            g_unix_set_fd_nonblocking(pipe_fds[1], TRUE, NULL);
            pipe_source_id = g_unix_fd_add_full(G_PRIORITY_HIGH,
                                                pipe_fds[0],
                                                G_IO_IN,
                                                pipe_signal_dispatch,
                                                NULL, NULL);
        }
    }

    ++listeners;
}

void obt_signal_stop(void)
{
    g_return_if_fail(listeners > 0);
    --listeners;

    if (!listeners) {
        g_clear_handle_id(&pipe_source_id, g_source_remove);

        /* remove user defined signal handlers */
        for (gint i = 0; i < NUM_SIGNALS; ++i) {
            for (GSList *it = callbacks[i]; it; ) {
                GSList *next = g_slist_next(it);
                ObtSignalCallback *cb = it->data;
                obt_signal_remove_callback(i, cb->func);
                it = next;
            }
            signals_fired[i] = 0;
        }

        if (pipe_fds[0] >= 0) {
            close(pipe_fds[0]);
            pipe_fds[0] = -1;
        }
        if (pipe_fds[1] >= 0) {
            close(pipe_fds[1]);
            pipe_fds[1] = -1;
        }

        /* release all the signals that cause core dumps */
        for (gint i = 0; i < NUM_CORE_SIGNALS; ++i) {
            if (all_signals[core_signals[i]].installed) {
                sigaction(core_signals[i],
                          &all_signals[core_signals[i]].oldact, NULL);
                all_signals[core_signals[i]].installed--;
            }
        }

#ifdef DEBUG
        for (gint i = 0; i < NUM_SIGNALS; ++i)
            g_assert(all_signals[i].installed == 0);
#endif
    }
}

void obt_signal_add_callback(gint sig, ObtSignalHandler func, gpointer data)
{
    ObtSignalCallback *cb;

    g_return_if_fail(func != NULL);
    g_return_if_fail(sig >= 0 && sig < NUM_SIGNALS);
    g_return_if_fail(!is_core_signal(sig));

    cb = g_new(ObtSignalCallback, 1);
    cb->func = func;
    cb->data = data;
    callbacks[sig] = g_slist_prepend(callbacks[sig], cb);

    /* install the signal handler */
    if (!all_signals[sig].installed) {
        if (is_glib_unix_signal(sig)) {
            all_signals[sig].glib_source_id =
                g_unix_signal_add_full(G_PRIORITY_HIGH, sig,
                                       glib_signal_dispatch,
                                       GINT_TO_POINTER(sig), NULL);
        }
        else {
            struct sigaction action;

            sigemptyset(&action.sa_mask);
            action.sa_handler = sighandler;
            action.sa_flags = SA_NOCLDSTOP;

            sigaction(sig, &action, &all_signals[sig].oldact);
        }
    }

    all_signals[sig].installed++;
}

void obt_signal_remove_callback(gint sig, ObtSignalHandler func)
{
    g_return_if_fail(func != NULL);
    g_return_if_fail(sig >= 0 && sig < NUM_SIGNALS);
    g_return_if_fail(!is_core_signal(sig));

    for (GSList *it = callbacks[sig]; it; it = g_slist_next(it)) {
        ObtSignalCallback *cb = it->data;
        if (cb->func == func) {
            g_assert(all_signals[sig].installed > 0);

            callbacks[sig] = g_slist_delete_link(callbacks[sig], it);
            g_free(cb);

            /* uninstall the signal handler */
            all_signals[sig].installed--;
            if (!all_signals[sig].installed) {
                if (all_signals[sig].glib_source_id)
                    g_clear_handle_id(&all_signals[sig].glib_source_id,
                                      g_source_remove);
                else
                    sigaction(sig, &all_signals[sig].oldact, NULL);
            }
            break;
        }
    }
}
