/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * save.c
 * Copyright (C) 2011 Akira TAGOH
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

int
main(int argc, char **argv)
{
	ezfc_config_t *config;
	ezfc_alias_t *alias;
	int i;

	if (argc < 4) {
		g_print("Usage: %s <lang> <alias> <font> ...\n", argv[0]);
		return 1;
	}

	config = ezfc_config_new();
	for (i = 1; i < argc; i += 3) {
		alias = ezfc_alias_new(argv[i + 1]);
		if (!ezfc_alias_set_font(alias, argv[i + 2], NULL)) {
			g_print("E: %s %s %s\n", argv[i], argv[i + 1], argv[i + 2]);
			ezfc_alias_unref(alias);
			continue;
		}
		ezfc_config_add_alias(config, argv[i], alias);
		ezfc_alias_unref(alias);
	}
	ezfc_config_save(config, NULL);

	ezfc_config_unref(config);

	return 0;
}
