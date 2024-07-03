/*
 * Copyright (C) 2024 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n.h>

#include "adw-spinner.h"

#include "adw-spinner-paintable.h"

/**
 * AdwSpinner:
 *
 * A widget showing a loading spinner.
 *
 * <picture>
 *   <source srcset="spinner-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="spinner.png" alt="spinner">
 * </picture>
 *
 * `AdwSpinner` shows a loading spinner at a specified size, set via
 * [property@Spinner:size]. The default size is 64×64 pixels, and it's capped at
 * 96×96 pixels. If it's larger than that, the visible spinner will be centered
 * inside the widget.
 *
 * See [class@SpinnerPaintable] for cases the cases where using a widget is
 * impractical or impossible, such as [property@StatusPage:paintable].
 *
 * ## CSS nodes
 *
 * `AdwSpinner` has a single node with the name `image` and the style class
 * `.spinner`.
 *
 * Since: 1.6
 */

struct _AdwSpinner
{
  GtkWidget parent_instance;

  AdwSpinnerPaintable *paintable;
  int size;
};

G_DEFINE_FINAL_TYPE (AdwSpinner, adw_spinner, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_SIZE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
adw_spinner_measure (GtkWidget      *widget,
                     GtkOrientation  orientation,
                     int             for_size,
                     int            *minimum,
                     int            *natural,
                     int            *minimum_baseline,
                     int            *natural_baseline)
{
  AdwSpinner *self = ADW_SPINNER (widget);

  if (minimum)
    *minimum = self->size;
  if (natural)
    *natural = self->size;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adw_spinner_snapshot (GtkWidget   *widget,
                      GtkSnapshot *snapshot)
{
  AdwSpinner *self = ADW_SPINNER (widget);
  int w, h;

  w = gtk_widget_get_width (widget);
  h = gtk_widget_get_height (widget);

  gtk_snapshot_translate (snapshot,
                          &GRAPHENE_POINT_INIT (roundf ((w - self->size) / 2),
                                                roundf ((h - self->size) / 2)));

  gdk_paintable_snapshot (GDK_PAINTABLE (self->paintable),
                          snapshot, self->size, self->size);
}

static void
adw_spinner_dispose (GObject *object)
{
  AdwSpinner *self = ADW_SPINNER (object);

  g_clear_object (&self->paintable);

  G_OBJECT_CLASS (adw_spinner_parent_class)->dispose (object);
}

static void
adw_spinner_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  AdwSpinner *self = ADW_SPINNER (object);

  switch (prop_id) {
  case PROP_SIZE:
    g_value_set_int (value, adw_spinner_get_size (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_spinner_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  AdwSpinner *self = ADW_SPINNER (object);

  switch (prop_id) {
  case PROP_SIZE:
    adw_spinner_set_size (self, g_value_get_int (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_spinner_class_init (AdwSpinnerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_spinner_dispose;
  object_class->get_property = adw_spinner_get_property;
  object_class->set_property = adw_spinner_set_property;

  widget_class->measure = adw_spinner_measure;
  widget_class->snapshot = adw_spinner_snapshot;

  /**
   * AdwSpinner:size:
   *
   * The size of the spinner.
   *
   * The spinner cannot be larger than 96×96 pixels. If the provided size is
   * larger, the visible spinner will be centered inside the widget.
   *
   * Since: 1.6
   */
  props[PROP_SIZE] =
    g_param_spec_int ("size", NULL, NULL,
                      0, G_MAXINT, 64,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "image");
}

static void
adw_spinner_init (AdwSpinner *self)
{
  gtk_widget_add_css_class (GTK_WIDGET (self), "spinner");

  self->size = 64;
  self->paintable = adw_spinner_paintable_new (GTK_WIDGET (self));

  g_signal_connect_swapped (self->paintable, "invalidate-size",
                            G_CALLBACK (gtk_widget_queue_resize), self);
  g_signal_connect_swapped (self->paintable, "invalidate-contents",
                            G_CALLBACK (gtk_widget_queue_draw), self);
}

/**
 * adw_spinner_new:
 *
 * Creates a new `AdwSpinner`.
 *
 * Returns: the newly created `AdwSpinner`
 *
 * Since: 1.6
 */
GtkWidget *
adw_spinner_new (void)
{
  return g_object_new (ADW_TYPE_SPINNER, NULL);
}

/**
 * adw_spinner_get_size:
 * @self: a spinner
 *
 * Gets the size of the spinner.
 *
 * Returns: the size of the spinner
 *
 * Since: 1.6
 */
int
adw_spinner_get_size (AdwSpinner *self)
{
  g_return_val_if_fail (ADW_IS_SPINNER (self), 0);

  return self->size;
}

/**
 * adw_spinner_set_size:
 * @self: a spinner
 * @size: The size of the spinner
 *
 * Sets the size of the spinner.
 *
 * The spinner cannot be larger than 96×96 pixels. If the provided size is
 * larger, the visible spinner will be centered inside the widget.
 *
 * Since: 1.6
 */
void
adw_spinner_set_size (AdwSpinner *self,
                      int         size)
{
  g_return_if_fail (ADW_IS_SPINNER (self));
  g_return_if_fail (size >= 0);

  if (self->size == size)
    return;

  self->size = size;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIZE]);
}
