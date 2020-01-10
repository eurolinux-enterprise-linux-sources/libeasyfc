/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-font.c
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include <hb-ft.h>
#include <hb-ot.h>
#include <libeasyfc/ezfc-error.h>
#include "ezfc-mem.h"
#include <libeasyfc/ezfc-font.h>
#include "ezfc-font-private.h"


/**
 * SECTION: ezfc-font
 * @Short_Description: A class for managing the font properties
 * @Title: ezfc_font_t
 *
 * This class provides an easy access to the font properties
 *
 * Since: 0.7
 */
typedef enum {
	EZFC_FONT_UNKNOWN   = 0,
	EZFC_FONT_SANS      = 1 << 0,
	EZFC_FONT_SERIF     = 1 << 1,
	EZFC_FONT_MONOSPACE = 1 << 2,
	EZFC_FONT_CURSIVE   = 1 << 3,
	EZFC_FONT_FANTASY   = 1 << 4,
	EZFC_FONT_END
} ezfc_font_variant_t;
typedef GList * (ezfc_proc_t) (GList *, const FcPattern *);

extern FT_Library _ezfc_get_freetype(void);

static gchar *__ezfc_font_alias_names[] = {
	"sans",
	"sans-serif",
	"serif",
	"monospace",
	"cursive",
	"fantasy",
	NULL
};

/*< private >*/
static FT_Face
_ezfc_font_get_face(const FcPattern *pattern)
{
	FcChar8 *file;
	int idx;
	FT_Face face = NULL;

	FcPatternGetString(pattern, FC_FILE, 0, &file);
	FcPatternGetInteger(pattern, FC_INDEX, 0, &idx);

	if (FT_New_Face(_ezfc_get_freetype(), (char *)file, idx, &face)) {
		g_warning("Unable to open the font file '%s' index %d",
			  file, idx);
	}

	return face;
}

static ezfc_font_variant_t
_ezfc_font_get_font_type(const gchar *font)
{
	gint i;

	for (i = 0; __ezfc_font_alias_names[i] != NULL; i++) {
		if (g_ascii_strcasecmp(font, __ezfc_font_alias_names[i]) == 0) {
			if (i == 0)
				i++;
			return 1 << (i - 1);
		}
	}

	return EZFC_FONT_UNKNOWN;
}

static ezfc_font_variant_t
_ezfc_font_get_font_type_from_pattern(const FcPattern *pattern)
{
	int spacing = 0;
	ezfc_font_variant_t retval = EZFC_FONT_UNKNOWN;
	TT_OS2 *os2;
	FT_Face face = NULL;
	FT_Byte cls_id, subcls_id;

	FcPatternGetInteger(pattern, FC_SPACING, 0, &spacing);
	if (spacing == FC_MONO ||
	    spacing == FC_DUAL ||
	    spacing == FC_CHARCELL) {
		retval = EZFC_FONT_MONOSPACE;
	}

	face = _ezfc_font_get_face(pattern);
	os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
	if (!os2)
		goto bail;
	/* See http://www.microsoft.com/typography/otspec/os2.htm#fc
	 * and http://www.microsoft.com/typography/otspec/ibmfc.htm
	 * for the reference of sFamilyClass.
	 */
	cls_id = (os2->sFamilyClass >> 8) & 0xff;
	subcls_id = os2->sFamilyClass & 0xff;
	switch (cls_id) {
	    case 0: /* No Classification */
	    case 6: /* reserved for future use */
	    case 11: /* reserved for future use */
	    case 13: /* Reserved */
	    case 14: /* Reserved */
		    break;
	    case 1: /* Oldstyle Serifs */
		    retval |= EZFC_FONT_SERIF;
		    if (subcls_id == 8) /* Calligraphic */
			    retval |= EZFC_FONT_CURSIVE;
		    break;
	    case 2: /* Transitional Serifs */
	    case 3: /* Modern Serifs */
		    retval |= EZFC_FONT_SERIF;
		    if (subcls_id == 2) /* Script */
			    retval |= EZFC_FONT_CURSIVE;
		    break;
	    case 4: /* Clarendon Serifs */
	    case 5: /* Slab Serifs */
	    case 7: /* Freeform Serifs */
		    retval |= EZFC_FONT_SERIF;
		    break;
	    case 8: /* Sans Serif */
		    retval |= EZFC_FONT_SANS;
		    break;
	    case 9: /* Ornamentals */
	    case 12: /* Symbolic */
		    retval |= EZFC_FONT_FANTASY;
		    break;
	    case 10: /* Scripts */
		    retval |= EZFC_FONT_CURSIVE;
		    break;
	    default:
		    g_warning("Unknown sFamilyClass class ID: %d", cls_id);
		    break;
	}
	/* See http://www.monotypeimaging.com/ProductsServices/pan1.aspx
	 * for the reference of Panose
	 */
	if (os2->panose[0] == 3) {
		/* Latin Hand Written */
		retval |= EZFC_FONT_CURSIVE;
	} else if (os2->panose[0] >= 4 &&
		   os2->panose[0] <= 5) {
		/* 4...Latin Decorative
		 * 5...Latin Symbol
		 */
		retval |= EZFC_FONT_FANTASY;
	} else if (os2->panose[0] == 2 &&
		   ((os2->panose[1] >= 11 &&
		     os2->panose[1] <= 13) ||
		    os2->panose[1] == 15)) {
		/* 2..Latin Text */
		/*   11...Normal Sans
		 *   12...Obtuse Sans
		 *   13...Perpendicular Sans
		 *   15...Pounded
		 */
		retval |= EZFC_FONT_SANS;
	} else if (os2->panose[0] == 1) {
		/* "Not Fit" in the family kind should be ignored. */
	} else if (os2->panose[0] == 0 &&
		   os2->panose[1] == 1) {
		/* Unable to determine the alias type */
	} else {
		retval |= EZFC_FONT_SERIF;
	}
	if ((os2->panose[0] == 2 &&
	     os2->panose[3] == 9) ||
	    (os2->panose[0] == 3 &&
	     os2->panose[3] == 3) ||
	    (os2->panose[0] == 4 &&
	     os2->panose[3] == 9) ||
	    (os2->panose[0] == 5 &&
	     os2->panose[3] == 3)) {
		retval |= EZFC_FONT_MONOSPACE;
	}
  bail:
	FT_Done_Face(face);

	return retval;
}

