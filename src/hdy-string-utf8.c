/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <stdlib.h>
#include <gmodule.h>
#include "hdy-string-utf8.h"

/**
 * SECTION:hdy-string-utf8
 * @short_description: #GString utf-8 helpers
 * @title: HdyStringUtf8
 *
 * Helpers to ease utf-8 handling based on #GString
 */

/**
 * hdy_string_utf8_truncate:
 * @string: a #GString
 * @len: the new size of the string
 *
 * Cut of the end of the string @string so that @len utf8 characters remain
 *
 * Returns: (transfer none): @string
 */
GString *
hdy_string_utf8_truncate (GString *string,
                          gsize    len)
{
  gint cutoff;
  gchar *off;

  g_return_val_if_fail (string != NULL, NULL);

  cutoff = MIN (len, g_utf8_strlen (string->str, -1));
  off = g_utf8_offset_to_pointer (string->str, cutoff);
  g_string_truncate (string, off - string->str);
  return string;
}

/**
 * hdy_string_utf8_len:
 * @string: a #GString
 *
 * Computes the length of the string in utf-8 characters. See #g_utf8_strlen.
 *
 * Returns: the length of @string in characters
 */
glong
hdy_string_utf8_len (GString *string)
{
  g_return_val_if_fail (string != NULL, 0);
  return g_utf8_strlen (string->str, -1);
}
