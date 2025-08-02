/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-shortcut-row-private.h"

#include "adw-shortcut-label-private.h"
#include "adw-preferences-group.h"
#include "adw-preferences-page.h"
#include "adw-preferences-row.h"
#include "adw-view-stack.h"
#include "adw-widget-utils-private.h"
#include "adw-wrap-box.h"

#include <glib/gi18n.h>

struct _AdwShortcutRow
{
  AdwPreferencesRow parent_instance;

  AdwShortcutsItem *item;

  GtkWidget *title;
  GtkWidget *subtitle;
  GtkWidget *accel_label;
};

G_DEFINE_FINAL_TYPE (AdwShortcutRow, adw_shortcut_row, ADW_TYPE_PREFERENCES_ROW)

enum {
  PROP_0,
  PROP_ITEM,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static gboolean
string_is_not_empty (GBinding     *binding,
                     const GValue *input,
                     GValue       *output,
                     gpointer      user_data)
{
  const char *str = g_value_get_string (input);

  g_value_set_boolean (output, str && str[0]);

  return TRUE;
}

static void
update_accel (AdwShortcutRow *self)
{
  const char *accel = adw_shortcuts_item_get_accelerator (self->item);
  const char *action_name = adw_shortcuts_item_get_action_name (self->item);
  char *action_accel = NULL;

  if (action_name && *action_name) {
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

    if (GTK_IS_WINDOW (root)) {
      GtkApplication *app = gtk_window_get_application (GTK_WINDOW (root));

      if (!GTK_IS_APPLICATION (app)) {
        GtkWindow *transient_for = gtk_window_get_transient_for (GTK_WINDOW (root));

        if (GTK_IS_WINDOW (transient_for))
          app = gtk_window_get_application (GTK_WINDOW (transient_for));
      }

      if (GTK_IS_APPLICATION (app)) {
        char **action_accels = gtk_application_get_accels_for_action (app, action_name);
        char *new_action_accels = g_strjoinv (" ", action_accels);

        g_free (action_accel);
        action_accel = new_action_accels;

        g_strfreev (action_accels);
      }
    }
  }

  if (action_accel && *action_accel)
    adw_shortcut_label_set_accelerator (ADW_SHORTCUT_LABEL (self->accel_label), action_accel);
  else
    adw_shortcut_label_set_accelerator (ADW_SHORTCUT_LABEL (self->accel_label), accel);

  g_free (action_accel);
}

static void
adw_shortcut_row_root (GtkWidget *widget)
{
  AdwShortcutRow *self = ADW_SHORTCUT_ROW (widget);
  GtkRoot *root;

  GTK_WIDGET_CLASS (adw_shortcut_row_parent_class)->root (widget);

  root = gtk_widget_get_root (widget);

  if (GTK_IS_WINDOW (root)) {
    g_signal_connect_swapped (root, "keys-changed", G_CALLBACK (update_accel), self);

    update_accel (self);
  }
}

static void
adw_shortcut_row_unroot (GtkWidget *widget)
{
  AdwShortcutRow *self = ADW_SHORTCUT_ROW (widget);
  GtkRoot *root = gtk_widget_get_root (widget);

  if (GTK_IS_WINDOW (root))
    g_signal_handlers_disconnect_by_func (root, update_accel, self);

  GTK_WIDGET_CLASS (adw_shortcut_row_parent_class)->unroot (widget);
}

static void
adw_shortcut_row_constructed (GObject *object)
{
  AdwShortcutRow *self = ADW_SHORTCUT_ROW (object);

  G_OBJECT_CLASS (adw_shortcut_row_parent_class)->constructed (object);

  g_object_bind_property (self->item, "title", self, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (self->item, "title", self->title, "label", G_BINDING_SYNC_CREATE);

  g_object_bind_property (self->item, "subtitle", self->subtitle, "label", G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (self->item, "subtitle", self->subtitle, "visible",
                               G_BINDING_SYNC_CREATE,
                               string_is_not_empty, NULL, NULL, NULL);

  g_signal_connect_object (self->item, "notify::accelerator",
                           G_CALLBACK (update_accel), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (self->item, "notify::action-name",
                           G_CALLBACK (update_accel), self, G_CONNECT_SWAPPED);
}

static void
adw_shortcut_row_dispose (GObject *object)
{
  AdwShortcutRow *self = ADW_SHORTCUT_ROW (object);

  g_clear_object (&self->item);

  self->title = NULL;
  self->subtitle = NULL;
  self->accel_label = NULL;

  G_OBJECT_CLASS (adw_shortcut_row_parent_class)->dispose (object);
}

static void
adw_shortcut_row_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwShortcutRow *self = ADW_SHORTCUT_ROW (object);

  switch (prop_id) {
  case PROP_ITEM:
    g_value_set_object (value, self->item);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcut_row_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwShortcutRow *self = ADW_SHORTCUT_ROW (object);

  switch (prop_id) {
  case PROP_ITEM:
    g_set_object (&self->item, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcut_row_class_init (AdwShortcutRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = adw_shortcut_row_constructed;
  object_class->dispose = adw_shortcut_row_dispose;
  object_class->get_property = adw_shortcut_row_get_property;
  object_class->set_property = adw_shortcut_row_set_property;

  widget_class->root = adw_shortcut_row_root;
  widget_class->unroot = adw_shortcut_row_unroot;

  props[PROP_ITEM] =
    g_param_spec_object ("item", NULL, NULL,
                         ADW_TYPE_SHORTCUTS_ITEM,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_shortcut_row_init (AdwShortcutRow *self)
{
  GtkWidget *box, *title_box;

  gtk_widget_add_css_class (GTK_WIDGET (self), "shortcut-row");

  box = adw_wrap_box_new ();
  adw_wrap_box_set_child_spacing (ADW_WRAP_BOX (box), 9);
  adw_wrap_box_set_line_spacing (ADW_WRAP_BOX (box), 6);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
  gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (self), box);

  title_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class (title_box, "title-box");
  gtk_widget_set_valign (title_box, GTK_ALIGN_CENTER);
  adw_wrap_box_append (ADW_WRAP_BOX (box), title_box);

  self->title = gtk_label_new (NULL);
  gtk_widget_add_css_class (self->title, "title");
  gtk_widget_set_hexpand (self->title, TRUE);
  gtk_widget_set_valign (self->title, GTK_ALIGN_BASELINE_FILL);
  gtk_label_set_xalign (GTK_LABEL (self->title), 0);
  gtk_label_set_width_chars (GTK_LABEL (self->title), 10);
  gtk_label_set_wrap (GTK_LABEL (self->title), TRUE);
  gtk_label_set_wrap_mode (GTK_LABEL (self->title), PANGO_WRAP_WORD_CHAR);
  gtk_box_append (GTK_BOX (title_box), self->title);

  self->subtitle = gtk_label_new (NULL);
  gtk_widget_add_css_class (self->subtitle, "subtitle");
  gtk_widget_set_hexpand (self->subtitle, TRUE);
  gtk_widget_set_valign (self->subtitle, GTK_ALIGN_BASELINE_FILL);
  gtk_label_set_xalign (GTK_LABEL (self->subtitle), 0);
  gtk_label_set_width_chars (GTK_LABEL (self->subtitle), 10);
  gtk_label_set_wrap (GTK_LABEL (self->subtitle), TRUE);
  gtk_label_set_wrap_mode (GTK_LABEL (self->subtitle), PANGO_WRAP_WORD_CHAR);
  gtk_box_append (GTK_BOX (title_box), self->subtitle);

  self->accel_label = adw_shortcut_label_new ("");
  gtk_widget_set_valign (self->accel_label, GTK_ALIGN_CENTER);
  adw_shortcut_label_set_disabled_text (ADW_SHORTCUT_LABEL (self->accel_label), _("No Shortcut"));
  adw_shortcut_label_set_wrap (ADW_SHORTCUT_LABEL (self->accel_label), TRUE);
  adw_wrap_box_append (ADW_WRAP_BOX (box), self->accel_label);
}

GtkWidget *
adw_shortcut_row_new (AdwShortcutsItem *item)
{
  g_return_val_if_fail (ADW_IS_SHORTCUTS_ITEM (item), NULL);

  return g_object_new (ADW_TYPE_SHORTCUT_ROW, "item", item, NULL);
}

AdwShortcutsItem *
adw_shortcut_row_get_item (AdwShortcutRow *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUT_ROW (self), NULL);

  return self->item;
}
