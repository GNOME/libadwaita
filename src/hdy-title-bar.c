/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "hdy-title-bar.h"

#include <glib/gi18n-lib.h>

/**
 * SECTION:hdy-title-bar
 * @short_description: A simple title bar container.
 * @Title: HdyTitleBar
 *
 * HdyTitleBar is meant to be used as the top-level widget of your window's
 * title bar. It will be drawn with the same style as a GtkHeaderBar but it
 * won't force a widget layout on you: you can put whatever widget you want in
 * it, including a GtkHeaderBar.
 *
 * HdyTitleBar becomes really useful when you want to animate header bars, like
 * an adaptive application using #HdyLeaflet would do.
 *
 * # CSS nodes
 *
 * #HdyTitleBar has a single CSS node with name headerbar.
 */

enum {
  PROP_0,
  PROP_SELECTION_MODE,
  LAST_PROP,
};

struct _HdyTitleBar
{
  GtkBin parent_instance;

  gboolean selection_mode;
};

G_DEFINE_TYPE (HdyTitleBar, hdy_title_bar, GTK_TYPE_BIN)

static GParamSpec *props[LAST_PROP];

/**
 * hdy_title_bar_set_selection_mode:
 * @self: a #HdyTitleBar
 * @selection_mode: %TRUE to enable the selection mode
 *
 * Sets whether @self is in selection mode.
 */
void
hdy_title_bar_set_selection_mode (HdyTitleBar *self,
                                  gboolean     selection_mode)
{
  GtkStyleContext *context;

  g_return_if_fail (HDY_IS_TITLE_BAR (self));

  selection_mode = !!selection_mode;

  context = gtk_widget_get_style_context (GTK_WIDGET (self));

  if (self->selection_mode == selection_mode)
    return;

  self->selection_mode = selection_mode;

  if (selection_mode)
    gtk_style_context_add_class (context, "selection-mode");
  else
    gtk_style_context_remove_class (context, "selection-mode");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTION_MODE]);
}

/**
 * hdy_title_bar_get_selection_mode:
 * @self: a #HdyTitleBar
 *
 * Returns whether whether @self is in selection mode.
 *
 * Returns: %TRUE if the title bar is in selection mode
 */
gboolean
hdy_title_bar_get_selection_mode (HdyTitleBar *self)
{
  g_return_val_if_fail (HDY_IS_TITLE_BAR (self), FALSE);

  return self->selection_mode;
}

static void
style_updated_cb (HdyTitleBar *self)
{
  GtkStyleContext *context;
  gboolean selection_mode;

  g_assert (HDY_IS_TITLE_BAR (self));

  context = gtk_widget_get_style_context (GTK_WIDGET (self));
  selection_mode = gtk_style_context_has_class (context, "selection-mode");

  if (self->selection_mode == selection_mode)
    return;

  self->selection_mode = selection_mode;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTION_MODE]);
}

