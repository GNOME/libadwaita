/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-window.h"
#include "adw-window-mixin-private.h"

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
 * titlebar area. It means [class@Gtk.HeaderBar] can be used as follows:
 *
 * ```xml
 * <object class="AdwWindow">
 *   <property name="content">
 *     <object class="GtkBox">
 *       <property name="orientation">vertical</property>
 *       <child>
 *         <object class="GtkHeaderBar"/>
 *       </child>
 *       <child>
 *         <!-- ... -->
 *       </child>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * Using [method@Gtk.Window.get_titlebar] and [method@Gtk.Window.set_titlebar]
 * is not supported and will result in a crash.
 *
 * Since: 1.0
 */

typedef struct
{
  AdwWindowMixin *mixin;
} AdwWindowPrivate;

static void adw_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwWindow, adw_window, GTK_TYPE_WINDOW,
                         G_ADD_PRIVATE (AdwWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_window_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

#define ADW_GET_WINDOW_MIXIN(obj) (((AdwWindowPrivate *) adw_window_get_instance_private (ADW_WINDOW (obj)))->mixin)

static void
adw_window_size_allocate (GtkWidget *widget,
                          int        width,
                          int        height,
                          int        baseline)
{
  adw_window_mixin_size_allocate (ADW_GET_WINDOW_MIXIN (widget),
                                  width,
                                  height,
                                  baseline);
}

static void
adw_window_finalize (GObject *object)
{
  AdwWindow *self = (AdwWindow *)object;
  AdwWindowPrivate *priv = adw_window_get_instance_private (self);

  g_clear_object (&priv->mixin);

  G_OBJECT_CLASS (adw_window_parent_class)->finalize (object);
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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_window_class_init (AdwWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = adw_window_finalize;
  object_class->get_property = adw_window_get_property;
  object_class->set_property = adw_window_set_property;
  widget_class->size_allocate = adw_window_size_allocate;

  /**
   * AdwWindow:content: (attributes org.gtk.Property.get=adw_window_get_content org.gtk.Property.set=adw_window_set_content)
   *
   * The content widget.
   *
   * Since: 1.0
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_window_init (AdwWindow *self)
{
  AdwWindowPrivate *priv = adw_window_get_instance_private (self);

  priv->mixin = adw_window_mixin_new (GTK_WINDOW (self),
                                      GTK_WINDOW_CLASS (adw_window_parent_class));
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
 *
 * Since: 1.0
 */
GtkWidget *
adw_window_new (void)
{
  return g_object_new (ADW_TYPE_WINDOW, NULL);
}

/**
 * adw_window_set_content: (attributes org.gtk.Method.set_property=content)
 * @self: a window
 * @content: (nullable): the content widget
 *
 * Sets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.set_child].
 *
 * Since: 1.0
 */
void
adw_window_set_content (AdwWindow *self,
                        GtkWidget *content)
{
  g_return_if_fail (ADW_IS_WINDOW (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  adw_window_mixin_set_content (ADW_GET_WINDOW_MIXIN (self), content);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_window_get_content: (attributes org.gtk.Method.get_property=content)
 * @self: a window
 *
 * Gets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.get_child].
 *
 * Returns: (nullable) (transfer none): the content widget of @self
 *
 * Since: 1.0
 */
GtkWidget *
adw_window_get_content (AdwWindow *self)
{
  g_return_val_if_fail (ADW_IS_WINDOW (self), NULL);

  return adw_window_mixin_get_content (ADW_GET_WINDOW_MIXIN (self));
}
