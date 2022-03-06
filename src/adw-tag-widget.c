/* adw-tag-widget.c
 *
 * SPDX-FileCopyrightText: 2022 Emmanuele Bassi
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-tag-widget-private.h"

#include "adw-tag-private.h"

struct _AdwTagWidget
{
  GtkWidget parent_instance;

  AdwTag *tag;

  GtkWidget *icon;
  GtkWidget *label;
  GtkWidget *close_button;
};

struct _AdwTagWidgetClass
{
  GtkWidgetClass parent_class;
};

enum
{
  PROP_TAG = 1,
  N_PROPS
};

static GParamSpec *obj_props[N_PROPS];

enum
{
  CLOSED,

  LAST_SIGNAL
};

static guint obj_signals[LAST_SIGNAL];


G_DEFINE_TYPE (AdwTagWidget, adw_tag_widget, GTK_TYPE_WIDGET)

static void
adw_tag_widget__close_clicked (AdwTagWidget *self)
{
  g_signal_emit (self, obj_signals[CLOSED], 0, self->tag);
}

static void
update_tag_icon (AdwTagWidget *self)
{
  gtk_widget_set_visible (self->icon, adw_tag_has_icon (self->tag));

  AdwTagIconType icon_type = adw_tag_get_icon_type (self->tag);
  switch (icon_type) {
  case ADW_TAG_ICON_GICON:
    {
      GIcon *icon = adw_tag_get_gicon (self->tag);
      gtk_image_set_from_gicon (GTK_IMAGE (self->icon), icon);
    }
    break;

  case ADW_TAG_ICON_PAINTABLE:
    {
      GdkPaintable *paintable = adw_tag_get_paintable (self->tag);
      gtk_image_set_from_paintable (GTK_IMAGE (self->icon), paintable);
    }
    break;

  case ADW_TAG_ICON_NONE:
  default:
    break;
  }
}

static void
adw_tag_widget_set_tag (AdwTagWidget *self,
                        AdwTag       *tag)
{
  if (g_set_object (&self->tag, tag)) {
    if (self->tag != NULL) {
      g_object_bind_property (self->tag, "show-close",
                              self->close_button, "visible",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
      g_object_bind_property (self->tag, "label",
                              self->label, "label",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

      /* We cannot use a simple property binding, because AdwTag has
       * multiple ways to provide an icon for the widget to display
       */
      g_signal_connect_swapped (self->tag,
                                "notify::has-icon",
                                G_CALLBACK (update_tag_icon),
                                self);

      update_tag_icon (self);
    }

    g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_TAG]);
  }
}

static void
adw_tag_widget__click_released (AdwTagWidget *self,
                                guint         n_press,
                                double        x,
                                double        y,
                                GtkGesture   *gesture)
{
  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);

  const char *action_name = adw_tag_get_action_name (self->tag);
  if (action_name == NULL)
    return;

  gtk_widget_activate_action_variant (GTK_WIDGET (self),
                                      action_name,
                                      adw_tag_get_action_target_value (self->tag));
}

static void
adw_tag_widget_dispose (GObject *gobject)
{
  AdwTagWidget *self = ADW_TAG_WIDGET (gobject);

  g_clear_pointer (&self->icon, gtk_widget_unparent);
  g_clear_pointer (&self->label, gtk_widget_unparent);
  g_clear_pointer (&self->close_button, gtk_widget_unparent);

  g_clear_object (&self->tag);

  G_OBJECT_CLASS (adw_tag_widget_parent_class)->dispose (gobject);
}

static void
adw_tag_widget_set_property (GObject      *gobject,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdwTagWidget *self = ADW_TAG_WIDGET (gobject);

  switch (prop_id) {
  case PROP_TAG:
    adw_tag_widget_set_tag (self, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

static void
adw_tag_widget_get_property (GObject    *gobject,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdwTagWidget *self = ADW_TAG_WIDGET (gobject);

  switch (prop_id) {
  case PROP_TAG:
    g_value_set_object (value, self->tag);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

static void
adw_tag_widget_class_init (AdwTagWidgetClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gobject_class->set_property = adw_tag_widget_set_property;
  gobject_class->get_property = adw_tag_widget_get_property;
  gobject_class->dispose = adw_tag_widget_dispose;

  obj_props[PROP_TAG] =
    g_param_spec_object ("tag", NULL, NULL, ADW_TYPE_TAG,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);

  obj_signals[CLOSED] =
    g_signal_new ("closed",
                  ADW_TYPE_TAG_WIDGET,
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  NULL,
                  G_TYPE_NONE, 1, ADW_TYPE_TAG);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/ui/adw-tag-widget.ui");

  gtk_widget_class_bind_template_child (widget_class, AdwTagWidget, icon);
  gtk_widget_class_bind_template_child (widget_class, AdwTagWidget, label);
  gtk_widget_class_bind_template_child (widget_class, AdwTagWidget, close_button);

  gtk_widget_class_bind_template_callback (widget_class, adw_tag_widget__click_released);
  gtk_widget_class_bind_template_callback (widget_class, adw_tag_widget__close_clicked);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
  gtk_widget_class_set_css_name (widget_class, "tag");
}

static void
adw_tag_widget_init (AdwTagWidget *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwTag *
adw_tag_widget_get_tag (AdwTagWidget *self)
{
  g_return_val_if_fail (ADW_IS_TAG_WIDGET (self), NULL);

  return self->tag;
}
