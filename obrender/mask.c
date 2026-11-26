/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   mask.c for the Openbox window manager
   Copyright (c) 2003-2007   Dana Jansens
   Copyright (c) 2003        Derek Foreman

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

#include "render.h"
#include "color.h"
#include "mask.h"

RrPixmapMask *RrPixmapMaskNew(const RrInstance *inst,
                              gint w, gint h, const gchar *data)
{
    gsize byte_size;
    RrPixmapMask *m;

    g_return_val_if_fail(w > 0 && h > 0, NULL);
    /* round up to nearest byte */
    if (!g_size_checked_mul(&byte_size, ((gsize)w + 7) / 8, (gsize)h))
        return NULL;

    m = g_new(RrPixmapMask, 1);
    m->inst = inst;
    m->width = w;
    m->height = h;
    m->data = g_memdup2(data, byte_size);
    m->mask = XCreateBitmapFromData(RrDisplay(inst), RrRootWindow(inst),
                                    data, w, h);
    return m;
}

void RrPixmapMaskFree(RrPixmapMask *m)
{
    if (m) {
        XFreePixmap(RrDisplay(m->inst), m->mask);
        g_free(m->data);
        g_free(m);
    }
}

void RrPixmapMaskDraw(Pixmap p, const RrTextureMask *m, const RrRect *area)
{
    gint x, y;
    if (m->mask == NULL) return; /* no mask given */

    /* set the clip region */
    x = area->x + (area->width - m->mask->width) / 2;
    y = area->y + (area->height - m->mask->height) / 2;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    XSetClipMask(RrDisplay(m->mask->inst), RrColorGC(m->color), m->mask->mask);
    XSetClipOrigin(RrDisplay(m->mask->inst), RrColorGC(m->color), x, y);

    /* fill in the clipped region */
    XFillRectangle(RrDisplay(m->mask->inst), p, RrColorGC(m->color), x, y,
                   x + m->mask->width, y + m->mask->height);

    /* unset the clip region */
    XSetClipMask(RrDisplay(m->mask->inst), RrColorGC(m->color), None);
    XSetClipOrigin(RrDisplay(m->mask->inst), RrColorGC(m->color), 0, 0);
}

RrPixmapMask *RrPixmapMaskCopy(const RrPixmapMask *src)
{
    gsize byte_size;
    RrPixmapMask *m;

    g_return_val_if_fail(src->width > 0 && src->height > 0, NULL);
    /* round up to nearest byte */
    if (!g_size_checked_mul(&byte_size, ((gsize)src->width + 7) / 8,
                            (gsize)src->height))
        return NULL;

    m = g_new(RrPixmapMask, 1);
    m->inst = src->inst;
    m->width = src->width;
    m->height = src->height;
    m->data = g_memdup2(src->data, byte_size);
    m->mask = XCreateBitmapFromData(RrDisplay(m->inst), RrRootWindow(m->inst),
                                    m->data, m->width, m->height);
    return m;
}
