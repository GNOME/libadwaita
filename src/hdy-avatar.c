/*
 * Copyright (C) 2020 Purism SPC
 * Copyright (C) 2020 Felipe Borges
 *
 * Authors:
 * Felipe Borges <felipeborges@gnome.org>
 * Julian Sparber <julian@sparber.net>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 */

#include "config.h"
#include <math.h>

#include "hdy-avatar.h"
#include "hdy-cairo-private.h"

#define NUMBER_OF_COLORS 14
/**
 * SECTION:hdy-avatar
 * @short_description: A widget displaying an image, with a generated fallback.
 * @Title: HdyAvatar
 *
 * #HdyAvatar is a widget to display a round avatar.
 * A provided image is made round before displaying, if no image is given this
 * widget generates a round fallback with the initials of the #HdyAvatar:text
 * on top of a colord background.
 * The color is picked based on the hash of the #HdyAvatar:text.
 * If #HdyAvatar:show-initials is set to %FALSE, `avatar-default-symbolic` is
 * shown in place of the initials.
 * Use hdy_avatar_set_image_load_func () to set a custom image.
 * Create a #HdyAvatarImageLoadFunc similar to this example:
 *
 * |[<!-- language="C" -->
 * static GdkPixbuf *
 * image_load_func (gint size, gpointer user_data)
 * {
 *   g_autoptr (GError) error = NULL;
 *   g_autoptr (GdkPixbuf) pixbuf = NULL;
 *   g_autofree gchar *file = gtk_file_chooser_get_filename ("avatar.png");
 *   gint width, height;
 *
 *   gdk_pixbuf_get_file_info (file, &width, &height);
 *
 *   pixbuf = gdk_pixbuf_new_from_file_at_scale (file,
 *                                              (width <= height) ? size : -1,
 *                                              (width >= height) ? size : -1,
 *                                              TRUE,
 *                                              error);
 *   if (error != NULL) {
 *    g_critical ("Failed to create pixbuf from file: %s", error->message);
 *
 *    return NULL;
 *   }
 *
 *   return pixbuf;
 * }
 * ]|
 *
 * # CSS nodes
 *
 * #HdyAvatar has a single CSS node with name avatar.
 *
 */

struct _HdyAvatar
{
  GtkDrawingArea parent_instance;

  gchar *icon_name;
  gchar *text;
  PangoLayout *layout;
  gboolean show_initials;
  guint color_class;
  gint size;
  cairo_surface_t *round_image;

  HdyAvatarImageLoadFunc load_image_func;
  gpointer load_image_func_target;
  GDestroyNotify load_image_func_target_destroy_notify;
};

G_DEFINE_TYPE (HdyAvatar, hdy_avatar, GTK_TYPE_DRAWING_AREA);

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_TEXT,
  PROP_SHOW_INITIALS,
  PROP_SIZE,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

static cairo_surface_t *
round_image (GdkPixbuf *pixbuf,
             gdouble size)
{
  g_autoptr (cairo_surface_t) surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, size, size);
  g_autoptr (cairo_t) cr = cairo_create (surface);

  /* Clip a circle */
  cairo_arc (cr, size / 2.0, size / 2.0, size / 2.0, 0, 2 * G_PI);
  cairo_clip (cr);
  cairo_new_path (cr);

  gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
  cairo_paint (cr);

  return g_steal_pointer (&surface);
}

static gchar *
extract_initials_from_text (const gchar *text)
{
  GString *initials;
  g_autofree gchar *p = g_utf8_strup (text, -1);
  g_autofree gchar *normalized = g_utf8_normalize (g_strstrip (p), -1, G_NORMALIZE_DEFAULT_COMPOSE);
  gunichar unichar;
  gchar *q = NULL;

  if (normalized == NULL)
    return NULL;

  initials = g_string_new ("");

  unichar = g_utf8_get_char (normalized);
  g_string_append_unichar (initials, unichar);

  q = g_utf8_strrchr (normalized, -1, ' ');
  if (q != NULL && g_utf8_next_char (q) != NULL) {
    q = g_utf8_next_char (q);

    unichar = g_utf8_get_char (q);
    g_string_append_unichar (initials, unichar);
  }

  return g_string_free (initials, FALSE);
}

