/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-config.c
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

#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libeasyfc/ezfc-error.h>
#include <libeasyfc/ezfc-font.h>
#include "ezfc-font-private.h"
#include <libeasyfc/ezfc-utils.h>
#include "ezfc-mem.h"
#include <libeasyfc/ezfc-config.h>

/**
 * SECTION:ezfc-config
 * @Short_Description: A class for managing the configuration file.
 * @Title: ezfc_config_t
 *
 * This class provides an easy access to assign an instance of #ezfc_alias_t
 * to a language and to read/write the configuration file.
 */

struct _ezfc_config_t {
	ezfc_mem_t  parent;
	GHashTable *aliases;
	GHashTable *fonts;
	GHashTable *subst;
	gchar      *name;
	gint        priority;
	gboolean    loaded;
	gboolean    migration;
};
typedef struct _ezfc_container_t {
	ezfc_mem_t          parent;
	gpointer            data;
	ezfc_destroy_func_t data_finalizer;
} ezfc_container_t;
typedef struct _ezfc_xml_const_t {
	gchar *name;
	gint   val;
} ezfc_xml_const_t;
typedef enum {
	EZFC_NODE_TYPE_PREFER = 0,
	EZFC_NODE_TYPE_ACCEPT,
	EZFC_NODE_TYPE_DEFAULT,
	EZFC_NODE_TYPE_END
} ezfc_node_t;

/*< private >*/
static void
_ezfc_config_alias_list_free(gpointer p)
{
	GList *l = p, *ll;

	for (ll = l; ll != NULL; ll = g_list_next(ll)) {
		ezfc_alias_unref(ll->data);
	}
	g_list_free(l);
}

static void
_ezfc_config_subst_list_free(gpointer p)
{
	GList *l = p, *ll;

	for (ll = l; ll != NULL; ll = g_list_next(ll)) {
		ezfc_font_unref(ll->data);
	}
	g_list_free(l);
}

static ezfc_container_t *
ezfc_container_new(ezfc_destroy_func_t func)
{
	ezfc_container_t *retval = ezfc_mem_alloc_object(sizeof (ezfc_container_t));

	if (retval) {
		retval->data_finalizer = func;
	}

	return retval;
}

#if 0
static ezfc_container_t *
ezfc_container_ref(ezfc_container_t *container)
{
	g_return_val_if_fail (container != NULL, NULL);

	return ezfc_mem_ref(&container->parent);
}
#endif

static void
ezfc_container_unref(ezfc_container_t *container)
{
	if (container)
		ezfc_mem_unref(&container->parent);
}

static void
ezfc_container_set(ezfc_container_t *container,
		   gpointer          p)
{
	g_return_if_fail (container != NULL);

	if (container->data)
		ezfc_mem_delete_ref(&container->parent, container->data);
	container->data = p;
	ezfc_mem_add_ref(&container->parent, container->data,
			 container->data_finalizer);
}

static gpointer
ezfc_container_get(const ezfc_container_t *container)
{
	g_return_val_if_fail (container != NULL, NULL);

	return container->data;
}

static xmlNodePtr
_ezfc_config_to_xml_node(const gchar    *language,
			 const gchar    *test_font,
			 const gpointer  edit_object,
			 ezfc_node_t     type)
{
	xmlNodePtr match, test, edit;
	FcPattern *pat;
	FcChar8 *family;
	const gchar *mode[] = {
		"prepend",
		"append",
		"append_last",
		NULL
	};

	g_return_val_if_fail (type < EZFC_NODE_TYPE_END, NULL);

	match = xmlNewNode(NULL,
			   (const xmlChar *)"match");
	xmlNewProp(match,
		   (const xmlChar *)"target",
		   (const xmlChar *)"pattern");

	if (language && language[0] != 0) {
		test = xmlNewChild(match, NULL,
				   (const xmlChar *)"test",
				   NULL);
		xmlNewProp(test,
			   (const xmlChar *)"name",
			   (const xmlChar *)FC_LANG);
		/* XXX: need to confirm if no regressions on compare="contains" for
		 *      ll-cc and ll only matching.
		 */
		xmlNewProp(test,
			   (const xmlChar *)"compare",
			   (const xmlChar *)"contains");
		xmlNewChild(test, NULL,
			    (const xmlChar *)"string",
			    (const xmlChar *)language);
	}

	test = xmlNewChild(match, NULL,
			   (const xmlChar *)"test",
			   NULL);
	xmlNewProp(test,
		   (const xmlChar *)"name",
		   (const xmlChar *)FC_FAMILY);
	xmlNewChild(test, NULL,
		    (const xmlChar *)"string",
		    (const xmlChar *)test_font);

	if (type == EZFC_NODE_TYPE_DEFAULT) {
		family = edit_object;
	} else {
		pat = edit_object;
		/* XXX: there are no way to peek everything in FcPattern efficiently.
		   so only support family name so far.
		*/
		if (FcPatternGetString(pat, FC_FAMILY, 0, &family) != FcResultMatch)
			return NULL;
	}
	edit = xmlNewChild(match, NULL,
			   (const xmlChar *)"edit",
			   NULL);
	xmlNewProp(edit,
		   (const xmlChar *)"name",
		   (const xmlChar *)FC_FAMILY);
	xmlNewProp(edit,
		   (const xmlChar *)"mode",
		   (const xmlChar *)mode[type]);
	xmlNewChild(edit, NULL,
		    (const xmlChar *)"string",
		    (const xmlChar *)family);

	return match;
}

static gboolean
_ezfc_config_real_to_xml(xmlNodePtr              root,
			 const gchar            *slang,
			 const ezfc_container_t *container,
			 gboolean               *no_elements)
{
	const GList *list, *l;
	xmlNodePtr node;

	list = ezfc_container_get(container);
	for (l = list; l != NULL; l = g_list_next(l)) {
		ezfc_alias_t *a = l->data;
		const gchar *n;
		gboolean is_subst;
		FcPattern *pat;
		FcChar8 *family;

		n = ezfc_alias_get_name(a);
		pat = ezfc_alias_get_font_pattern(a);
		is_subst = !ezfc_font_is_alias_font(n);
		if (is_subst) {
			node = _ezfc_config_to_xml_node(slang,
							n, pat,
							EZFC_NODE_TYPE_ACCEPT);
			xmlAddChild(root, node);
		} else {
			if (FcPatternGetString(pat, FC_FAMILY, 0, &family) != FcResultMatch)
				return FALSE;
			node = _ezfc_config_to_xml_node(slang,
							(const gchar *)family,
							(const gpointer)n,
							EZFC_NODE_TYPE_DEFAULT);
			xmlAddChild(root, node);
			node = _ezfc_config_to_xml_node(slang,
							n, pat,
							EZFC_NODE_TYPE_PREFER);
			xmlAddChild(root, node);
		}
		*no_elements = FALSE;
		FcPatternDestroy(pat);
	}

	return TRUE;
}