static gboolean
_ezfc_font_is_font_a(const FcPattern *pattern,
		     const gchar     *alias_name)
{
	ezfc_font_variant_t type = _ezfc_font_get_font_type(alias_name);
	ezfc_font_variant_t ptype = _ezfc_font_get_font_type_from_pattern(pattern);

	return (type & ptype) == type;
}

static GList *
_ezfc_font_get_fonts_proc(const gchar *language,
			  const gchar *alias_name,
			  gboolean     localized_font_name,
			  ezfc_proc_t  func)
{
	FcPattern *pat;
	FcFontSet *fs;
	FcObjectSet *os;
	GList *retval = NULL;
	FcLangSet *ls = NULL;

	g_return_val_if_fail (func != NULL, NULL);

	if (alias_name) {
		g_return_val_if_fail (ezfc_font_is_alias_font(alias_name), NULL);
	}
	pat = FcPatternCreate();
	if (language && language[0] != 0) {
		ls = FcLangSetCreate();
		FcLangSetAdd(ls, (const FcChar8 *)language);
		FcPatternAddLangSet(pat, FC_LANG, ls);
	}
	if (!localized_font_name)
		FcPatternAddString(pat, FC_NAMELANG, (const FcChar8 *)"en");
	os = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_INDEX, FC_SPACING, NULL);
	fs = FcFontList(NULL, pat, os);
	FcObjectSetDestroy(os);
	FcPatternDestroy(pat);
	if (ls)
		FcLangSetDestroy(ls);
	if (fs) {
		gint i;

		for (i = 0; i < fs->nfont; i++) {
			if (!alias_name || _ezfc_font_is_font_a(fs->fonts[i], alias_name)) {
				retval = func(retval, fs->fonts[i]);
			}
		}
		FcFontSetDestroy(fs);
	}

	return retval;
}

static GList *
_ezfc_font_get_fonts_cb_family(GList           *list,
			       const FcPattern *pattern)
{
	FcChar8 *family;
	GList *l;
	gboolean added = FALSE;

	FcPatternGetString(pattern, FC_FAMILY, 0, &family);
	for (l = list; l != NULL; l = g_list_next(l)) {
		gint r = g_strcmp0(l->data, (const gchar *)family);
		GList *ll;

		if (r == 0) {
			/* do not add it */
			added = TRUE;
			break;
		} else if (r > 0) {
			ll = g_list_prepend(l, g_strdup((const gchar *)family));

			if (list == l)
				list = ll;
			added = TRUE;
			break;
		}
	}
	if (!added)
		list = g_list_append(list, g_strdup((const gchar *)family));

	return list;
}

static GList *
_ezfc_font_get_fonts_cb_pattern(GList           *list,
				const FcPattern *pattern)
{
	return g_list_append(list, FcPatternDuplicate(pattern));
}

static GList *
_ezfc_font_get_available_features(GList     *retval,
				  hb_face_t *hbface,
				  hb_tag_t   table_tag)
{
	unsigned int count;
	hb_tag_t *p;
	int i;

	count = hb_ot_layout_table_get_feature_tags(hbface, table_tag, 0, NULL, NULL);
	p = g_new0(hb_tag_t, count + 1);
	hb_ot_layout_table_get_feature_tags(hbface, table_tag, 0, &count, p);
	p[count] = 0;
	for (i = 0; i < count; i++) {
		gchar *tag = g_malloc(sizeof (char) * 5);

		tag[0] = (p[i] >> 24) & 0xff;
		tag[1] = (p[i] >> 16) & 0xff;
		tag[2] = (p[i] >>  8) & 0xff;
		tag[3] = (p[i]      ) & 0xff;
		tag[4] = 0;
		if (!g_list_find_custom(retval, tag, (GCompareFunc)strcmp))
			retval = g_list_append(retval, tag);
		else
			g_free(tag);
	}

	return retval;
}

