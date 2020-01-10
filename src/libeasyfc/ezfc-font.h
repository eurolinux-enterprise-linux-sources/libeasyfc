/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-font.h
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
#if !defined (__EZFC_H__INSIDE) && !defined (__EZFC_COMPILATION)
#error "Only <libeasyfc/ezfc.h> can be included directly."
#endif

#ifndef __EZFC_FONT_H__
#define __EZFC_FONT_H__

#include <glib.h>
#include <fontconfig/fontconfig.h>
#include <libeasyfc/ezfc-utils.h>

G_BEGIN_DECLS

/**
 * ezfc_font_t:
 *
 * All the fields in the <structname>ezfc_font_t</structname>
 * structure are private to the #ezfc_font_t implementation.
 */
typedef struct _ezfc_font_t	ezfc_font_t;

/**
 * ezfc_font_subpixel_render_t:
 * @EZFC_FONT_ANTIALIAS_UNKNOWN: unknown state on using the sub-pixel rendering.
 * @EZFC_FONT_ANTIALIAS_NONE: no use of the sub-pixel rendering
 * @EZFC_FONT_ANTIALIAS_GRAY: Use the gray-scaled sub-pixel rendering
 * @EZFC_FONT_ANTIALIAS_RGB: Use the sub-pixel rendering with the sub-pixel geometry RGB.
 * @EZFC_FONT_ANTIALIAS_BGR: Use the sub-pixel rendering with the sub-pixel geometry BGR.
 * @EZFC_FONT_ANTIALIAS_VRGB: Use the sub-pixel rendering with the sub-pixel geometry VRGB.
 * @EZFC_FONT_ANTIALIAS_VBGR: Use the sub-pixel rendering with the sub-pixel geometry VBGR.
 * @EZFC_FONT_ANTIALIAS_END: No real value, but just a terminator.
 *
 * The sub-pixel rendering option to be used in ezfc_font_set_subpixel_rendering().
 */
typedef enum _ezfc_font_subpixel_render_t {
	EZFC_FONT_ANTIALIAS_UNKNOWN = 0,
	EZFC_FONT_ANTIALIAS_NONE,
	EZFC_FONT_ANTIALIAS_GRAY,
	EZFC_FONT_ANTIALIAS_RGB,
	EZFC_FONT_ANTIALIAS_BGR,
	EZFC_FONT_ANTIALIAS_VRGB,
	EZFC_FONT_ANTIALIAS_VBGR,
	EZFC_FONT_ANTIALIAS_END
} ezfc_font_subpixel_render_t;

/**
 * ezfc_font_hintstyle_t:
 * @EZFC_FONT_HINTSTYLE_UNKNOWN: unknown state in the hintstyle.
 * @EZFC_FONT_HINTSTYLE_NONE: No use of autohinting
 * @EZFC_FONT_HINTSTYLE_SLIGHT: Use slight autohinting
 * @EZFC_FONT_HINTSTYLE_MEDIUM:Use medium autohinting
 * @EZFC_FONT_HINTSTYLE_FULL:Use full autohinting
 * @EZFC_FONT_HINTSTYLE_END: No real value, but just a terminator.
 *
 * The hintstyle option to be used for ezfc_font_set_hintstyle().
 */
typedef enum _ezfc_font_hintstyle_t {
	EZFC_FONT_HINTSTYLE_UNKNOWN = 0,
	EZFC_FONT_HINTSTYLE_NONE,
	EZFC_FONT_HINTSTYLE_SLIGHT,
	EZFC_FONT_HINTSTYLE_MEDIUM,
	EZFC_FONT_HINTSTYLE_FULL,
	EZFC_FONT_HINTSTYLE_END
} ezfc_font_hintstyle_t;

gboolean                     ezfc_font_is_alias_font              (const gchar                  *alias_name);
GList                       *ezfc_font_get_list                   (const gchar                  *language,
                                                                   const gchar                  *alias_name,
                                                                   gboolean                      localized_font_name);