static gboolean
_ezfc_config_real_to_font_xml(xmlNodePtr   root,
			      ezfc_font_t *font,
			      gboolean    *no_elements)
{
	xmlNodePtr match, test, edit;
	const gchar *family = ezfc_font_get_family(font);
	gint masks, n = 0;
	GList *feat = ezfc_font_get_features(font);

	masks = ezfc_font_get_boolean_masks(font);
	if (masks == 0 && !feat)
		return TRUE;
	match = xmlNewNode(NULL,
			   (const xmlChar *)"match");
	xmlNewProp(match,
		   (const xmlChar *)"target",
		   (const xmlChar *)"font");
	test = xmlNewChild(match, NULL,
			   (const xmlChar *)"test",
			   NULL);
	xmlNewProp(test,
		   (const xmlChar *)"name",
		   (const xmlChar *)FC_FAMILY);
	xmlNewChild(test, NULL,
		    (const xmlChar *)"string",
		    (const xmlChar *)family);
	while (masks != 0) {
		gint i = (masks & 1) << n;
		gboolean f;
		static const gchar *hintstyles[] = {
			NULL,
			"hintnone",
			"hintslight",
			"hintmedium",
			"hintfull",
			NULL
		};
		static const gchar *rgba[] = {
			"unknown",
			"rgb",
			"bgr",
			"vrgb",
			"vbgr",
			"none",
			NULL
		};

		if (i == 0 || i >= EZFC_FONT_MASK_END)
			goto bail;
		edit = xmlNewChild(match, NULL,
				   (const xmlChar *)"edit",
				   NULL);
		switch (i) {
		    case EZFC_FONT_MASK_HINTING:
			    xmlNewProp(edit,
				       (const xmlChar *)"name",
				       (const xmlChar *)FC_HINTING);
			    f = ezfc_font_get_hinting(font);
			    break;
		    case EZFC_FONT_MASK_AUTOHINT:
			    xmlNewProp(edit,
				       (const xmlChar *)"name",
				       (const xmlChar *)FC_AUTOHINT);
			    f = ezfc_font_get_autohinting(font);
			    break;
		    case EZFC_FONT_MASK_ANTIALIAS:
			    xmlNewProp(edit,
				       (const xmlChar *)"name",
				       (const xmlChar *)FC_ANTIALIAS);
			    f = ezfc_font_get_antialiasing(font);
			    break;
		    case EZFC_FONT_MASK_EMBEDDED_BITMAP:
			    xmlNewProp(edit,
				       (const xmlChar *)"name",
				       (const xmlChar *)FC_EMBEDDED_BITMAP);
			    f = ezfc_font_get_embedded_bitmap(font);
			    break;
		    case EZFC_FONT_MASK_RGBA:
			    xmlNewProp(edit,
				       (const xmlChar *)"name",
				       (const xmlChar *)FC_RGBA);
			    xmlNewProp(edit,
				       (const xmlChar *)"mode",
				       (const xmlChar *)"assign");
			    xmlNewChild(edit, NULL,
					(const xmlChar *)"const",
					(const xmlChar *)rgba[ezfc_font_get_rgba(font)]);
			    goto bail;
		    case EZFC_FONT_MASK_HINTSTYLE:
			    xmlNewProp(edit,
				       (const xmlChar *)"name",
				       (const xmlChar *)FC_HINT_STYLE);
			    xmlNewProp(edit,
				       (const xmlChar *)"mode",
				       (const xmlChar *)"assign");
			    xmlNewChild(edit, NULL,
					(const xmlChar *)"const",
					(const xmlChar *)hintstyles[ezfc_font_get_hintstyle(font)]);
			    goto bail;
		    default:
			    g_return_val_if_reached(FALSE);
		}
		xmlNewProp(edit,
			   (const xmlChar *)"mode",
			   (const xmlChar *)"assign");
		xmlNewChild(edit, NULL,
			    (const xmlChar *)"bool",
			    (const xmlChar *)(f ? "true" : "false"));
	  bail:
		masks >>= 1;
		n++;
	}
	if (feat) {
		GList *l;

		edit = xmlNewChild(match, NULL,
				   (const xmlChar *)"edit",
				   NULL);
		xmlNewProp(edit,
			   (const xmlChar *)"name",
			   (const xmlChar *)FC_FONT_FEATURES);
		xmlNewProp(edit,
			   (const xmlChar *)"mode",
			   (const xmlChar *)"append");
		for (l = feat; l != NULL; l = g_list_next(l)) {
			const xmlChar *n = l->data;

			xmlNewChild(edit, NULL,
				    (const xmlChar *)"string",
				    n);
		}
		g_list_free(feat);
	}
	xmlAddChild(root, match);
	*no_elements = FALSE;

	return TRUE;
}

static gboolean
_ezfc_config_real_to_subst_xml(xmlNodePtr              root,
			       const gchar            *family_name,
			       const ezfc_container_t *container,
			       gboolean               *no_elements)
{
	const GList *list, *l;
	xmlNodePtr match, test, edit;

	match = xmlNewNode(NULL,
			   (const xmlChar *)"match");
	xmlNewProp(match,
		   (const xmlChar *)"target",
		   (const xmlChar *)"pattern");
	test = xmlNewChild(match, NULL,
			   (const xmlChar *)"test",
			   NULL);
	xmlNewProp(test,
		   (const xmlChar *)"name",
		   (const xmlChar *)FC_FAMILY);
	xmlNewChild(test, NULL,
		    (const xmlChar *)"string",
		    (const xmlChar *)family_name);
	edit = xmlNewChild(match, NULL,
			   (const xmlChar *)"edit",
			   NULL);
	xmlNewProp(edit,
		   (const xmlChar *)"name",
		   (const xmlChar *)FC_FAMILY);
	xmlNewProp(edit,
		   (const xmlChar *)"mode",
		   (const xmlChar *)"append_last");
	xmlNewProp(edit,
		   (const xmlChar *)"binding",
		   (const xmlChar *)"same");
	list = ezfc_container_get(container);
	for (l = list; l != NULL; l = g_list_next(l)) {
		ezfc_font_t *f = l->data;
		FcPattern *pat;
		FcChar8 *n;
		int i;

		pat = ezfc_font_get_pattern(f);
		for (i = 0; ; i++) {
			if (FcPatternGetString(pat, FC_FAMILY, i, &n) != FcResultMatch)
				break;
			xmlNewChild(edit, NULL,
				    (const xmlChar *)"string",
				    (const xmlChar *)n);
		}
		FcPatternDestroy(pat);
	}
	xmlAddChild(root, match);
	*no_elements = FALSE;

	return TRUE;
}

