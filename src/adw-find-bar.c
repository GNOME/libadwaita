/*
 * adw-find-bar.c
 *
 * Copyright 2023 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-find-bar.h"

struct _AdwFindBar
{
  GtkWidget parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwFindBar, adw_find_bar, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_CHILD,
  PROP_REVEALED,
  PROP_REPLACE_ENABLED,
  PROP_FIND_TEXT,
  PROP_REPLACE_TEXT,
  PROP_FIND_PLACEHOLDER_TEXT,
  PROP_REPLACE_PLACEHOLDER_TEXT,
  PROP_MATCH_COUNT,
  PROP_MATCH_POSITION,
  PROP_CASE_SENSITIVE,
  PROP_USE_REGEX,
  PROP_MATCH_WHOLE_WORDS,
  PROP_NARROW,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

enum {
  SIGNAL_SEARCH_STARTED,
  SIGNAL_SEARCH_STOPPED,
  SIGNAL_SEARCH_CHANGED,
  SIGNAL_NEXT_MATCH,
  SIGNAL_PREVIOUS_MATCH,
  SIGNAL_REPLACE_MATCH,
  SIGNAL_REPLACE_ALL,
  SIGNAL_LAST_SIGNAL
};

static guint signals[SIGNAL_LAST_SIGNAL];

AdwFindBar *
adw_find_bar_new (void)
{
  return g_object_new (ADW_TYPE_FIND_BAR, NULL);
}

static void
adw_find_bar_finalize (GObject *object)
{
  AdwFindBar *self = (AdwFindBar *)object;

  G_OBJECT_CLASS (adw_find_bar_parent_class)->finalize (object);
}

static void
adw_find_bar_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwFindBar *self = ADW_FIND_BAR (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
adw_find_bar_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwFindBar *self = ADW_FIND_BAR (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
adw_find_bar_class_init (AdwFindBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = adw_find_bar_finalize;
  object_class->get_property = adw_find_bar_get_property;
  object_class->set_property = adw_find_bar_set_property;
}

static void
adw_find_bar_init (AdwFindBar *self)
{

}
