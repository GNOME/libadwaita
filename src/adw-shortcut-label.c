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

#include "adw-shortcut-label-private.h"

#include "adw-widget-utils-private.h"
#include "adw-wrap-layout.h"

#include <glib/gi18n.h>

#define GETTEXT_PACKAGE_GTK "gtk40"
#define GTK_KEY_LABEL(str) (g_dpgettext2 (GETTEXT_PACKAGE_GTK, "keyboard label", (str)))

/**
 * AdwShortcutLabel:
 *
 * A widget that displays a keyboard shortcut.
 *
 * <picture>
 *   <source srcset="shortcut-label-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="shortcut-label.png" alt="shortcut-label">
 * </picture>
 *
 * The shown shortcut can be set using the [property@ShortcutLabel:accelerator]
 * property.
 *
 * Optionally, if no shortcut is set, `AdwShortcutLabel` will display a
 * placeholder set with the [property@ShortcutLabel:disabled-text] property.
 *
 * The following types of shortcuts can be displayed:
 *
 * - A single shortcut in [func@Gtk.accelerator_parse] format, e.g. `<Control>C`:

 *     <picture>
 *       <source srcset="shortcut-label-single-dark.png" media="(prefers-color-scheme: dark)">
 *       <img src="shortcut-label-single.png" alt="shortcut-label-single">
 *     </picture>
 *
 * - Multiple alternative shortcuts, separated with spaces, e.g. `<Shift>A Home`:
 *
 *     <picture>
 *       <source srcset="shortcut-label-alternative-dark.png" media="(prefers-color-scheme: dark)">
 *       <img src="shortcut-label-alternative.png" alt="shortcut-label-alternative">
 *     </picture>
 *
 * - A range of shortcuts, separated with `...`, e.g. `<Alt>1...9`:
 *
 *     <picture>
 *       <source srcset="shortcut-label-range-dark.png" media="(prefers-color-scheme: dark)">
 *       <img src="shortcut-label-range.png" alt="shortcut-label-range">
 *     </picture>
 *
 * - Multiple keys pressed at once, separated with `&`, e.g. `Control_L&Control_R`:
 *
 *     <picture>
 *       <source srcset="shortcut-label-multiple-dark.png" media="(prefers-color-scheme: dark)">
 *       <img src="shortcut-label-multiple.png" alt="shortcut-label-multiple">
 *     </picture>
 *
 * - Multiple shortcuts or keys, pressed sequentially, separated with `+`, e.g. `<Control>C+<Control>X`:
 *
 *     <picture>
 *       <source srcset="shortcut-label-sequence-dark.png" media="(prefers-color-scheme: dark)">
 *       <img src="shortcut-label-sequence.png" alt="shortcut-label-sequence">
 *     </picture>
 *
 * ::: note
 *     `<`, `>` and `&` need to be escaped as `&lt;`, `&gt;` and `&amp;` when used in UI files.
 *
 * ## CSS nodes
 *
 * `AdwShortcutLabel` has a single CSS node with name `shortcut-label`. The
 * individual keycap labels each have the `.keycap` style class, while the
 * labels separating them have the `.dimmed` style class.
 *
 * ## Accessibility
 *
 * `AdwShortcutLabel` uses the `GTK_ACCESSIBLE_ROLE_GENERIC` role.
 *
 * See also: [class@ShortcutsDialog].
 *
 * Since: 1.8
 */

struct _AdwShortcutLabel
{
  GtkWidget parent_instance;

  char *accelerator;
  char *disabled_text;
  gboolean wrap;
};

