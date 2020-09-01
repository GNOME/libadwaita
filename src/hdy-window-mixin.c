/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-cairo-private.h"
#include "hdy-deck.h"
#include "hdy-nothing-private.h"
#include "hdy-window-mixin-private.h"

typedef enum {
  HDY_CORNER_TOP_LEFT,
  HDY_CORNER_TOP_RIGHT,
  HDY_CORNER_BOTTOM_LEFT,
  HDY_CORNER_BOTTOM_RIGHT,
  HDY_N_CORNERS,
} HdyCorner;

/**
 * PRIVATE:hdy-window-mixin
 * @short_description: A helper object for #HdyWindow and #HdyApplicationWindow
 * @title: HdyWindowMixin
 * @See_also: #HdyApplicationWindow, #HdyWindow
 * @stability: Private
 *
 * The HdyWindowMixin object contains the implementation of the HdyWindow and
 * HdyApplicationWindow classes, providing a way to make a GtkWindow subclass
 * that has masked window corners on all sides and no titlebar by default,
 * allowing for more freedom with how to handle the titlebar for applications.
 *
 * Since: 1.0
 */

struct _HdyWindowMixin
{
  GObject parent;

  GtkWindow *window;
  GtkWindowClass *klass;

  GtkWidget *content;
  GtkWidget *titlebar;
  cairo_surface_t *masks[HDY_N_CORNERS];
  gint last_border_radius;

  GtkStyleContext *decoration_context;
  GtkStyleContext *overlay_context;

  GtkWidget *child;
};

G_DEFINE_TYPE (HdyWindowMixin, hdy_window_mixin, G_TYPE_OBJECT)

static GtkStyleContext *
create_child_context (HdyWindowMixin *self)
{
  GtkStyleContext *parent = gtk_widget_get_style_context (GTK_WIDGET (self->window));
  GtkStyleContext *child = gtk_style_context_new ();

  gtk_style_context_set_parent (child, parent);
  gtk_style_context_set_screen (child, gtk_style_context_get_screen (parent));
  gtk_style_context_set_frame_clock (child, gtk_style_context_get_frame_clock (parent));

  g_signal_connect_object (child,
                           "changed",
                           G_CALLBACK (gtk_widget_queue_draw),
                           self->window,
                           G_CONNECT_SWAPPED);

  return child;
}

static void
update_child_context (HdyWindowMixin  *self,
                      GtkStyleContext *context,
                      const gchar     *name)
{
  g_autoptr (GtkWidgetPath) path = gtk_widget_path_new ();
  GtkStyleContext *parent = gtk_widget_get_style_context (GTK_WIDGET (self->window));
  gint position;

  gtk_widget_path_append_for_widget (path, GTK_WIDGET (self->window));
  position = gtk_widget_path_append_type (path, GTK_TYPE_WIDGET);
  gtk_widget_path_iter_set_object_name (path, position, name);

  gtk_style_context_set_path (context, path);
  gtk_style_context_set_state (context, gtk_style_context_get_state (parent));
}

static void
style_changed_cb (HdyWindowMixin *self)
{
  update_child_context (self, self->decoration_context, "decoration");
  update_child_context (self, self->overlay_context, "decoration-overlay");
}

static gboolean
window_state_event_cb (HdyWindowMixin *self,
                       GdkEvent       *event,
                       GtkWidget      *widget)
{
  style_changed_cb (self);

  return GDK_EVENT_PROPAGATE;
}

static void
size_allocate_cb (HdyWindowMixin *self,
                  GtkAllocation  *alloc)
{
  /* We don't want to allow any other titlebar */
  if (gtk_window_get_titlebar (self->window) != self->titlebar)
    g_error ("gtk_window_set_titlebar() is not supported for HdyWindow");
}

static gboolean
is_fullscreen (HdyWindowMixin *self)
{
  GdkWindow *window = gtk_widget_get_window (GTK_WIDGET (self->window));

  return !!(gdk_window_get_state (window) & GDK_WINDOW_STATE_FULLSCREEN);
}

static gboolean
supports_client_shadow (HdyWindowMixin *self)
{
  GtkStyleContext *context = gtk_widget_get_style_context (GTK_WIDGET (self->window));

  /*
   * GtkWindow adds this when it can't draw proper decorations, e.g. on a
   * non-composited WM on X11. This is documented, so we can rely on this
   * instead of copying the (pretty extensive) check.
   */
  return !gtk_style_context_has_class (context, "solid-csd");
}

static void
max_borders (GtkBorder *one,
             GtkBorder *two)
{
  one->top = MAX (one->top, two->top);
  one->right = MAX (one->right, two->right);
  one->bottom = MAX (one->bottom, two->bottom);
  one->left = MAX (one->left, two->left);
}