static gboolean
_ezfc_config_to_xml(ezfc_config_t  *config,
		    xmlChar       **output,
		    int            *size)
{
	xmlDocPtr doc;
	xmlNodePtr root;
	gchar *comment;
	GHashTableIter iter;
	gboolean no_elements = TRUE;
	gpointer key, val;

	doc = xmlNewDoc((const xmlChar *)"1.0");
	doc->encoding = xmlStrdup((const xmlChar *)"UTF-8");
	xmlCreateIntSubset(doc,
			   (const xmlChar *)"fontconfig",
			   NULL,
			   (const xmlChar *)"../fonts.dtd");
	root = xmlNewDocNode(doc,
			     NULL,
			     (const xmlChar *)"fontconfig",
			     NULL);
	xmlDocSetRootElement(doc, root);
	comment = g_strdup_printf("\n"
				  "\tTHIS FILE WAS GENERATED BY libeasyfc %s\n"
				  "\tDO NOT EDIT THIS DIRECTLY\n"
				  "\tANY CHANGES HAS BEEN MADE MANUALLY MAY BE LOST.\n",
				  VERSION);
	xmlAddChild(root, xmlNewComment((const xmlChar *)comment));
	g_free(comment);

	g_hash_table_iter_init(&iter, config->aliases);
	while (g_hash_table_iter_next(&iter, &key, &val)) {
		const gchar *slang = key;
		const ezfc_container_t *container = val;

		if (!slang || slang[0] == 0) {
			/* "any" language should be added at the end. */
			continue;
		}
		if (!_ezfc_config_real_to_xml(root, slang, container, &no_elements))
			return FALSE;
	}
	val = g_hash_table_lookup(config->aliases, "");
	if (val)
		if (!_ezfc_config_real_to_xml(root, NULL, val, &no_elements))
			return FALSE;
	g_hash_table_iter_init(&iter, config->fonts);
	while (g_hash_table_iter_next(&iter, &key, &val)) {
		ezfc_font_t *font = val;

		if (!_ezfc_config_real_to_font_xml(root, font, &no_elements))
			return FALSE;
	}
	g_hash_table_iter_init(&iter, config->subst);
	while (g_hash_table_iter_next(&iter, &key, &val)) {
		const gchar *n = key;
		const ezfc_container_t *container = val;

		if (!_ezfc_config_real_to_subst_xml(root, n, container, &no_elements))
			return FALSE;
	}

	xmlDocDumpFormatMemory(doc, output, size, 1);

	xmlFreeDoc(doc);

	return !no_elements;
}

static GList *
_ezfc_config_parse_string_node(xmlNodePtr node)
{
	xmlChar *s = NULL;
	GList *retval = NULL;

	while (node != NULL) {
		if (node->type == XML_COMMENT_NODE || node->type == XML_TEXT_NODE);
		else if (node->type == XML_ELEMENT_NODE &&
		    xmlStrcmp(node->name, (const xmlChar *)"string") == 0) {
			s = xmlNodeGetContent(node);
			retval = g_list_append(retval, s);
		} else {
			g_warning("Unexpected node: type = %d, name = %s",
				  node->type, node->name);
			break;
		}
		node = node->next;
	}

	return retval;
}

static gboolean
_ezfc_config_parse_bool_node(xmlNodePtr node)
{
	gboolean retval = FALSE;
	xmlChar *s;
	gchar *ss;

	while (node != NULL) {
		if (node->type == XML_COMMENT_NODE || node->type == XML_TEXT_NODE)
			node = node->next;
		if (node->type == XML_ELEMENT_NODE &&
		    xmlStrcmp(node->name, (const xmlChar *)"bool") == 0) {
			s = xmlNodeGetContent(node);
			ss = g_ascii_strdown((const gchar *)s, -1);
			xmlFree(s);
			if (ss[0] == 't' || ss[0] == 'y' || ss[0] == '1')
				retval = TRUE;
			break;
		} else {
			g_warning("Unexpected node: type = %d, name = %s",
				  node->type, node->name);
			break;
		}
	}

	return retval;
}

static gboolean
_ezfc_config_parse_const_node(xmlNodePtr        node,
			      ezfc_xml_const_t *list,
			      gint             *ret)
{
	gboolean retval = FALSE;
	xmlChar *s;
	ezfc_xml_const_t *c;

	while (node != NULL) {
		if (node->type == XML_COMMENT_NODE || node->type == XML_TEXT_NODE)
			node = node->next;
		if (node->type == XML_ELEMENT_NODE &&
		    xmlStrcmp(node->name, (const xmlChar *)"const") == 0) {
			s = xmlNodeGetContent(node);
			for (c = list; c != NULL; c++) {
				if (g_ascii_strcasecmp(c->name, (const gchar *)s) == 0) {
					*ret = c->val;
					retval = TRUE;
					break;
				}
			}
			xmlFree(s);
			break;
		} else {
			g_warning("Unexpected node: type = %d, name = %s",
				  node->type, node->name);
			break;
		}
	}

	return retval;
}

