/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-config.h
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

#ifndef __EZFC_CONFIG_H__
#define __EZFC_CONFIG_H__

#include <libeasyfc/ezfc-alias.h>
#include <libeasyfc/ezfc-font.h>

G_BEGIN_DECLS

/**
 * ezfc_config_t:
 *
 * All the fields in the <structname>ezfc_config_t</structname>
 * structure are private to the #ezfc_config_t implementation.
 */
typedef struct _ezfc_config_t		ezfc_config_t;


ezfc_config_t *ezfc_config_new              (void);
ezfc_config_t *ezfc_config_ref              (ezfc_config_t  *config);
void           ezfc_config_unref            (ezfc_config_t  *config);
void           ezfc_config_set_priority     (ezfc_config_t  *config,
                                             guint           priority);
gint           ezfc_config_get_priority     (ezfc_config_t  *config);
void           ezfc_config_set_name         (ezfc_config_t  *config,
                                             const gchar    *name);
const gchar   *ezfc_config_get_name         (ezfc_config_t  *config);
gboolean       ezfc_config_add_alias        (ezfc_config_t  *config,
                                             const gchar    *language,
                                             ezfc_alias_t   *alias);
gboolean       ezfc_config_remove_alias     (ezfc_config_t  *config,
                                             const gchar    *language,
                                             const gchar    *alias_name);
gboolean       ezfc_config_remove_aliases   (ezfc_config_t  *config,
                                             const gchar    *language);
gboolean       ezfc_config_add_font         (ezfc_config_t  *config,
                                             ezfc_font_t    *font);
gboolean       ezfc_config_remove_font      (ezfc_config_t  *config,
                                             const gchar    *family);
gboolean       ezfc_config_remove_fonts     (ezfc_config_t  *config);
gboolean       ezfc_config_add_subst        (ezfc_config_t  *config,
					     const gchar    *family_name,
					     ezfc_font_t    *subst);
gboolean       ezfc_config_remove_subst     (ezfc_config_t  *config,
					     const gchar    *family_name,
					     const gchar    *subst_name);
gboolean       ezfc_config_remove_substs    (ezfc_config_t  *config,
					     const gchar    *family_name);
GList         *ezfc_config_get_language_list(ezfc_config_t  *config);
const GList   *ezfc_config_get_aliases      (ezfc_config_t  *config,
                                             const gchar    *language);
GList         *ezfc_config_get_fonts        (ezfc_config_t  *config);
GList         *ezfc_config_get_subst_family (ezfc_config_t  *config);
const GList   *ezfc_config_get_substs       (ezfc_config_t  *config,
					     const gchar    *family_name);
gboolean       ezfc_config_load             (ezfc_config_t  *config,
                                             GError        **error);
gboolean       ezfc_config_save             (ezfc_config_t  *config,
                                             GError        **error);
GString       *ezfc_config_save_to_buffer   (ezfc_config_t  *config,
                                             GError        **error);
void           ezfc_config_dump             (ezfc_config_t  *config);
void           ezfc_config_set_migration    (ezfc_config_t  *config,
                                             gboolean        flag);


G_END_DECLS

#endif /* __EZFC_CONFIG_H__ */