static gchar *
_ezfc_font_escape_fontname(const gchar *font_name)
{
	GString *retval = g_string_new(NULL);
	size_t i, n = strlen(font_name);

	/* fontconfig uses -,: as a separator to operate FcNameParse().
	 * if the font name contains any of them, it may causes
	 * unexpected result. so those has to be escaped.
	 */
	for (i = 0; i < n; i++) {
		if (font_name[i] == '-' ||
		    font_name[i] == ',' ||
		    font_name[i] == ':')
			g_string_append_c(retval, '\\');
		g_string_append_c(retval, font_name[i]);
	}

	return g_string_free(retval, FALSE);
}

/*< protected >*/
void
ezfc_font_init(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_if_fail (font != NULL);

	priv->check_font_existence = TRUE;
}

/*< public >*/

/**
 * ezfc_font_is_alias_font:
 * @alias_name: the alias font name
 *
 * Checks if @alias_name is one of sans-serif, serif, monospace, cursive or fantasy.
 *
 * Returns: %TRUE if @alias_name is an alias font name, otherwise %FALSE.
 */
gboolean
ezfc_font_is_alias_font(const gchar *alias_name)
{
	ezfc_font_variant_t t = _ezfc_font_get_font_type(alias_name);

	return t != EZFC_FONT_UNKNOWN;
}

/**
 * ezfc_font_get_list:
 * @language: (allow-none): the language name fontconfig can deal with.
 * @alias_name: (allow-none): the alias name to obtain the fonts list for.
 * @localized_font_name: %TRUE to include the localized font name if available,
 *                       %FALSE for English font name only.
 *
 * Obtains the fonts list being assigned to @alias_name for @language.
 *
 * Note that @localized_font_name doesn't take effect yet. this is just
 * a reservation for future improvement.
 *
 * Returns: (element-type utf8) (transfer full): a #GList contains the font family name.
 *          if no valid families, %NULL then.
 */
GList *
ezfc_font_get_list(const gchar *language,
		   const gchar *alias_name,
		   gboolean     localized_font_name)
{
	return _ezfc_font_get_fonts_proc(language, alias_name,
					 localized_font_name,
					 _ezfc_font_get_fonts_cb_family);
}

/**
 * ezfc_font_get_pattern_list:
 * @language: (allow-none): the language name fontconfig can deal with.
 * @alias_name: (allow-none): the alias name to obtain the fonts pettern list for.
 *
 * Obtains #FcPattern list being assigned to @alias_name for @language.
 *
 * Returns: (element-type FcPattern) (transfer full): a #GList contains #FcPattern, otherwise %NULL.
 */
GList *
ezfc_font_get_pattern_list(const gchar *language,
			   const gchar *alias_name)
{
	return _ezfc_font_get_fonts_proc(language, alias_name, TRUE,
					 _ezfc_font_get_fonts_cb_pattern);
}

/**
 * ezfc_font_get_alias_name_from_pattern:
 * @pattern: a #FcPattern.
 *
 * Analize @pattern and returns a alias name string according to the result.
 *
 * Returns: (transfer container) (element-type utf8): a #GList containing
 *          a static string for the alias name.
 */
GList *
ezfc_font_get_alias_name_from_pattern(const FcPattern *pattern)
{
	GList *retval = NULL;
	ezfc_font_variant_t type = _ezfc_font_get_font_type_from_pattern(pattern);
	gint i, j;

	if (type == EZFC_FONT_UNKNOWN)
		return NULL;

	for (i = EZFC_FONT_SANS, j = 1; i < EZFC_FONT_END; i <<= 1, j++) {
		if ((type & i) == i)
			retval = g_list_append(retval, __ezfc_font_alias_names[j]);
	}

	return retval;
}

/**
 * ezfc_font_new:
 *
 * Create an instance of #ezfc_font_t.
 *
 * Returns: a #ezfc_font_t.
 */
ezfc_font_t *
ezfc_font_new(void)
{
	ezfc_font_private_t *retval = ezfc_mem_alloc_object(sizeof (ezfc_font_private_t));

	if (retval) {
		ezfc_font_init((ezfc_font_t *)retval);
	}

	return (ezfc_font_t *)retval;
}

/**
 * ezfc_font_ref:
 * @font: a #ezfc_font_t.
 *
 * Increases the reference count of @font.
 *
 * Returns: (transfer none): the same @font object.
 */
ezfc_font_t *
ezfc_font_ref(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	return ezfc_mem_ref(&priv->parent);
}

/**
 * ezfc_font_unref:
 * @font: a #ezfc_font_t.
 *
 * Decreases the reference count of @font. when its reference count
 * drops to 0, the object is finalized (i.e. its memory is freed).
 */
void
ezfc_font_unref(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	if (priv)
		ezfc_mem_unref(&priv->parent);
}