static gboolean
_ezfc_config_from_xml(ezfc_config_t  *config,
		      const gchar    *conffile,
		      GError        **error)
{
	xmlDocPtr doc = NULL;
	xmlXPathContextPtr xctxt = NULL;
	xmlXPathObjectPtr xobj = NULL;
	gboolean retval = TRUE;
	GError *err = NULL;
	int i, n;

	xmlInitParser();

	doc = xmlParseFile(conffile);
	if (!doc) {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_XML,
			    "Unable to read the configuration.");
		retval = FALSE;
		goto bail;
	}
	xctxt = xmlXPathNewContext(doc);
	if (!xctxt) {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_XML,
			    "Unable to create a XPath context.");
		retval = FALSE;
		goto bail;
	}
	xobj = xmlXPathEvalExpression((const xmlChar *)"/fontconfig/match[(@target = \"pattern\" or not(@*)) and (test[@name = \"family\"] and test/string/text()) and (edit[@name = \"family\"] and edit/string/text())]", xctxt);
	if (!xobj) {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_XML,
			    "No valid elements can be proceeded in this library.");
		retval = FALSE;
		goto bail;
	}

	n = xmlXPathNodeSetGetLength(xobj->nodesetval);

	for (i = 0; i < n; i++) {
		xmlNodePtr match = xmlXPathNodeSetItem(xobj->nodesetval, i);
		xmlNodePtr node;
		xmlChar *attr, *lang = NULL, *alias = NULL;
		FcPattern *pat = FcPatternCreate();
		ezfc_alias_t *a;
		ezfc_font_t *f;
		gboolean is_subst = FALSE;
		GList *l, *ll;

		node = match->children;
		while (node != NULL) {
			if (node->type == XML_TEXT_NODE) {
				/* ignore the text node here */
				goto bail1;
			}
			if (xmlStrcmp(node->name, (const xmlChar *)"test") == 0) {
				attr = xmlGetProp(node, (const xmlChar *)"name");
				if (xmlStrcmp(attr, (const xmlChar *)FC_LANG) == 0) {
					if (lang)
						xmlFree(lang);
					l = _ezfc_config_parse_string_node(node->children);
					/* multiple node is not well supported */
					lang = xmlStrdup(l->data);
					g_list_free_full(l, xmlFree);
				} else if (xmlStrcmp(attr, (const xmlChar *)FC_FAMILY) == 0) {
					if (alias)
						xmlFree(alias);
					l = _ezfc_config_parse_string_node(node->children);
					/* multiple node is not well supported */
					alias = xmlStrdup(l->data);
					g_list_free_full(l, xmlFree);
				} else {
					g_warning("Unexpected value in the name attribute on <test>: %s\n",
						  attr);
				}
				xmlFree(attr);
			} else if (xmlStrcmp(node->name, (const xmlChar *)"edit") == 0) {
				attr = xmlGetProp(node, (const xmlChar *)"mode");
				if (xmlStrcmp(attr, (const xmlChar *)"append_last") == 0) {
					/* kind of the alternatives of <default> */
					/* We are assuming that there should be the <prefer> rule too.
					 * so ignoring except if binding is "same".
					 */
					xmlFree(attr);
					attr = xmlGetProp(node, (const xmlChar *)"binding");
					if (!attr || xmlStrcmp(attr, (const xmlChar *)"same"))
					{
						if (attr)
							xmlFree(attr);
						goto bail2;
					}
					/* deal with this as the substitute font definition */
					xmlFree(attr);
					attr = xmlGetProp(node, (const xmlChar *)"name");
					if (xmlStrcmp(attr, (const xmlChar *)FC_FAMILY) == 0) {
						l = _ezfc_config_parse_string_node(node->children);
						for (ll = l; ll != NULL; ll = g_list_next(ll))
							FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)ll->data);
						g_list_free_full(l, xmlFree);
					} else {
						g_warning("Unexpected value in the name attribute: %s",
							  attr);
					}
					is_subst = TRUE;
				} else {
					xmlFree(attr);
					attr = xmlGetProp(node, (const xmlChar *)"name");
					if (xmlStrcmp(attr, (const xmlChar *)FC_FAMILY) == 0) {
						l = _ezfc_config_parse_string_node(node->children);
						for (ll = l; ll != NULL; ll = g_list_next(ll))
							FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)ll->data);
						g_list_free_full(l, xmlFree);
					} else {
						g_warning("Unexpected value in the name attribute: %s",
							  attr);
					}
				}
				xmlFree(attr);
			} else {
				g_warning("Unexpected element in <match>: %s\n", node->name);
			}
		  bail1:
			node = node->next;
		}
		if (!lang)
			lang = xmlStrdup((const xmlChar *)"");
		if (!alias) {
			g_warning("No alias name is defined.");
			goto bail2;
		}
		if (is_subst) {
			f = ezfc_font_new();
			if (!ezfc_font_set_pattern(f, pat, &err)) {
				ezfc_font_unref(f);
				goto bail2;
			}
			l = ezfc_font_canonicalize(f, &err);
			if (!l) {
				ezfc_font_unref(f);
				goto bail2;
			}
			for (ll = l; ll != NULL; ll = g_list_next(ll)) {
				ezfc_font_t *ff = ll->data;

				ezfc_config_add_subst(config, (const gchar *)alias, ff);
			}
			ezfc_font_unref(f);
			g_list_free_full(l, (GDestroyNotify)ezfc_font_unref);
		} else {
			a = ezfc_alias_new((const gchar *)alias);
			if (!ezfc_alias_set_font_pattern(a, pat, &err)) {
				ezfc_alias_unref(a);
				goto bail2;
			}
			ezfc_config_add_alias(config, (const gchar *)lang, a);
			ezfc_alias_unref(a);
		}
	  bail2:
		if (lang)
			xmlFree(lang);
		if (alias)
			xmlFree(alias);
		if (pat)
			FcPatternDestroy(pat);
		if (err)
			break;
	}

	xobj = xmlXPathEvalExpression((const xmlChar *)"/fontconfig/match[@target = \"font\" and (test[@name = \"family\"] and test/string/text())]", xctxt);
	if (!xobj) {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_XML,
			    "No valid elements for targeting fonts can be proceeded in this library.");
		retval = FALSE;
		goto bail;
	}

	n = xmlXPathNodeSetGetLength(xobj->nodesetval);

	for (i = 0; i < n; i++) {
		xmlNodePtr match = xmlXPathNodeSetItem(xobj->nodesetval, i);
		xmlNodePtr node;
		xmlChar *attr;
		ezfc_font_t *f = NULL;
		GList *l, *ll;

		node = match->children;
		while (node != NULL) {
			if (node->type == XML_TEXT_NODE) {
				/* ignore the text node here */
				goto bail3;
			}
			if (xmlStrcmp(node->name, (const xmlChar *)"test") == 0) {
				attr = xmlGetProp(node, (const xmlChar *)"name");
				if (xmlStrcmp(attr, (const xmlChar *)FC_FAMILY) == 0) {
					f = ezfc_font_new();
					l = _ezfc_config_parse_string_node(node->children);
					/* multiple values is not well supported */
					ezfc_font_add_family(f, (const gchar *)l->data, &err);
					g_list_free_full(l, xmlFree);
				}
				xmlFree(attr);
			} else if (xmlStrcmp(node->name, (const xmlChar *)"edit") == 0) {
				attr = xmlGetProp(node, (const xmlChar *)"mode");
				if (xmlStrcmp(attr, (const xmlChar *)"append") == 0) {
					xmlFree(attr);
					attr = xmlGetProp(node, (const xmlChar *)"name");
					if (xmlStrcmp(attr, (const xmlChar *)FC_FONT_FEATURES) == 0) {
						l = _ezfc_config_parse_string_node(node->children);
						for (ll = l; ll != NULL; ll = g_list_next(ll)) {
							ezfc_font_add_feature(f, (const gchar *)ll->data);
						}
						g_list_free(l);
					} else {
						g_warning("Unexpected value in the name attribute: %s",
							  attr);
					}
					xmlFree(attr);
				} else if (xmlStrcmp(attr, (const xmlChar *)"assign") != 0) {
					g_warning("Unexpected mode for <match target=\"font\"><edit>: %s",
						  attr);
					xmlFree(attr);
					ezfc_font_unref(f);
					goto bail4;
				} else {
					xmlFree(attr);
					attr = xmlGetProp(node, (const xmlChar *)"name");
					if (xmlStrcmp(attr, (const xmlChar *)FC_ANTIALIAS) == 0) {
						ezfc_font_set_antialiasing(f, _ezfc_config_parse_bool_node(node->children));
					} else if (xmlStrcmp(attr, (const xmlChar *)FC_AUTOHINT) == 0) {
						ezfc_font_set_autohinting(f, _ezfc_config_parse_bool_node(node->children));
					} else if (xmlStrcmp(attr, (const xmlChar *)FC_EMBEDDED_BITMAP) == 0) {
						ezfc_font_set_embedded_bitmap(f, _ezfc_config_parse_bool_node(node->children));
					} else if (xmlStrcmp(attr, (const xmlChar *)FC_HINTING) == 0) {
						ezfc_font_set_hinting(f, _ezfc_config_parse_bool_node(node->children));
					} else if (xmlStrcmp(attr, (const xmlChar *)FC_HINT_STYLE) == 0) {
						ezfc_xml_const_t hintstyle[] = {
							{"hintnone", EZFC_FONT_HINTSTYLE_NONE},
							{"hintslight", EZFC_FONT_HINTSTYLE_SLIGHT},
							{"hintmedium", EZFC_FONT_HINTSTYLE_MEDIUM},
							{"hintfull", EZFC_FONT_HINTSTYLE_FULL},
							{NULL, 0}
						};
						gint ret;

						if (!_ezfc_config_parse_const_node(node->children, hintstyle, &ret)) {
							ret = EZFC_FONT_HINTSTYLE_UNKNOWN;
						}
						ezfc_font_set_hintstyle(f, ret);
					} else if (xmlStrcmp(attr, (const xmlChar *)FC_RGBA) == 0) {
						ezfc_xml_const_t rgba[] = {
							{"unknown", FC_RGBA_UNKNOWN},
							{"rgb", FC_RGBA_RGB},
							{"bgr", FC_RGBA_BGR},
							{"vrgb", FC_RGBA_VRGB},
							{"vbgr", FC_RGBA_VBGR},
							{"none", FC_RGBA_NONE},
							{NULL, 0}
						};
						gint ret;

						if (!_ezfc_config_parse_const_node(node->children, rgba, &ret)) {
							ret = FC_RGBA_UNKNOWN;
						}
						ezfc_font_set_rgba(f, ret);
					} else {
						g_warning("Unexpected value in the name attribute: %s",
							  attr);
					}
					xmlFree(attr);
				}
			} else {
				g_warning("Unexpected element in <match target=\"font\">: %s\n", node->name);
			}
		  bail3:
			node = node->next;
		}
		if (f)
			ezfc_config_add_font(config, f);
		ezfc_font_unref(f);
	  bail4:
		if (err)
			break;
	}
  bail:
	if (err) {
		if (error)
			*error = g_error_copy(err);
		else
			g_warning(err->message);
		g_error_free(err);
	}
	if (xobj)
		xmlXPathFreeObject(xobj);
	if (xctxt)
		xmlXPathFreeContext(xctxt);
	if (doc)
		xmlFreeDoc(doc);
	xmlCleanupParser();

	return retval;
}

