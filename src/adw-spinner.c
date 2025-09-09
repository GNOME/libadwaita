/*
 * Copyright (C) 2024 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n.h>

#include "adw-spinner.h"

#include "adw-spinner-paintable.h"

#define MIN_SIZE 16
#define MAX_SIZE 64

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
 * The size of the spinner depends on the available size, never smaller than
 * 16×16 pixels and never larger than 64×64 pixels.
 *
 * Use the [property@Gtk.Widget:halign] and [property@Gtk.Widget:valign]
 * properties in combination with [property@Gtk.Widget:width-request] and
 * [property@Gtk.Widget:height-request] for fine sizing control.
 *
 * For example, the following snippet shows the spinner at 48×48 pixels:
 *
 * ```xml
 * <object class="AdwSpinner">
 *   <property name="halign">center</property>
 *   <property name="valign">center</property>
 *   <property name="width-request">48</property>
 *   <property name="height-request">48</property>
 * </object>
 * ```
 *
 * See [class@SpinnerPaintable] for cases where using a widget is impractical or
 * impossible, such as [property@StatusPage:paintable].
 *
 * ## CSS nodes
 *
 * `AdwSpinner` has a single node with the name `image` and the style class
 * `.spinner`.
 *
 * ## Accessibility
 *
 * `AdwSpinner` uses the [enum@Gtk.AccessibleRole.progress-bar] role.
 *
 * Since: 1.6
 */

struct _AdwSpinner
{
  GtkWidget parent_instance;

  AdwSpinnerPaintable *paintable;
};

G_DEFINE_FINAL_TYPE (AdwSpinner, adw_spinner, GTK_TYPE_WIDGET)

static void
adw_spinner_measure (GtkWidget      *widget,
                     GtkOrientation  orientation,
                     int             for_size,
                     int            *minimum,
                     int            *natural,
                     int            *minimum_baseline,
                     int            *natural_baseline)
{
  if (minimum)
    *minimum = MIN_SIZE;
  if (natural)
    *natural = MIN_SIZE;
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

  gdk_paintable_snapshot (GDK_PAINTABLE (self->paintable),
                          snapshot, w, h);
}

static void
adw_spinner_dispose (GObject *object)
{
  AdwSpinner *self = ADW_SPINNER (object);

  g_clear_object (&self->paintable);

  G_OBJECT_CLASS (adw_spinner_parent_class)->dispose (object);
}

static void
adw_spinner_class_init (AdwSpinnerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_spinner_dispose;

  widget_class->measure = adw_spinner_measure;
  widget_class->snapshot = adw_spinner_snapshot;

  gtk_widget_class_set_css_name (widget_class, "image");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_PROGRESS_BAR);
}

static void
adw_spinner_init (AdwSpinner *self)
{
  gtk_widget_add_css_class (GTK_WIDGET (self), "spinner");

  self->paintable = adw_spinner_paintable_new (GTK_WIDGET (self));

  g_signal_connect_swapped (self->paintable, "invalidate-size",
                            G_CALLBACK (gtk_widget_queue_resize), self);
  g_signal_connect_swapped (self->paintable, "invalidate-contents",
                            G_CALLBACK (gtk_widget_queue_draw), self);

  gtk_accessible_update_state (GTK_ACCESSIBLE (self),
                               GTK_ACCESSIBLE_STATE_BUSY, TRUE,
                               -1);
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