static void
update_custom_image (HdyAvatar *self)
{
  g_autoptr (GdkPixbuf) pixbuf = NULL;
  gint scale_factor;
  gint size;
  gboolean was_custom = FALSE;

  if (self->round_image != NULL) {
    g_clear_pointer (&self->round_image, cairo_surface_destroy);
    was_custom = TRUE;
  }

  if (self->load_image_func != NULL) {
    scale_factor = gtk_widget_get_scale_factor (GTK_WIDGET (self));
    size = MIN (gtk_widget_get_allocated_width (GTK_WIDGET (self)),
                gtk_widget_get_allocated_height (GTK_WIDGET (self)));
    pixbuf = self->load_image_func (size * scale_factor, self->load_image_func_target);
    if (pixbuf != NULL) {
      self->round_image = round_image (pixbuf, (gdouble) size * scale_factor);
      cairo_surface_set_device_scale (self->round_image, scale_factor, scale_factor);
    }
  }

  if (was_custom || self->round_image != NULL)
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
set_class_color (HdyAvatar *self)
{
  GtkStyleContext *context = gtk_widget_get_style_context (GTK_WIDGET (self));
  g_autofree GRand *rand = NULL;
  g_autofree gchar *new_class = NULL;
  g_autofree gchar *old_class = g_strdup_printf ("color%d", self->color_class);

  gtk_style_context_remove_class (context, old_class);

  if (self->text == NULL || strlen (self->text) == 0) {
    /* Use a random color if we don't have a text */
    rand = g_rand_new ();
    self->color_class = g_rand_int_range (rand, 1, NUMBER_OF_COLORS);
  } else {
    self->color_class = (g_str_hash (self->text) % NUMBER_OF_COLORS) + 1;
  }

  new_class = g_strdup_printf ("color%d", self->color_class);
  gtk_style_context_add_class (context, new_class);
}

static void
set_class_contrasted (HdyAvatar *self, gint size)
{
  GtkStyleContext *context = gtk_widget_get_style_context (GTK_WIDGET (self));

  if (size < 25)
    gtk_style_context_add_class (context, "contrasted");
  else
    gtk_style_context_remove_class (context, "contrasted");
}

static void
clear_pango_layout (HdyAvatar *self)
{
  g_clear_object (&self->layout);
}

static void
ensure_pango_layout (HdyAvatar *self)
{
  g_autofree gchar *initials = NULL;

  if (self->layout != NULL || self->text == NULL || strlen (self->text) == 0)
    return;

  initials = extract_initials_from_text (self->text);
  self->layout = gtk_widget_create_pango_layout (GTK_WIDGET (self), initials);
}

static void
set_font_size (HdyAvatar *self,
               gint size)
{
  GtkStyleContext *context;
  PangoFontDescription *font_desc;
  gint width, height;
  gdouble padding;
  gdouble sqr_size;
  gdouble max_size;
  gdouble new_font_size;

  if (self->round_image != NULL || self->layout == NULL)
    return;

  context = gtk_widget_get_style_context (GTK_WIDGET (self));
  gtk_style_context_get (context, gtk_style_context_get_state (context),
                         "font", &font_desc, NULL);

  pango_layout_set_font_description (self->layout, font_desc);
  pango_layout_get_pixel_size (self->layout, &width, &height);

  /* This is the size of the biggest square fitting inside the circle */
  sqr_size = (gdouble)size / 1.4142;
  /* The padding has to be a function of the overall size.
   * The 0.4 is how steep the linear function grows and the -5 is just
   * an adjustment for smaller sizes which doesn't have a big impact on bigger sizes.
   * Make also sure we don't have a negative padding */
  padding = MAX (size * 0.4 - 5, 0);
  max_size = sqr_size - padding;
  new_font_size = (gdouble)height * (max_size / (gdouble)width);

  font_desc = pango_font_description_copy (font_desc);
  pango_font_description_set_absolute_size (font_desc,
                                            CLAMP (new_font_size, 0, max_size) * PANGO_SCALE);
  pango_layout_set_font_description (self->layout, font_desc);
  pango_font_description_free (font_desc);
}

static void
hdy_avatar_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  HdyAvatar *self = HDY_AVATAR (object);

  switch (property_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, hdy_avatar_get_icon_name (self));
    break;

  case PROP_TEXT:
    g_value_set_string (value, hdy_avatar_get_text (self));
    break;

  case PROP_SHOW_INITIALS:
    g_value_set_boolean (value, hdy_avatar_get_show_initials (self));
    break;

  case PROP_SIZE:
    g_value_set_int (value, hdy_avatar_get_size (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_avatar_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  HdyAvatar *self = HDY_AVATAR (object);

  switch (property_id) {
  case PROP_ICON_NAME:
    hdy_avatar_set_icon_name (self, g_value_get_string (value));
    break;

  case PROP_TEXT:
    hdy_avatar_set_text (self, g_value_get_string (value));
    break;

  case PROP_SHOW_INITIALS:
    hdy_avatar_set_show_initials (self, g_value_get_boolean (value));
    break;

  case PROP_SIZE:
    hdy_avatar_set_size (self, g_value_get_int (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_avatar_finalize (GObject *object)
{
  HdyAvatar *self = HDY_AVATAR (object);

  g_clear_pointer (&self->icon_name, g_free);
  g_clear_pointer (&self->text, g_free);
  g_clear_pointer (&self->round_image, cairo_surface_destroy);
  g_clear_object (&self->layout);

  if (self->load_image_func_target_destroy_notify != NULL)
    self->load_image_func_target_destroy_notify (self->load_image_func_target);

  G_OBJECT_CLASS (hdy_avatar_parent_class)->finalize (object);
}

static gboolean
hdy_avatar_draw (GtkWidget *widget,
                 cairo_t   *cr)
{
  HdyAvatar *self = HDY_AVATAR (widget);
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  gint width = gtk_widget_get_allocated_width (widget);
  gint height = gtk_widget_get_allocated_height (widget);
  gint size = MIN (width, height);
  gdouble x = (gdouble)(width - size) / 2.0;
  gdouble y = (gdouble)(height - size) / 2.0;
  const gchar *icon_name;
  gint scale;
  GdkRGBA color;
  g_autoptr (GtkIconInfo) icon = NULL;
  g_autoptr (GdkPixbuf) pixbuf = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (cairo_surface_t) surface = NULL;

  set_class_contrasted (HDY_AVATAR (widget), size);

  gtk_render_frame (context, cr, x, y, size, size);

  if (self->round_image) {
    cairo_set_source_surface (cr, self->round_image, x, y);
    cairo_paint (cr);

    return FALSE;
  }

  gtk_render_background (context, cr, x, y, size, size);
  ensure_pango_layout (HDY_AVATAR (widget));

  if (self->show_initials && self->layout != NULL) {
    set_font_size (HDY_AVATAR (widget), size);
    pango_layout_get_pixel_size (self->layout, &width, &height);

    gtk_render_layout (context, cr,
                       ((gdouble)(size - width) / 2.0) + x,
                       ((gdouble)(size - height) / 2.0) + y,
                       self->layout);

    return FALSE;
  }

  icon_name = self->icon_name && *self->icon_name != '\0' ?
    self->icon_name : "avatar-default-symbolic";
  scale = gtk_widget_get_scale_factor (widget);
  icon = gtk_icon_theme_lookup_icon_for_scale (gtk_icon_theme_get_default (),
                                     icon_name,
                                     size / 2, scale,
                                     GTK_ICON_LOOKUP_FORCE_SYMBOLIC);
  if (icon == NULL) {
    g_critical ("Failed to load icon `%s'", icon_name);

    return FALSE;
  }

  gtk_style_context_get_color (context, gtk_style_context_get_state (context), &color);
  pixbuf = gtk_icon_info_load_symbolic (icon, &color, NULL, NULL, NULL, NULL, &error);
  if (error != NULL) {
    g_critical ("Failed to load icon `%s': %s", icon_name, error->message);

    return FALSE;
  }

  surface = gdk_cairo_surface_create_from_pixbuf (pixbuf, scale,
                                                  gtk_widget_get_window (widget));

  width = cairo_image_surface_get_width (surface);
  height = cairo_image_surface_get_height (surface);
  gtk_render_icon_surface (context, cr, surface,
                           (((gdouble)size - ((gdouble)width / (gdouble)scale)) / 2.0) + x,
                           (((gdouble)size - ((gdouble)height / (gdouble)scale)) / 2.0) + y);

  return FALSE;
}

/* This private method is prefixed by the class name because it will be a
 * virtual method in GTK 4.
 */
static void
hdy_avatar_measure (GtkWidget      *widget,
                    GtkOrientation  orientation,
                    int             for_size,
                    int            *minimum,
                    int            *natural,
                    int            *minimum_baseline,
                    int            *natural_baseline)
{
  HdyAvatar *self = HDY_AVATAR (widget);

  if (minimum)
    *minimum = self->size;
  if (natural)
    *natural = self->size;
}

static void
hdy_avatar_get_preferred_width (GtkWidget *widget,
                                gint      *minimum,
                                gint      *natural)
{
  hdy_avatar_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                      minimum, natural, NULL, NULL);
}

static void
hdy_avatar_get_preferred_width_for_height (GtkWidget *widget,
                                           gint       height,
                                           gint      *minimum,
                                           gint      *natural)
{
  hdy_avatar_measure (widget, GTK_ORIENTATION_HORIZONTAL, height,
                      minimum, natural, NULL, NULL);
}

static void
hdy_avatar_get_preferred_height (GtkWidget *widget,
                                 gint      *minimum,
                                 gint      *natural)
{
  hdy_avatar_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                      minimum, natural, NULL, NULL);
}

static void
hdy_avatar_get_preferred_height_for_width (GtkWidget *widget,
                                           gint       width,
                                           gint      *minimum,
                                           gint      *natural)
{
  hdy_avatar_measure (widget, GTK_ORIENTATION_VERTICAL, width,
                      minimum, natural, NULL, NULL);
}

static GtkSizeRequestMode
hdy_avatar_get_request_mode (GtkWidget *widget)
{
  return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
hdy_avatar_size_allocate (GtkWidget     *widget,
                          GtkAllocation *allocation)
{
  GtkAllocation clip;

  gtk_render_background_get_clip (gtk_widget_get_style_context (widget),
                                  allocation->x,
                                  allocation->y,
                                  allocation->width,
                                  allocation->height,
                                  &clip);

  GTK_WIDGET_CLASS (hdy_avatar_parent_class)->size_allocate (widget, allocation);
  gtk_widget_set_clip (widget, &clip);
}

static void
hdy_avatar_class_init (HdyAvatarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = hdy_avatar_finalize;

  object_class->set_property = hdy_avatar_set_property;
  object_class->get_property = hdy_avatar_get_property;

  widget_class->draw = hdy_avatar_draw;
  widget_class->get_request_mode = hdy_avatar_get_request_mode;
  widget_class->get_preferred_width = hdy_avatar_get_preferred_width;
  widget_class->get_preferred_height = hdy_avatar_get_preferred_height;
  widget_class->get_preferred_width_for_height = hdy_avatar_get_preferred_width_for_height;
  widget_class->get_preferred_height_for_width = hdy_avatar_get_preferred_height_for_width;
  widget_class->size_allocate = hdy_avatar_size_allocate;

  /**
   * HdyAvatar:size:
   *
   * The avatar size of the avatar.
   */
  props[PROP_SIZE] =
    g_param_spec_int ("size",
                      "Size",
                      "The size of the avatar",
                      -1, INT_MAX, -1,
                      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyAvatar:icon-name:
   *
   * The name of the icon in the icon theme to use when the icon should be
   * displayed.
   * If no name is set, the avatar-default-symbolic icon will be used.
   * If the name doesn't match a valid icon, it is an error and no icon will be
   * displayed.
   * If the icon theme is changed, the image will be updated automatically.
   *
   * Since: 1.0
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         "Icon name",
                         "The name of the icon from the icon theme",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyAvatar:text:
   *
   * The text used for the initials and for generating the color.
   * If #HdyAvatar:show-initials is %FALSE it's only used to generate the color.
   */
  props[PROP_TEXT] =
    g_param_spec_string ("text",
                         "Text",
                         "The text used to generate the color and the initials",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyAvatar:show_initials:
   *
   * Whether to show the initials or the fallback icon on the generated avatar.
   */
  props[PROP_SHOW_INITIALS] =
    g_param_spec_boolean ("show-initials",
                          "Show initials",
                          "Whether to show the initials",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "avatar");
}

static void
hdy_avatar_init (HdyAvatar *self)
{
  set_class_color (self);
  g_signal_connect (self, "notify::scale-factor", G_CALLBACK (update_custom_image), NULL);
  g_signal_connect (self, "size-allocate", G_CALLBACK (update_custom_image), NULL);
  g_signal_connect (self, "screen-changed", G_CALLBACK (clear_pango_layout), NULL);
}

/**
 * hdy_avatar_new:
 * @size: The size of the avatar
 * @text: (nullable): The text used to generate the color and initials if
 * @show_initials is %TRUE. The color is selected at random if @text is empty.
 * @show_initials: whether to show the initials or the fallback icon on
 * top of the color generated based on @text.
 *
 * Creates a new #HdyAvatar.
 *
 * Returns: the newly created #HdyAvatar
 */
GtkWidget *
hdy_avatar_new (gint         size,
                const gchar *text,
                gboolean     show_initials)
{
  return g_object_new (HDY_TYPE_AVATAR,
                       "size", size,
                       "text", text,
                       "show-initials", show_initials,
                       NULL);
}

/**
 * hdy_avatar_get_icon_name:
 * @self: a #HdyAvatar
 *
 * Gets the name of the icon in the icon theme to use when the icon should be
 * displayed.
 *
 * Returns: (nullable) (transfer none): the name of the icon from the icon theme.
 *
 * Since: 1.0
 */
const gchar *
hdy_avatar_get_icon_name (HdyAvatar *self)
{
  g_return_val_if_fail (HDY_IS_AVATAR (self), NULL);

  return self->icon_name;
}

/**
 * hdy_avatar_set_icon_name:
 * @self: a #HdyAvatar
 * @icon_name: (nullable): the name of the icon from the icon theme
 *
 * Sets the name of the icon in the icon theme to use when the icon should be
 * displayed.
 * If no name is set, the avatar-default-symbolic icon will be used.
 * If the name doesn't match a valid icon, it is an error and no icon will be
 * displayed.
 * If the icon theme is changed, the image will be updated automatically.
 *
 * Since: 1.0
 */
void
hdy_avatar_set_icon_name (HdyAvatar   *self,
                          const gchar *icon_name)
{
  g_return_if_fail (HDY_IS_AVATAR (self));

  if (g_strcmp0 (self->icon_name, icon_name) == 0)
    return;

  g_clear_pointer (&self->icon_name, g_free);
  self->icon_name = g_strdup (icon_name);

  if (!self->round_image &&
      (!self->show_initials || self->layout == NULL))
    gtk_widget_queue_draw (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * hdy_avatar_get_text:
 * @self: a #HdyAvatar
 *
 * Get the text used to generate the fallback initials and color
 *
 * Returns: (nullable) (transfer none): returns the text used to generate
 * the fallback initials. This is the internal string used by
 * the #HdyAvatar, and must not be modified.
 */
const gchar *
hdy_avatar_get_text (HdyAvatar *self)
{
  g_return_val_if_fail (HDY_IS_AVATAR (self), NULL);

  return self->text;
}

/**
 * hdy_avatar_set_text:
 * @self: a #HdyAvatar
 * @text: (nullable): the text used to get the initials and color
 *
 * Set the text used to generate the fallback initials color
 */
void
hdy_avatar_set_text (HdyAvatar   *self,
                     const gchar *text)
{
  g_return_if_fail (HDY_IS_AVATAR (self));

  if (g_strcmp0 (self->text, text) == 0)
    return;

  g_clear_pointer (&self->text, g_free);
  self->text = g_strdup (text);

  clear_pango_layout (self);
  set_class_color (self);
  gtk_widget_queue_draw (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TEXT]);
}

/**
 * hdy_avatar_get_show_initials:
 * @self: a #HdyAvatar
 *
 * Returns whether initials are used for the fallback or the icon.
 *
 * Returns: %TRUE if the initials are used for the fallback.
 */
gboolean
hdy_avatar_get_show_initials (HdyAvatar *self)
{
  g_return_val_if_fail (HDY_IS_AVATAR (self), FALSE);

  return self->show_initials;
}

/**
 * hdy_avatar_set_show_initials:
 * @self: a #HdyAvatar
 * @show_initials: whether the initials should be shown on the fallback avatar
 * or the icon.
 *
 * Sets whether the initials should be shown on the fallback avatar or the icon.
 */
void
hdy_avatar_set_show_initials (HdyAvatar *self,
                              gboolean   show_initials)
{
  g_return_if_fail (HDY_IS_AVATAR (self));

  if (self->show_initials == show_initials)
    return;

  self->show_initials = show_initials;

  gtk_widget_queue_draw (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_INITIALS]);
}

/**
 * hdy_avatar_set_image_load_func:
 * @self: a #HdyAvatar
 * @load_image: (closure user_data) (nullable): callback to set a custom image
 * @user_data: (nullable): user data passed to @load_image
 * @destroy: (nullable): destroy notifier for @user_data
 *
 * A callback which is called when the custom image need to be reloaded for some
 * reason (e.g. scale-factor changes).
 */
void
hdy_avatar_set_image_load_func (HdyAvatar *self,
                                HdyAvatarImageLoadFunc load_image,
                                gpointer user_data,
                                GDestroyNotify destroy)
{
  g_return_if_fail (HDY_IS_AVATAR (self));
  g_return_if_fail (user_data != NULL || (user_data == NULL && destroy == NULL));

  if (self->load_image_func_target_destroy_notify != NULL)
    self->load_image_func_target_destroy_notify (self->load_image_func_target);

  self->load_image_func = load_image;
  self->load_image_func_target = user_data;
  self->load_image_func_target_destroy_notify = destroy;

  update_custom_image (self);
}

/**
 * hdy_avatar_get_size:
 * @self: a #HdyAvatar
 *
 * Returns the size of the avatar.
 *
 * Returns: the size of the avatar.
 */
gint
hdy_avatar_get_size (HdyAvatar *self)
{
  g_return_val_if_fail (HDY_IS_AVATAR (self), 0);

  return self->size;
}

/**
 * hdy_avatar_set_size:
 * @self: a #HdyAvatar
 * @size: The size to be used for the avatar
 *
 * Sets the size of the avatar.
 */
void
hdy_avatar_set_size (HdyAvatar *self,
                     gint       size)
{
  g_return_if_fail (HDY_IS_AVATAR (self));
  g_return_if_fail (size >= -1);

  if (self->size == size)
    return;

  self->size = size;

  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIZE]);
}