static gchar *
_ezfc_config_get_conf_dir(ezfc_config_t *config)
{
	const gchar *xdgcfgdir = g_get_user_config_dir();
	gchar *fcconfdir = g_build_filename(xdgcfgdir, "fontconfig", "conf.d", NULL);

	return fcconfdir;
}

static gchar *
_ezfc_config_get_old_conf_dir(ezfc_config_t *config)
{
	const gchar *homedir = g_get_home_dir();
	gchar *fcconfdir = g_build_filename(homedir, ".fonts.conf.d", NULL);

	return fcconfdir;
}

static gchar *
_ezfc_config_get_conf_filename(ezfc_config_t *config)
{
	gchar *confname;

	if (config->name)
		confname = g_strdup_printf("%03d-%s-ezfc.conf", config->priority, config->name);
	else
		confname = g_strdup_printf("%03d-ezfc.conf", config->priority);

	return confname;
}

static gchar *
_ezfc_config_get_conf_name(ezfc_config_t *config)
{
	gchar *fcconfdir = _ezfc_config_get_conf_dir(config);
	gchar *confname = _ezfc_config_get_conf_filename(config);
	gchar *ezfcconf = g_build_filename(fcconfdir, confname, NULL);

	g_free(confname);
	g_free(fcconfdir);

	return ezfcconf;
}

static gchar *
_ezfc_config_get_old_conf_name(ezfc_config_t *config)
{
	gchar *fcconfdir = _ezfc_config_get_old_conf_dir(config);
	gchar *confname = _ezfc_config_get_conf_filename(config);
	gchar *ezfcconf = g_build_filename(fcconfdir, confname, NULL);

	g_free(confname);
	g_free(fcconfdir);

	return ezfcconf;
}

/*< public >*/

/**
 * ezfc_config_new:
 *
 * Create a new instance of a #ezfc_config_t.
 *
 * Returns: (transfer full): a new instance of #ezfc_config_t.
 */
ezfc_config_t *
ezfc_config_new(void)
{
	ezfc_config_t *retval = ezfc_mem_alloc_object(sizeof (ezfc_config_t));

	if (retval) {
		retval->aliases = g_hash_table_new_full(g_str_hash,
							g_str_equal,
							g_free,
							(GDestroyNotify)ezfc_container_unref);
		retval->fonts = g_hash_table_new_full(g_str_hash,
						      g_str_equal,
						      g_free,
						      (GDestroyNotify)ezfc_font_unref);
		retval->subst = g_hash_table_new_full(g_str_hash,
						      g_str_equal,
						      g_free,
						      (GDestroyNotify)ezfc_container_unref);
		retval->name = NULL;
		retval->priority = 0;
		retval->loaded = FALSE;
		retval->migration = TRUE;
		ezfc_mem_add_ref(&retval->parent, retval->aliases,
				 (ezfc_destroy_func_t)g_hash_table_destroy);
		ezfc_mem_add_ref(&retval->parent, retval->fonts,
				 (ezfc_destroy_func_t)g_hash_table_destroy);
		ezfc_mem_add_ref(&retval->parent, retval->subst,
				 (ezfc_destroy_func_t)g_hash_table_destroy);
	}

	return retval;
}

/**
 * ezfc_config_ref:
 * @config: a #ezfc_config_t.
 *
 * Increases the refernce count of @config.
 *
 * Returns: (transfer none): the same @config object.
 */
ezfc_config_t *
ezfc_config_ref(ezfc_config_t *config)
{
	g_return_val_if_fail (config != NULL, NULL);

	return ezfc_mem_ref(&config->parent);
}

/**
 * ezfc_config_unref:
 * @config: a #ezfc_config_t.
 *
 * Decreases the reference count of @config. when its reference count
 * drops to 0, the object is finalized (i.e. its memory is freed).
 */
void
ezfc_config_unref(ezfc_config_t *config)
{
	if (config)
		ezfc_mem_unref(&config->parent);
}

/**
 * ezfc_config_set_priority:
 * @config: a #ezfc_config_t.
 * @priority: a priority number that is used for a filename. it has to be
 *            within 3 digits. so the maximum value is 999.
 *
 * Set @priority to @config instance.
 */
