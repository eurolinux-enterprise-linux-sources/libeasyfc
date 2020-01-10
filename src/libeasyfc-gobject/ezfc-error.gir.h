/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * ezfc-error.h
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
#if !defined (__EZFC_H__INSIDE) && !defined (__EZFC_COMPILATION)
#error "Only <libeasyfc/ezfc.h> can be included directly."
#endif

#ifndef __EZFC_ERROR_H__
#define __EZFC_ERROR_H__

#include <glib.h>

G_BEGIN_DECLS

/**
 * SECTION:ezfc-error
 * @Short_Description: An enumerated type for error handling in libeasyfc.
 * @Title: EzfcError
 *
 * This type defines an error type being used for #GError.
 */

/**
 * EZFC_ERROR:
 *
 * A #GQuark value being used in libeasyfc.
 */
#define EZFC_ERROR	(ezfc_error_get_quark())

/**
 * EzfcError:
 * @EZFC_ERR_UNKNOWN: unknown error happened.
 * @EZFC_ERR_SUCCESS: an operation is succeeded.
 * @EZFC_ERR_OOM: Out of memory occurred.
 * @EZFC_ERR_FAIL_ON_FC: an error happened in fontconfig.
 * @EZFC_ERR_NO_VALID_FONT: no valid font is available on the system.
 * @EZFC_ERR_NO_FAMILY: no font family name found in a class.
 * @EZFC_ERR_NO_CONFIG_DIR: no valid configuration directory found.
 * @EZFC_ERR_NO_ELEMENTS: no elements found to write.
 * @EZFC_ERR_NO_CONFIG_FILE: no configuration file was available on the filesystem.
 * @EZFC_ERR_FAIL_ON_XML: an error happened in libxml2.
 * @EZFC_ERR_FAIL_ON_LIBC: an error happened in libc.
 * @EZFC_ERR_END: No real error, but just a terminator.
 *
 * Error code used in libeasyfc.
 */
typedef enum _EzfcError {
	EZFC_ERR_UNKNOWN = -1,
	EZFC_ERR_SUCCESS = 0,
	EZFC_ERR_OOM,
	EZFC_ERR_FAIL_ON_FC,
	EZFC_ERR_NO_VALID_FONT,
	EZFC_ERR_NO_FAMILY,
	EZFC_ERR_NO_CONFIG_DIR,
	EZFC_ERR_NO_ELEMENTS,
	EZFC_ERR_NO_CONFIG_FILE,
	EZFC_ERR_FAIL_ON_XML,
	EZFC_ERR_FAIL_ON_LIBC,
	EZFC_ERR_END
} EzfcError;


GQuark ezfc_error_get_quark(void);

G_END_DECLS

#endif /* __EZFC_ERROR_H__ */