/**
 * ezfc_font_get_pattern:
 * @font: a #ezfc_font_t.
 *
 * Obtains #FcPattern in #ezfc_font_t.
 *
 * Returns: a duplicate of #FcPattern in the instance. it has to be freed.
 *          %NULL if @font doesn't have any font pattern.
 */
FcPattern *
ezfc_font_get_pattern(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, NULL);

	if (priv->pattern)
		return FcPatternDuplicate(priv->pattern);

	return NULL;
}

/**
 * ezfc_font_set_pattern:
 * @font: a #ezfc_font_t.
 * @pattern: a #FcPattern.
 * @error: (allow-none): a #GError.
 *
 * Set @pattern as the font pattern. @font keeps a duplicate instance of
 * @pattern.
 *
 * Returns: %TRUE if it successfully is set. otherwise %FALSE.
 */
gboolean
ezfc_font_set_pattern(ezfc_font_t      *font,
		      const FcPattern  *pattern,
		      GError          **error)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	FcPattern *pat;
	gboolean retval = TRUE;
	GError *err = NULL;

	g_return_val_if_fail (font != NULL, FALSE);
	g_return_val_if_fail (pattern != NULL, FALSE);

	pat = FcPatternDuplicate(pattern);
	if (!pat) {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_OOM,
			    "Unable to make a duplicate of FcPattern");
		retval = FALSE;
		goto bail;
	}
	if (priv->pattern)
		ezfc_mem_remove_ref(&priv->parent, priv->pattern);
	priv->pattern = pat;
	ezfc_mem_add_ref(&priv->parent, pat, (ezfc_destroy_func_t)FcPatternDestroy);

  bail:
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
 * ezfc_font_get_family:
 * @font: a #ezfc_font_t.
 *
 * Obtains the font family name in first place in @font.
 *
 * Returns: the font name.
 */
const gchar *
ezfc_font_get_family(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	gchar *family = NULL;

	g_return_val_if_fail (font != NULL, NULL);

	if (priv->pattern &&
	    FcPatternGetString(priv->pattern, FC_FAMILY, 0, (FcChar8 **)&family) != FcResultMatch)
		return NULL;

	return family;
}

/**
 * ezfc_font_get_families:
 * @font: a #ezfc_font_t.
 *
 * Obtains font family names in @font.
 *
 * Returns: (transfer container) (element-type utf8): a #GList containing
 *          the static string of font family names
 *
 * Since: 0.11
 */
GList *
ezfc_font_get_families(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	GList *retval = NULL;
	FcChar8 *s;

	g_return_val_if_fail (font != NULL, NULL);

	if (priv->pattern) {
		int i;

		for (i = 0; ; i++) {
			if (FcPatternGetString(priv->pattern,
					       FC_FAMILY, i, &s) != FcResultMatch)
				break;
			retval = g_list_append(retval, s);
		}
	}

	return retval;
}

/**
 * ezfc_font_find:
 * @font: a #ezfc_font_t.
 * @font_name: a font name.
 *
 * Check if @font contains @font_name.
 *
 * Returns: %TRUE if it contains, otherwise %FALSE.
 *
 * Since: 0.11
 */
gboolean
ezfc_font_find(ezfc_font_t *font,
	       const gchar *font_name)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	gboolean retval = FALSE;
	FcChar8 *s;
	int i;

	g_return_val_if_fail (font != NULL, FALSE);
	g_return_val_if_fail (font_name != NULL, FALSE);

	if (priv->pattern) {
		for (i = 0; ; i++) {
			if (FcPatternGetString(priv->pattern, FC_FAMILY, i, &s) != FcResultMatch)
				break;
			if (g_ascii_strcasecmp(font_name, (const gchar *)s) == 0) {
				retval = TRUE;
				break;
			}
		}
	}

	return retval;
}

/**
 * ezfc_font_set_family:
 * @font: a #ezfc_font_t.
 * @font_name: a font name.
 * @error: (allow-none): a #GError.
 *
 * Set @font_name as the font family name used for the font font.
 *
 * Returns: %TRUE if it successfully is set. otherwise %FALSE.
 *
 * Deprecated: 0.11. Use ezfc_font_add_family().
 */
gboolean
ezfc_font_set_family(ezfc_font_t   *font,
		     const gchar   *font_name,
		     GError       **error)
{
	return ezfc_font_add_family(font, font_name, error);
}

/**
 * ezfc_font_add_family:
 * @font: a #ezfc_font_t.
 * @font_name: a font name.
 * @error: (allow-none): a #GError.
 *
 * Add @font_name as the font family name used for the font font.
 *
 * Returns: %TRUE if it successfully is set. otherwise %FALSE.
 *
 * Since: 0.11
 */
