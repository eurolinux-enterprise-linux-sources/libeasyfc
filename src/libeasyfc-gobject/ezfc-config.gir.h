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

#include <libeasyfc-gobject/ezfc-alias.gir.h>
#include <libeasyfc-gobject/ezfc-font.gir.h>

G_BEGIN_DECLS

/**
 * EzfcConfig:
 *
 * All the fields in the <structname>EzfcConfig</structname>
 * structure are private to the #EzfcConfig implementation.
 */
typedef struct _EzfcConfig		EzfcConfig;


EzfcConfig *ezfc_config_new              (void);
EzfcConfig *ezfc_config_ref              (EzfcConfig  *config);
void           ezfc_config_unref            (EzfcConfig  *config);
void           ezfc_config_set_priority     (EzfcConfig  *config,
                                             guint           priority);
gint           ezfc_config_get_priority     (EzfcConfig  *config);
void           ezfc_config_set_name         (EzfcConfig  *config,
                                             const gchar    *name);
const gchar   *ezfc_config_get_name         (EzfcConfig  *config);
gboolean       ezfc_config_add_alias        (EzfcConfig  *config,
                                             const gchar    *language,
                                             EzfcAlias   *alias);
gboolean       ezfc_config_remove_alias     (EzfcConfig  *config,
                                             const gchar    *language,
                                             const gchar    *alias_name);
gboolean       ezfc_config_remove_aliases   (EzfcConfig  *config,
                                             const gchar    *language);
gboolean       ezfc_config_add_font         (EzfcConfig  *config,
                                             EzfcFont    *font);
gboolean       ezfc_config_remove_font      (EzfcConfig  *config,
                                             const gchar    *family);
gboolean       ezfc_config_remove_fonts     (EzfcConfig  *config);
gboolean       ezfc_config_add_subst        (EzfcConfig  *config,
					     const gchar    *family_name,
					     EzfcFont    *subst);
gboolean       ezfc_config_remove_subst     (EzfcConfig  *config,
					     const gchar    *family_name,
					     const gchar    *subst_name);
gboolean       ezfc_config_remove_substs    (EzfcConfig  *config,
					     const gchar    *family_name);
GList         *ezfc_config_get_language_list(EzfcConfig  *config);
const GList   *ezfc_config_get_aliases      (EzfcConfig  *config,
                                             const gchar    *language);
GList         *ezfc_config_get_fonts        (EzfcConfig  *config);
GList         *ezfc_config_get_subst_family (EzfcConfig  *config);
const GList   *ezfc_config_get_substs       (EzfcConfig  *config,
					     const gchar    *family_name);
gboolean       ezfc_config_load             (EzfcConfig  *config,
                                             GError        **error);
gboolean       ezfc_config_save             (EzfcConfig  *config,
                                             GError        **error);
GString       *ezfc_config_save_to_buffer   (EzfcConfig  *config,
                                             GError        **error);
void           ezfc_config_dump             (EzfcConfig  *config);
void           ezfc_config_set_migration    (EzfcConfig  *config,
                                             gboolean        flag);


G_END_DECLS

#endif /* __EZFC_CONFIG_H__ */