GList                       *ezfc_font_get_pattern_list           (const gchar                  *language,
                                                                   const gchar                  *alias_name);
GList                       *ezfc_font_get_alias_name_from_pattern(const FcPattern              *pattern);
ezfc_font_t                 *ezfc_font_new                        (void);
ezfc_font_t                 *ezfc_font_ref                        (ezfc_font_t                  *font);
void                         ezfc_font_unref                      (ezfc_font_t                  *font);
FcPattern                   *ezfc_font_get_pattern                (ezfc_font_t                  *font);
gboolean                     ezfc_font_set_pattern                (ezfc_font_t                  *font,
                                                                   const FcPattern              *pattern,
                                                                   GError                      **error);
const gchar                 *ezfc_font_get_family                 (ezfc_font_t                  *font);
GList                       *ezfc_font_get_families               (ezfc_font_t                  *font);
gboolean                     ezfc_font_find                       (ezfc_font_t                  *font,
								   const gchar                  *font_name);
gboolean                     ezfc_font_add_family                 (ezfc_font_t                  *font,
                                                                   const gchar                  *font_name,
                                                                   GError                      **error);
gboolean                     ezfc_font_remove                     (ezfc_font_t                  *font,
								   GError                      **error);
gboolean                     ezfc_font_remove_family              (ezfc_font_t                  *font,
								   const gchar                  *font_name,
								   GError                      **error);
void                         ezfc_font_check_existence            (ezfc_font_t                  *font,
                                                                   gboolean                      flag);
void                         ezfc_font_set_hinting                (ezfc_font_t                  *font,
                                                                   gboolean                      flag);
gboolean                     ezfc_font_get_hinting                (ezfc_font_t                  *font);
void                         ezfc_font_set_autohinting            (ezfc_font_t                  *font,
                                                                   gboolean                      flag);
gboolean                     ezfc_font_get_autohinting            (ezfc_font_t                  *font);
void                         ezfc_font_set_antialiasing           (ezfc_font_t                  *font,
                                                                   gboolean                      flag);
gboolean                     ezfc_font_get_antialiasing           (ezfc_font_t                  *font);
void                         ezfc_font_set_hintstyle              (ezfc_font_t                  *font,
								   ezfc_font_hintstyle_t         hintstyle);
ezfc_font_hintstyle_t        ezfc_font_get_hintstyle              (ezfc_font_t                  *font);
void                         ezfc_font_set_embedded_bitmap        (ezfc_font_t                  *font,
                                                                   gboolean                      flag);
gboolean                     ezfc_font_get_embedded_bitmap        (ezfc_font_t                  *font);
void                         ezfc_font_set_rgba                   (ezfc_font_t                  *font,
                                                                   gint                          val);
gint                         ezfc_font_get_rgba                   (ezfc_font_t                  *font);
gboolean                     ezfc_font_set_subpixel_rendering     (ezfc_font_t                  *font,
                                                                   ezfc_font_subpixel_render_t   mode);
ezfc_font_subpixel_render_t  ezfc_font_get_subpixel_rendering     (ezfc_font_t                  *font);
gboolean                     ezfc_font_add_feature                (ezfc_font_t                  *font,
								   const gchar                  *feature);
gboolean                     ezfc_font_remove_feature             (ezfc_font_t                  *font,
								   const gchar                  *feature);
GList                       *ezfc_font_get_features               (ezfc_font_t                  *font);
GList                       *ezfc_font_get_available_features     (ezfc_font_t                  *font);
GList                       *ezfc_font_canonicalize               (ezfc_font_t                  *font,
								   GError                      **error);

#ifndef EZFC_DISABLE_DEPRECATED
EZFC_DEPRECATED_FOR(ezfc_font_add_family)
gboolean                     ezfc_font_set_family                 (ezfc_font_t                  *font,
                                                                   const gchar                  *font_name,
                                                                   GError                      **error);
#endif /* !EZFC_DISABLE_DEPRECATED */

G_END_DECLS

#endif /* __EZFC_FONT_H__ */
