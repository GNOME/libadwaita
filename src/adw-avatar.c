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

#include "adw-avatar.h"
#include "adw-gizmo-private.h"

#define NUMBER_OF_COLORS 14
/**
 * AdwAvatar:
 *
 * A widget displaying an image, with a generated fallback.
 *
 * `AdwAvatar` is a widget that shows a round avatar.
 *
 * `AdwAvatar` generates an avatar with the initials of  the
 * [property@Adw.Avatar:text] on top of a colored background.
 *
 * The color is picked based on the hash of the [property@Adw.Avatar:text].
 *
 * If [property@Adw.Avatar:show-initials] is set to `FALSE`,
 * [property@Adw.Avatar:icon-name] or `avatar-default-symbolic` is shown instead
 * of the initials.
 *
 * [method@Adw.Avatar.set_image_load_func] can be used to provide a custom
 * image.
 *
 * ## CSS nodes
 *
 * `AdwAvatar` has a single CSS node with name `avatar`.
 *
 * Since: 1.0
 */

struct _AdwAvatar
{
  GtkWidget parent_instance;

  GtkWidget *gizmo;
  GtkLabel *label;
  GtkImage *icon;
  GtkImage *custom_image;

  char *icon_name;
  char *text;
  gboolean show_initials;
  guint color_class;
  int size;
  GdkTexture *texture;
  int round_image_size;

  AdwAvatarImageLoadFunc load_image_func;
  gpointer load_image_func_target;
  GDestroyNotify load_image_func_target_destroy_notify;
};

G_DEFINE_TYPE (AdwAvatar, adw_avatar, GTK_TYPE_WIDGET);

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_TEXT,
  PROP_SHOW_INITIALS,
  PROP_SIZE,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