void
ezfc_config_set_priority(ezfc_config_t *config,
			 guint          priority)
{
	g_return_if_fail (config != NULL);
	g_return_if_fail (priority < 1000);

	config->priority = priority;
}

/**
 * ezfc_config_get_priority:
 * @config: a #ezfc_config_t.
 *
 * Obtains the priority number in @config.
 *
 * Returns: the priority number. if any errors happens, returns -1.
 */
gint
ezfc_config_get_priority(ezfc_config_t *config)
{
	g_return_val_if_fail (config != NULL, -1);

	return config->priority;
}

/**
 * ezfc_config_set_name:
 * @config: a #ezfc_config_t.
 * @name: (allow-none): additional configuration name.
 *
 * Set @name as the additional configuration name. this is an optional to
 * make the change in the filename for output.
 */
void
ezfc_config_set_name(ezfc_config_t *config,
		     const gchar   *name)
{
	g_return_if_fail (config != NULL);

	if (config->name)
		ezfc_mem_remove_ref(&config->parent, config->name);
	config->name = g_strdup(name);
	if (config->name)
		ezfc_mem_add_ref(&config->parent, config->name,
				 (ezfc_destroy_func_t)g_free);
}

/**
 * ezfc_config_get_name:
 * @config: a #ezfc_config_t.
 *
 * Obtains the configuration name that is set by ezfc_config_set_name().
 *
 * Returns: the configuration name.
 */
const gchar *
ezfc_config_get_name(ezfc_config_t *config)
{
	g_return_val_if_fail (config != NULL, NULL);

	return config->name;
}

/**
 * ezfc_config_add_alias:
 * @config: a #ezfc_config_t.
 * @language: (allow-none): a language name to add @alias for or %NULL for global settings.
 * @alias: a #ezfc_alias_t.
 *
 * Add a @alias font for @language language. if giving %NULL to @language,
 * @alias takes effect for any languages.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 */
gboolean
ezfc_config_add_alias(ezfc_config_t *config,
		      const gchar   *language,
		      ezfc_alias_t  *alias)
{
	gchar *lang;
	ezfc_container_t *container;
	GList *list, *l;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (alias != NULL, FALSE);
	g_return_val_if_fail (ezfc_alias_get_font(alias) != NULL, FALSE);

	if (language)
		lang = g_strdup(language);
	else
		lang = g_strdup("");
	container = g_hash_table_lookup(config->aliases, lang);
	if (!container) {
		container = ezfc_container_new((ezfc_destroy_func_t)_ezfc_config_alias_list_free);
		g_hash_table_insert(config->aliases,
				    g_strdup(lang),
				    container);
	}
	l = list = ezfc_container_get(container);
	while (l != NULL) {
		ezfc_alias_t *a = l->data;

		if (g_ascii_strcasecmp(ezfc_alias_get_name(a),
				       ezfc_alias_get_name(alias)) == 0) {
			l->data = ezfc_alias_ref(alias);
			ezfc_alias_unref(a);
			goto bail;
		}
		if (g_list_next(l) == NULL)
			break;
		l = g_list_next(l);
	}
	l = g_list_append(l, ezfc_alias_ref(alias));
  bail:
	if (!list)
		list = l;
	ezfc_container_set(container, list);
	g_free(lang);

	return TRUE;
}

/**
 * ezfc_config_remove_alias:
 * @config: a #ezfc_config_t.
 * @language: (allow-none): a language name to remove @alias_name from.
 * @alias_name: a alias font name to remove.
 *
 * Removes @alias_name assigned for @language language from @config.
 *
 * Returns: %TRUE if it's successfully removed, otherwise %FALSE.
 */
gboolean
ezfc_config_remove_alias(ezfc_config_t *config,
			 const gchar   *language,
			 const gchar   *alias_name)
{
	ezfc_container_t *container;
	GList *list, *l;
	gchar *alias, *lang;
	gboolean retval = TRUE;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (alias_name != NULL, FALSE);

	if (language)
		lang = g_strdup(language);
	else
		lang = g_strdup("");
	if ((container = g_hash_table_lookup(config->aliases, lang)) == NULL) {
		g_free(lang);

		return FALSE;
	}

	if (g_ascii_strcasecmp(alias_name, "sans") == 0)
		alias = g_strdup("sans-serif");
	else
		alias = g_strdup(alias_name);

	list = ezfc_container_get(container);

	for (l = list; l != NULL; l = g_list_next(l)) {
		ezfc_alias_t *a = l->data;

		if (g_ascii_strcasecmp(ezfc_alias_get_name(a),
				       alias) == 0) {
			if (g_list_length(list) == 1) {
				g_hash_table_remove(config->aliases, lang);
			} else {
				if (l == list) {
					list = g_list_delete_link(l, l);
					ezfc_container_set(container, list);
				} else {
					l = g_list_delete_link(l, l);
				}
				ezfc_alias_unref(a);
			}
			goto bail;
		}
	}
	retval = FALSE;
  bail:
	g_free(alias);
	g_free(lang);

	return retval;
}

/**
 * ezfc_config_remove_aliases:
 * @config: a #ezfc_config_t.
 * @language: (allow-none): a language name to remove @alias_name from.
 *
 * Removes all of aliases assigned for @language language from @config.
 *
 * Returns: %TRUE if it's successfully removed, otherwise %FALSE.
 */
gboolean
ezfc_config_remove_aliases(ezfc_config_t *config,
			   const gchar   *language)
{
	gboolean retval;
	gchar *lang;

	g_return_val_if_fail (config != NULL, FALSE);

	if (language)
		lang = g_strdup(language);
	else
		lang = g_strdup("");

	retval = g_hash_table_remove(config->aliases, lang);
	g_free(lang);

	return retval;
}

/**
 * ezfc_config_add_font:
 * @config: a #ezfc_config_t.
 * @font: a #ezfc_font_t.
 *
 * Add a #font font to generate non-language-specific, non-alias-specific
 * rules.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 */
gboolean
ezfc_config_add_font(ezfc_config_t *config,
		     ezfc_font_t   *font)
{
	const gchar *family;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (font != NULL, FALSE);

	family = ezfc_font_get_family(font);
	g_hash_table_replace(config->fonts, g_strdup(family), ezfc_font_ref(font));

	return TRUE;
}

/**
 * ezfc_config_remove_font:
 * @config: a #ezfc_config_t.
 * @family: a family name to be removed.
 *
 * Remove a #ezfc_font_t instance corresponding to @family.
 *
 * Returns: %TRUE if it's successfully removed, otherwise %FALSE.
 */
gboolean
ezfc_config_remove_font(ezfc_config_t *config,
			const gchar   *family)
{
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (family != NULL, FALSE);

	return g_hash_table_remove(config->fonts, family);
}

/**
 * ezfc_config_remove_fonts:
 * @config: a #ezfc_config_t.
 *
 * Remove all of fonts from @config, which added by ezfc_config_add_font().
 *
 * Returns: %TRUE if it's successfully removed, otherwise %FALSE.
 */