gboolean
ezfc_font_add_family(ezfc_font_t   *font,
		     const gchar   *font_name,
		     GError       **error)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	FcPattern *pat, *match = NULL;
	FcResult result;
	gboolean retval = TRUE;
	GError *err = NULL;
	gchar *escaped_font_name;

	g_return_val_if_fail (font != NULL, FALSE);
	g_return_val_if_fail (font_name != NULL, FALSE);

	escaped_font_name = _ezfc_font_escape_fontname(font_name);
	pat = FcNameParse((FcChar8 *)escaped_font_name);
	if (!pat) {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_FC,
			    "Unable to parse `%s'",
			    font_name);
		retval = FALSE;
		goto bail;
	}
	if (!priv->check_font_existence) {
		retval = ezfc_font_set_pattern(font, pat, &err);
		goto bail;
	}
	if ((match = FcFontMatch(NULL, pat, &result))) {
		FcChar8 *v1, *v2;
		int i;

		if (FcPatternGetString(pat, FC_FAMILY, 0, &v1) != FcResultMatch) {
			g_set_error(&err, EZFC_ERROR, EZFC_ERR_NO_FAMILY,
				    "Pattern doesn't contain any family name");
			retval = FALSE;
			goto bail1;
		}
		/* may need to check all of values assigned to the specific object
		 * because it might contains multiple values. e.g. family name in
		 * different language.
		 */
		for (i = 0; ; i++) {
			if (FcPatternGetString(match, FC_FAMILY, i, &v2) != FcResultMatch) {
				if (i == 0)
					g_set_error(&err, EZFC_ERROR, EZFC_ERR_NO_FAMILY,
						    "Pattern doesn't contain any family name");
				else
					g_set_error(&err, EZFC_ERROR, EZFC_ERR_NO_VALID_FONT,
						    "No such fonts available: %s",
						    font_name);
				retval = FALSE;
				goto bail1;
			}
			if (FcStrCmpIgnoreCase(v1, v2) == 0) {
				retval = ezfc_font_set_pattern(font, pat, &err);
				break;
			}
		}
	  bail1:
		FcPatternDestroy(match);
	} else {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_NO_VALID_FONT,
			    "No such fonts available: %s",
			    font_name);
		retval = FALSE;
		goto bail;
	}
  bail:
	if (pat)
		FcPatternDestroy(pat);
	g_free(escaped_font_name);

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
 * ezfc_font_remove:
 * @font: a #ezfc_font_t.
 * @error: (allow-none): a #GError.
 *
 * Removes all of families in @font.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 *
 * Since: 0.11
 */
gboolean
ezfc_font_remove(ezfc_font_t  *font,
		 GError      **error)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	gboolean retval = FALSE;
	GError *err = NULL;

	g_return_val_if_fail (font != NULL, FALSE);

	if (priv->pattern) {
		if (FcPatternDel(priv->pattern, FC_FAMILY)) {
			retval = TRUE;
		} else {
			g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_FC,
				    "Pattern doesn't contain any family name");
		}
	} else {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_NO_FAMILY,
			    "No patterns available");
	}
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
 * ezfc_font_remove_family:
 * @font: a #ezfc_font_t.
 * @font_name: a font name to be removed.
 * @error: (allow-none): a #GError.
 *
 * Removes @font_name from @font.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 *
 * Since: 0.11
 */
gboolean
ezfc_font_remove_family(ezfc_font_t  *font,
			const gchar  *font_name,
			GError      **error)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	gboolean retval = FALSE;
	GError *err = NULL;
	int i;
	FcChar8 *s;

	g_return_val_if_fail (font != NULL, FALSE);
	g_return_val_if_fail (font_name != NULL, FALSE);

	if (priv->pattern) {
		for (i = 0; ; i++) {
			if (FcPatternGetString(priv->pattern, FC_FAMILY, i, &s) != FcResultMatch)
				break;
			if (g_ascii_strcasecmp(font_name, (const gchar *)s) == 0) {
				if (FcPatternRemove(priv->pattern, FC_FAMILY, i)) {
					retval = TRUE;
				} else {
					g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_FC,
						    "Unable to remove a family: %s",
						    font_name);
				}
				break;
			}
		}
	}

	return retval;
}

/**
 * ezfc_font_check_existence:
 * @font: a #ezfc_font_t.
 * @flag: a boolean value.
 *
 * Set a flag whether checking the font existence when invoking
 * ezfc_font_set_family().
 */
void
ezfc_font_check_existence(ezfc_font_t *font,
			  gboolean     flag)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_if_fail (font != NULL);

	priv->check_font_existence = (flag == TRUE);
}

/**
 * ezfc_font_set_hinting:
 * @font: a #ezfc_font_t.
 * @flag: a boolean value.
 *
 * Set a flag whether the font use the own hints for rendering
 */
void
ezfc_font_set_hinting(ezfc_font_t *font,
		      gboolean     flag)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_if_fail (font != NULL);

	priv->masks |= EZFC_FONT_MASK_HINTING;
	priv->hinting = (flag == TRUE);
}

/**
 * ezfc_font_get_hinting:
 * @font: a #ezfc_font_t.
 *
 * Obtain a boolean value about the hinting usage in @font.
 *
 * Returns: %TRUE if the hinting is enabled. otherwise %FALSE.
 */