static void
get_shadow_width (HdyWindowMixin  *self,
                  GtkStyleContext *context,
                  GtkBorder       *shadow_width)
{
  GtkStateFlags state;
  GtkBorder margin = { 0 };
  GtkAllocation content_alloc, alloc;
  GtkWidget *titlebar;

  *shadow_width = margin;

  if (!gtk_window_get_decorated (self->window))
    return;

  if (gtk_window_is_maximized (self->window) ||
      is_fullscreen (self))
    return;

  if (!gtk_widget_is_toplevel (GTK_WIDGET (self->window)))
    return;

  state = gtk_style_context_get_state (context);

  gtk_style_context_get_margin (context, state, &margin);

  gtk_widget_get_allocation (GTK_WIDGET (self->window), &alloc);
  gtk_widget_get_allocation (self->content, &content_alloc);

  titlebar = gtk_window_get_titlebar (self->window);
  if (titlebar && gtk_widget_get_visible (titlebar)) {
    GtkAllocation titlebar_alloc;

    gtk_widget_get_allocation (titlebar, &titlebar_alloc);

    content_alloc.y = titlebar_alloc.y;
    content_alloc.height += titlebar_alloc.height;
  }

  /*
   * Since we can't get shadow extents the normal way,
   * we have to compare window and content allocation instead.
   */
  shadow_width->left = content_alloc.x - alloc.x;
  shadow_width->right = alloc.width - content_alloc.width - content_alloc.x;
  shadow_width->top = content_alloc.y - alloc.y;
  shadow_width->bottom = alloc.height - content_alloc.height - content_alloc.y;

  max_borders (shadow_width, &margin);
}

static void
create_masks (HdyWindowMixin *self,
               cairo_t        *cr,
               gint            border_radius)
{
  gint scale_factor = gtk_widget_get_scale_factor (GTK_WIDGET (self->window));
  gdouble radius_correction = 0.5 / scale_factor;
  gdouble r = border_radius - radius_correction;
  gint i;

  for (i = 0; i < HDY_N_CORNERS; i++)
    g_clear_pointer (&self->masks[i], cairo_surface_destroy);

  if (r <= 0)
    return;

  for (i = 0; i < HDY_N_CORNERS; i++) {
    g_autoptr (cairo_t) mask_cr = NULL;

    self->masks[i] =
      cairo_surface_create_similar_image (cairo_get_target (cr),
                                          CAIRO_FORMAT_A8,
                                          border_radius * scale_factor,
                                          border_radius * scale_factor);

    mask_cr = cairo_create (self->masks[i]);

    cairo_scale (mask_cr, scale_factor, scale_factor);
    cairo_set_source_rgb (mask_cr, 0, 0, 0);
    cairo_arc (mask_cr,
               (i % 2 == 0) ? r : radius_correction,
               (i / 2 == 0) ? r : radius_correction,
               r,
               0, G_PI * 2);
    cairo_fill (mask_cr);
  }
}

void
hdy_window_mixin_add (HdyWindowMixin *self,
                      GtkWidget      *widget)
{
  if (GTK_IS_POPOVER (widget))
    GTK_CONTAINER_CLASS (self->klass)->add (GTK_CONTAINER (self->window),
                                            widget);
  else {
    g_return_if_fail (self->child == NULL);

    self->child = widget;
    gtk_container_add (GTK_CONTAINER (self->content), widget);
  }
}

void
hdy_window_mixin_remove (HdyWindowMixin *self,
                         GtkWidget      *widget)
{
  GtkWidget *titlebar = gtk_window_get_titlebar (self->window);

  if (widget == self->content ||
      widget == titlebar ||
      GTK_IS_POPOVER (widget))
    GTK_CONTAINER_CLASS (self->klass)->remove (GTK_CONTAINER (self->window),
                                               widget);
  else if (widget == self->child) {
    self->child = NULL;
    gtk_container_remove (GTK_CONTAINER (self->content), widget);
  }
}

void
hdy_window_mixin_forall (HdyWindowMixin *self,
                         gboolean        include_internals,
                         GtkCallback     callback,
                         gpointer        callback_data)
{
  if (include_internals) {
    GTK_CONTAINER_CLASS (self->klass)->forall (GTK_CONTAINER (self->window),
                                               include_internals,
                                               callback,
                                               callback_data);

    return;
  }

  if (self->child)
    (*callback) (self->child, callback_data);
}

typedef struct {
  HdyWindowMixin *self;
  cairo_t *cr;
} HdyWindowMixinDrawData;