static void
hdy_title_bar_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  HdyTitleBar *self = HDY_TITLE_BAR (object);

  switch (prop_id) {
  case PROP_SELECTION_MODE:
    g_value_set_boolean (value, hdy_title_bar_get_selection_mode (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_title_bar_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  HdyTitleBar *self = HDY_TITLE_BAR (object);

  switch (prop_id) {
  case PROP_SELECTION_MODE:
    hdy_title_bar_set_selection_mode (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static gboolean
hdy_title_bar_draw (GtkWidget *widget,
                    cairo_t   *cr)
{
  GtkStyleContext *context;

  context = gtk_widget_get_style_context (widget);
  /* GtkWidget draws nothing by default so we have to render the background
   * explicitly for HdyTitleBar to render the typical titlebar background.
   */
  gtk_render_background (context,
                         cr,
                         0, 0,
                         gtk_widget_get_allocated_width (widget),
                         gtk_widget_get_allocated_height (widget));

  return GTK_WIDGET_CLASS (hdy_title_bar_parent_class)->draw (widget, cr);
}

/* This private method is prefixed by the class name because it will be a
 * virtual method in GTK 4.
 */
static void
hdy_title_bar_measure (GtkWidget      *widget,
                       GtkOrientation  orientation,
                       gint            for_size,
                       gint           *minimum,
                       gint           *natural,
                       gint           *minimum_baseline,
                       gint           *natural_baseline)
{
  GtkWidget *child;
  gint parent_min, parent_nat;
  gint css_width, css_height, css_min;

  child = gtk_bin_get_child (GTK_BIN (widget));

  gtk_style_context_get (gtk_widget_get_style_context (widget),
                         gtk_widget_get_state_flags (widget),
                         "min-width", &css_width,
                         "min-height", &css_height,
                         NULL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    css_min = css_width;
  else
    css_min = css_height;

  if (child)
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      if (for_size != 1)
        gtk_widget_get_preferred_width_for_height (child,
                                                   MAX (for_size, css_height),
                                                   &parent_min, &parent_nat);
      else
        gtk_widget_get_preferred_width (child, &parent_min, &parent_nat);
    else
      if (for_size != 1)
        gtk_widget_get_preferred_height_for_width (child,
                                                   MAX (for_size, css_width),
                                                   &parent_min, &parent_nat);
      else
        gtk_widget_get_preferred_height (child, &parent_min, &parent_nat);
  else {
    parent_min = 0;
    parent_nat = 0;
  }

  if (minimum)
    *minimum = MAX (parent_min, css_min);

  if (natural)
    *natural = MAX (parent_nat, css_min);

  if (minimum_baseline)
    *minimum_baseline = -1;

  if (natural_baseline)
    *natural_baseline = -1;
}

static void
hdy_title_bar_get_preferred_width (GtkWidget *widget,
                                   gint      *minimum,
                                   gint      *natural)
{
  hdy_title_bar_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                         minimum, natural, NULL, NULL);
}

static void
hdy_title_bar_get_preferred_width_for_height (GtkWidget *widget,
                                              gint       height,
                                              gint      *minimum,
                                              gint      *natural)
{
  hdy_title_bar_measure (widget, GTK_ORIENTATION_HORIZONTAL, height,
                         minimum, natural, NULL, NULL);
}

static void
hdy_title_bar_get_preferred_height (GtkWidget *widget,
                                    gint      *minimum,
                                    gint      *natural)
{
  hdy_title_bar_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                         minimum, natural, NULL, NULL);
}

static void
hdy_title_bar_get_preferred_height_for_width (GtkWidget *widget,
                                              gint       width,
                                              gint      *minimum,
                                              gint      *natural)
{
  hdy_title_bar_measure (widget, GTK_ORIENTATION_VERTICAL, width,
                         minimum, natural, NULL, NULL);
}

static void
hdy_title_bar_size_allocate (GtkWidget     *widget,
                             GtkAllocation *allocation)
{
  GtkAllocation clip;

  gtk_render_background_get_clip (gtk_widget_get_style_context (widget),
                                  allocation->x,
                                  allocation->y,
                                  allocation->width,
                                  allocation->height,
                                  &clip);

  GTK_WIDGET_CLASS (hdy_title_bar_parent_class)->size_allocate (widget, allocation);
  gtk_widget_set_clip (widget, &clip);
}

static void
hdy_title_bar_class_init (HdyTitleBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = hdy_title_bar_get_property;
  object_class->set_property = hdy_title_bar_set_property;

  widget_class->draw = hdy_title_bar_draw;
  widget_class->get_preferred_width = hdy_title_bar_get_preferred_width;
  widget_class->get_preferred_width_for_height = hdy_title_bar_get_preferred_width_for_height;
  widget_class->get_preferred_height = hdy_title_bar_get_preferred_height;
  widget_class->get_preferred_height_for_width = hdy_title_bar_get_preferred_height_for_width;
  widget_class->size_allocate = hdy_title_bar_size_allocate;

  /**
   * HdyTitleBar:selection_mode:
   *
   * %TRUE if the title bar is in selection mode.
   */
  props[PROP_SELECTION_MODE] =
      g_param_spec_boolean ("selection-mode",
                            _("Selection mode"),
                            _("Whether or not the title bar is in selection mode"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_accessible_role (widget_class, ATK_ROLE_TITLE_BAR);
  /* Adwaita states it expects a headerbar to be the top-level titlebar widget,
   * so style-wise HdyTitleBar pretends to be one as its role is to be the
   * top-level titlebar widget.
   */
  gtk_widget_class_set_css_name (widget_class, "headerbar");
  gtk_container_class_handle_border_width (container_class);
}

static void
hdy_title_bar_init (HdyTitleBar *self)
{
  GtkStyleContext *context;

  context = gtk_widget_get_style_context (GTK_WIDGET (self));
  /* Ensure the widget has the titlebar style class. */
  gtk_style_context_add_class (context, "titlebar");

  g_signal_connect (self, "style-updated", G_CALLBACK (style_updated_cb), NULL);
}

/**
 * hdy_title_bar_new:
 *
 * Creates a new #HdyTitleBar.
 *
 * Returns: a new #HdyTitleBar
 */
GtkWidget *
hdy_title_bar_new (void)
{
  return g_object_new (HDY_TYPE_TITLE_BAR, NULL);
}