gboolean
ezfc_font_get_hinting(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, FALSE);

	return priv->hinting;
}

/**
 * ezfc_font_set_autohinting:
 * @font: a #ezfc_font_t.
 * @flag: a boolean value.
 *
 * Set a flag whether the font use the auto-hinting for rendering
 */
void
ezfc_font_set_autohinting(ezfc_font_t *font,
			  gboolean     flag)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_if_fail (font != NULL);

	priv->masks |= EZFC_FONT_MASK_AUTOHINT;
	priv->autohinting = (flag == TRUE);
}

/**
 * ezfc_font_get_autohinting:
 * @font: a #ezfc_font_t.
 *
 * Obtain a boolean value about the auto-hinting usage in @font.
 *
 * Returns: %TRUE if the auto-hinting is enabled. otherwise %FALSE.
 */
gboolean
ezfc_font_get_autohinting(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, FALSE);

	return priv->autohinting;
}

/**
 * ezfc_font_set_hintstyle:
 * @font: a #ezfc_font_t.
 * @hintstyle: a #ezfc_font_hintstyle_t.
 *
 * Set a hintstyle for @font.
 */
void
ezfc_font_set_hintstyle(ezfc_font_t           *font,
			ezfc_font_hintstyle_t  hintstyle)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_if_fail (font != NULL);
	g_return_if_fail (hintstyle > EZFC_FONT_HINTSTYLE_UNKNOWN && hintstyle < EZFC_FONT_HINTSTYLE_END);

	priv->masks |= EZFC_FONT_MASK_HINTSTYLE;
	priv->hintstyle = hintstyle;
}

/**
 * ezfc_font_get_hintstyle:
 * @font: a #ezfc_font_t.
 *
 * Obtain the hintstyle in @font.
 *
 * Returns: a #ezfc_font_hintstyle_t.
 */
ezfc_font_hintstyle_t
ezfc_font_get_hintstyle(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, EZFC_FONT_HINTSTYLE_UNKNOWN);

	return priv->hintstyle;
}

/**
 * ezfc_font_set_antialiasing:
 * @font: a #ezfc_font_t.
 * @flag: a boolean value.
 *
 * Set a flag whether the font use the antialiasing.
 */
void
ezfc_font_set_antialiasing(ezfc_font_t *font,
			   gboolean     flag)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_if_fail (font != NULL);

	priv->masks |= EZFC_FONT_MASK_ANTIALIAS;
	priv->antialiasing = (flag == TRUE);
}

/**
 * ezfc_font_get_antialiasing:
 * @font: a #ezfc_font_t.
 *
 * Obtain a boolean value about the anti-aliasing usage in @font.
 *
 * Returns: %TRUE if the antialiasing is enabled. otherwise %FALSE.
 */
gboolean
ezfc_font_get_antialiasing(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, FALSE);

	return priv->antialiasing;
}

/**
 * ezfc_font_set_embedded_bitmap:
 * @font: a #ezfc_font_t.
 * @flag: a boolean value.
 *
 * Set a flag whether the font use the embedded bitmap.
 * Note that Enabling the embedded bitmap may causes disabling the antialias.
 */
void
ezfc_font_set_embedded_bitmap(ezfc_font_t *font,
			      gboolean     flag)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_if_fail (font != NULL);

	priv->masks |= EZFC_FONT_MASK_EMBEDDED_BITMAP;
	priv->embedded_bitmap = (flag == TRUE);
}

/**
 * ezfc_font_get_embedded_bitmap:
 * @font: a #ezfc_font_t.
 *
 * Obtain a boolean value about the embedded bitmap usage in @font.
 *
 * Returns: %TRUE if the embedded bitmap is enabled, otherwise %FALSE.
 */
gboolean
ezfc_font_get_embedded_bitmap(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, FALSE);

	return priv->embedded_bitmap;
}

/**
 * ezfc_font_set_rgba:
 * @font: a #ezfc_font_t.
 * @val: an integer value corresponding to FC_RGBA_*.
 *
 * Set @val as the sub-pixel ordering
 */
void
ezfc_font_set_rgba(ezfc_font_t *font,
		   gint         val)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_if_fail (font != NULL);

	priv->masks |= EZFC_FONT_MASK_RGBA;
	priv->rgba = val;
}

/**
 * ezfc_font_get_rgba:
 * @font: a #ezfc_font_t.
 *
 * Obtains current sub-pixel ordering in @font.
 *
 * Returns: the sub-pixel ordering value in the integer.
 */
gint
ezfc_font_get_rgba(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, FC_RGBA_UNKNOWN);

	return priv->rgba;
}

/**
 * ezfc_font_get_boolean_masks:
 * @font: a #ezfc_font_t.
 *
 * Obtain masks that any boolean values are set to @font.
 * This is set when ezfc_font_set_hinting(), ezfc_font_set_autohinting()
 * and ezfc_font_set_antialiasing() is called.
 *
 * Returns: an integer value
 */
gint
ezfc_font_get_boolean_masks(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, 0);

	return priv->masks;
}

