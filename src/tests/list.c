/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * list.c
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
#include <glib.h>
#include <libeasyfc/ezfc.h>
#include <locale.h>

int
main(int argc, char **argv)
{
	GList *l, *ll;

	if (argc < 3) {
		g_print("Usage: %s <lang> <alias>\n", argv[0]);
		return 1;
	}

	setlocale(LC_ALL, "");
	ezfc_init();

	l = ezfc_font_get_list(argv[1], argv[2], TRUE);
	for (ll = l; ll != NULL; ll = g_list_next(ll)) {
		g_print("%s\n", (gchar *)ll->data);
		g_free(ll->data);
	}
	g_list_free(l);

	ezfc_finalize();

	return 0;
}