static void
draw_popover_cb (GtkWidget              *child,
                 HdyWindowMixinDrawData *data)
{
  HdyWindowMixin *self = data->self;
  GdkWindow *window;
  cairo_t *cr = data->cr;

  if (child == self->content ||
      child == gtk_window_get_titlebar (self->window) ||
      !gtk_widget_get_visible (child) ||
      !gtk_widget_get_child_visible (child))
    return;

  window = gtk_widget_get_window (child);

  if (gtk_widget_get_has_window (child))
    window = gdk_window_get_parent (window);

  if (!gtk_cairo_should_draw_window (cr, window))
    return;

  gtk_container_propagate_draw (GTK_CONTAINER (self->window), child, cr);
}

static inline void
mask_corner (HdyWindowMixin  *self,
             cairo_t         *cr,
             gint             scale_factor,
             gint             corner,
             gint             x,
             gint             y)
{
  cairo_save (cr);
  cairo_scale (cr, 1.0 / scale_factor, 1.0 / scale_factor);
  cairo_mask_surface (cr,
                      self->masks[corner],
                      x * scale_factor,
                      y * scale_factor);
  cairo_restore (cr);
}

gboolean
hdy_window_mixin_draw (HdyWindowMixin *self,
                       cairo_t        *cr)
{
  HdyWindowMixinDrawData data;
  GtkWidget *widget = GTK_WIDGET (self->window);
  GdkWindow *window = gtk_widget_get_window (widget);

  if (gtk_cairo_should_draw_window (cr, window)) {
    GtkStyleContext *context;
    gboolean should_mask_corners;
    GdkRectangle clip = { 0 };
    gint width, height, x, y, w, h, r, scale_factor;
    GtkWidget *titlebar;
    g_autoptr (cairo_surface_t) surface = NULL;
    g_autoptr (cairo_t) surface_cr = NULL;
    GtkBorder shadow;

    /* Use the parent drawing unless we have a reason to use masking */
    if (!gtk_window_get_decorated (self->window) ||
        !supports_client_shadow (self) ||
        is_fullscreen (self))
      return GTK_WIDGET_CLASS (self->klass)->draw (GTK_WIDGET (self->window), cr);

    context = gtk_widget_get_style_context (widget);

    get_shadow_width (self, self->decoration_context, &shadow);

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    x = shadow.left;
    y = shadow.top;
    w = width - shadow.left - shadow.right;
    h = height - shadow.top - shadow.bottom;

    gtk_style_context_get (context,
                           gtk_style_context_get_state (context),
                           GTK_STYLE_PROPERTY_BORDER_RADIUS, &r,
                           NULL);

    r = CLAMP (r, 0, MIN (w / 2, h / 2));

    if (!gdk_cairo_get_clip_rectangle (cr, &clip)) {
      clip.x = 0;
      clip.y = 0;
      clip.width = w;
      clip.height = h;
    }

    gtk_render_background (self->decoration_context, cr, x, y, w, h);
    gtk_render_frame (self->decoration_context, cr, x, y, w, h);

    cairo_save (cr);

    scale_factor = gtk_widget_get_scale_factor (widget);

    if (r * scale_factor != self->last_border_radius) {
      create_masks (self, cr, r);
      self->last_border_radius = r * scale_factor;
    }

    should_mask_corners = !gtk_window_is_maximized (self->window) &&
                          r > 0 &&
                          ((clip.x              < x +     r && clip.y               < y +     r) ||
                           (clip.x              < x +     r && clip.y + clip.height > y + h - r) ||
                           (clip.x + clip.width > x + w - r && clip.y + clip.height > y + h - r) ||
                           (clip.x + clip.width > x + w - r && clip.y               < y +     r));


    if (should_mask_corners) {
      surface = gdk_window_create_similar_surface (window,
                                                   CAIRO_CONTENT_COLOR_ALPHA,
                                                   MAX (clip.width, 1),
                                                   MAX (clip.height, 1));
      surface_cr = cairo_create (surface);
      cairo_surface_set_device_offset (surface, -clip.x * scale_factor, -clip.y * scale_factor);
    } else {
      surface_cr = cairo_reference (cr);
    }

    if (!gtk_widget_get_app_paintable (widget)) {
        gtk_render_background (context, surface_cr, x, y, w, h);
        gtk_render_frame (context, surface_cr, x, y, w, h);
    }

    titlebar = gtk_window_get_titlebar (self->window);

    gtk_container_propagate_draw (GTK_CONTAINER (self->window), self->content, surface_cr);
    gtk_container_propagate_draw (GTK_CONTAINER (self->window), titlebar, surface_cr);

    gtk_render_background (self->overlay_context, surface_cr, x, y, w, h);
    gtk_render_frame (self->overlay_context, surface_cr, x, y, w, h);

    if (should_mask_corners) {
      cairo_set_source_surface (cr, surface, 0, 0);

      cairo_rectangle (cr, x + r, y, w - r * 2, r);
      cairo_rectangle (cr, x + r, y + h - r, w - r * 2, r);
      cairo_rectangle (cr, x, y + r, w, h - r * 2);
      cairo_fill (cr);

      if (clip.x < x + r && clip.y < y + r)
        mask_corner (self, cr, scale_factor,
                     HDY_CORNER_TOP_LEFT, x, y);

      if (clip.x + clip.width > x + w - r && clip.y < y + r)
        mask_corner (self, cr, scale_factor,
                     HDY_CORNER_TOP_RIGHT, x + w - r, y);

      if (clip.x < x + r && clip.y + clip.height > y + h - r)
        mask_corner (self, cr, scale_factor,
                     HDY_CORNER_BOTTOM_LEFT, x, y + h - r);

      if (clip.x + clip.width > x + w - r && clip.y + clip.height > y + h - r)
        mask_corner (self, cr, scale_factor,
                     HDY_CORNER_BOTTOM_RIGHT, x + w - r, y + h - r);

      cairo_surface_flush (surface);
    }

    cairo_restore (cr);
  }

  data.self = self;
  data.cr = cr;
  gtk_container_forall (GTK_CONTAINER (self->window),
                        (GtkCallback) draw_popover_cb,
                        &data);

  return GDK_EVENT_PROPAGATE;
}

