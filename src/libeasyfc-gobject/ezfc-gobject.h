/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-gobject.h
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
#ifndef __EZFC_EZFC_GOBJECT_H__
#define __EZFC_EZFC_GOBJECT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define EZFC_TYPE_ALIAS		ezfc_alias_get_type()
#define EZFC_TYPE_CONFIG	ezfc_config_get_type()
#define EZFC_TYPE_FONT		ezfc_font_get_type()


GType ezfc_alias_get_type(void);
GType ezfc_config_get_type(void);
GType ezfc_font_get_type(void);

G_END_DECLS

#endif /* __EZFC_EZFC_GOBJECT_H__ */
