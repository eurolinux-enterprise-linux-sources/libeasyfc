/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc.c
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

#include <ft2build.h>
#include FT_FREETYPE_H
#include "ezfc.h"

FT_Library _ezfc_get_freetype(void);


static gboolean __ezfc_is_initialized = FALSE;
static FT_Library __ezfc_ftlib = NULL;

/*< private >*/

/*< public >*/

/**
 * ezfc_error_get_quark:
 *
 * Obtains a #GQuark being used for #GError.
 *
 * Returns: a #GQuark.
 */
GQuark
ezfc_error_get_quark(void)
{
	static GQuark quark = 0;

	if (quark == 0)
		quark = g_quark_from_static_string("ezfc-error-quark");

	return quark;
}

/**
 * ezfc_version:
 *
 * Obtain the version of libeasyfc.
 *
 * Returns: a version string.
 */
const gchar *
ezfc_version(void)
{
	return VERSION;
}

/**
 * ezfc_init:
 *
 * Initialize the library.
 */
void
ezfc_init(void)
{
	if (!__ezfc_is_initialized) {
		__ezfc_is_initialized = TRUE;
		FT_Init_FreeType(&__ezfc_ftlib);
		FcInit();
	}
}

/**
 * ezfc_finalize:
 *
 * Finalize the library.
 */
void
ezfc_finalize(void)
{
	if (__ezfc_is_initialized) {
		__ezfc_is_initialized = FALSE;
		FcFini();
		FT_Done_FreeType(__ezfc_ftlib);
		__ezfc_ftlib = NULL;
	}
}

FT_Library
_ezfc_get_freetype(void)
{
	g_return_val_if_fail (__ezfc_is_initialized, NULL);

	return __ezfc_ftlib;
}

/**
 * ezfc_is_alias_font:
 * @alias_name: the alias font name
 *
 * Checks if @alias_name is one of sans-serif, serif, monospace, cursive or fantasy.
 *
 * Returns: %TRUE if @alias_name is an alias font name, otherwise %FALSE.
 *
 * Deprecated: 0.7: Use ezfc_font_is_alias_font().
 */
gboolean
ezfc_is_alias_font(const gchar *alias_name)
{
	return ezfc_font_is_alias_font(alias_name);
}

/**
 * ezfc_get_fonts_list:
 * @language: (allow-none): the language name fontconfig can deal with.
 * @alias_name: (allow-none): the alias name to obtain the fonts list for.
 *
 * Obtains the fonts list being assigned to @alias_name for @language.
 *
 * Returns: (element-type utf8) (transfer full): a #GList contains the font family name.
 *          if no valid families, %NULL then.
 *
 * Deprecated: 0.7: Use ezfc_font_get_list().
 */
GList *
ezfc_get_fonts_list(const gchar *language,
		    const gchar *alias_name)
{
	return ezfc_font_get_list(language, alias_name, TRUE);
}

/**
 * ezfc_get_fonts_pattern_list:
 * @language: (allow-none): the language name fontconfig can deal with.
 * @alias_name: (allow-none): the alias name to obtain the fonts pettern list for.
 *
 * Obtains #FcPattern list being assigned to @alias_name for @language.
 *
 * Returns: (element-type FcPattern) (transfer full): a #GList contains #FcPattern, otherwise %NULL.
 *
 * Deprecated: 0.7: Use ezfc_font_get_pattern_list().
 */
GList *
ezfc_get_fonts_pattern_list(const gchar *language,
			    const gchar *alias_name)
{
	return ezfc_font_get_pattern_list(language, alias_name);
}
