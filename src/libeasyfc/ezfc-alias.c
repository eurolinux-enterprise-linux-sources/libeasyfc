/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-alias.c
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <libeasyfc/ezfc-error.h>
#include "ezfc-mem.h"
#include "ezfc-font-private.h"
#include <libeasyfc/ezfc-font.h>
#include <libeasyfc/ezfc-alias.h>

/**
 * SECTION:ezfc-alias
 * @Short_Description: A class for managing the alias font name.
 * @Title: ezfc_alias_t
 *
 * This class provides an easy access to the alias font name and the font
 * that actually being assigned to it.
 */

struct _ezfc_alias_t {
	ezfc_font_private_t  parent;
	gchar               *alias_name;
};

/*< private >*/

/*< public >*/

/**
 * ezfc_alias_new:
 * @alias_name: the alias font name like sans-serif, serif and monospace.
 *              This can be the usual font name. in this case, the font added by
 *              ezfc_alias_set_font() behaves as the substitution font of it.
 *
 * Create an instance of #ezfc_alias_t.
 *
 * Returns: a #ezfc_alias_t.
 */
ezfc_alias_t *
ezfc_alias_new(const gchar *alias_name)
{
	ezfc_alias_t *retval = ezfc_mem_alloc_object(sizeof (ezfc_alias_t));

	if (retval) {
		ezfc_font_init((ezfc_font_t *)&retval->parent);

		if (g_ascii_strcasecmp(alias_name, "sans") == 0)
			retval->alias_name = g_strdup("sans-serif");
		else
			retval->alias_name = g_strdup(alias_name);
		ezfc_mem_add_ref((ezfc_mem_t *)retval, retval->alias_name, g_free);
	}

	return retval;
}

/**
 * ezfc_alias_ref:
 * @alias: a #ezfc_alias_t.
 *
 * Increases the reference count of @alias.
 *
 * Returns: (transfer none): the same @alias.
 */
ezfc_alias_t *
ezfc_alias_ref(ezfc_alias_t *alias)
{
	g_return_val_if_fail (alias != NULL, NULL);

	return (ezfc_alias_t *)ezfc_font_ref((ezfc_font_t *)&alias->parent);
}

/**
 * ezfc_alias_unref:
 * @alias: a #ezfc_alias_t.
 *
 * Decreases the reference count of @alias. When its reference count
 * drops to 0, the object is finalized (i.e. its memory is freed).
 */
void
ezfc_alias_unref(ezfc_alias_t *alias)
{
	if (alias)
		ezfc_font_unref((ezfc_font_t *)&alias->parent);
}

/**
 * ezfc_alias_get_name:
 * @alias: a #ezfc_alias_t.
 *
 * Obtains the alias font name in @alias object.
 *
 * Returns: the alias font name.
 */
const gchar *
ezfc_alias_get_name(ezfc_alias_t *alias)
{
	g_return_val_if_fail (alias != NULL, NULL);

	return alias->alias_name;
}

/**
 * ezfc_alias_get_font_pattern:
 * @alias: a #ezfc_alias_t.
 *
 * Obtains #FcPattern in #ezfc_alias_t.
 *
 * Returns: a duplicate of #FcPattern in the instance. it has to be freed.
 *          %NULL if @alias doesn't have any font pattern.
 */
FcPattern *
ezfc_alias_get_font_pattern(ezfc_alias_t *alias)
{
	g_return_val_if_fail (alias != NULL, NULL);

	return ezfc_font_get_pattern((ezfc_font_t *)&alias->parent);
}

/**
 * ezfc_alias_set_font_pattern:
 * @alias: a #ezfc_alias_t.
 * @pattern: a #FcPattern.
 * @error: (allow-none): a #GError.
 *
 * Set @pattern as the font pattern. @alias keeps a duplicate instance of
 * @pattern.
 *
 * Returns: %TRUE if it successfully is set. otherwise %FALSE.
 */
gboolean
ezfc_alias_set_font_pattern(ezfc_alias_t     *alias,
			    const FcPattern  *pattern,
			    GError          **error)
{
	g_return_val_if_fail (alias != NULL, FALSE);

	return ezfc_font_set_pattern((ezfc_font_t *)&alias->parent, pattern, error);
}

/**
 * ezfc_alias_get_font:
 * @alias: a #ezfc_alias_t.
 *
 * Obtains the font that is set as the alias font for @alias.
 *
 * Returns: the font name.
 */
const gchar *
ezfc_alias_get_font(ezfc_alias_t *alias)
{
	g_return_val_if_fail (alias != NULL, NULL);

	return ezfc_font_get_family((ezfc_font_t *)&alias->parent);
}

/**
 * ezfc_alias_set_font:
 * @alias: a #ezfc_alias_t.
 * @font_name: a font name.
 * @error: (allow-none): a #GError.
 *
 * Set @font_name as the font family name used for the alias font.
 *
 * Returns: %TRUE if it successfully is set. otherwise %FALSE.
 */
gboolean
ezfc_alias_set_font(ezfc_alias_t  *alias,
		    const gchar   *font_name,
		    GError       **error)
{
	GError *err = NULL;
	gboolean retval;

	g_return_val_if_fail (alias != NULL, FALSE);

	if (!ezfc_font_remove((ezfc_font_t *)&alias->parent, &err)) {
		if (err->code == EZFC_ERR_NO_FAMILY) {
			g_clear_error(&err);
		} else {
			return FALSE;
		}
	}

	retval = ezfc_font_add_family((ezfc_font_t *)&alias->parent, font_name, &err);

	if (err) {
		if (error)
			*error = g_error_copy(err);
		else
			g_warning(err->message);
		g_error_free(err);
	}

	return retval;
}

/**
 * ezfc_alias_check_font_existence:
 * @alias: a #ezfc_alias_t.
 * @flag: a boolean value.
 *
 * Set a flag whether checking the font existence when invoking
 * ezfc_alias_set_font().
 */
void
ezfc_alias_check_font_existence(ezfc_alias_t *alias,
				gboolean      flag)
{
	g_return_if_fail (alias != NULL);

	ezfc_font_check_existence((ezfc_font_t *)&alias->parent, flag);
}
