/*
 * Copyright (c) 2013 Red Hat, Inc.
 * Copyright (C) 2019 Purism SPC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-header-bar.h"

#include "adw-enums.h"
#include "adw-gizmo-private.h"

/**
 * SECTION:adw-header-bar
 * @short_description: A box with a centered child.
 * @Title: AdwHeaderBar
 * @See_also: #GtkHeaderBar, #AdwViewSwitcherTitle, #AdwWindowTitle
 *
 * AdwHeaderBar is similar to #GtkHeaderBar but is designed to fix some of its
 * shortcomings for adaptive applications.
 *
 * # CSS nodes
 *
 * |[<!-- language="plain" -->
 * headerbar
 * ╰── windowhandle
 *     ╰── box
 *         ├── widget
 *         │   ╰── box.start
 *         │       ├── windowcontrols.start
 *         │       ╰── [other children]
 *         ├── [Title Widget]
 *         ╰── widget
 *             ╰── box.end
 *                 ├── [other children]
 *                 ╰── windowcontrols.end
 *
 * Since: 1.0
 */

/**
 * AdwCenteringPolicy:
 * @ADW_CENTERING_POLICY_LOOSE: Keep the title centered when possible
 * @ADW_CENTERING_POLICY_STRICT: Keep the title centered at all cost
 *
 * Since: 1.0
 */

#define MIN_TITLE_CHARS 5

#define MOBILE_WINDOW_WIDTH  480
#define MOBILE_WINDOW_HEIGHT 800

struct _AdwHeaderBar {
  GtkWidget parent_instance;

  GtkWidget *handle;
  GtkWidget *center_box;
  GtkWidget *start_bin;
  GtkWidget *end_bin;

  GtkWidget *start_box;
  GtkWidget *end_box;

  GtkWidget *title_label;
  GtkWidget *title_widget;

  GtkWidget *start_window_controls;
  GtkWidget *end_window_controls;

  char *decoration_layout;

  guint show_start_title_buttons : 1;
  guint show_end_title_buttons : 1;
  guint track_default_decoration : 1;

  AdwCenteringPolicy centering_policy;
  gboolean is_mobile_window;

  GtkSizeGroup *size_group;
};

enum {
  PROP_0,
  PROP_TITLE_WIDGET,
  PROP_SHOW_START_TITLE_BUTTONS,
  PROP_SHOW_END_TITLE_BUTTONS,
  PROP_DECORATION_LAYOUT,
  PROP_CENTERING_POLICY,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP] = { NULL, };

static void adw_header_bar_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwHeaderBar, adw_header_bar, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                                                adw_header_bar_buildable_init));

static GtkBuildableIface *parent_buildable_iface;

static void
create_start_window_controls (AdwHeaderBar *self)
{
  GtkWidget *controls = gtk_window_controls_new (GTK_PACK_START);
  g_object_bind_property (self, "decoration-layout",
                          controls, "decoration-layout",
                          G_BINDING_SYNC_CREATE);
  g_object_bind_property (controls, "empty",
                          controls, "visible",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);
  gtk_box_prepend (GTK_BOX (self->start_box), controls);
  self->start_window_controls = controls;
}

static void
create_end_window_controls (AdwHeaderBar *self)
{
  GtkWidget *controls = gtk_window_controls_new (GTK_PACK_END);
  g_object_bind_property (self, "decoration-layout",
                          controls, "decoration-layout",
                          G_BINDING_SYNC_CREATE);
  g_object_bind_property (controls, "empty",
                          controls, "visible",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);
  gtk_box_append (GTK_BOX (self->end_box), controls);
  self->end_window_controls = controls;
}

static void
update_title (AdwHeaderBar *self)
{
  GtkRoot *root;
  const char *title = NULL;

  if (!self->title_label)
    return;

  root = gtk_widget_get_root (GTK_WIDGET (self));

  if (GTK_IS_WINDOW (root))
    title = gtk_window_get_title (GTK_WINDOW (root));

  if (!title)
    title = g_get_application_name ();

  if (!title)
    title = g_get_prgname ();

  gtk_label_set_text (GTK_LABEL (self->title_label), title);
}