G_DEFINE_FINAL_TYPE (AdwShortcutLabel, adw_shortcut_label, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_ACCELERATOR,
  PROP_DISABLED_TEXT,
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
  char key_label[16];
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
      memset (key_label, 0, sizeof (key_label));
      if (key >= GDK_KEY_KP_Space && key <= GDK_KEY_KP_9) {
        key_label[0] = 'K';
        key_label[1] = 'P';
        key_label[2] = ' ';
        g_unichar_to_utf8 (g_unichar_toupper (ch), key_label + 3);
      } else {
        g_unichar_to_utf8 (g_unichar_toupper (ch), key_label);
      }
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
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  char **keys = NULL;
  int i;
  guint n_mods;

  gtk_widget_set_direction (GTK_WIDGET (box), GTK_TEXT_DIR_LTR);

  keys = get_labels (key, modifier, &n_mods);
  for (i = 0; keys[i]; i++) {
    GtkWidget *disp = gtk_label_new (keys[i]);

    gtk_widget_add_css_class (disp, "keycap");
    gtk_label_set_use_markup (GTK_LABEL (disp), TRUE);

    gtk_box_append (GTK_BOX (box), disp);
  }

  gtk_widget_set_parent (box, self);

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
  gboolean retval = TRUE;
  char **accels;
  int k;

  accels = g_strsplit (str, "+", 0);
  for (k = 0; accels[k]; k++) {
    if (k > 0)
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

  clear_children (self);

  if (self->accelerator == NULL || self->accelerator[0] == '\0') {
    GtkWidget *label;

    label = dim_label (self->disabled_text);

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
  g_free (self->disabled_text);

  G_OBJECT_CLASS (adw_shortcut_label_parent_class)->finalize (object);
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
  case PROP_DISABLED_TEXT:
    g_value_set_string (value, adw_shortcut_label_get_disabled_text (self));
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
  case PROP_DISABLED_TEXT:
    adw_shortcut_label_set_disabled_text (self, g_value_get_string (value));
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
   * The displayed accelerator.
   *
   * Since: 1.8
   */
  props[PROP_ACCELERATOR] =
    g_param_spec_string ("accelerator", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwShortcutLabel:disabled-text:
   *
   * The text displayed when no accelerator is set.
   *
   * Since: 1.8
   */
  props[PROP_DISABLED_TEXT] =
    g_param_spec_string ("disabled-text", NULL, NULL,
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
  self->disabled_text = g_strdup ("");

  rebuild (self);
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
 * Gets the accelerator displayed by @self.
 *
 * Returns: (transfer none): the displayed accelerator
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
 * @accelerator: the accelerator to be displayed
 *
 * Sets the accelerator to be displayed by @self.
 *
 * Since: 1.8
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

/**
 * adw_shortcut_label_get_disabled_text:
 * @self: a shortcut label
 *
 * Gets the text displayed by @self when no accelerator is set.
 *
 * Returns: (transfer none): the text displayed when no accelerator is set
 *
 * Since: 1.8
 */
const char *
adw_shortcut_label_get_disabled_text (AdwShortcutLabel *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUT_LABEL (self), NULL);

  return self->disabled_text;
}

/**
 * adw_shortcut_label_set_disabled_text:
 * @self: a shortcut label
 * @disabled_text: the text displayed when no accelerator is set
 *
 * Sets the text to be displayed by @self when no accelerator is set.
 *
 * Since: 1.8
 */
void
adw_shortcut_label_set_disabled_text (AdwShortcutLabel *self,
                                      const char       *disabled_text)
{
  g_return_if_fail (ADW_IS_SHORTCUT_LABEL (self));
  g_return_if_fail (disabled_text != NULL);

  if (!g_set_str (&self->disabled_text, disabled_text))
    return;

  rebuild (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DISABLED_TEXT]);
}

void
adw_shortcut_label_set_wrap (AdwShortcutLabel *self,
                             gboolean          wrap)
{
  GtkLayoutManager *layout;

  g_return_if_fail (ADW_IS_SHORTCUT_LABEL (self));

  wrap = !!wrap;

  if (self->wrap == wrap)
    return;

  self->wrap = wrap;

  if (wrap) {
    layout = adw_wrap_layout_new ();
    adw_wrap_layout_set_child_spacing (ADW_WRAP_LAYOUT (layout), 6);
    adw_wrap_layout_set_line_spacing (ADW_WRAP_LAYOUT (layout), 6);
  } else {
    layout = gtk_box_layout_new (GTK_ORIENTATION_HORIZONTAL);
  }

  rebuild (self);

  gtk_widget_set_layout_manager (GTK_WIDGET (self), layout);
}
