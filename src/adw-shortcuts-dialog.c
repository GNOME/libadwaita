/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-shortcuts-dialog.h"

#include "adw-shortcut-label.h"
#include "adw-preferences-group.h"
#include "adw-widget-utils-private.h"

/**
 * AdwShortcutsDialog:
 *
 * TODO
 *
 * Since: 1.8
 */

struct _AdwShortcutsDialog
{
  AdwDialog parent_instance;

  GMenuModel *shortcuts;
  GtkBox *content;
};

G_DEFINE_FINAL_TYPE (AdwShortcutsDialog, adw_shortcuts_dialog, ADW_TYPE_DIALOG)

enum {
  PROP_0,
  PROP_SHORTCUTS,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
list_header_cb (GtkListBoxRow *row,
                GtkListBoxRow *before,
                gpointer       user_data)
{
  gboolean section_start = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (row), "-adw-section-start"));

  if (section_start && before)
    gtk_list_box_row_set_header (row, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL));
  else
    gtk_list_box_row_set_header (row, NULL);
}

static GtkWidget *
create_row (const char *title,
            const char *accel)
{
  GtkWidget *box, *row, *label, *accel_widget;

  label = gtk_label_new (title);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_label_set_xalign (GTK_LABEL (label), 0);
  gtk_label_set_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD_CHAR);

  accel_widget = adw_shortcut_label_new (accel);
  gtk_widget_set_valign (accel_widget, GTK_ALIGN_CENTER);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_append (GTK_BOX (box), label);
  gtk_box_append (GTK_BOX (box), accel_widget);

  row = gtk_list_box_row_new ();
  gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row), box);

  return row;
}

static void
append_inner_section (GtkListBox *list,
                      GMenuModel *model)
{
  int i, n = g_menu_model_get_n_items (model);
  gboolean section_start = TRUE;

  for (i = 0; i < n; i++) {
    GMenuModel *section = g_menu_model_get_item_link (model, i, G_MENU_LINK_SECTION);
    char *label = NULL, *accel = NULL;
    GtkWidget *row;

    if (section) {
      append_inner_section (list, section);
      g_object_unref (section);
      section_start = TRUE;
      continue;
    }

    g_menu_model_get_item_attribute (model, i, G_MENU_ATTRIBUTE_LABEL, "s", &label);
    g_menu_model_get_item_attribute (model, i, "accel", "s", &accel);

    row = create_row (label, accel);

    if (section_start) {
      g_object_set_data (G_OBJECT (row), "-adw-section-start", GINT_TO_POINTER (1));
      section_start = FALSE;
    }

    gtk_list_box_append (list, row);

    g_free (accel);
    g_free (label);
  }
}

static void
append_section (AdwShortcutsDialog *self,
                const char         *title,
                GMenuModel         *model)
{
  GtkWidget *group = adw_preferences_group_new ();
  GtkWidget *list = gtk_list_box_new ();

  gtk_widget_add_css_class (list, "card");
  gtk_widget_add_css_class (list, "shortcuts-section");
  gtk_list_box_set_selection_mode (GTK_LIST_BOX (list), GTK_SELECTION_NONE);

  adw_preferences_group_set_title (ADW_PREFERENCES_GROUP (group), title);
  adw_preferences_group_add (ADW_PREFERENCES_GROUP (group), list);

  gtk_list_box_set_header_func (GTK_LIST_BOX (list),
                                (GtkListBoxUpdateHeaderFunc) list_header_cb,
                                NULL, NULL);

  append_inner_section (GTK_LIST_BOX (list), model);

  gtk_box_append (self->content, group);
}

static void
rebuild_shortcuts (AdwShortcutsDialog *self)
{
  int i, n = g_menu_model_get_n_items (self->shortcuts);

  // TODO delete old groups

  for (i = 0; i < n; i++) {
    GMenuModel *section = g_menu_model_get_item_link (self->shortcuts, i, G_MENU_LINK_SECTION);
    char *title = NULL;

    if (!section)
      continue;

    g_menu_model_get_item_attribute (self->shortcuts, i, "title", "s", &title);

    append_section (self, title, section);

    g_object_unref (section);
    g_free (title);
  }
}

static void
adw_shortcuts_dialog_dispose (GObject *object)
{
  AdwShortcutsDialog *self = ADW_SHORTCUTS_DIALOG (object);

  g_clear_object (&self->shortcuts);

  G_OBJECT_CLASS (adw_shortcuts_dialog_parent_class)->dispose (object);
}

static void
adw_shortcuts_dialog_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  AdwShortcutsDialog *self = ADW_SHORTCUTS_DIALOG (object);

  switch (prop_id) {
  case PROP_SHORTCUTS:
    g_value_set_object (value, adw_shortcuts_dialog_get_shortcuts (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcuts_dialog_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  AdwShortcutsDialog *self = ADW_SHORTCUTS_DIALOG (object);

  switch (prop_id) {
  case PROP_SHORTCUTS:
    adw_shortcuts_dialog_set_shortcuts (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcuts_dialog_class_init (AdwShortcutsDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_shortcuts_dialog_dispose;
  object_class->get_property = adw_shortcuts_dialog_get_property;
  object_class->set_property = adw_shortcuts_dialog_set_property;

  /**
   * AdwShortcutsDialog:shortcuts:
   *
   * Shortcuts menu model.
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_SHORTCUTS] =
    g_param_spec_object ("shortcuts", NULL, NULL,
                         G_TYPE_MENU_MODEL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-shortcuts-dialog.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, content);
}

static void
adw_shortcuts_dialog_init (AdwShortcutsDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

/**
 * adw_shortcuts_dialog_new:
 *
 * Creates a new `AdwShortcutsDialog`.
 *
 * Returns: the newly created `AdwShortcutsDialog`
 *
 * Since: 1.8
 */
AdwDialog *
adw_shortcuts_dialog_new (void)
{
  return g_object_new (ADW_TYPE_SHORTCUTS_DIALOG, NULL);
}

/**
 * adw_shortcuts_dialog_get_shortcuts:
 * @self: a shortcuts dialog
 *
 * TODO
 *
 * Returns: (transfer none) (nullable): TODO
 *
 * Since: 1.8
 */
GMenuModel *
adw_shortcuts_dialog_get_shortcuts (AdwShortcutsDialog *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUTS_DIALOG (self), NULL);

  return self->shortcuts;
}

/**
 * adw_shortcuts_dialog_set_shortcuts:
 * @self: a shortcuts dialog
 * @shortcuts: (nullable): TODO
 *
 * TODO
 */
void
adw_shortcuts_dialog_set_shortcuts (AdwShortcutsDialog *self,
                                    GMenuModel         *shortcuts)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_DIALOG (self));
  g_return_if_fail (shortcuts == NULL || G_IS_MENU_MODEL (shortcuts));

  if (self->shortcuts == shortcuts)
    return;

  // TODO track changes too
  // TODO can changes occur in submodels?

  g_set_object (&self->shortcuts, shortcuts);

  rebuild_shortcuts (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHORTCUTS]);
}
