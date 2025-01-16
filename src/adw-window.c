/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-window.h"

#include "adw-adaptive-preview-private.h"
#include "adw-breakpoint-bin-private.h"
#include "adw-dialog-host-private.h"
#include "adw-dialog-private.h"
#include "adw-gizmo-private.h"
#include "adw-main-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwWindow:
 *
 * A freeform window.
 *
 * <picture>
 *   <source srcset="window-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="window.png" alt="window">
 * </picture>
 *
 * The `AdwWindow` widget is a subclass of [class@Gtk.Window] which has no
 * titlebar area. Instead, [class@ToolbarView] can be used together with
 * [class@HeaderBar] or [class@Gtk.HeaderBar] as follows:
 *
 * ```xml
 * <object class="AdwWindow">
 *   <property name="content">
 *     <object class="AdwToolbarView">
 *       <child type="top">
 *         <object class="AdwHeaderBar"/>
 *       </child>
 *       <property name="content">
 *         <!-- ... -->
 *       </property>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * Using [property@Gtk.Window:titlebar] or [property@Gtk.Window:child]
 * is not supported and will result in a crash. Use [property@Window:content]
 * instead.
 *
 * ## Dialogs
 *
 * `AdwWindow` can contain [class@Dialog]. Use [method@Dialog.present] with the
 * window or a widget within a window to show a dialog.
 *
 * ## Breakpoints
 *
 * `AdwWindow` can be used with [class@Breakpoint] the same way as
 * [class@BreakpointBin]. Refer to that widget's documentation for details.
 *
 * Example:
 *
 * ```xml
 * <object class="AdwWindow">
 *   <property name="content">
 *     <object class="AdwToolbarView">
 *       <child type="top">
 *         <object class="AdwHeaderBar"/>
 *       </child>
 *       <property name="content">
 *         <!-- ... -->
 *       </property>
 *       <child type="bottom">
 *         <object class="GtkActionBar" id="bottom_bar">
 *           <property name="revealed">True</property>
 *           <property name="visible">False</property>
 *         </object>
 *       </child>
 *     </object>
 *   </property>
 *   <child>
 *     <object class="AdwBreakpoint">
 *       <condition>max-width: 500px</condition>
 *       <setter object="bottom_bar" property="visible">True</setter>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * When breakpoints are used, the minimum size must be larger than the smallest
 * UI state. `AdwWindow` defaults to the minimum size of 360×200 px. If that's
 * too small, set the [property@Gtk.Widget:width-request] and
 * [property@Gtk.Widget:height-request] properties manually.
 *
 * ## Adaptive Preview
 *
 * `AdwWindow` has a debug tool called adaptive preview. It can be opened from
 * GTK Inspector or by pressing <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>M</kbd>,
 * and controlled via the [property@Window:adaptive-preview] property.
 */

typedef struct
{
  GtkWidget *titlebar;
  GtkWidget *bin;
  GtkWidget *dialog_host;
  GtkWidget *adaptive_preview;
} AdwWindowPrivate;

static void adw_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwWindow, adw_window, GTK_TYPE_WINDOW,
                         G_ADD_PRIVATE (AdwWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_window_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  PROP_CURRENT_BREAKPOINT,
  PROP_DIALOGS,
  PROP_VISIBLE_DIALOG,
  PROP_ADAPTIVE_PREVIEW,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
notify_current_breakpoint_cb (AdwWindow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CURRENT_BREAKPOINT]);
}

static void
notify_visible_dialog_cb (AdwWindow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_DIALOG]);
}

static void
adaptive_preview_exit_cb (AdwWindow *self)
{
  adw_window_set_adaptive_preview (self, FALSE);
}

static gboolean
toggle_adaptive_preview_cb (AdwWindow *self)
{
  if (!adw_get_inspector_keybinding_enabled ())
    return GDK_EVENT_PROPAGATE;

  gboolean open = adw_window_get_adaptive_preview (self);

  adw_window_set_adaptive_preview (self, !open);

  return GDK_EVENT_STOP;
}

