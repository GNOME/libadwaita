/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-window.h"
#include "adw-window-mixin-private.h"

/**
 * SECTION:adw-window
 * @short_description: A freeform window.
 * @title: AdwWindow
 * @See_also: #AdwApplicationWindow
 *
 * The AdwWindow widget is a subclass of #GtkWindow which has no titlebar area
 * and provides rounded corners on all sides, ensuring they can never be
 * overlapped by the content. This makes it safe to use headerbars in the
 * content area as follows:
 *
 * |[
 * <object class="AdwWindow"/>
 *   <child>
 *     <object class="GtkBox">
 *       <property name="orientation">vertical</property>
 *       <child>
 *         <object class="AdwHeaderBar"/>
 *       </child>
 *       <child>
 *         ...
 *       </child>
 *     </object>
 *   </child>
 * </object>
 * ]|
 *
 * #AdwWindow allows to easily implement titlebar autohiding by putting the
 * headerbar inside a #GtkRevealer, and to show titlebar above content by
 * putting it into a #GtkOverlay instead of #GtkBox.
 *
 * if the window has a #GtkGLArea, it may bring a slight performance regression
 * when the window is not fullscreen, tiled or maximized.
 *
 * Using gtk_window_get_titlebar() and gtk_window_set_titlebar() is not
 * supported and will result in a crash.
 *
 * # CSS nodes
 *
 * #AdwWindow has a main CSS node with the name window and style classes
 * .background, .csd and .unified.
 *
 * The .solid-csd style class on the main node is used for client-side
 * decorations without invisible borders.
 *
 * #AdwWindow also represents window states with the following
 * style classes on the main node: .tiled, .maximized, .fullscreen.
 *
 * It contains the subnodes decoration for window shadow and/or border,
 * decoration-overlay for the sheen on top of the window, widget.titlebar, and
 * deck, which contains the child inside the window.
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
  PROP_CHILD,
  LAST_PROP = PROP_0,
};

#define ADW_GET_WINDOW_MIXIN(obj) (((AdwWindowPrivate *) adw_window_get_instance_private (ADW_WINDOW (obj)))->mixin)

static void
adw_window_size_allocate (GtkWidget *widget,
                          gint       width,
                          gint       height,
                          gint       baseline)
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
  case PROP_CHILD:
    g_value_set_object (value, adw_window_get_child (self));
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
  case PROP_CHILD:
    adw_window_set_child (self, g_value_get_object (value));
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

  g_object_class_override_property (object_class, PROP_CHILD, "child");
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
                                const gchar  *type)
{
  if (!g_strcmp0 (type, "titlebar"))
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
  else if (GTK_IS_WIDGET (child))
    adw_window_set_child (ADW_WINDOW (buildable), GTK_WIDGET (child));
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
 * Creates a new #AdwWindow.
 *
 * Returns: (transfer full): a newly created #AdwWindow
 *
 * Since: 1.0
 */
GtkWidget *
adw_window_new (void)
{
  return g_object_new (ADW_TYPE_WINDOW, NULL);
}

/**
 * adw_window_set_child:
 * @self: a #AdwWindow
 * @child: (allow-none): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.0
 */
void
adw_window_set_child (AdwWindow *self,
                      GtkWidget *child)
{
  g_return_if_fail (ADW_IS_WINDOW (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  adw_window_mixin_set_child (ADW_GET_WINDOW_MIXIN (self), child);

  g_object_notify (G_OBJECT (self), "child");
}

/**
 * adw_window_get_child:
 * @self: a #AdwWindow
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.0
 */
GtkWidget *
adw_window_get_child (AdwWindow *self)
{
  g_return_val_if_fail (ADW_IS_WINDOW (self), NULL);

  return adw_window_mixin_get_child (ADW_GET_WINDOW_MIXIN (self));
}