/**
 * ezfc_font_set_subpixel_rendering:
 * @font: a #ezfc_font_t.
 * @mode: a #ezfc_font_subpixel_render_t.
 *
 * This is just convenient to change the several configuration for subpixel
 * rendering.
 *
 * Returns: %TRUE if the sub-pixel rendering is enabled. otherwise %FALSE.
 */
gboolean
ezfc_font_set_subpixel_rendering(ezfc_font_t                 *font,
				 ezfc_font_subpixel_render_t  mode)
{
	g_return_val_if_fail (font != NULL, FALSE);
	g_return_val_if_fail (mode < EZFC_FONT_ANTIALIAS_END, FALSE);

	switch (mode) {
	    case EZFC_FONT_ANTIALIAS_NONE:
		    ezfc_font_set_antialiasing(font, FALSE);
		    ezfc_font_set_rgba(font, FC_RGBA_NONE);
		    break;
	    case EZFC_FONT_ANTIALIAS_GRAY:
		    ezfc_font_set_antialiasing(font, TRUE);
		    ezfc_font_set_rgba(font, FC_RGBA_NONE);
		    break;
	    case EZFC_FONT_ANTIALIAS_RGB:
		    ezfc_font_set_antialiasing(font, TRUE);
		    ezfc_font_set_rgba(font, FC_RGBA_RGB);
		    break;
	    case EZFC_FONT_ANTIALIAS_BGR:
		    ezfc_font_set_antialiasing(font, TRUE);
		    ezfc_font_set_rgba(font, FC_RGBA_BGR);
		    break;
	    case EZFC_FONT_ANTIALIAS_VRGB:
		    ezfc_font_set_antialiasing(font, TRUE);
		    ezfc_font_set_rgba(font, FC_RGBA_VRGB);
		    break;
	    case EZFC_FONT_ANTIALIAS_VBGR:
		    ezfc_font_set_antialiasing(font, TRUE);
		    ezfc_font_set_rgba(font, FC_RGBA_VBGR);
		    break;
	    default:
		    g_return_val_if_reached (FALSE);
	}

	return TRUE;
}

/**
 * ezfc_font_get_subpixel_rendering:
 * @font: a #ezfc_font_t.
 *
 * Obtain current status about the sub-pixel rendering in @font.
 *
 * Returns: current mode in the sub-pixel rendering.
 */
ezfc_font_subpixel_render_t
ezfc_font_get_subpixel_rendering(ezfc_font_t *font)
{
	gboolean antialias;
	gint rgba;
	ezfc_font_subpixel_render_t retval;

	g_return_val_if_fail (font != NULL, EZFC_FONT_ANTIALIAS_UNKNOWN);

	antialias = ezfc_font_get_antialiasing(font);
	rgba = ezfc_font_get_rgba(font);

	if (antialias) {
		switch (rgba) {
		    case FC_RGBA_NONE:
			    retval = EZFC_FONT_ANTIALIAS_GRAY;
			    break;
		    case FC_RGBA_RGB:
			    retval = EZFC_FONT_ANTIALIAS_RGB;
			    break;
		    case FC_RGBA_BGR:
			    retval = EZFC_FONT_ANTIALIAS_BGR;
			    break;
		    case FC_RGBA_VRGB:
			    retval = EZFC_FONT_ANTIALIAS_VRGB;
			    break;
		    case FC_RGBA_VBGR:
			    retval = EZFC_FONT_ANTIALIAS_VBGR;
			    break;
		    default:
			    retval = EZFC_FONT_ANTIALIAS_UNKNOWN;
			    break;
		}
	} else {
		/* no matter what rgba is, if antialias is false,
		 * it should be None.
		 */
		retval = EZFC_FONT_ANTIALIAS_NONE;
	}

	return retval;
}

/**
 * ezfc_font_add_feature:
 * @font: a #ezfc_font_t.
 * @feature: feature name to be added
 *
 * Add @feature font feature to @font.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 *
 * Since: 0.12
 */
gboolean
ezfc_font_add_feature(ezfc_font_t *font,
		      const gchar *feature)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	GList *l;

	g_return_val_if_fail (font != NULL, FALSE);
	g_return_val_if_fail (feature != NULL && feature[0] != 0, FALSE);

	if (!priv->pattern)
		priv->pattern = FcPatternCreate();

	l = ezfc_font_get_features(font);
	if (g_list_find_custom(l, feature, (GCompareFunc)strcmp)) {
		g_list_free(l);
		return TRUE;
	}
	g_list_free(l);

	return FcPatternAddString(priv->pattern, FC_FONT_FEATURES, (const FcChar8 *)feature);
}

/**
 * ezfc_font_remove_feature:
 * @font: a #ezfc_font_t.
 * @feature: feature name to be removed
 *
 * Remove @feature from @font if available.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 *
 * Since: 0.12
 */