static void
adw_window_size_allocate (GtkWidget *widget,
                          int        width,
                          int        height,
                          int        baseline)
{
  AdwWindow *self = ADW_WINDOW (widget);
  AdwWindowPrivate *priv = adw_window_get_instance_private (self);
  GtkWidget *child;

  /* We don't want to allow any other titlebar */
  if (gtk_window_get_titlebar (GTK_WINDOW (self)) != priv->titlebar)
    g_error ("gtk_window_set_titlebar() is not supported for AdwWindow");

  child = gtk_window_get_child (GTK_WINDOW (self));
  if (child != priv->dialog_host && child != priv->adaptive_preview)
    g_error ("gtk_window_set_child() is not supported for AdwWindow");

  GTK_WIDGET_CLASS (adw_window_parent_class)->size_allocate (widget,
                                                             width,
                                                             height,
                                                             baseline);
}

static void
adw_window_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdwWindow *self = ADW_WINDOW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adw_window_get_content (self));
    break;
  case PROP_CURRENT_BREAKPOINT:
    g_value_set_object (value, adw_window_get_current_breakpoint (self));
    break;
  case PROP_DIALOGS:
    g_value_take_object (value, adw_window_get_dialogs (self));
    break;
  case PROP_VISIBLE_DIALOG:
    g_value_set_object (value, adw_window_get_visible_dialog (self));
    break;
  case PROP_ADAPTIVE_PREVIEW:
    g_value_set_boolean (value, adw_window_get_adaptive_preview (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_window_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdwWindow *self = ADW_WINDOW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    adw_window_set_content (self, g_value_get_object (value));
    break;
  case PROP_ADAPTIVE_PREVIEW:
    adw_window_set_adaptive_preview (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_window_class_init (AdwWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_window_get_property;
  object_class->set_property = adw_window_set_property;
  widget_class->size_allocate = adw_window_size_allocate;

  /**
   * AdwWindow:content:
   *
   * The content widget.
   *
   * This property should always be used instead of [property@Gtk.Window:child].
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWindow:current-breakpoint:
   *
   * The current breakpoint.
   *
   * Since: 1.4
   */
  props[PROP_CURRENT_BREAKPOINT] =
    g_param_spec_object ("current-breakpoint", NULL, NULL,
                         ADW_TYPE_BREAKPOINT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwWindow:dialogs:
   *
   * The open dialogs.
   *
   * Since: 1.5
   */
  props[PROP_DIALOGS] =
    g_param_spec_object ("dialogs", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwWindow:visible-dialog:
   *
   * The currently visible dialog
   *
   * Since: 1.5
   */
  props[PROP_VISIBLE_DIALOG] =
    g_param_spec_object ("visible-dialog", NULL, NULL,
                         ADW_TYPE_DIALOG,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwWindow:adaptive-preview:
   *
   * Whether adaptive preview is currently open.
   *
   * Adaptive preview is a debugging tool used for testing the window
   * contents at specific screen sizes, simulating mobile environment.
   *
   * Adaptive preview can always be accessed from inspector. This function
   * allows applications to open it manually.
   *
   * Most applications should not use this property.
   *
   * Since: 1.7
   */
  props[PROP_ADAPTIVE_PREVIEW] =
    g_param_spec_boolean ("adaptive-preview", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_window_init (AdwWindow *self)
{
  AdwWindowPrivate *priv = adw_window_get_instance_private (self);
  GtkEventController *shortcut_controller;
  GtkShortcut *shortcut;

  priv->titlebar = adw_gizmo_new_with_role ("nothing", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                            NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_visible (priv->titlebar, FALSE);
  gtk_window_set_titlebar (GTK_WINDOW (self), priv->titlebar);

  priv->dialog_host = adw_dialog_host_new ();
  gtk_window_set_child (GTK_WINDOW (self), priv->dialog_host);
  adw_dialog_host_set_proxy (ADW_DIALOG_HOST (priv->dialog_host), GTK_WIDGET (self));

  priv->bin = adw_breakpoint_bin_new ();
  adw_breakpoint_bin_set_warning_widget (ADW_BREAKPOINT_BIN (priv->bin),
                                         GTK_WIDGET (self));
  adw_dialog_host_set_child (ADW_DIALOG_HOST (priv->dialog_host), priv->bin);

  g_signal_connect_swapped (priv->bin, "notify::current-breakpoint",
                            G_CALLBACK (notify_current_breakpoint_cb), self);
  g_signal_connect_swapped (priv->dialog_host, "notify::visible-dialog",
                            G_CALLBACK (notify_visible_dialog_cb), self);

  gtk_widget_set_size_request (GTK_WIDGET (self), 360, 200);

  if (adw_is_adaptive_preview ())
    adw_window_set_adaptive_preview (self, TRUE);

  shortcut = gtk_shortcut_new (gtk_keyval_trigger_new (GDK_KEY_M, GDK_CONTROL_MASK | GDK_SHIFT_MASK),
                               gtk_callback_action_new ((GtkShortcutFunc) toggle_adaptive_preview_cb, self, NULL));

  shortcut_controller = gtk_shortcut_controller_new ();
  gtk_shortcut_controller_set_scope (GTK_SHORTCUT_CONTROLLER (shortcut_controller), GTK_SHORTCUT_SCOPE_GLOBAL);
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (shortcut_controller), shortcut);
  gtk_widget_add_controller (GTK_WIDGET (self), shortcut_controller);
}

static void
adw_window_buildable_add_child (GtkBuildable *buildable,
                                GtkBuilder   *builder,
                                GObject      *child,
                                const char   *type)
{
  if (!g_strcmp0 (type, "titlebar"))
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
  else if (GTK_IS_WIDGET (child))
    adw_window_set_content (ADW_WINDOW (buildable), GTK_WIDGET (child));
  else if (ADW_IS_BREAKPOINT (child))
    adw_window_add_breakpoint (ADW_WINDOW (buildable),
                               g_object_ref (ADW_BREAKPOINT (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_window_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_window_buildable_add_child;
}

/**
 * adw_window_new:
 *
 * Creates a new `AdwWindow`.
 *
 * Returns: the newly created `AdwWindow`
 */
GtkWidget *
adw_window_new (void)
{
  return g_object_new (ADW_TYPE_WINDOW, NULL);
}

/**
 * adw_window_set_content:
 * @self: a window
 * @content: (nullable): the content widget
 *
 * Sets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.set_child].
 */
void
adw_window_set_content (AdwWindow *self,
                        GtkWidget *content)
{
  AdwWindowPrivate *priv;

  g_return_if_fail (ADW_IS_WINDOW (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  priv = adw_window_get_instance_private (self);

  if (adw_window_get_content (self) == content)
    return;

  if (content)
    g_return_if_fail (gtk_widget_get_parent (content) == NULL);

  adw_breakpoint_bin_set_child (ADW_BREAKPOINT_BIN (priv->bin), content);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_window_get_content:
 * @self: a window
 *
 * Gets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.get_child].
 *
 * Returns: (nullable) (transfer none): the content widget of @self
 */
GtkWidget *
adw_window_get_content (AdwWindow *self)
{
  AdwWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_WINDOW (self), NULL);

  priv = adw_window_get_instance_private (self);

  return adw_breakpoint_bin_get_child (ADW_BREAKPOINT_BIN (priv->bin));
}

/**
 * adw_window_add_breakpoint:
 * @self: a window
 * @breakpoint: (transfer full): the breakpoint to add
 *
 * Adds @breakpoint to @self.
 *
 * Since: 1.4
 */
void
adw_window_add_breakpoint (AdwWindow     *self,
                           AdwBreakpoint *breakpoint)
{
  AdwWindowPrivate *priv;

  g_return_if_fail (ADW_IS_WINDOW (self));
  g_return_if_fail (ADW_IS_BREAKPOINT (breakpoint));

  priv = adw_window_get_instance_private (self);

  adw_breakpoint_bin_add_breakpoint (ADW_BREAKPOINT_BIN (priv->bin), breakpoint);
}

/**
 * adw_window_get_current_breakpoint:
 * @self: a window
 *
 * Gets the current breakpoint.
 *
 * Returns: (nullable) (transfer none): the current breakpoint
 *
 * Since: 1.4
 */
AdwBreakpoint *
adw_window_get_current_breakpoint (AdwWindow *self)
{
  AdwWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_WINDOW (self), NULL);

  priv = adw_window_get_instance_private (self);

  return adw_breakpoint_bin_get_current_breakpoint (ADW_BREAKPOINT_BIN (priv->bin));
}

/**
 * adw_window_get_dialogs:
 * @self: a window
 *
 * Returns a [iface@Gio.ListModel] that contains the open dialogs of @self.
 *
 * This can be used to keep an up-to-date view.
 *
 * Returns: (transfer full): a list model for the dialogs of @self
 *
 * Since: 1.5
 */
GListModel *
adw_window_get_dialogs (AdwWindow *self)
{
  AdwWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_WINDOW (self), NULL);

  priv = adw_window_get_instance_private (self);

  return adw_dialog_host_get_dialogs (ADW_DIALOG_HOST (priv->dialog_host));
}

/**
 * adw_window_get_visible_dialog:
 * @self: a window
 *
 * Returns the currently visible dialog in @self, if there's one.
 *
 * Returns: (transfer none) (nullable): the visible dialog
 *
 * Since: 1.5
 */
AdwDialog *
adw_window_get_visible_dialog (AdwWindow *self)
{
  AdwWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_WINDOW (self), NULL);

  priv = adw_window_get_instance_private (self);

  return adw_dialog_host_get_visible_dialog (ADW_DIALOG_HOST (priv->dialog_host));
}

/**
 * adw_window_get_adaptive_preview:
 * @self: a window
 *
 * Gets whether adaptive preview for @self is currently open.
 *
 * Returns: whether adaptive preview is open.
 *
 * Since: 1.7
 */
gboolean
adw_window_get_adaptive_preview (AdwWindow *self)
{
  AdwWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_WINDOW (self), FALSE);

  priv = adw_window_get_instance_private (self);

  return priv->adaptive_preview != NULL;
}

/**
 * adw_window_set_adaptive_preview:
 * @self: a window
 * @adaptive_preview: whether to open adaptive preview
 *
 * Sets whether adaptive preview for @self is currently open.
 *
 * Adaptive preview is a debugging tool used for testing the window
 * contents at specific screen sizes, simulating mobile environment.
 *
 * Adaptive preview can always be accessed from inspector. This function
 * allows applications to open it manually.
 *
 * Most applications should not use this function.
 *
 * Since: 1.7
 */
void
adw_window_set_adaptive_preview (AdwWindow *self,
                                 gboolean   adaptive_preview)
{
  AdwWindowPrivate *priv;

  g_return_if_fail (ADW_IS_WINDOW (self));

  priv = adw_window_get_instance_private (self);

  if (adaptive_preview == adw_window_get_adaptive_preview (self))
    return;

  g_object_ref (priv->dialog_host);

  if (adaptive_preview) {
    priv->adaptive_preview = adw_adaptive_preview_new ();
    gtk_window_set_child (GTK_WINDOW (self), priv->adaptive_preview);
    g_signal_connect_swapped (priv->adaptive_preview, "exit",
                              G_CALLBACK (adaptive_preview_exit_cb), self);
    adw_adaptive_preview_set_child (ADW_ADAPTIVE_PREVIEW (priv->adaptive_preview),
                                    priv->dialog_host);
  } else {
    adw_adaptive_preview_set_child (ADW_ADAPTIVE_PREVIEW (priv->adaptive_preview),
                                    NULL);
    gtk_window_set_child (GTK_WINDOW (self), priv->dialog_host);
    priv->adaptive_preview = NULL;
  }

  g_object_unref (priv->dialog_host);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ADAPTIVE_PREVIEW]);
}
