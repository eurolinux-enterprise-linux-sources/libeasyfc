/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-alias.h
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

#ifndef __EZFC_ALIAS_H__
#define __EZFC_ALIAS_H__

#include <glib.h>
#include <fontconfig/fontconfig.h>

G_BEGIN_DECLS

/**
 * EzfcAlias:
 *
 * All the fields in the <structname>EzfcAlias</structname>
 * structure are private to the #EzfcAlias implementation.
 */
typedef struct _EzfcAlias	EzfcAlias;


EzfcAlias *ezfc_alias_new                 (const gchar      *alias_name);
EzfcAlias *ezfc_alias_ref                 (EzfcAlias     *alias);
void          ezfc_alias_unref               (EzfcAlias     *alias);
const gchar  *ezfc_alias_get_name            (EzfcAlias     *alias);
FcPattern    *ezfc_alias_get_font_pattern    (EzfcAlias     *alias);
gboolean      ezfc_alias_set_font_pattern    (EzfcAlias     *alias,
                                              const FcPattern  *pattern,
                                              GError          **error);
const gchar  *ezfc_alias_get_font            (EzfcAlias     *alias);
gboolean      ezfc_alias_set_font            (EzfcAlias     *alias,
                                              const gchar      *font_name,
                                              GError          **error);
void          ezfc_alias_check_font_existence(EzfcAlias     *alias,
                                              gboolean          flag);

G_END_DECLS

#endif /* __EZFC_ALIAS_H__ */