gboolean
ezfc_font_remove_feature(ezfc_font_t *font,
			 const gchar *feature)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;

	g_return_val_if_fail (font != NULL, FALSE);
	g_return_val_if_fail (feature != NULL && feature[0] != 0, FALSE);

	if (priv->pattern) {
		int i;
		FcChar8 *s;

		for (i = 0; ; i++) {
			if (FcPatternGetString(priv->pattern,
					       FC_FONT_FEATURES, i,
					       &s) == FcResultMatch) {
				return FcPatternRemove(priv->pattern,
						       FC_FONT_FEATURES,
						       i);
			}
		}
	}

	return FALSE;
}

/**
 * ezfc_font_get_features:
 * @font: a #ezfc_font_t.
 *
 * Obtains font features list that @font has.
 *
 * Returns: (transfer container) (element-type utf8): a #GList containing
 *          the static string of feature name.
 *
 * Since: 0.12
 */
GList *
ezfc_font_get_features(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	GList *retval = NULL;

	g_return_val_if_fail (font != NULL, NULL);

	if (priv->pattern) {
		int i;
		FcChar8 *s;

		for (i = 0; ; i++) {
			if (FcPatternGetString(priv->pattern,
					       FC_FONT_FEATURES, i,
					       &s) != FcResultMatch)
				break;
			retval = g_list_append(retval, s);
		}
	}

	return retval;
}

/**
 * ezfc_font_get_available_features:
 * @font: a #ezfc_font_t.
 *
 * Obtains available font features in @font.
 *
 * Returns: (transfer full) (element-type utf8): a #GList containing
 *          memory-allocated string of feature name that is available
 *          in @font. strings in #GList has to be freed when it isn't
 *          needed anymore.
 *
 * Since: 0.12
 */
GList *
ezfc_font_get_available_features(ezfc_font_t *font)
{
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	GList *retval = NULL;

	g_return_val_if_fail (font != NULL, NULL);

	if (priv->pattern) {
		FcPattern *match = NULL;
		FcResult result;
		FT_Library ftlib;

		if (FT_Init_FreeType(&ftlib) != FT_Err_Ok)
			return NULL;
		if ((match = FcFontMatch(NULL, priv->pattern, &result))) {
			FcChar8 *file;
			FT_Face face;
			int idx;
			hb_face_t *hbface;

			if (FcPatternGetString(match, FC_FILE, 0, &file) != FcResultMatch) {
				goto bail;
			}
			if (FcPatternGetInteger(match, FC_INDEX, 0, &idx) != FcResultMatch)
				idx = 0;
			if (FT_New_Face(ftlib, (const char *)file, idx, &face) != FT_Err_Ok)
				goto bail;
			hbface = hb_ft_face_create_cached(face);
			retval = _ezfc_font_get_available_features(retval, hbface, HB_OT_TAG_GDEF);
			retval = _ezfc_font_get_available_features(retval, hbface, HB_OT_TAG_GSUB);
			retval = _ezfc_font_get_available_features(retval, hbface, HB_OT_TAG_GPOS);
			hb_face_destroy(hbface);
			FT_Done_Face(face);
		}
	  bail:
		FcPatternDestroy(match);
		FT_Done_FreeType(ftlib);
	}

	return retval;
}

/**
 * ezfc_font_canonicalize:
 * @font: a #ezfc_font_t.
 * @error: (allow-none): a #GError.
 *
 * Split up @font to #ezfc_font_t that has one family name only.
 *
 * Returns: (transfer full) (element-type ezfc_font_t): a #GList contains
 *          #ezfc_font_t, otherwise %NULL.
 *
 * Since: 0.11
 */
GList *
ezfc_font_canonicalize(ezfc_font_t  *font,
		       GError      **error)
{
	GList *retval = NULL;
	ezfc_font_private_t *priv = (ezfc_font_private_t *)font;
	GError *err = NULL;

	g_return_val_if_fail (font != NULL, NULL);

	if (priv->pattern) {
		int i;
		FcChar8 *s;
		ezfc_font_t *f;
		FcPattern *pat;

		for (i = 0; ; i++) {
			if (FcPatternGetString(priv->pattern, FC_FAMILY, i, &s) != FcResultMatch)
				break;
			f = ezfc_font_new();
			if (!f) {
				g_set_error(&err, EZFC_ERROR, EZFC_ERR_OOM,
					    "Unable to create a canonicalized font instance");
				goto bail;
			}
			memcpy(&((ezfc_font_private_t *)f)->masks,
			       &priv->masks, sizeof (ezfc_font_private_t) - G_STRUCT_OFFSET (ezfc_font_private_t, masks));
			pat = FcPatternDuplicate(priv->pattern);
			FcPatternDel(pat, FC_FAMILY);
			ezfc_font_set_pattern(f, pat, &err);
			FcPatternDestroy(pat);
			if (!ezfc_font_add_family(f, (const gchar *)s, &err)) {
				ezfc_font_unref(f);
				goto bail;
			}
			retval = g_list_append(retval, f);
		}
	}
  bail:
	if (err) {
		if (error)
			*error = g_error_copy(err);
		else
			g_warning(err->message);
		g_error_free(err);
	}

	return retval;
}
