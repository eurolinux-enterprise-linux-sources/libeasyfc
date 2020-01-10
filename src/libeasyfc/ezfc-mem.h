/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-mem.h
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
#ifndef __EZFC_MEM_H__
#define __EZFC_MEM_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _ezfc_mem_t	ezfc_mem_t;

typedef void (* ezfc_destroy_func_t)	(gpointer data);

struct _ezfc_mem_t {
	volatile guint  ref_count;
	gsize           size;
	GHashTable     *refs;
	GList          *weak_pointers;
};

gpointer ezfc_mem_alloc_object       (gsize                size);
gpointer ezfc_mem_ref                (ezfc_mem_t          *object);
void     ezfc_mem_unref              (ezfc_mem_t          *object);
void     ezfc_mem_add_ref            (ezfc_mem_t          *object,
                                      gpointer             p,
                                      ezfc_destroy_func_t  func);
void     ezfc_mem_remove_ref         (ezfc_mem_t          *object,
                                      gpointer             p);
void     ezfc_mem_delete_ref         (ezfc_mem_t          *object,
				      gpointer             p);
void     ezfc_mem_add_weak_pointer   (ezfc_mem_t          *object,
                                      gpointer            *p);
void     ezfc_mem_remove_weak_pointer(ezfc_mem_t          *object,
                                      gpointer            *p);

G_END_DECLS

#endif /* __EZFC_MEM_H__ */
