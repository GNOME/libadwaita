/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include <glib/gi18n.h>

#include "adw-inspector-page-private.h"

#include <adwaita.h>
#include "adw-gizmo-private.h"
#include "adw-preferences-group.h"
#include "adw-settings-private.h"

struct _AdwInspectorPage
{
  AdwBin parent_instance;

  AdwSettings *settings;

  AdwSwitchRow *support_color_schemes_row;
  AdwComboRow *color_scheme_row;
  AdwSwitchRow *high_contrast_row;
  AdwSwitchRow *support_accent_colors_row;
  AdwComboRow *accent_color_row;
  AdwPreferencesGroup *adaptive_preview_group;

  GObject *object;

  gboolean realized;
};

G_DEFINE_FINAL_TYPE (AdwInspectorPage, adw_inspector_page, ADW_TYPE_BIN)

enum {
  PROP_0,
  PROP_TITLE,
  PROP_OBJECT,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static gboolean
is_window_compatible (GObject *item)
{
  return ADW_IS_WINDOW (item) || ADW_IS_APPLICATION_WINDOW (item);
}

static GtkWidget *
create_window_row_cb (GtkWindow        *window,
                      AdwInspectorPage *self)
{
  GtkWidget *row, *btn;

  row = adw_action_row_new ();
  adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (row), FALSE);
  g_object_bind_property (window, "title", row, "title", G_BINDING_SYNC_CREATE);
  adw_action_row_set_subtitle (ADW_ACTION_ROW (row), G_OBJECT_TYPE_NAME (window));

