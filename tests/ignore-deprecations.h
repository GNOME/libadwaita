/*
 * Copyright 2026 Zander Brown <zbrown@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <stdarg.h>

#include <glib.h>

G_BEGIN_DECLS


typedef struct _AdwIgnoreDeprecation AdwIgnoreDeprecation;
struct _AdwIgnoreDeprecation {
  GRegex *pattern;
  GLogLevelFlags old_flags;
  GLogFunc real_log_func;
};


G_GNUC_UNUSED
static void
adw_ignore_deprecation_log (const char     *log_domain,
                            GLogLevelFlags  log_level,
                            const char     *message,
                            gpointer        user_data)
{
  AdwIgnoreDeprecation *self = user_data;
  GLogLevelFlags effective_level = log_level;

  /* Make warnings fatal again, unless they match the pattern */
  if (log_level & G_LOG_LEVEL_WARNING &&
      !g_regex_match (self->pattern, message, G_REGEX_MATCH_DEFAULT, NULL)) {
    effective_level |= G_LOG_FLAG_FATAL;
  }

  /* Lucky for us gtest_default_log_handler doesn't use user_data… */
  self->real_log_func (log_domain, effective_level, message, NULL);
}


G_GNUC_UNUSED
static inline AdwIgnoreDeprecation *
adw_ignore_deprecation_new (const char *first, ...)
{
  AdwIgnoreDeprecation *self = g_new0 (AdwIgnoreDeprecation, 1);
  GError *error = NULL;
  GString *pattern = g_string_new ("The property (");
  char *first_escaped = g_regex_escape_string (first, -1);
  va_list args;

  g_string_append (pattern, first_escaped);

  va_start (args, first);

  while (TRUE) {
    const char *current = va_arg (args, const char *);
    char *escaped = NULL;

    if (!current) {
      break;
    }

    escaped = g_regex_escape_string (current, -1);

    g_string_append_c (pattern, '|');
    g_string_append (pattern, escaped);

    g_free (escaped);
  }

  va_end (args);

  g_string_append (pattern, ") is deprecated and shouldn't be used anymore.*");

  self->pattern =
    g_regex_new (pattern->str, G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT, &error);
  g_assert_no_error (error);

  self->old_flags = g_log_get_always_fatal ();

  /* Drop warnings from fatal mask so we can ignore property deprecations */
  g_log_set_always_fatal (self->old_flags & (~G_LOG_LEVEL_WARNING));

  self->real_log_func =
    g_log_set_default_handler (adw_ignore_deprecation_log, self);
  g_assert_false (self->real_log_func == adw_ignore_deprecation_log);

  g_free (first_escaped);
  g_string_free (pattern, TRUE);
  g_clear_error (&error);

  return self;
}


G_GNUC_UNUSED
static inline void
adw_ignore_deprecation_free (AdwIgnoreDeprecation *self)
{
  g_clear_pointer (&self->pattern, g_regex_unref);
  g_log_set_always_fatal (self->old_flags);
  g_log_set_default_handler (self->real_log_func, NULL);
  g_free (self);
}


G_DEFINE_AUTOPTR_CLEANUP_FUNC (AdwIgnoreDeprecation, adw_ignore_deprecation_free)


G_END_DECLS
