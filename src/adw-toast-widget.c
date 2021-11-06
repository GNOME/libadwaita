/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-toast-widget-private.h"

#define TOAST_DURATION 5000

struct _AdwToastWidget {
  GtkWidget parent_instance;

  AdwToast *toast;

  guint hide_timeout_id;
  gint inhibit_count;
};

enum {
  PROP_0,
  PROP_TOAST,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (AdwToastWidget, adw_toast_widget, GTK_TYPE_WIDGET)

static gboolean
string_is_not_empty (gpointer    user_data,
                     const char *string)
{
  return string && string[0];
}

static gboolean
timeout_cb (AdwToastWidget *self)
{
  self->hide_timeout_id = 0;

  adw_toast_dismiss (self->toast);

  return G_SOURCE_REMOVE;
}

static void
start_timeout (AdwToastWidget *self)
{
  if (!self->hide_timeout_id)
    self->hide_timeout_id =
      g_timeout_add (TOAST_DURATION,
                     G_SOURCE_FUNC (timeout_cb),
                     self);
}

static void
end_timeout (AdwToastWidget *self)
{
  g_clear_handle_id (&self->hide_timeout_id, g_source_remove);
}

static void
inhibit_hide (AdwToastWidget *self)
{
  if (self->inhibit_count++ == 0)
    end_timeout (self);
}

static void
uninhibit_hide (AdwToastWidget  *self)
{
  g_assert (self->inhibit_count);

  if (--self->inhibit_count == 0)
    start_timeout (self);
}

static void
dismiss (AdwToastWidget *self)
{
  end_timeout (self);

  adw_toast_dismiss (self->toast);
}

static gboolean
close_idle_cb (AdwToastWidget *self)
{
  dismiss (self);
  g_object_unref (self);

  return G_SOURCE_REMOVE;
}

static void
action_clicked_cb (AdwToastWidget *self)
{
  end_timeout (self);

  /* Keep the widget alive through the idle. Otherwise it may be immediately
   * destroyed if animations are disabled */
  g_idle_add (G_SOURCE_FUNC (close_idle_cb), g_object_ref (self));
}

static void
adw_toast_widget_dispose (GObject *object)
{
  AdwToastWidget *self = ADW_TOAST_WIDGET (object);
  GtkWidget *child;

  end_timeout (self);

  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self))))
    gtk_widget_unparent (child);

  g_clear_pointer (&self->toast, g_object_unref);

  G_OBJECT_CLASS (adw_toast_widget_parent_class)->dispose (object);
}

static void
adw_toast_widget_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwToastWidget *self = ADW_TOAST_WIDGET (object);

  switch (prop_id) {
  case PROP_TOAST:
    g_value_set_object (value, self->toast);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toast_widget_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwToastWidget *self = ADW_TOAST_WIDGET (object);

  switch (prop_id) {
  case PROP_TOAST:
    g_set_object (&self->toast, g_value_get_object (value));
    end_timeout (self);
    start_timeout (self);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toast_widget_class_init (AdwToastWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_toast_widget_dispose;
  object_class->get_property = adw_toast_widget_get_property;
  object_class->set_property = adw_toast_widget_set_property;

  props[PROP_TOAST] =
      g_param_spec_object ("toast",
                           "Toast",
                           "The displayed toast",
                           ADW_TYPE_TOAST,
                           G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-toast-widget.ui");

  gtk_widget_class_bind_template_callback (widget_class, string_is_not_empty);
  gtk_widget_class_bind_template_callback (widget_class, action_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, dismiss);
  gtk_widget_class_bind_template_callback (widget_class, inhibit_hide);
  gtk_widget_class_bind_template_callback (widget_class, uninhibit_hide);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "toast");
}

static void
adw_toast_widget_init (AdwToastWidget *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
adw_toast_widget_new (AdwToast *toast)
{
  g_assert (ADW_IS_TOAST (toast));

  return g_object_new (ADW_TYPE_TOAST_WIDGET,
                       "toast", toast,
                       NULL);
}