  btn = gtk_toggle_button_new ();
  gtk_button_set_icon_name (GTK_BUTTON (btn), "adw-adaptive-preview-symbolic");
  gtk_widget_set_tooltip_text (btn, _("Adaptive Preview"));
  gtk_widget_set_valign (btn, GTK_ALIGN_CENTER);
  gtk_widget_add_css_class (btn, "flat");
  g_object_bind_property (window, "adaptive-preview", btn, "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  adw_action_row_add_suffix (ADW_ACTION_ROW (row), btn);

  return row;
}

static void
color_scheme_changed_cb (AdwInspectorPage *self)
{
  AdwEnumListItem *item = adw_combo_row_get_selected_item (self->color_scheme_row);
  AdwSystemColorScheme color_scheme = adw_enum_list_item_get_value (item);

  adw_settings_override_color_scheme (self->settings, color_scheme);
}

static void
support_color_schemes_changed_cb (AdwInspectorPage *self)
{
  gboolean supports = adw_switch_row_get_active (self->support_color_schemes_row);

  adw_settings_override_system_supports_color_schemes (self->settings, supports);

  if (supports)
    color_scheme_changed_cb (self);
}

static void
high_contrast_changed_cb (AdwInspectorPage *self)
{
  gboolean hc = adw_switch_row_get_active (self->high_contrast_row);

  adw_settings_override_high_contrast (self->settings, hc);
}

static void
accent_color_changed_cb (AdwInspectorPage *self)
{
  AdwEnumListItem *item = adw_combo_row_get_selected_item (self->accent_color_row);
  AdwAccentColor accent_color = adw_enum_list_item_get_value (item);

  adw_settings_override_accent_color (self->settings, accent_color);
}

static void
support_accent_colors_changed_cb (AdwInspectorPage *self)
{
  gboolean supports = adw_switch_row_get_active (self->support_accent_colors_row);

  adw_settings_override_system_supports_accent_colors (self->settings, supports);

  if (supports)
    accent_color_changed_cb (self);
}

static char *
get_system_color_scheme_name (AdwEnumListItem *item,
                              gpointer         user_data)
{
  switch (adw_enum_list_item_get_value (item)) {
  case ADW_SYSTEM_COLOR_SCHEME_DEFAULT:
    return g_strdup (_("No Preference"));
  case ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK:
    return g_strdup (_("Prefer Dark"));
  case ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT:
    return g_strdup (_("Prefer Light"));
  default:
    return NULL;
  }
  return "";
}

static char *
get_accent_color_name (AdwEnumListItem *item,
                       gpointer         user_data)
{
  switch (adw_enum_list_item_get_value (item)) {
  case ADW_ACCENT_COLOR_BLUE:
    return g_strdup (_("Blue"));
  case ADW_ACCENT_COLOR_TEAL:
    return g_strdup (_("Teal"));
  case ADW_ACCENT_COLOR_GREEN:
    return g_strdup (_("Green"));
  case ADW_ACCENT_COLOR_YELLOW:
    return g_strdup (_("Yellow"));
  case ADW_ACCENT_COLOR_ORANGE:
    return g_strdup (_("Orange"));
  case ADW_ACCENT_COLOR_RED:
    return g_strdup (_("Red"));
  case ADW_ACCENT_COLOR_PINK:
    return g_strdup (_("Pink"));
  case ADW_ACCENT_COLOR_PURPLE:
    return g_strdup (_("Purple"));
  case ADW_ACCENT_COLOR_SLATE:
    return g_strdup (_("Slate"));
  default:
    g_assert_not_reached ();
  }
}

static void
selected_item_changed (AdwComboRow *row,
                       GParamSpec  *pspec,
                       GtkListItem *item)
{
  GtkWidget *checkmark = g_object_get_data (G_OBJECT (item), "checkmark");

  if (adw_combo_row_get_selected_item (row) == gtk_list_item_get_item (item))
    gtk_widget_set_opacity (checkmark, 1);
  else
    gtk_widget_set_opacity (checkmark, 0);
}

static void
color_snapshot_cb (AdwGizmo    *color,
                   GtkSnapshot *snapshot)
{
  GtkListItem *item = g_object_get_data (G_OBJECT (color), "item");
  AdwEnumListItem *enum_list_item;
  AdwAccentColor accent;
  GdkRGBA rgba;
  int w, h;

  w = gtk_widget_get_width (GTK_WIDGET (color));
  h = gtk_widget_get_height (GTK_WIDGET (color));

  enum_list_item = ADW_ENUM_LIST_ITEM (gtk_list_item_get_item (item));
  accent = adw_enum_list_item_get_value (enum_list_item);

  adw_accent_color_to_rgba (accent, &rgba);

  gtk_snapshot_append_color (snapshot, &rgba, &GRAPHENE_RECT_INIT (0, 0, w, h));
}

static void
accent_color_item_setup_cb (GtkSignalListItemFactory *factory,
                            GtkListItem              *item)
{
  GtkWidget *box, *color, *title, *checkmark;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  color = adw_gizmo_new_with_role ("color",
                                   GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                   NULL, NULL,
                                   color_snapshot_cb,
                                   NULL, NULL, NULL);
  gtk_widget_set_valign (color, GTK_ALIGN_CENTER);
  gtk_widget_set_overflow (color, GTK_OVERFLOW_HIDDEN);
  gtk_box_append (GTK_BOX (box), color);

  title = gtk_label_new (NULL);
  gtk_label_set_xalign (GTK_LABEL (title), 0.0);
  gtk_label_set_ellipsize (GTK_LABEL (title), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars (GTK_LABEL (title), 20);
  gtk_widget_set_valign (title, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (box), title);

  checkmark = g_object_new (GTK_TYPE_IMAGE,
                            "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                            "icon-name", "object-select-symbolic",
                            NULL);
  gtk_box_append (GTK_BOX (box), checkmark);

  g_object_set_data (G_OBJECT (item), "box", box);
  g_object_set_data (G_OBJECT (item), "color", color);
  g_object_set_data (G_OBJECT (item), "title", title);
  g_object_set_data (G_OBJECT (item), "checkmark", checkmark);

  g_object_set_data (G_OBJECT (color), "item", item);

  gtk_list_item_set_child (item, box);
}

static void
accent_color_item_bind_cb (GtkSignalListItemFactory *factory,
                           GtkListItem              *item,
                           AdwInspectorPage         *self)
{
  AdwComboRow *row = self->accent_color_row;
  GtkWidget *box, *color, *title, *checkmark;
  AdwEnumListItem *enum_list_item;
  char *accent_name;
  GtkWidget *popup;

  enum_list_item = ADW_ENUM_LIST_ITEM (gtk_list_item_get_item (item));
  accent_name = get_accent_color_name (enum_list_item, NULL);

  box = g_object_get_data (G_OBJECT (item), "box");
  color = g_object_get_data (G_OBJECT (item), "color");
  title = g_object_get_data (G_OBJECT (item), "title");
  checkmark = g_object_get_data (G_OBJECT (item), "checkmark");

  gtk_label_set_label (GTK_LABEL (title), accent_name);

  gtk_widget_queue_draw (color);

  popup = gtk_widget_get_ancestor (title, GTK_TYPE_POPOVER);
  if (popup && gtk_widget_is_ancestor (popup, GTK_WIDGET (row))) {
    gtk_box_set_spacing (GTK_BOX (box), 0);
    gtk_widget_set_visible (checkmark, TRUE);
    g_signal_connect (row, "notify::selected-item",
                      G_CALLBACK (selected_item_changed), item);
    selected_item_changed (row, NULL, item);
  } else {
    gtk_box_set_spacing (GTK_BOX (box), 6);
    gtk_widget_set_visible (checkmark, FALSE);
  }

  g_free (accent_name);
}

static void
accent_color_item_unbind_cb (GtkSignalListItemFactory *factory,
                             GtkListItem              *item,
                             AdwInspectorPage         *self)
{
  AdwComboRow *row = self->accent_color_row;

  g_signal_handlers_disconnect_by_func (row, selected_item_changed, item);
}

static void
adw_inspector_page_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwInspectorPage *self = ADW_INSPECTOR_PAGE (object);

  switch (prop_id) {
  case PROP_TITLE:
    /* Translators: The name of the library, not the stylesheet */
    g_value_set_string (value, _("Adwaita"));
    break;
  case PROP_OBJECT:
    g_value_set_object (value, self->object);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_inspector_page_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwInspectorPage *self = ADW_INSPECTOR_PAGE (object);

  switch (prop_id) {
  case PROP_OBJECT:
    g_set_object (&self->object, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_inspector_page_dispose (GObject *object)
{
  AdwInspectorPage *self = ADW_INSPECTOR_PAGE (object);

  if (self->settings) {
    adw_settings_end_override (self->settings);
    self->settings = NULL;
  }

  g_clear_object (&self->object);

  G_OBJECT_CLASS (adw_inspector_page_parent_class)->dispose (object);
}

static void
adw_inspector_page_realize (GtkWidget *widget)
{
  AdwInspectorPage *self = ADW_INSPECTOR_PAGE (widget);

  GTK_WIDGET_CLASS (adw_inspector_page_parent_class)->realize (widget);

  if (!self->realized) {
    gtk_icon_theme_add_resource_path (gtk_icon_theme_get_for_display (gtk_widget_get_display (GTK_WIDGET (self))),
                                    "/org/gnome/Adwaita/icons");
    self->realized = TRUE;
  }
}

static void
adw_inspector_page_class_init (AdwInspectorPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_inspector_page_get_property;
  object_class->set_property = adw_inspector_page_set_property;
  object_class->dispose = adw_inspector_page_dispose;

  widget_class->realize = adw_inspector_page_realize;

  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "Libadwaita",
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_OBJECT] =
    g_param_spec_object ("object", NULL, NULL,
                         G_TYPE_OBJECT,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-inspector-page.ui");

  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, support_color_schemes_row);
  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, color_scheme_row);
  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, high_contrast_row);
  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, support_accent_colors_row);
  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, accent_color_row);
  gtk_widget_class_bind_template_child (widget_class, AdwInspectorPage, adaptive_preview_group);

