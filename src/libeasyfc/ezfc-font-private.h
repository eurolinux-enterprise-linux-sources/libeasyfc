/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-font-private.h
 * Copyright (C) 2011-2013 Akira TAGOH
 * 
 * Authors:
 *   Akira TAGOH  <akira@tagoh.org>
 * 
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __EZFC_FONT_PRIVATE_H__
#define __EZFC_FONT_PRIVATE_H__

#include <glib.h>
#include "ezfc-mem.h"
#include <fontconfig/fontconfig.h>
#include "ezfc-font.h"

G_BEGIN_DECLS

typedef struct _ezfc_font_private_t {
	ezfc_mem_t             parent;
	FcPattern             *pattern;
	gboolean               check_font_existence;
	gint                   masks;
	gint                   rgba;
	ezfc_font_hintstyle_t  hintstyle;
	gboolean               hinting;
	gboolean               autohinting;
	gboolean               antialiasing;
	gboolean               embedded_bitmap;
} ezfc_font_private_t;

typedef enum _ezfc_font_mask_t {
	EZFC_FONT_MASK_HINTING         = 1 << 0,
	EZFC_FONT_MASK_AUTOHINT        = 1 << 1,
	EZFC_FONT_MASK_ANTIALIAS       = 1 << 2,
	EZFC_FONT_MASK_EMBEDDED_BITMAP = 1 << 3,
	EZFC_FONT_MASK_RGBA            = 1 << 4,
	EZFC_FONT_MASK_HINTSTYLE       = 1 << 5,
	EZFC_FONT_MASK_END
} ezfc_font_mask_t;

void ezfc_font_init             (ezfc_font_t *font);
gint ezfc_font_get_boolean_masks(ezfc_font_t *font);

G_END_DECLS

#endif /* __EZFC_FONT_PRIVATE_H__ */