gboolean
ezfc_config_remove_fonts(ezfc_config_t *config)
{
	g_return_val_if_fail (config != NULL, FALSE);

	g_hash_table_remove_all(config->fonts);

	return TRUE;
}

/**
 * ezfc_config_add_subst:
 * @config: a #ezfc_config_t.
 * @family_name: a family name to be substituted.
 * @subst: a #ezfc_font_t for substitute font
 *
 * Add a #subst font as a substitute font for @family_name.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 *
 * Since: 0.11
 */
gboolean
ezfc_config_add_subst(ezfc_config_t *config,
		      const gchar   *family_name,
		      ezfc_font_t   *subst)
{
	ezfc_container_t *container;
	GList *list, *l;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (family_name != NULL && family_name[0], FALSE);
	g_return_val_if_fail (subst != NULL, FALSE);

	container = g_hash_table_lookup(config->subst, family_name);
	if (!container) {
		container = ezfc_container_new((ezfc_destroy_func_t)_ezfc_config_subst_list_free);
		g_hash_table_insert(config->subst,
				    g_strdup(family_name),
				    container);
	}
	l = list = ezfc_container_get(container);
	while (l != NULL) {
		ezfc_font_t *f = l->data;

		if (g_ascii_strcasecmp(ezfc_font_get_family(f),
				       ezfc_font_get_family(subst)) == 0) {
			l->data = ezfc_font_ref(subst);
			ezfc_font_unref(f);
			goto bail;
		}
		if (g_list_next(l) == NULL)
			break;
		l = g_list_next(l);
	}
	l = g_list_append(l, ezfc_font_ref(subst));
  bail:
	if (!list)
		list = l;
	ezfc_container_set(container, list);

	return TRUE;
}

/**
 * ezfc_config_remove_subst:
 * @config: a #ezfc_config_t.
 * @family_name: a family name to be substituted.
 * @subst_name: a substitute font name
 *
 * Remove @subst_name from the substitute font list of @family_name.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 *
 * Since: 0.11
 */
gboolean
ezfc_config_remove_subst(ezfc_config_t *config,
			 const gchar   *family_name,
			 const gchar   *subst_name)
{
	ezfc_container_t *container;
	GList *list, *l;
	gboolean retval = TRUE;
	GError *err = NULL;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (family_name != NULL && family_name[0], FALSE);
	g_return_val_if_fail (subst_name != NULL && subst_name[0], FALSE);

	if ((container = g_hash_table_lookup(config->subst, family_name)) == NULL)
		return FALSE;

	list = ezfc_container_get(container);
	for (l = list; l != NULL; l = g_list_next(l)) {
		ezfc_font_t *f = l->data;

		if (ezfc_font_remove_family(f, subst_name, &err)) {
			GList *ll = ezfc_font_get_families(f);

			if (!ll) {
				if (g_list_length(list) == 1) {
					g_hash_table_remove(config->subst, family_name);
				} else {
					if (l == list) {
						list = g_list_delete_link(l, l);
						ezfc_container_set(container, list);
					} else {
						l = g_list_delete_link(l, l);
					}
					ezfc_font_unref(f);
				}
			} else {
				g_list_free(ll);
			}
			goto bail;
		}
	}
	retval = FALSE;
  bail:

	return retval;
}

/**
 * ezfc_config_remove_substs:
 * @config: a #ezfc_config_t.
 * @family_name: a family name to be substituted.
 *
 * Remove all of substitute font list of @family_name.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 *
 * Since: 0.11
 */
gboolean
ezfc_config_remove_substs(ezfc_config_t *config,
			  const gchar   *family_name)
{
	gboolean retval;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (family_name != NULL && family_name[0], FALSE);

	retval = g_hash_table_remove(config->subst, family_name);

	return retval;
}

/**
 * ezfc_config_get_language_list:
 * @config: a #ezfc_config_t.
 *
 * Obtains the list of languages registered by ezfc_config_add_alias() in @config.
 *
 * Returns: (element-type utf8) (transfer none): a #GList contains languages or %NULL.
 */
GList *
ezfc_config_get_language_list(ezfc_config_t *config)
{
	g_return_val_if_fail (config != NULL, NULL);

	return g_hash_table_get_keys(config->aliases);
}

/**
 * ezfc_config_get_aliases:
 * @config: a #ezfc_config_t.
 * @language: (allow-none): a language name referenced to the alias
 *
 * Obtains the list of #ezfc_alias_t in #ezfc_config_t instance.
 *
 * Returns: (element-type ezfc_alias_t) (transfer none): a #GList contains #ezfc_alias_t or %NULL.
 */
const GList *
ezfc_config_get_aliases(ezfc_config_t *config,
			const gchar   *language)
{
	ezfc_container_t *container;
	const GList *retval;
	gchar *lang;

	g_return_val_if_fail (config != NULL, NULL);

	if (language)
		lang = g_strdup(language);
	else
		lang = g_strdup("");
	container = g_hash_table_lookup(config->aliases, lang);
	if (!container)
		retval = NULL;
	else
		retval = ezfc_container_get(container);
	g_free(lang);

	return retval;
}

/**
 * ezfc_config_get_fonts:
 * @config: a #ezfc_config_t.
 *
 * Obtains the list of #ezfc_font_t in @config.
 *
 * Returns: (element-type ezfc_font_t) (transfer container): a #GList contains #ezfc_font_t or %NULL.
 */
GList *
ezfc_config_get_fonts(ezfc_config_t *config)
{
	g_return_val_if_fail (config != NULL, NULL);

	return g_hash_table_get_values(config->fonts);
}

/**
 * ezfc_config_get_subst_family:
 * @config: a #ezfc_config_t.
 *
 * Obtains the list of the family name being substituted
 *
 * Returns: (element-type utf8) (transfer none): a #GList contains languages or %NULL.
 *
 * Since: 0.11
 */
GList *
ezfc_config_get_subst_family(ezfc_config_t *config)
{
	g_return_val_if_fail (config != NULL, NULL);

	return g_hash_table_get_keys(config->subst);
}

/**
 * ezfc_config_get_substs:
 * @config: a #ezfc_config_t.
 * @family_name: a family name being substituted.
 *
 * Obtains the list of #ezfc_font_t to be substituted for @family_name.
 *
 * Returns: (element-type ezfc_font_t) (transfer none): a #GList contains #ezfc_font_t or %NULL.
 *
 * Since: 0.11
 */
const GList *
ezfc_config_get_substs(ezfc_config_t *config,
		       const gchar   *family_name)
{
	ezfc_container_t *container;
	const GList *retval;

	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (family_name != NULL && family_name[0], NULL);

	container = g_hash_table_lookup(config->subst, family_name);
	if (!container)
		retval = NULL;
	else
		retval = ezfc_container_get(container);

	return retval;
}