  gtk_widget_class_bind_template_callback (widget_class, get_system_color_scheme_name);
  gtk_widget_class_bind_template_callback (widget_class, get_accent_color_name);
  gtk_widget_class_bind_template_callback (widget_class, accent_color_item_setup_cb);
  gtk_widget_class_bind_template_callback (widget_class, accent_color_item_bind_cb);
  gtk_widget_class_bind_template_callback (widget_class, accent_color_item_unbind_cb);
  gtk_widget_class_bind_template_callback (widget_class, support_color_schemes_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, support_accent_colors_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, color_scheme_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, high_contrast_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, accent_color_changed_cb);
}

static void
adw_inspector_page_init (AdwInspectorPage *self)
{
  AdwSystemColorScheme color_scheme;
  AdwAccentColor accent_color;
  gboolean supports, hc;
  GtkFilter *filter;
  GtkFilterListModel *windows;

  self->settings = adw_settings_get_default ();

  gtk_widget_init_template (GTK_WIDGET (self));

  adw_settings_start_override (self->settings);

  color_scheme = adw_settings_get_color_scheme (self->settings);
  adw_combo_row_set_selected (self->color_scheme_row, color_scheme);

  supports = adw_settings_get_system_supports_color_schemes (self->settings);
  adw_switch_row_set_active (self->support_color_schemes_row, supports);

  hc = adw_settings_get_high_contrast (self->settings);
  adw_switch_row_set_active (self->high_contrast_row, hc);

  accent_color = adw_settings_get_accent_color (self->settings);
  adw_combo_row_set_selected (self->accent_color_row, accent_color);

  supports = adw_settings_get_system_supports_accent_colors (self->settings);
  adw_switch_row_set_active (self->support_accent_colors_row, supports);

  filter = GTK_FILTER (gtk_custom_filter_new ((GtkCustomFilterFunc) is_window_compatible, NULL, NULL));
  windows = gtk_filter_list_model_new (g_object_ref (gtk_window_get_toplevels ()), filter);

  adw_preferences_group_bind_model (self->adaptive_preview_group,
                                    G_LIST_MODEL (windows),
                                    (GtkListBoxCreateWidgetFunc) create_window_row_cb,
                                    self,
                                    NULL);

  g_object_unref (windows);
}