static char *
extract_initials_from_text (const char *text)
{
  GString *initials;
  g_autofree char *p = g_utf8_strup (text, -1);
  g_autofree char *normalized = g_utf8_normalize (g_strstrip (p), -1, G_NORMALIZE_DEFAULT_COMPOSE);
  gunichar unichar;
  char *q = NULL;

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
update_visibility (AdwAvatar *self)
{
  gboolean has_custom_image = self->texture != NULL;
  gboolean has_initials = self->show_initials && self->text && strlen (self->text);

  gtk_widget_set_visible (GTK_WIDGET (self->label), !has_custom_image && has_initials);
  gtk_widget_set_visible (GTK_WIDGET (self->icon), !has_custom_image && !has_initials);
  gtk_widget_set_visible (GTK_WIDGET (self->custom_image), has_custom_image);
}

static void
update_custom_image (AdwAvatar *self,
                     int        width,
                     int        height,
                     int        scale_factor)
{
  g_autoptr (GdkPixbuf) pixbuf = NULL;
  int new_size;

  new_size = MIN (width, height) * scale_factor;

  if (self->round_image_size != new_size && self->texture != NULL) {
    self->round_image_size = -1;
  }
  g_clear_object (&self->texture);

  if (self->load_image_func != NULL && self->texture == NULL) {
    pixbuf = self->load_image_func (new_size, self->load_image_func_target);
    if (pixbuf != NULL) {
      if (width != height) {
        GdkPixbuf *subpixbuf;

        subpixbuf = gdk_pixbuf_new_subpixbuf (pixbuf,
                                              (width - new_size) / 2,
                                              (height - new_size) / 2,
                                              new_size,
                                              new_size);

        g_object_unref (pixbuf);

        pixbuf = subpixbuf;
      }

      self->texture = gdk_texture_new_for_pixbuf (pixbuf);
      self->round_image_size = new_size;
    }
  }

  if (self->texture)
    gtk_widget_add_css_class (self->gizmo, "image");
  else
    gtk_widget_remove_css_class (self->gizmo, "image");

  gtk_image_set_from_paintable (self->custom_image, GDK_PAINTABLE (self->texture));
  update_visibility (self);
}

static void
set_class_color (AdwAvatar *self)
{
  g_autofree GRand *rand = NULL;
  g_autofree char *new_class = NULL;
  g_autofree char *old_class = g_strdup_printf ("color%d", self->color_class);

  gtk_widget_remove_css_class (self->gizmo, old_class);

  if (self->text == NULL || strlen (self->text) == 0) {
    /* Use a random color if we don't have a text */
    rand = g_rand_new ();
    self->color_class = g_rand_int_range (rand, 1, NUMBER_OF_COLORS);
  } else {
    self->color_class = (g_str_hash (self->text) % NUMBER_OF_COLORS) + 1;
  }

  new_class = g_strdup_printf ("color%d", self->color_class);

  gtk_widget_add_css_class (self->gizmo, new_class);
}

static void
update_initials (AdwAvatar *self)
{
  g_autofree char *initials = NULL;

  if (self->texture != NULL ||
      !self->show_initials ||
      self->text == NULL ||
      strlen (self->text) == 0)
    return;

  initials = extract_initials_from_text (self->text);

  gtk_label_set_label (self->label, initials);
}

static void
update_icon (AdwAvatar *self)
{
  if (self->icon_name)
    gtk_image_set_from_icon_name (self->icon, self->icon_name);
  else
    gtk_image_set_from_icon_name (self->icon, "avatar-default-symbolic");
}

static void
update_font_size (AdwAvatar *self)
{
  int width, height;
  double padding;
  double sqr_size;
  double max_size;
  double new_font_size;
  PangoAttrList *attributes;

  if (self->texture != NULL ||
      !self->show_initials ||
      self->text == NULL ||
      strlen (self->text) == 0)
    return;

  /* Reset font size first to avoid rounding errors */
  attributes = pango_attr_list_new ();
  gtk_label_set_attributes (self->label, attributes);

  pango_layout_get_pixel_size (gtk_label_get_layout (self->label), &width, &height);

  /* This is the size of the biggest square fitting inside the circle */
  sqr_size = (double) self->size / 1.4142;
  /* The padding has to be a function of the overall size.
   * The 0.4 is how steep the linear function grows and the -5 is just
   * an adjustment for smaller sizes which doesn't have a big impact on bigger sizes.
   * Make also sure we don't have a negative padding */
  padding = MAX (self->size * 0.4 - 5, 0);
  max_size = sqr_size - padding;
  new_font_size = (double) height * (max_size / (double) width);

  pango_attr_list_change (attributes, pango_attr_size_new_absolute (CLAMP (new_font_size, 0, max_size) * PANGO_SCALE));
  gtk_label_set_attributes (self->label, attributes);

  pango_attr_list_unref (attributes);
}

static void
adw_avatar_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdwAvatar *self = ADW_AVATAR (object);

  switch (property_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_avatar_get_icon_name (self));
    break;

  case PROP_TEXT:
    g_value_set_string (value, adw_avatar_get_text (self));
    break;

  case PROP_SHOW_INITIALS:
    g_value_set_boolean (value, adw_avatar_get_show_initials (self));
    break;

  case PROP_SIZE:
    g_value_set_int (value, adw_avatar_get_size (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_avatar_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdwAvatar *self = ADW_AVATAR (object);

  switch (property_id) {
  case PROP_ICON_NAME:
    adw_avatar_set_icon_name (self, g_value_get_string (value));
    break;

  case PROP_TEXT:
    adw_avatar_set_text (self, g_value_get_string (value));
    break;

  case PROP_SHOW_INITIALS:
    adw_avatar_set_show_initials (self, g_value_get_boolean (value));
    break;

  case PROP_SIZE:
    adw_avatar_set_size (self, g_value_get_int (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_avatar_dispose (GObject *object)
{
  AdwAvatar *self = ADW_AVATAR (object);

  g_clear_pointer (&self->gizmo, gtk_widget_unparent);

  self->label = NULL;
  self->icon = NULL;
  self->custom_image = NULL;

  G_OBJECT_CLASS (adw_avatar_parent_class)->dispose (object);
}

static void
adw_avatar_finalize (GObject *object)
{
  AdwAvatar *self = ADW_AVATAR (object);

  g_clear_pointer (&self->icon_name, g_free);
  g_clear_pointer (&self->text, g_free);
  g_clear_object (&self->texture);

  if (self->load_image_func_target_destroy_notify != NULL)
    self->load_image_func_target_destroy_notify (self->load_image_func_target);

  G_OBJECT_CLASS (adw_avatar_parent_class)->finalize (object);
}

static void
adw_avatar_class_init (AdwAvatarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_avatar_dispose;
  object_class->finalize = adw_avatar_finalize;
  object_class->set_property = adw_avatar_set_property;
  object_class->get_property = adw_avatar_get_property;

  /**
   * AdwAvatar:icon-name: (attributes org.gtk.Property.get=adw_avatar_get_icon_name org.gtk.Property.set=adw_avatar_set_icon_name)
   *
   * The name of an icon to use as a fallback.
   *
   * If no name is set, `avatar-default-symbolic` will be used.
   *
   * Since: 1.0
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         "Icon name",
                         "The name of an icon to use as a fallback",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAvatar:text: (attributes org.gtk.Property.get=adw_avatar_get_text org.gtk.Property.set=adw_avatar_set_text)
   *
   * Sets the text used to generate the fallback initials and color.
   *
   * It's only used to generate the color if [property@Adw.Avatar:show-initials]
   * is `FALSE`.
   *
   * Since: 1.0
   */
  props[PROP_TEXT] =
    g_param_spec_string ("text",
                         "Text",
                         "The text used to generate the color and the initials",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAvatar:show-initials: (attributes org.gtk.Property.get=adw_avatar_get_show_initials org.gtk.Property.set=adw_avatar_set_show_initials)
   *
   * Whether initials are used instead of an icon on the fallback avatar.
   *
   * See [property@Adw.Avatar:icon-name] for how to change the fallback icon.
   *
   * Since: 1.0
   */
  props[PROP_SHOW_INITIALS] =
    g_param_spec_boolean ("show-initials",
                          "Show initials",
                          "Whether initials are used instead of an icon on the fallback avatar",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAvatar:size: (attributes org.gtk.Property.get=adw_avatar_get_size org.gtk.Property.set=adw_avatar_set_size)
   *
   * The size of the avatar.
   *
   * Since: 1.0
   */
  props[PROP_SIZE] =
    g_param_spec_int ("size",
                      "Size",
                      "The size of the avatar",
                      -1, INT_MAX, -1,
                      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
adw_avatar_init (AdwAvatar *self)
{
  self->gizmo = adw_gizmo_new ("avatar", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_overflow (self->gizmo, GTK_OVERFLOW_HIDDEN);
  gtk_widget_set_halign (self->gizmo, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (self->gizmo, GTK_ALIGN_CENTER);
  gtk_widget_set_layout_manager (self->gizmo, gtk_bin_layout_new ());
  gtk_widget_set_parent (self->gizmo, GTK_WIDGET (self));

  self->label = GTK_LABEL (gtk_label_new (NULL));
  gtk_widget_set_parent (GTK_WIDGET (self->label), self->gizmo);

  self->icon = GTK_IMAGE (gtk_image_new ());
  gtk_widget_set_parent (GTK_WIDGET (self->icon), self->gizmo);

  self->custom_image = GTK_IMAGE (gtk_image_new ());
  gtk_widget_set_parent (GTK_WIDGET (self->custom_image), self->gizmo);

  set_class_color (self);
  update_initials (self);
  update_font_size (self);
  update_icon (self);
  update_visibility (self);

  g_signal_connect (self, "notify::scale-factor", G_CALLBACK (update_custom_image), NULL);
  g_signal_connect (self, "notify::root", G_CALLBACK (update_font_size), NULL);
}

/**
 * adw_avatar_new:
 * @size: The size of the avatar
 * @text: (nullable): the text used to get the initials and color
 * @show_initials: whether to use initials instead of an icon as fallback
 *
 * Creates a new `AdwAvatar`.
 *
 * Returns: the newly created `AdwAvatar`
 *
 * Since: 1.0
 */
GtkWidget *
adw_avatar_new (int         size,
                const char *text,
                gboolean    show_initials)
{
  return g_object_new (ADW_TYPE_AVATAR,
                       "size", size,
                       "text", text,
                       "show-initials", show_initials,
                       NULL);
}

/**
 * adw_avatar_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: a `AdwAvatar`
 *
 * Gets the name of an icon to use as a fallback.
 *
 * Returns: (nullable): the icon name
 *
 * Since: 1.0
 */
const char *
adw_avatar_get_icon_name (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), NULL);

  return self->icon_name;
}

/**
 * adw_avatar_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: a `AdwAvatar`
 * @icon_name: (nullable): the icon name
 *
 * Sets the name of an icon to use as a fallback.
 *
 * If no name is set, `avatar-default-symbolic` will be used.
 *
 * Since: 1.0
 */
void
adw_avatar_set_icon_name (AdwAvatar  *self,
                          const char *icon_name)
{
  g_return_if_fail (ADW_IS_AVATAR (self));

  if (g_strcmp0 (self->icon_name, icon_name) == 0)
    return;

  g_clear_pointer (&self->icon_name, g_free);
  self->icon_name = g_strdup (icon_name);

  update_icon (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adw_avatar_get_text: (attributes org.gtk.Method.get_property=text)
 * @self: a `AdwAvatar`
 *
 * Gets the text used to generate the fallback initials and color.
 *
 * Returns: (nullable): the text used to generate the fallback initials and
 *   color
 *
 * Since: 1.0
 */
const char *
adw_avatar_get_text (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), NULL);

  return self->text;
}

/**
 * adw_avatar_set_text: (attributes org.gtk.Method.set_property=text)
 * @self: a `AdwAvatar`
 * @text: (nullable): the text used to get the initials and color
 *
 * Sets the text used to generate the fallback initials and color.
 *
 * Since: 1.0
 */
void
adw_avatar_set_text (AdwAvatar  *self,
                     const char *text)
{
  g_return_if_fail (ADW_IS_AVATAR (self));

  if (g_strcmp0 (self->text, text) == 0)
    return;

  g_clear_pointer (&self->text, g_free);
  self->text = g_strdup (text);

  set_class_color (self);

  update_initials (self);
  update_font_size (self);
  update_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TEXT]);
}

/**
 * adw_avatar_get_show_initials: (attributes org.gtk.Method.get_property=show-initials)
 * @self: a `AdwAvatar`
 *
 * Gets whether initials are used instead of an icon on the fallback avatar.
 *
 * Returns: whether initials are used instead of an icon as fallback
 *
 * Since: 1.0
 */
gboolean
adw_avatar_get_show_initials (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), FALSE);

  return self->show_initials;
}

/**
 * adw_avatar_set_show_initials: (attributes org.gtk.Method.set_property=show-initials)
 * @self: a `AdwAvatar`
 * @show_initials: whether to use initials instead of an icon as fallback
 *
 * Sets whether to use initials instead of an icon on the fallback avatar.
 *
 * Since: 1.0
 */
void
adw_avatar_set_show_initials (AdwAvatar *self,
                              gboolean   show_initials)
{
  g_return_if_fail (ADW_IS_AVATAR (self));

  if (self->show_initials == show_initials)
    return;

  self->show_initials = show_initials;

  update_initials (self);
  update_font_size (self);
  update_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_INITIALS]);
}

/**
 * adw_avatar_set_image_load_func:
 * @self: a `AdwAvatar`
 * @load_image: (scope notified) (nullable): callback to set a custom image
 * @user_data: (closure load_image) (nullable): user data passed to @load_image
 * @destroy: (destroy user_data) (nullable): destroy notifier for @user_data
 *
 * Sets a function that loads a custom image.
 *
 * @load_image will be called when the custom image needs to be reloaded for
 * some reason (e.g. scale-factor changes).
 *
 * Since: 1.0
 */
void
adw_avatar_set_image_load_func (AdwAvatar              *self,
                                AdwAvatarImageLoadFunc  load_image,
                                gpointer                user_data,
                                GDestroyNotify          destroy)
{
  g_return_if_fail (ADW_IS_AVATAR (self));
  g_return_if_fail (user_data != NULL || (user_data == NULL && destroy == NULL));

  if (self->load_image_func_target_destroy_notify != NULL)
    self->load_image_func_target_destroy_notify (self->load_image_func_target);

  self->load_image_func = load_image;
  self->load_image_func_target = user_data;
  self->load_image_func_target_destroy_notify = destroy;

  update_custom_image (self,
                       gtk_widget_get_width (GTK_WIDGET (self)),
                       gtk_widget_get_height (GTK_WIDGET (self)),
                       gtk_widget_get_scale_factor (GTK_WIDGET (self)));
}

/**
 * adw_avatar_get_size: (attributes org.gtk.Method.get_property=size)
 * @self: a `AdwAvatar`
 *
 * Gets the size of the avatar.
 *
 * Returns: the size of the avatar
 *
 * Since: 1.0
 */
int
adw_avatar_get_size (AdwAvatar *self)
{
  g_return_val_if_fail (ADW_IS_AVATAR (self), 0);

  return self->size;
}

/**
 * adw_avatar_set_size: (attributes org.gtk.Method.set_property=size)
 * @self: a `AdwAvatar`
 * @size: The size of the avatar
 *
 * Sets the size of the avatar.
 *
 * Since: 1.0
 */
void
adw_avatar_set_size (AdwAvatar *self,
                     int        size)
{
  g_return_if_fail (ADW_IS_AVATAR (self));
  g_return_if_fail (size >= -1);

  if (self->size == size)
    return;

  self->size = size;

  gtk_widget_set_size_request (self->gizmo, size, size);
  gtk_image_set_pixel_size (self->icon, size / 2);

  if (size < 25)
    gtk_widget_add_css_class (self->gizmo, "contrasted");
  else
    gtk_widget_remove_css_class (self->gizmo, "contrasted");

  update_font_size (self);
  update_custom_image (self,
                       gtk_widget_get_width (GTK_WIDGET (self)),
                       gtk_widget_get_height (GTK_WIDGET (self)),
                       gtk_widget_get_scale_factor (GTK_WIDGET (self)));

  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIZE]);
}

/**
 * adw_avatar_draw_to_pixbuf:
 * @self: a `AdwAvatar`
 * @size: The size of the pixbuf
 * @scale_factor: The scale factor
 *
 * Renders @self into a [class@GdkPixbuf.Pixbuf] at @size and @scale_factor.
 *
 * This can be used to export the fallback avatar.
 *
 * Returns: (transfer full): the pixbuf
 *
 * Since: 1.0
 */
GdkPixbuf *
adw_avatar_draw_to_pixbuf (AdwAvatar *self,
                           int        size,
                           int        scale_factor)
{
  GtkSnapshot *snapshot;
  g_autoptr (GskRenderNode) node = NULL;
  cairo_surface_t *surface;
  cairo_t *cr;
  graphene_rect_t bounds;

  g_return_val_if_fail (ADW_IS_AVATAR (self), NULL);
  g_return_val_if_fail (size > 0, NULL);
  g_return_val_if_fail (scale_factor > 0, NULL);

  update_custom_image (self, size, size, scale_factor);

  snapshot = gtk_snapshot_new ();
  GTK_WIDGET_GET_CLASS (self)->snapshot (GTK_WIDGET (self), snapshot);

  node = gtk_snapshot_free_to_node (snapshot);

  gsk_render_node_get_bounds (node, &bounds);
  graphene_rect_round_to_pixel (&bounds);
  graphene_rect_scale (&bounds, scale_factor, scale_factor, &bounds);

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                        bounds.size.width,
                                        bounds.size.height);
  cairo_surface_set_device_scale (surface, scale_factor, scale_factor);
  cr = cairo_create (surface);

  cairo_translate (cr, -bounds.origin.x, -bounds.origin.y);

  gsk_render_node_draw (node, cr);

  return gdk_pixbuf_get_from_surface (surface, 0, 0,
                                      bounds.size.width,
                                      bounds.size.height);
  return NULL;
}