/**
 * ezfc_config_load:
 * @config: a #ezfc_config_t.
 * @error: (allow-none): a #GError.
 *
 * Read the configuration file and rebuild the object.
 * You have to invoke ezfc_config_set_priority() and ezfc_config_set_name()
 * first to read the appropriate configuration file.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 */
gboolean
ezfc_config_load(ezfc_config_t  *config,
		 GError        **error)
{
	gchar *ezfcconf;
	gboolean retval = TRUE;
	GError *err = NULL;

	g_return_val_if_fail (config != NULL, FALSE);

	if (config->migration) {
		ezfcconf = _ezfc_config_get_old_conf_name(config);
		if (!g_file_test(ezfcconf, G_FILE_TEST_EXISTS))
			goto try_new_one;
	} else {
	  try_new_one:
		ezfcconf = _ezfc_config_get_conf_name(config);
		if (!g_file_test(ezfcconf, G_FILE_TEST_IS_REGULAR)) {
			g_set_error(&err, EZFC_ERROR, EZFC_ERR_NO_CONFIG_FILE,
				    "No such config file available: %s",
				    ezfcconf);
			retval = FALSE;
			goto bail;
		}
	}
	if (!(retval = _ezfc_config_from_xml(config, ezfcconf, &err)))
		goto bail;
	config->loaded = TRUE;

  bail:
	if (err) {
		if (error)
			*error = g_error_copy(err);
		else
			g_warning(err->message);
		g_error_free(err);
	}
	g_free(ezfcconf);

	return retval;
}

/**
 * ezfc_config_save:
 * @config: a #ezfc_config_t.
 * @error: (allow-none): a #GError.
 *
 * Write the data to the configuration file. you may want to invoke
 * ezfc_config_set_priority() and ezfc_config_set_name() first to
 * write it to the appropriate configuration file.
 *
 * Returns: %TRUE if it's successfully completed, otherwise %FALSE.
 */
gboolean
ezfc_config_save(ezfc_config_t  *config,
		 GError        **error)
{
	GString *xml = NULL;
	gchar *ezfcconf, *fcconfdir;
	GError *err = NULL;
	gboolean retval = TRUE;

	g_return_val_if_fail (config != NULL, FALSE);

	fcconfdir = _ezfc_config_get_conf_dir(config);
	ezfcconf = _ezfc_config_get_conf_name(config);
	if (!g_file_test(fcconfdir, G_FILE_TEST_EXISTS)) {
		g_mkdir_with_parents(fcconfdir, 0755);
	} else {
		if (!g_file_test(fcconfdir, G_FILE_TEST_IS_DIR)) {
			g_set_error(&err, EZFC_ERROR, EZFC_ERR_NO_CONFIG_DIR,
				    "Unable to create a config dir: %s",
				    fcconfdir);
			retval = FALSE;
			goto bail;
		}
	}
	if ((xml = ezfc_config_save_to_buffer(config, &err)) == NULL) {
		if (config->loaded) {
			/* elements might be removed. */
			if (g_unlink(ezfcconf) == -1) {
				if (config->migration) {
					goto cleanup;
				} else {
					g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_LIBC,
						    g_strerror(errno));
				}
			} else {
				g_clear_error(&err);
			}
		}
		goto bail;
	}

	if (!g_file_set_contents(ezfcconf, xml->str, xml->len, &err))
		goto bail;
	if (config->migration) {
		gchar *fn, *olddir;

	  cleanup:
		fn = _ezfc_config_get_old_conf_name(config);
		olddir = _ezfc_config_get_old_conf_dir(config);
		/* Do not try to remove if the directory is symlink */
		if (g_file_test(fn, G_FILE_TEST_EXISTS) && !g_file_test(olddir, G_FILE_TEST_IS_SYMLINK)) {
			if (g_unlink(fn) == -1) {
				g_set_error(&err, EZFC_ERROR, EZFC_ERR_FAIL_ON_LIBC,
					    g_strerror(errno));
			}
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
	g_free(fcconfdir);
	g_free(ezfcconf);
	if (xml)
		g_string_free(xml, TRUE);

	return retval;
}

/**
 * ezfc_config_save_to_buffer:
 * @config: a #ezfc_config_t.
 * @error: (allow-none): a #GError.
 *
 * Write the data to the buffer.
 *
 * Returns: a #GString containing a xml data. %NULL if fails.
 */
GString *
ezfc_config_save_to_buffer(ezfc_config_t  *config,
			   GError        **error)
{
	xmlChar *xml;
	int size;
	GError *err = NULL;
	GString *retval = NULL;

	g_return_val_if_fail (config != NULL, FALSE);

	if (!_ezfc_config_to_xml(config, &xml, &size)) {
		g_set_error(&err, EZFC_ERROR, EZFC_ERR_NO_ELEMENTS,
			    "No configuration to output");
	}

	if (err) {
		if (error)
			*error = g_error_copy(err);
		else
			g_warning(err->message);
		g_error_free(err);
	} else {
		retval = g_string_new((const gchar *)xml);
	}
	xmlFree(xml);

	return retval;
}

/**
 * ezfc_config_dump:
 * @config: a #ezfc_config_t.
 *
 * Output the object data to the standard output.
 */
void
ezfc_config_dump(ezfc_config_t *config)
{
	GHashTableIter iter;
	gpointer key, val;

	g_return_if_fail (config != NULL);

	g_hash_table_iter_init(&iter, config->aliases);
	while (g_hash_table_iter_next(&iter, &key, &val)) {
		ezfc_container_t *container = val;
		const gchar *lang = key;
		GList *l = ezfc_container_get(container);

		if (lang[0] == 0)
			g_print("<any>:\n");
		else
			g_print("%s:\n", lang);
		for (; l != NULL; l = g_list_next(l)) {
			ezfc_alias_t *a = l->data;
			FcChar8 *result;
			FcPattern *pat;

			pat = ezfc_alias_get_font_pattern(a);
			result = FcPatternFormat(pat,
						 (FcChar8 *)"%{=unparse}");
			g_print("\t%s: %s\n",
				ezfc_alias_get_name(a),
				result);
			FcStrFree(result);
			FcPatternDestroy(pat);
		}
	}
}

/**
 * ezfc_config_set_migration:
 * @config: a #ezfc_config_t.
 * @flag: a #gboolean.
 *
 * Set a flag to migrate the configuration file on the older place to
 * the new one where XDG Base Directory Specification defines.
 * If @flag is %TRUE, ezfc_config_load() will tries to read the config
 * file from the old path prior to new place and the old file will be
 * removed during ezfc_config_save().
 *
 * This feature is enabled by default.
 *
 * Since: 0.8
*/
void
ezfc_config_set_migration(ezfc_config_t *config,
			  gboolean       flag)
{
	g_return_if_fail (config != NULL);

	config->migration = (flag == TRUE);
}