void
hdy_window_mixin_destroy (HdyWindowMixin *self)
{
  if (self->content) {
    hdy_window_mixin_remove (self, self->content);
    gtk_widget_destroy (self->content);
    self->content = NULL;
    self->child = NULL;
  }

  GTK_WIDGET_CLASS (self->klass)->destroy (GTK_WIDGET (self->window));
}

void
hdy_window_mixin_buildable_add_child (HdyWindowMixin *self,
                                      GtkBuilder     *builder,
                                      GObject        *child,
                                      const gchar    *type)
{
  GtkBuildable *buildable = GTK_BUILDABLE (self->window);

  if (!type)
    gtk_container_add (GTK_CONTAINER (buildable), GTK_WIDGET (child));
  else
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
}

static void
hdy_window_mixin_finalize (GObject *object)
{
  HdyWindowMixin *self = (HdyWindowMixin *)object;
  gint i;

  for (i = 0; i < HDY_N_CORNERS; i++)
    g_clear_pointer (&self->masks[i], cairo_surface_destroy);
  g_clear_object (&self->decoration_context);
  g_clear_object (&self->overlay_context);

  G_OBJECT_CLASS (hdy_window_mixin_parent_class)->finalize (object);
}

static void
hdy_window_mixin_class_init (HdyWindowMixinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = hdy_window_mixin_finalize;
}

static void
hdy_window_mixin_init (HdyWindowMixin *self)
{
}

HdyWindowMixin *
hdy_window_mixin_new (GtkWindow      *window,
                      GtkWindowClass *klass)
{
  HdyWindowMixin *self;
  GtkStyleContext *context;

  g_return_val_if_fail (GTK_IS_WINDOW (window), NULL);
  g_return_val_if_fail (GTK_IS_WINDOW_CLASS (klass), NULL);
  g_return_val_if_fail (GTK_IS_BUILDABLE (window), NULL);

  self = g_object_new (HDY_TYPE_WINDOW_MIXIN, NULL);

  self->window = window;
  self->klass = klass;

  gtk_widget_add_events (GTK_WIDGET (window), GDK_STRUCTURE_MASK);

  g_signal_connect_object (window,
                           "style-updated",
                           G_CALLBACK (style_changed_cb),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (window,
                           "window-state-event",
                           G_CALLBACK (window_state_event_cb),
                           self,
                           G_CONNECT_SWAPPED | G_CONNECT_AFTER);

  g_signal_connect_object (window,
                           "size-allocate",
                           G_CALLBACK (size_allocate_cb),
                           self,
                           G_CONNECT_SWAPPED);

  self->decoration_context = create_child_context (self);
  self->overlay_context = create_child_context (self);

  style_changed_cb (self);

  self->content = hdy_deck_new ();
  gtk_widget_set_vexpand (self->content, TRUE);
  gtk_widget_show (self->content);
  GTK_CONTAINER_CLASS (self->klass)->add (GTK_CONTAINER (self->window),
                                          self->content);

  self->titlebar = hdy_nothing_new ();
  gtk_widget_set_no_show_all (self->titlebar, TRUE);
  gtk_window_set_titlebar (self->window, self->titlebar);

  context = gtk_widget_get_style_context (GTK_WIDGET (self->window));
  gtk_style_context_add_class (context, "unified");

  return self;
}