static void
construct_title_label (AdwHeaderBar *self)
{
  GtkWidget *label;

  g_assert (self->title_label == NULL);

  label = gtk_label_new (NULL);
  gtk_widget_add_css_class (label, "title");
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_label_set_wrap (GTK_LABEL (label), FALSE);
  gtk_label_set_single_line_mode (GTK_LABEL (label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_width_chars (GTK_LABEL (label), MIN_TITLE_CHARS);
  gtk_center_box_set_center_widget (GTK_CENTER_BOX (self->center_box), label);

  self->title_label = label;

  update_title (self);
}

static void
adw_header_bar_root (GtkWidget *widget)
{
  GtkWidget *root;

  GTK_WIDGET_CLASS (adw_header_bar_parent_class)->root (widget);

  root = GTK_WIDGET (gtk_widget_get_root (widget));

  if (GTK_IS_WINDOW (root))
    g_signal_connect_swapped (root, "notify::title",
                              G_CALLBACK (update_title), widget);

  update_title (ADW_HEADER_BAR (widget));
}

static void
adw_header_bar_unroot (GtkWidget *widget)
{
  g_signal_handlers_disconnect_by_func (gtk_widget_get_root (widget),
                                        update_title, widget);

  GTK_WIDGET_CLASS (adw_header_bar_parent_class)->unroot (widget);
}

static void
adw_header_bar_dispose (GObject *object)
{
  AdwHeaderBar *self = ADW_HEADER_BAR (object);

  self->title_widget = NULL;
  self->title_label = NULL;
  self->start_box = NULL;
  self->end_box = NULL;
  self->start_bin = NULL;
  self->end_bin = NULL;

  g_clear_object (&self->size_group);
  g_clear_pointer (&self->handle, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_header_bar_parent_class)->dispose (object);
}

static void
adw_header_bar_finalize (GObject *object)
{
  AdwHeaderBar *self = ADW_HEADER_BAR (object);

  g_clear_pointer (&self->decoration_layout, g_free);

  G_OBJECT_CLASS (adw_header_bar_parent_class)->finalize (object);
}

static void
adw_header_bar_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdwHeaderBar *self = ADW_HEADER_BAR (object);

  switch (prop_id) {
  case PROP_TITLE_WIDGET:
    g_value_set_object (value, self->title_widget);
    break;
  case PROP_SHOW_START_TITLE_BUTTONS:
    g_value_set_boolean (value, adw_header_bar_get_show_start_title_buttons (self));
    break;
  case PROP_SHOW_END_TITLE_BUTTONS:
    g_value_set_boolean (value, adw_header_bar_get_show_end_title_buttons (self));
    break;
  case PROP_DECORATION_LAYOUT:
    g_value_set_string (value, adw_header_bar_get_decoration_layout (self));
    break;
  case PROP_CENTERING_POLICY:
    g_value_set_enum (value, adw_header_bar_get_centering_policy (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_header_bar_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdwHeaderBar *self = ADW_HEADER_BAR (object);

  switch (prop_id) {
  case PROP_TITLE_WIDGET:
    adw_header_bar_set_title_widget (self, g_value_get_object (value));
    break;
  case PROP_SHOW_START_TITLE_BUTTONS:
    adw_header_bar_set_show_start_title_buttons (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_END_TITLE_BUTTONS:
    adw_header_bar_set_show_end_title_buttons (self, g_value_get_boolean (value));
    break;
  case PROP_DECORATION_LAYOUT:
    adw_header_bar_set_decoration_layout (self, g_value_get_string (value));
    break;
  case PROP_CENTERING_POLICY:
    adw_header_bar_set_centering_policy (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_header_bar_pack (AdwHeaderBar *self,
                     GtkWidget    *widget,
                     GtkPackType   pack_type)
{
  if (pack_type == GTK_PACK_START)
    gtk_box_append (GTK_BOX (self->start_box), widget);
  else if (pack_type == GTK_PACK_END) {
    gtk_box_append (GTK_BOX (self->end_box), widget);
    gtk_box_reorder_child_after (GTK_BOX (self->end_box), widget, NULL);
  }
}

static GtkSizeRequestMode
adw_header_bar_get_request_mode (GtkWidget *widget)
{
  GtkWidget *w;
  int wfh = 0, hfw = 0;

  for (w = gtk_widget_get_first_child (widget);
       w != NULL;
       w = gtk_widget_get_next_sibling (w)) {
    GtkSizeRequestMode mode = gtk_widget_get_request_mode (w);

    switch (mode) {
    case GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH:
      hfw ++;
      break;
    case GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT:
      wfh ++;
      break;
    case GTK_SIZE_REQUEST_CONSTANT_SIZE:
    default:
      break;
    }
  }

  if (hfw == 0 && wfh == 0)
    return GTK_SIZE_REQUEST_CONSTANT_SIZE;
  else
    return wfh > hfw ?
        GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT :
        GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}


static void
adw_header_bar_class_init (AdwHeaderBarClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  object_class->dispose = adw_header_bar_dispose;
  object_class->finalize = adw_header_bar_finalize;
  object_class->get_property = adw_header_bar_get_property;
  object_class->set_property = adw_header_bar_set_property;

  widget_class->root = adw_header_bar_root;
  widget_class->unroot = adw_header_bar_unroot;
  widget_class->get_request_mode = adw_header_bar_get_request_mode;

  props[PROP_TITLE_WIDGET] =
    g_param_spec_object ("title-widget",
                         _("Title Widget"),
                         _("Title widget to display"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE|G_PARAM_STATIC_STRINGS);

  /**
   * AdwHeaderBar:show-start-title-buttons:
   *
   * Whether to show title buttons like close, minimize, maximize.
   *
   * Which buttons are actually shown and where is determined
   * by the #AdwHeaderBar:decoration-layout property, and by
   * the state of the window (e.g. a close button will not be
   * shown if the window can't be closed).
   *
   * Since: 1.0
   */
  props[PROP_SHOW_START_TITLE_BUTTONS] =
    g_param_spec_boolean ("show-start-title-buttons",
                          _("Show start title buttons"),
                          _("Whether to show start title buttons"),
                          TRUE,
                          G_PARAM_READWRITE|G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwHeaderBar:show-end-title-buttons:
   *
   * Whether to show title buttons like close, minimize, maximize.
   *
   * Which buttons are actually shown and where is determined
   * by the #AdwHeaderBar:decoration-layout property, and by
   * the state of the window (e.g. a close button will not be
   * shown if the window can't be closed).
   *
   * Since: 1.0
   */
  props[PROP_SHOW_END_TITLE_BUTTONS] =
    g_param_spec_boolean ("show-end-title-buttons",
                          _("Show end title buttons"),
                          _("Whether to show end title buttons"),
                          TRUE,
                          G_PARAM_READWRITE|G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwHeaderBar:decoration-layout:
   *
   * The decoration layout for buttons. If this property is
   * not set, the #GtkSettings:gtk-decoration-layout setting
   * is used.
   *
   * See adw_header_bar_set_decoration_layout() for information
   * about the format of this string.
   *
   * Since: 1.0
   */
  props[PROP_DECORATION_LAYOUT] =
    g_param_spec_string ("decoration-layout",
                         _("Decoration Layout"),
                         _("The layout for window decorations"),
                         NULL,
                         G_PARAM_READWRITE);

  props[PROP_CENTERING_POLICY] =
    g_param_spec_enum ("centering-policy",
                       _("Centering policy"),
                       _("The policy to horizontally align the center widget"),
                       ADW_TYPE_CENTERING_POLICY, ADW_CENTERING_POLICY_LOOSE,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "headerbar");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_header_bar_init (AdwHeaderBar *self)
{
  self->title_widget = NULL;
  self->decoration_layout = NULL;
  self->show_start_title_buttons = TRUE;
  self->show_end_title_buttons = TRUE;

  self->handle = gtk_window_handle_new ();
  gtk_widget_set_parent (self->handle, GTK_WIDGET (self));

  self->center_box = gtk_center_box_new ();
  gtk_window_handle_set_child (GTK_WINDOW_HANDLE (self->handle), self->center_box);

  self->start_bin = adw_gizmo_new ("widget", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_layout_manager (self->start_bin, gtk_bin_layout_new ());
  gtk_center_box_set_start_widget (GTK_CENTER_BOX (self->center_box), self->start_bin);

  self->end_bin = adw_gizmo_new ("widget", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_layout_manager (self->end_bin, gtk_bin_layout_new ());
  gtk_center_box_set_end_widget (GTK_CENTER_BOX (self->center_box), self->end_bin);

  self->start_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_halign (self->start_box, GTK_ALIGN_START);
  gtk_widget_add_css_class (self->start_box, "start");
  gtk_widget_set_parent (self->start_box, self->start_bin);

  self->end_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_halign (self->end_box, GTK_ALIGN_END);
  gtk_widget_add_css_class (self->end_box, "end");
  gtk_widget_set_parent (self->end_box, self->end_bin);

  self->size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  construct_title_label (self);
  create_start_window_controls (self);
  create_end_window_controls (self);
}

static void
adw_header_bar_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder   *builder,
                                    GObject      *child,
                                    const char   *type)
{
  if (g_strcmp0 (type, "title") == 0)
    adw_header_bar_set_title_widget (ADW_HEADER_BAR (buildable), GTK_WIDGET (child));
  else if (g_strcmp0 (type, "start") == 0)
    adw_header_bar_pack_start (ADW_HEADER_BAR (buildable), GTK_WIDGET (child));
  else if (g_strcmp0 (type, "end") == 0)
    adw_header_bar_pack_end (ADW_HEADER_BAR (buildable), GTK_WIDGET (child));
  else if (type == NULL && GTK_IS_WIDGET (child))
    adw_header_bar_pack_start (ADW_HEADER_BAR (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_header_bar_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = adw_header_bar_buildable_add_child;

  parent_buildable_iface = g_type_interface_peek_parent (iface);
}

/**
 * adw_header_bar_new:
 *
 * Creates a new #AdwHeaderBar widget.
 *
 * Returns: a new #AdwHeaderBar
 *
 * Since: 1.0
 */
GtkWidget *
adw_header_bar_new (void)
{
  return GTK_WIDGET (g_object_new (ADW_TYPE_HEADER_BAR, NULL));
}

/**
 * adw_header_bar_pack_start:
 * @self: A #AdwHeaderBar
 * @child: the #GtkWidget to be added to @self
 *
 * Adds @child to @self, packed with reference to the
 * start of the @self.
 *
 * Since: 1.0
 */
void
adw_header_bar_pack_start (AdwHeaderBar *self,
                           GtkWidget    *child)
{
  adw_header_bar_pack (self, child, GTK_PACK_START);
}

/**
 * adw_header_bar_pack_end:
 * @self: A #AdwHeaderBar
 * @child: the #GtkWidget to be added to @self
 *
 * Adds @child to @self, packed with reference to the
 * end of the @self.
 *
 * Since: 1.0
 */
void
adw_header_bar_pack_end (AdwHeaderBar *self,
                         GtkWidget    *child)
{
  adw_header_bar_pack (self, child, GTK_PACK_END);
}

/**
 * adw_header_bar_set_title_widget:
 * @self: a #AdwHeaderBar
 * @title_widget: (allow-none): a widget to use for a title
 *
 * Sets the title for the #AdwHeaderBar.
 *
 * When set to %NULL, the headerbar will display the title of the window it is
 * contained in.
 *
 * The title should help a user identify the current view. To achieve the same
 * style as the builtin title, use the “title” style class.
 *
 * You should set the title widget to %NULL, for the window title label to be
 * visible again.
 *
 * Since: 1.0
 */
void
adw_header_bar_set_title_widget (AdwHeaderBar *self,
                                 GtkWidget    *title_widget)
{
  g_return_if_fail (ADW_IS_HEADER_BAR (self));

  if (title_widget)
    g_return_if_fail (GTK_IS_WIDGET (title_widget));

  /* No need to do anything if the title widget stays the same */
  if (self->title_widget == title_widget)
    return;

  gtk_center_box_set_center_widget (GTK_CENTER_BOX (self->center_box), NULL);
  self->title_widget = NULL;

  if (title_widget != NULL) {
    self->title_widget = title_widget;

    gtk_center_box_set_center_widget (GTK_CENTER_BOX (self->center_box), title_widget);

    self->title_label = NULL;
  } else if (self->title_label == NULL)
    construct_title_label (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE_WIDGET]);
}

/**
 * adw_header_bar_get_title_widget:
 * @self: a #AdwHeaderBar
 *
 * Retrieves the title widget widget of the header. See
 * adw_header_bar_set_title_widget().
 *
 * Returns: (nullable) (transfer none): the title widget
 *    of the header, or %NULL if none has been set explicitly.
 *
 * Since: 1.0
 */
GtkWidget *
adw_header_bar_get_title_widget (AdwHeaderBar *self)
{
  g_return_val_if_fail (ADW_IS_HEADER_BAR (self), NULL);

  return self->title_widget;
}

/**
 * adw_header_bar_remove:
 * @self: a #AdwHeaderBar
 * @child: the child to remove
 *
 * Removes a child from @self, after it has been added
 * with adw_header_bar_pack_start(), adw_header_bar_pack_end()
 * or adw_header_bar_set_title_widget().
 *
 * Since: 1.0
 */
void
adw_header_bar_remove (AdwHeaderBar *self,
                       GtkWidget    *child)
{
  GtkWidget *parent;

  g_return_if_fail (ADW_IS_HEADER_BAR (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  parent = gtk_widget_get_parent (child);

  if (parent == self->start_box)
    gtk_box_remove (GTK_BOX (self->start_box), child);
  else if (parent == self->end_box)
    gtk_box_remove (GTK_BOX (self->end_box), child);
  else if (parent == self->center_box)
    gtk_center_box_set_center_widget (GTK_CENTER_BOX (self->center_box), NULL);
}

/**
 * adw_header_bar_get_show_start_title_buttons:
 * @self: a #AdwHeaderBar
 *
 * Returns whether this header bar shows the standard window
 * title buttons.
 *
 * Returns: %TRUE if title buttons are shown
 *
 * Since: 1.0
 */
gboolean
adw_header_bar_get_show_start_title_buttons (AdwHeaderBar *self)
{
  g_return_val_if_fail (ADW_IS_HEADER_BAR (self), FALSE);

  return self->show_start_title_buttons;
}

/**
 * adw_header_bar_set_show_start_title_buttons:
 * @self: a #AdwHeaderBar
 * @setting: %TRUE to show standard title buttons
 *
 * Sets whether this header bar shows the standard window title buttons
 * including close, maximize, and minimize.
 *
 * Since: 1.0
 */
void
adw_header_bar_set_show_start_title_buttons (AdwHeaderBar *self,
                                             gboolean      setting)
{
  g_return_if_fail (ADW_IS_HEADER_BAR (self));

  setting = setting != FALSE;

  if (self->show_start_title_buttons == setting)
    return;

  self->show_start_title_buttons = setting;

  if (self->start_box) {
    if (setting) {
      create_start_window_controls (self);
    } else if (self->start_box && self->start_window_controls) {
      gtk_box_remove (GTK_BOX (self->start_box), self->start_window_controls);
      self->start_window_controls = NULL;
    }
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_START_TITLE_BUTTONS]);
}

/**
 * adw_header_bar_get_show_end_title_buttons:
 * @self: a #AdwHeaderBar
 *
 * Returns whether this header bar shows the standard window
 * title buttons.
 *
 * Returns: %TRUE if title buttons are shown
 *
 * Since: 1.0
 */
gboolean
adw_header_bar_get_show_end_title_buttons (AdwHeaderBar *self)
{
  g_return_val_if_fail (ADW_IS_HEADER_BAR (self), FALSE);

  return self->show_end_title_buttons;
}

/**
 * adw_header_bar_set_show_end_title_buttons:
 * @self: a #AdwHeaderBar
 * @setting: %TRUE to show standard title buttons
 *
 * Sets whether this header bar shows the standard window title buttons
 * including close, maximize, and minimize.
 *
 * Since: 1.0
 */
void
adw_header_bar_set_show_end_title_buttons (AdwHeaderBar *self,
                                           gboolean      setting)
{
  g_return_if_fail (ADW_IS_HEADER_BAR (self));

  setting = setting != FALSE;

  if (self->show_end_title_buttons == setting)
    return;

  self->show_end_title_buttons = setting;

  if (self->end_box) {
    if (setting) {
      create_end_window_controls (self);
    } else if (self->end_box && self->end_window_controls) {
      gtk_box_remove (GTK_BOX (self->end_box), self->end_window_controls);
      self->end_window_controls = NULL;
    }
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_END_TITLE_BUTTONS]);
}

/**
 * adw_header_bar_set_decoration_layout:
 * @self: a #AdwHeaderBar
 * @layout: (nullable): a decoration layout, or %NULL to unset the layout
 *
 * Sets the decoration layout for this header bar, overriding
 * the #GtkSettings:gtk-decoration-layout setting.
 *
 * There can be valid reasons for overriding the setting, such
 * as a header bar design that does not allow for buttons to take
 * room on the right, or only offers room for a single close button.
 * Split header bars are another example for overriding the
 * setting.
 *
 * The format of the string is button names, separated by commas.
 * A colon separates the buttons that should appear on the left
 * from those on the right. Recognized button names are minimize,
 * maximize, close and icon (the window icon).
 *
 * For example, “icon:minimize,maximize,close” specifies an icon
 * on the left, and minimize, maximize and close buttons on the right.
 *
 * Since: 1.0
 */
void
adw_header_bar_set_decoration_layout (AdwHeaderBar *self,
                                      const char   *layout)
{
  g_return_if_fail (ADW_IS_HEADER_BAR (self));

  g_clear_pointer (&self->decoration_layout, g_free);
  self->decoration_layout = g_strdup (layout);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DECORATION_LAYOUT]);
}

/**
 * adw_header_bar_get_decoration_layout:
 * @self: a #AdwHeaderBar
 *
 * Gets the decoration layout set with
 * adw_header_bar_set_decoration_layout().
 *
 * Returns: (nullable): the decoration layout
 *
 * Since: 1.0
 */
const char *
adw_header_bar_get_decoration_layout (AdwHeaderBar *self)
{
  g_return_val_if_fail (ADW_IS_HEADER_BAR (self), NULL);

  return self->decoration_layout;
}

/**
 * adw_header_bar_get_centering_policy:
 * @self: a #AdwHeaderBar
 *
 * Gets the policy @self follows to horizontally align its center widget.
 *
 * Returns: the centering policy
 *
 * Since: 1.0
 */
AdwCenteringPolicy
adw_header_bar_get_centering_policy (AdwHeaderBar *self)
{
  g_return_val_if_fail (ADW_IS_HEADER_BAR (self), ADW_CENTERING_POLICY_LOOSE);

  return self->centering_policy;
}

/**
 * adw_header_bar_set_centering_policy:
 * @self: a #AdwHeaderBar
 * @centering_policy: the centering policy
 *
 * Sets the policy @self must follow to horizontally align its center widget.
 *
 * Since: 1.0
 */
void
adw_header_bar_set_centering_policy (AdwHeaderBar       *self,
                                     AdwCenteringPolicy  centering_policy)
{
  g_return_if_fail (ADW_IS_HEADER_BAR (self));

  if (self->centering_policy == centering_policy)
    return;

  self->centering_policy = centering_policy;

  if (self->centering_policy == ADW_CENTERING_POLICY_STRICT) {
    gtk_size_group_add_widget (self->size_group, self->start_bin);
    gtk_size_group_add_widget (self->size_group, self->end_bin);
  } else {
    gtk_size_group_remove_widget (self->size_group, self->start_bin);
    gtk_size_group_remove_widget (self->size_group, self->end_bin);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CENTERING_POLICY]);
}
