/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-application-window.h"
#include "adw-window-mixin-private.h"

/**
 * AdwApplicationWindow:
 *
 * A freeform application window.
 *
 * <picture>
 *   <source srcset="application-window-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="application-window.png" alt="application-window">
 * </picture>
 *
 * `AdwApplicationWindow` is a [class@Gtk.ApplicationWindow] subclass providing
 * the same features as [class@Window].
 *
 * See [class@Window] for details.
 *
 * Example of an `AdwApplicationWindow` UI definition:
 *
 * ```xml
 * <object class="AdwApplicationWindow">
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
 * Using [property@Gtk.Application:menubar] is not supported and may result in
 * visual glitches.
 */

typedef struct
{
  AdwWindowMixin *mixin;
} AdwApplicationWindowPrivate;

static void adw_application_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwApplicationWindow, adw_application_window, GTK_TYPE_APPLICATION_WINDOW,
                         G_ADD_PRIVATE (AdwApplicationWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_application_window_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

#define ADW_GET_WINDOW_MIXIN(obj) (((AdwApplicationWindowPrivate *) adw_application_window_get_instance_private (ADW_APPLICATION_WINDOW (obj)))->mixin)

static void
adw_application_window_size_allocate (GtkWidget *widget,
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
adw_application_window_finalize (GObject *object)
{
  AdwApplicationWindow *self = (AdwApplicationWindow *)object;
  AdwApplicationWindowPrivate *priv = adw_application_window_get_instance_private (self);

  g_clear_object (&priv->mixin);

  G_OBJECT_CLASS (adw_application_window_parent_class)->finalize (object);
}

static void
adw_application_window_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AdwApplicationWindow *self = ADW_APPLICATION_WINDOW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adw_application_window_get_content (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_application_window_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AdwApplicationWindow *self = ADW_APPLICATION_WINDOW (object);

  switch (prop_id) {
  case PROP_CONTENT:
    adw_application_window_set_content (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_application_window_class_init (AdwApplicationWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = adw_application_window_finalize;
  object_class->get_property = adw_application_window_get_property;
  object_class->set_property = adw_application_window_set_property;
  widget_class->size_allocate = adw_application_window_size_allocate;

  /**
   * AdwApplicationWindow:content: (attributes org.gtk.Property.get=adw_application_window_get_content org.gtk.Property.set=adw_application_window_set_content)
   *
   * The content widget.
   *
   * This property should always be used instead of [property@Gtk.Window:child].
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);}

static void
adw_application_window_init (AdwApplicationWindow *self)
{
  AdwApplicationWindowPrivate *priv = adw_application_window_get_instance_private (self);

  priv->mixin = adw_window_mixin_new (GTK_WINDOW (self),
                                      GTK_WINDOW_CLASS (adw_application_window_parent_class));

  gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), FALSE);
}

static void
adw_application_window_buildable_add_child (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const char   *type)
{
  if (!g_strcmp0 (type, "titlebar"))
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
  else if (GTK_IS_WIDGET (child))
    adw_application_window_set_content (ADW_APPLICATION_WINDOW (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_application_window_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_application_window_buildable_add_child;
}

/**
 * adw_application_window_new:
 * @app: an application instance
 *
 * Creates a new `AdwApplicationWindow` for @app.
 *
 * Returns: the newly created `AdwApplicationWindow`
 */
GtkWidget *
adw_application_window_new (GtkApplication *app)
{
  return g_object_new (ADW_TYPE_APPLICATION_WINDOW,
                       "application", app,
                       NULL);
}

/**
 * adw_application_window_set_content: (attributes org.gtk.Method.set_property=content)
 * @self: an application window
 * @content: (nullable): the content widget
 *
 * Sets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.set_child].
 */
void
adw_application_window_set_content (AdwApplicationWindow *self,
                                    GtkWidget            *content)
{
  g_return_if_fail (ADW_IS_APPLICATION_WINDOW (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  if (adw_application_window_get_content (self) == content)
    return;

  adw_window_mixin_set_content (ADW_GET_WINDOW_MIXIN (self), content);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_application_window_get_content: (attributes org.gtk.Method.get_property=content)
 * @self: an application window
 *
 * Gets the content widget of @self.
 *
 * This method should always be used instead of [method@Gtk.Window.get_child].
 *
 * Returns: (nullable) (transfer none): the content widget of @self
 */
GtkWidget *
adw_application_window_get_content (AdwApplicationWindow *self)
{
  g_return_val_if_fail (ADW_IS_APPLICATION_WINDOW (self), NULL);

  return adw_window_mixin_get_content (ADW_GET_WINDOW_MIXIN (self));
}
