/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-utils.h
 * Copyright (C) 2011-2012 Akira TAGOH
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
#if !defined (__EZFC_H__INSIDE) && !defined (__EZFC_COMPILATION)
#error "Only <libeasyfc/ezfc.h> can be included directly."
#endif

#ifndef __EZFC_UTILS_H__
#define __EZFC_UTILS_H__

#include <glib.h>

G_BEGIN_DECLS

/**
 * SECTION:ezfc-utils
 * @Short_Description: A collection of the utility functions
 * @Title: Utilities
 *
 * This collects some utility functions.
 */

const gchar *ezfc_version (void);
void         ezfc_init    (void);
void         ezfc_finalize(void);

#ifndef G_DEPRECATED
#define EZFC_DEPRECATED		G_GNUC_DEPRECATED
#else
#define EZFC_DEPRECATED		G_DEPRECATED
#endif

#ifndef G_DEPRECATED_FOR
#define EZFC_DEPRECATED_FOR(f)	G_GNUC_DEPRECATED_FOR(f)
#else
#define EZFC_DEPRECATED_FOR(f)	G_DEPRECATED_FOR(f)
#endif

#ifndef EZFC_DISABLE_DEPRECATED
EZFC_DEPRECATED_FOR(ezfc_font_is_alias_font)
gboolean  ezfc_is_alias_font         (const gchar *alias_name);

EZFC_DEPRECATED_FOR(ezfc_font_get_list)
GList    *ezfc_get_fonts_list        (const gchar *language,
				      const gchar *alias_name);

EZFC_DEPRECATED_FOR(ezfc_font_get_pattern_list)
GList    *ezfc_get_fonts_pattern_list(const gchar *language,
				      const gchar *alias_name);
#endif /* !EZFC_DISABLE_DEPRECATED */

G_END_DECLS

#endif /* __EZFC_UTILS_H__ */
