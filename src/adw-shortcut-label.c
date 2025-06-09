/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

// Heavily based on gtkshortcutlabel.c
// Source: https://gitlab.gnome.org/GNOME/gtk/-/blob/df9ec772952826dbda5bf3afbae35b282c85f268/gtk/deprecated/gtkshortcutlabel.c

#include "config.h"

#include "adw-shortcut-label.h"

#include "adw-widget-utils-private.h"

#include <glib/gi18n.h>

#define GETTEXT_PACKAGE_GTK "gtk40"
#define GTK_KEY_LABEL(str) (g_dpgettext2 (GETTEXT_PACKAGE_GTK, "keyboard label", (str)))

/**
 * AdwShortcutLabel:
 *
 * TODO
 *
 * Since: 1.8
 */

struct _AdwShortcutLabel
{
  GtkWidget parent_instance;

  char *accelerator;
};

G_DEFINE_FINAL_TYPE (AdwShortcutLabel, adw_shortcut_label, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_ACCELERATOR,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static char *
get_modifier_label (guint key)
{
  const char *subscript;
  const char *label;

  switch (key) {
  case GDK_KEY_Shift_L:
  case GDK_KEY_Control_L:
  case GDK_KEY_Alt_L:
  case GDK_KEY_Meta_L:
  case GDK_KEY_Super_L:
  case GDK_KEY_Hyper_L:
    /* Translators: This string is used to mark left/right variants of modifier
     * keys in the shortcut window (e.g. Control_L vs Control_R). Please keep
     * this string very short, ideally just a single character, since it will
     * be rendered as part of the key.
     */
    subscript = C_("keyboard side marker", "L");
    break;
  case GDK_KEY_Shift_R:
  case GDK_KEY_Control_R:
  case GDK_KEY_Alt_R:
  case GDK_KEY_Meta_R:
  case GDK_KEY_Super_R:
  case GDK_KEY_Hyper_R:
    /* Translators: This string is used to mark left/right variants of modifier
     * keys in the shortcut window (e.g. Control_L vs Control_R). Please keep
     * this string very short, ideally just a single character, since it will
     * be rendered as part of the key.
     */
    subscript = C_("keyboard side marker", "R");
    break;
  default:
    g_assert_not_reached ();
   }

 switch (key) {
 case GDK_KEY_Shift_L:   case GDK_KEY_Shift_R:
   label = GTK_KEY_LABEL ("Shift");
   break;
 case GDK_KEY_Control_L: case GDK_KEY_Control_R:
   label = GTK_KEY_LABEL ("Ctrl");
   break;
 case GDK_KEY_Alt_L:     case GDK_KEY_Alt_R:
   label = GTK_KEY_LABEL ("Alt");
   break;
 case GDK_KEY_Meta_L:    case GDK_KEY_Meta_R:
   label = GTK_KEY_LABEL ("Meta");
   break;
 case GDK_KEY_Super_L:   case GDK_KEY_Super_R:
   label = GTK_KEY_LABEL ("Super");
   break;
 case GDK_KEY_Hyper_L:   case GDK_KEY_Hyper_R:
   label = GTK_KEY_LABEL ("Hyper");
   break;
  default:
    g_assert_not_reached ();
   }

  return g_strdup_printf ("%s <small><b>%s</b></small>", label, subscript);
}

static char **
get_labels (guint key, GdkModifierType modifier, guint *n_mods)
{
  const char *labels[16];
  GList *freeme = NULL;
  char key_label[6];
  const char *tmp;
  gunichar ch;
  int i = 0;
  char **retval;

  if (modifier & GDK_SHIFT_MASK)
    labels[i++] = GTK_KEY_LABEL ("Shift");
  if (modifier & GDK_CONTROL_MASK)
    labels[i++] = GTK_KEY_LABEL ("Ctrl");
  if (modifier & GDK_ALT_MASK)
    labels[i++] = GTK_KEY_LABEL ("Alt");
  if (modifier & GDK_SUPER_MASK)
    labels[i++] = GTK_KEY_LABEL ("Super");
  if (modifier & GDK_HYPER_MASK)
    labels[i++] = GTK_KEY_LABEL ("Hyper");
  if (modifier & GDK_META_MASK)
#ifndef GDK_WINDOWING_MACOS
    labels[i++] = GTK_KEY_LABEL ("Meta");
#else
    labels[i++] = "⌘";
#endif

  *n_mods = i;

  ch = gdk_keyval_to_unicode (key);
  if (ch && ch < 0x80 && g_unichar_isgraph (ch)) {
    switch (ch) {
    case '<':
      labels[i++] = "&lt;";
      break;
    case '>':
      labels[i++] = "&gt;";
      break;
    case '&':
      labels[i++] = "&amp;";
      break;
    case '"':
      labels[i++] = "&quot;";
      break;
    case '\'':
      labels[i++] = "&apos;";
      break;
    case '\\':
      labels[i++] = GTK_KEY_LABEL ("Backslash");
      break;
    default:
      memset (key_label, 0, 6);
      g_unichar_to_utf8 (g_unichar_toupper (ch), key_label);
      labels[i++] = key_label;
      break;
    }
  } else {
    switch (key) {
    case GDK_KEY_Shift_L:   case GDK_KEY_Shift_R:
    case GDK_KEY_Control_L: case GDK_KEY_Control_R:
    case GDK_KEY_Alt_L:     case GDK_KEY_Alt_R:
    case GDK_KEY_Meta_L:    case GDK_KEY_Meta_R:
    case GDK_KEY_Super_L:   case GDK_KEY_Super_R:
    case GDK_KEY_Hyper_L:   case GDK_KEY_Hyper_R:
      freeme = g_list_prepend (freeme, get_modifier_label (key));
      labels[i++] = (const char *)freeme->data;
      break;
    case GDK_KEY_Left:
      labels[i++] = "\xe2\x86\x90";
      break;
    case GDK_KEY_Up:
      labels[i++] = "\xe2\x86\x91";
      break;
    case GDK_KEY_Right:
      labels[i++] = "\xe2\x86\x92";
      break;
    case GDK_KEY_Down:
      labels[i++] = "\xe2\x86\x93";
      break;
    case GDK_KEY_space:
      labels[i++] = "\xe2\x90\xa3";
      break;
    case GDK_KEY_Return:
      labels[i++] = "\xe2\x8f\x8e";
      break;
    case GDK_KEY_Page_Up:
      labels[i++] = GTK_KEY_LABEL ("Page_Up");
      break;
    case GDK_KEY_Page_Down:
      labels[i++] = GTK_KEY_LABEL ("Page_Down");
      break;
    default:
      tmp = gdk_keyval_name (gdk_keyval_to_lower (key));
      if (tmp != NULL)
        {
          if (tmp[0] != 0 && tmp[1] == 0)
            {
              key_label[0] = g_ascii_toupper (tmp[0]);
              key_label[1] = '\0';
              labels[i++] = key_label;
            }
          else
            {
              labels[i++] = GTK_KEY_LABEL (tmp);
            }
        }
    }
  }

  labels[i] = NULL;

  retval = g_strdupv ((char **)labels);

  g_list_free_full (freeme, g_free);

  return retval;
}

static GtkWidget *
dim_label (const char *text)
{
  GtkWidget *label;

  label = gtk_label_new (text);
  gtk_widget_add_css_class (label, "dimmed");

  return label;
}

static void
display_shortcut (GtkWidget       *self,
                  guint            key,
                  GdkModifierType  modifier)
{
  char **keys = NULL;
  int i;
  guint n_mods;

  keys = get_labels (key, modifier, &n_mods);
  for (i = 0; keys[i]; i++) {
    GtkWidget *disp = gtk_label_new (keys[i]);

    gtk_widget_add_css_class (disp, "keycap");
    gtk_label_set_use_markup (GTK_LABEL (disp), TRUE);

    gtk_widget_set_parent (disp, self);
  }

  g_strfreev (keys);
}

static gboolean
parse_combination (AdwShortcutLabel *self,
                   const char       *str)
{
  char **accels;
  int k;
  GdkModifierType modifier = 0;
  guint key = 0;
  gboolean retval = TRUE;

  accels = g_strsplit (str, "&", 0);
  for (k = 0; accels[k]; k++) {
    if (!gtk_accelerator_parse (accels[k], &key, &modifier)) {
      retval = FALSE;
      break;
    }

    display_shortcut (GTK_WIDGET (self), key, modifier);
  }

  g_strfreev (accels);

  return retval;
}

static gboolean
parse_sequence (AdwShortcutLabel *self,
                const char       *str)
{
  char **accels;
  int k;
  gboolean retval = TRUE;

  accels = g_strsplit (str, "+", 0);
  for (k = 0; accels[k]; k++) {
    if (k > 0)
      // TODO if we're going with an arrow, need to reverse it for RTL
      gtk_widget_set_parent (dim_label ("→"), GTK_WIDGET (self));

    if (!parse_combination (self, accels[k])) {
      retval = FALSE;
      break;
    }
  }

  g_strfreev (accels);

  return retval;
}

static gboolean
parse_range (AdwShortcutLabel *self,
             const char       *str)
{
  char *dots;

  dots = strstr (str, "...");
  if (!dots)
    return parse_sequence (self, str);

  dots[0] = '\0';
  if (!parse_sequence (self, str))
    return FALSE;

  gtk_widget_set_parent (dim_label ("⋯"), GTK_WIDGET (self));

  if (!parse_sequence (self, dots + 3))
    return FALSE;

  return TRUE;
}

static void
clear_children (AdwShortcutLabel *self)
{
  GtkWidget *child;

  child = gtk_widget_get_first_child (GTK_WIDGET (self));

  while (child) {
    GtkWidget *next = gtk_widget_get_next_sibling (child);

    gtk_widget_unparent (child);

    child = next;
  }
}

static void
rebuild (AdwShortcutLabel *self)
{
  char **accels;
  int k;
  GtkAccessibleRelation relation = GTK_ACCESSIBLE_RELATION_LABELLED_BY;
  GValue value = G_VALUE_INIT;
  GList *parts = NULL;
  GtkWidget *child;

  gtk_accessible_reset_relation (GTK_ACCESSIBLE (self), GTK_ACCESSIBLE_RELATION_LABELLED_BY);

  clear_children (self);

  if (self->accelerator == NULL || self->accelerator[0] == '\0') {
    GtkWidget *label;

    label = dim_label (_("No Shortcut"));

    gtk_widget_set_parent (label, GTK_WIDGET (self));
    return;
  }

  accels = g_strsplit (self->accelerator, " ", 0);

  for (k = 0; accels[k]; k++) {
    if (k > 0)
      gtk_widget_set_parent (dim_label ("/"), GTK_WIDGET (self));

    if (!parse_range (self, accels[k])) {
      g_warning ("Failed to parse %s, part of accelerator '%s'", accels[k], self->accelerator);
      break;
    }
  }

  g_strfreev (accels);

  /* All of the child labels are a part of our a11y label */
  for(child = gtk_widget_get_last_child (GTK_WIDGET (self));
      child != NULL;
      child = gtk_widget_get_prev_sibling (child)) {
    parts = g_list_prepend (parts, child);
  }

  gtk_accessible_relation_init_value (relation, &value);
  g_value_set_pointer (&value, parts);
  gtk_accessible_update_relation_value (GTK_ACCESSIBLE (self),
                                        1, &relation, &value);
}

static void
adw_shortcut_label_dispose (GObject *object)
{
  AdwShortcutLabel *self = ADW_SHORTCUT_LABEL (object);

  clear_children (self);

  G_OBJECT_CLASS (adw_shortcut_label_parent_class)->dispose (object);
}

static void
adw_shortcut_label_finalize (GObject *object)
{
  AdwShortcutLabel *self = ADW_SHORTCUT_LABEL (object);

  g_free (self->accelerator);

  G_OBJECT_CLASS (adw_shortcut_label_parent_class)->dispose (object);
}

static void
adw_shortcut_label_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwShortcutLabel *self = ADW_SHORTCUT_LABEL (object);

  switch (prop_id) {
  case PROP_ACCELERATOR:
    g_value_set_string (value, adw_shortcut_label_get_accelerator (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcut_label_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwShortcutLabel *self = ADW_SHORTCUT_LABEL (object);

  switch (prop_id) {
  case PROP_ACCELERATOR:
    adw_shortcut_label_set_accelerator (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcut_label_class_init (AdwShortcutLabelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_shortcut_label_dispose;
  object_class->finalize = adw_shortcut_label_finalize;
  object_class->get_property = adw_shortcut_label_get_property;
  object_class->set_property = adw_shortcut_label_set_property;

  /**
   * AdwShortcutLabel:accelerator:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_ACCELERATOR] =
    g_param_spec_string ("accelerator", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "shortcut-label");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
}

static void
adw_shortcut_label_init (AdwShortcutLabel *self)
{
  self->accelerator = g_strdup ("");
}

/**
 * adw_shortcut_label_new:
 * @accelerator: the accelerator to show
 *
 * Creates a new `AdwShortcutLabel` showing @accelerator.
 *
 * Returns: the newly created `AdwShortcutLabel`
 *
 * Since: 1.8
 */
GtkWidget *
adw_shortcut_label_new (const char *accelerator)
{
  g_return_val_if_fail (accelerator != NULL, NULL);

  return g_object_new (ADW_TYPE_SHORTCUT_LABEL,
                       "accelerator", accelerator,
                       NULL);
}

/**
 * adw_shortcut_label_get_accelerator:
 * @self: a shortcut label
 *
 * TODO
 *
 * Returns: (transfer none): TODO
 *
 * Since: 1.8
 */
const char *
adw_shortcut_label_get_accelerator (AdwShortcutLabel *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUT_LABEL (self), NULL);

  return self->accelerator;
}

/**
 * adw_shortcut_label_set_accelerator:
 * @self: a shortcut label
 * @accelerator: TODO
 *
 * TODO
 */
void
adw_shortcut_label_set_accelerator (AdwShortcutLabel *self,
                                    const char       *accelerator)
{
  g_return_if_fail (ADW_IS_SHORTCUT_LABEL (self));
  g_return_if_fail (accelerator != NULL);

  if (!g_set_str (&self->accelerator, accelerator))
    return;

  rebuild (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACCELERATOR]);
}
