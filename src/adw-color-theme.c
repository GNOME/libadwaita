/*
 * Copyright (C) 2021 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-color-theme-private.h"

#include "adw-macros-private.h"
#include "adw-settings-private.h"
#include <math.h>
#include <gtk/gtk.h>

struct _AdwColorTheme
{
  GObject parent_instance;

  GHashTable *colors;
  gboolean dark;
  char *color_css;
};

G_DEFINE_FINAL_TYPE (AdwColorTheme, adw_color_theme, G_TYPE_OBJECT);

#define WHITE GDK_RGBA ("ffffff")
#define BLACK GDK_RGBA ("000000")
/*
 * Adapted from https://gitlab.gnome.org/World/design/palette/-/blob/4aaa771584f3084f56502597d823c867500b90ef/src/palette.vala#L202
 * (C) Zander Brown, Tobias Bernard
 */
#define USE_DARK(r, g, b) \
(((r * 255.0) * 0.299) + ((g * 255.0) * 0.587) + ((b * 255.0) * 0.114)) > 160.0

typedef struct {
  float hue;
  float saturation;
  float lightness;
  float alpha;
} AdwHSLA;

enum {
  SIGNAL_COLORS_CHANGED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

enum {
  PROP_0,
  PROP_IS_DARK,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
adw_hsla_from_rgba (GdkRGBA *rgba,
                    AdwHSLA *out)
{
  float cmin = MIN (MIN (rgba->red, rgba->green), rgba->blue);
  float cmax = MAX (MAX (rgba->red, rgba->green), rgba->blue);
  float delta = cmax - cmin;

  if (delta == 0.0f)
    out->hue = 0.0f;
  else if (cmax == rgba->red)
    out->hue = fmodf ((rgba->green - rgba->blue) / delta, 6.0f);
  else if (cmax == rgba->green)
    out->hue = (rgba->blue - rgba->red) / delta + 2.0f;
  else
    out->hue = (rgba->red - rgba->green) / delta + 4.0f;

  out->hue = roundf (out->hue * 60.0f);
  if (out->hue < 0.0f)
    out->hue += 360.0f;

  out->lightness = (cmax + cmin) / 2.0f;

  out->saturation = delta == 0.0f ? 0.0f : delta / (1.0f - fabsf (2.0f * out->lightness - 1.0f));

  out->saturation *= 100.0f;
  out->lightness *= 100.0f;

  out->alpha = rgba->alpha;
}

static void
adw_hsla_to_rgba (AdwHSLA *hsla,
                  GdkRGBA *out)
{
  float hue = hsla->hue;
  float saturation = hsla->saturation / 100.0f;
  float lightness = hsla->lightness / 100.0f;
  float chroma = (1.0f - fabsf (2.0f * lightness - 1.0f)) * saturation;
  float secondary_component = chroma * (1.0f - fabsf (fmodf (hue / 60.0f, 2.0f) - 1.0f));
  float match = lightness - chroma / 2.0f;

  out->red = 0.0f;
  out->green = 0.0f;
  out->blue = 0.0f;

  if (0.0f <= hue && hue < 60.0f) {
    out->red = chroma;
    out->green = secondary_component;
    out->blue = 0.0f;
  } else if (60.0f <= hue && hue < 120.0f) {
    out->red = secondary_component;
    out->green = chroma;
    out->blue = 0.0f;
  } else if (120.0f <= hue && hue < 180.0f) {
    out->red = 0.0f;
    out->green = chroma;
    out->blue = secondary_component;
  } else if  (180.0f <= hue && hue < 240.0f) {
    out->red = 0.0f;
    out->green = secondary_component;
    out->blue = chroma;
  } else if (240.0f <= hue && hue < 300.0f) {
    out->red = secondary_component;
    out->green = 0.0f;
    out->blue = chroma;
  } else if (300.0f <= hue && hue < 360.0f) {
    out->red = chroma;
    out->green = 0.0f;
    out->blue = secondary_component;
  }

  out->red = roundf ((out->red + match) * 255.0f) / 255.0f;
  out->green = roundf ((out->green + match) * 255.0f) / 255.0f;
  out->blue = roundf ((out->blue + match) * 255.0f) / 255.0f;
  out->alpha = hsla->alpha;
}

static void
set_color (AdwColorTheme *self,
           const char    *color,
           GdkRGBA       *rgba)
{
  g_hash_table_replace (self->colors, g_strdup (color), gdk_rgba_copy (rgba));
}

static void
transparent_black (float    alpha,
                   GdkRGBA *out)
{
  out->red = 0.0f;
  out->green = 0.0f;
  out->blue = 0.0f;
  out->alpha = 1.0f - alpha;
}

static void
transparent_white (float    alpha,
                   GdkRGBA *out)
{
  out->red = 1.0f;
  out->green = 1.0f;
  out->blue = 1.0f;
  out->alpha = 1.0f - alpha;
}

static void
calculate_shade (GdkRGBA *bg,
                 GdkRGBA *out)
{
  if (USE_DARK (bg->red, bg->green, bg->blue))
    transparent_black (0.93, out);
  else
    transparent_black (0.64, out);
}

static void
calculate_backdrop (GdkRGBA *bg,
                    GdkRGBA *out)
{
  AdwHSLA hsla;

  adw_hsla_from_rgba (bg, &hsla);

  if (hsla.lightness < 50.0f)
    hsla.lightness -= 5.0f;
  else
    hsla.lightness += 6.0f;

  adw_hsla_to_rgba (&hsla, out);
}

static void
calculate_scrollbar_outline (GdkRGBA *bg,
                             GdkRGBA *out)
{
  AdwHSLA hsla;

  adw_hsla_from_rgba (bg, &hsla);

  if (hsla.lightness < 50.0f) {
    transparent_black (0.5, out);
  } else {
    out->red = 1.0f;
    out->green = 1.0f;
    out->blue = 1.0f;
    out->alpha = 1.0f;
  }

}

static void
calculate_accent_fg (GdkRGBA *bg,
                     GdkRGBA *out)
{
  if (USE_DARK (bg->red, bg->green, bg->blue)) {
    transparent_black (0.2, out);
  } else {
    out->red = 1.0f;
    out->green = 1.0f;
    out->blue = 1.0f;
    out->alpha = 1.0f;
  }
}

static void
hueshift_accent (AdwColorTheme *self,
                 GdkRGBA       *accent,
                 GdkRGBA       *out)
{
  AdwHSLA hsla;

  adw_hsla_from_rgba (accent, &hsla);

  if (self->dark) {
    hsla.lightness += 22.0f;
  } else {
    hsla.saturation += 1.0f;
    hsla.lightness -= 7.0f;
  }

  adw_hsla_to_rgba (&hsla, out);
}

static void
adw_color_theme_constructed (GObject *object)
{
  AdwColorTheme *self = ADW_COLOR_THEME (object);
  GdkRGBA default_shade;
  GdkRGBA default_fg;
  GdkRGBA main_color;
  GdkRGBA calculated_color;

  G_OBJECT_CLASS (adw_color_theme_parent_class)->constructed (object);

  transparent_black (self->dark ? 0.64 : 0.93, &default_shade);
  self->dark ? default_fg = WHITE : transparent_black (0.2, &default_fg);

  main_color = GDK_RGBA ("3584e4");
  set_color (self, "accent_bg_color", &main_color);

  calculate_accent_fg (&main_color, &calculated_color);
  set_color (self, "accent_fg_color", &calculated_color);

  hueshift_accent (self, &main_color, &calculated_color);
  set_color (self, "accent_color", &calculated_color);

  main_color = GDK_RGBA ("e01b24");
  set_color (self, "destructive_bg_color", &main_color);

  calculate_accent_fg (&main_color, &calculated_color);
  set_color (self, "destructive_fg_color", &calculated_color);

  hueshift_accent (self, &main_color, &calculated_color);
  set_color (self, "destructive_color", &calculated_color);

  main_color = GDK_RGBA ("33d17a");
  set_color (self, "success_color", &main_color);

  main_color = GDK_RGBA ("e5a50a");
  set_color (self, "warning_color", &main_color);

  main_color = GDK_RGBA ("e01b24");
  set_color (self, "error_color", &main_color);

  main_color = self->dark ? GDK_RGBA ("1e1e1e") : WHITE;
  set_color (self, "view_bg_color", &main_color);

  calculated_color = self->dark ? WHITE : BLACK;
  set_color (self, "view_fg_color", &calculated_color);

  main_color = self->dark ? GDK_RGBA ("303030") : GDK_RGBA ("ebebeb");
  set_color (self, "headerbar_bg_color", &main_color);
  set_color (self, "headerbar_fg_color", &default_fg);
  set_color (self, "headerbar_border_color", &default_fg);
  set_color (self, "headerbar_shade_color", &default_shade);

  calculate_backdrop (&main_color, &calculated_color);
  set_color (self, "headerbar_backdrop_color", &calculated_color);

  if (self->dark)
    transparent_white (0.92, &main_color);
  else
    main_color = WHITE;

  set_color (self, "card_bg_color", &main_color);
  set_color (self, "card_fg_color", &default_fg);
  set_color (self, "card_shade_color", &default_shade);

  main_color = self->dark ? GDK_RGBA ("383838") : WHITE;
  set_color (self, "popover_bg_color", &main_color);
  set_color (self, "popover_fg_color", &default_fg);

  main_color = self->dark ? GDK_RGBA ("242424") : GDK_RGBA ("fafafa");
  set_color (self, "window_bg_color", &main_color);
  set_color (self, "window_fg_color", &default_fg);
  set_color (self, "shade_color", &default_shade);

  calculate_scrollbar_outline (&main_color, &calculated_color);
  set_color (self, "scrollbar_outline_color", &calculated_color);
}

static void
adw_color_theme_dispose (GObject *object)
{
  AdwColorTheme *self = ADW_COLOR_THEME (object);

  g_clear_pointer (&self->colors, g_hash_table_unref);
  g_clear_pointer (&self->color_css, g_free);

  G_OBJECT_CLASS (adw_color_theme_parent_class)->dispose (object);
}

static void
adw_color_theme_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdwColorTheme *self = ADW_COLOR_THEME (object);

  switch (prop_id) {
  case PROP_IS_DARK:
    g_value_set_boolean (value, self->dark);
    break;
  default:
    g_assert_not_reached ();
    break;
  }
}

static void
adw_color_theme_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  AdwColorTheme *self = ADW_COLOR_THEME (object);

  switch (prop_id) {
  case PROP_IS_DARK:
    self->dark = g_value_get_boolean (value);
    break;
  default:
    g_assert_not_reached ();
    break;
  }
}

static void
adw_color_theme_class_init (AdwColorThemeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_color_theme_constructed;
  object_class->dispose = adw_color_theme_dispose;
  object_class->get_property = adw_color_theme_get_property;
  object_class->set_property = adw_color_theme_set_property;

  props[PROP_IS_DARK] =
    g_param_spec_boolean ("is-dark",
                          "Dark",
                          "Whether the color theme is for a dark style or not",
                          FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  signals[SIGNAL_COLORS_CHANGED] =
    g_signal_new ("colors-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_color_theme_init (AdwColorTheme *self)
{
  self->colors = g_hash_table_new_full (g_str_hash,
                                        g_str_equal,
                                        g_free,
                                        (GDestroyNotify) gdk_rgba_free);
}

static char *
color_to_string (AdwColor color)
{
  g_autoptr (GEnumClass) klass = g_type_class_ref (ADW_TYPE_COLOR);
  GEnumValue *val = g_enum_get_value (klass, color);
  GString *str = g_string_new (val->value_nick);

  g_string_replace (str, "-", "_", 0);

  return g_string_free (str, FALSE);
}

/**
 * adw_color_theme_new_light:
 *
 * Creates a new light `AdwColorTheme`.
 *
 * Returns: the newly created `AdwColorTheme`
 *
 * Since: 1.0
 */
AdwColorTheme *
adw_color_theme_new_light (void)
{
  return g_object_new (ADW_TYPE_COLOR_THEME,
                       "is-dark", FALSE,
                       NULL);
}

/**
 * adw_color_theme_new_dark:
 *
 * Creates a new dark `AdwColorTheme`.
 *
 * Returns: the newly created `AdwColorTheme`
 *
 * Since: 1.0
 */
AdwColorTheme *
adw_color_theme_new_dark (void)
{
  return g_object_new (ADW_TYPE_COLOR_THEME,
                       "is-dark", TRUE,
                       NULL);
}

/**
 * adw_color_theme_set_color_from_rgba:
 * @self: a `AdwColorTheme`
 * @color: which color to set
 * @rgba: A [struct@GdkRGBA] color value
 *
 * Sets the `color` to `rgba`
 *
 * Since: 1.0
 */
void
adw_color_theme_set_color_from_rgba (AdwColorTheme *self,
                                     AdwColor       color,
                                     GdkRGBA       *rgba)
{
  GdkRGBA fg;
  GdkRGBA accent;
  GdkRGBA shade;
  GdkRGBA outline;
  GdkRGBA backdrop;
  g_autofree char *color_key = color_to_string (color);

  g_return_if_fail (ADW_IS_COLOR_THEME (self));

  switch (color) {
  case ADW_COLOR_ACCENT_BG_COLOR:
    calculate_accent_fg (rgba, &fg);
    hueshift_accent (self, rgba, &accent);

    set_color (self, "accent_bg_color", rgba);
    set_color (self, "accent_fg_color", &fg);
    set_color (self, "accent_color", &accent);
    break;
  case ADW_COLOR_DESTRUCTIVE_BG_COLOR:
    calculate_accent_fg (rgba, &fg);
    hueshift_accent (self, rgba, &accent);

    set_color (self, "destructive_bg_color", rgba);
    set_color (self, "destructive_fg_color", &fg);
    set_color (self, "destructive_color", &accent);
    break;
  case ADW_COLOR_WINDOW_BG_COLOR:
    calculate_shade (rgba, &shade);
    calculate_scrollbar_outline (rgba, &outline);

    set_color (self, color_key, rgba);
    set_color (self, "shade_color", &shade);
    set_color (self, "scrollbar_outline_color", &outline);
    break;
  case ADW_COLOR_HEADERBAR_BG_COLOR:
    calculate_shade (rgba, &shade);
    calculate_backdrop (rgba, &backdrop);

    set_color (self, color_key, rgba);
    set_color (self, "headerbar_shade_color", &shade);
    set_color (self, "headerbar_backdrop_color", &backdrop);
    break;
  case ADW_COLOR_CARD_BG_COLOR:
    calculate_shade (rgba, &shade);

    set_color (self, color_key, rgba);
    set_color (self, "card_shade_color", &shade);
    break;
  case ADW_COLOR_SUCCESS_COLOR:
  case ADW_COLOR_WARNING_COLOR:
  case ADW_COLOR_ERROR_COLOR:
  case ADW_COLOR_WINDOW_FG_COLOR:
  case ADW_COLOR_VIEW_FG_COLOR:
  case ADW_COLOR_VIEW_BG_COLOR:
  case ADW_COLOR_HEADERBAR_FG_COLOR:
  case ADW_COLOR_HEADERBAR_BORDER_COLOR:
  case ADW_COLOR_CARD_FG_COLOR:
  case ADW_COLOR_POPOVER_FG_COLOR:
  case ADW_COLOR_POPOVER_BG_COLOR:
    set_color (self, color_key, rgba);
    break;
  default:
    g_assert_not_reached ();
    break;
  }

  g_signal_emit (self, signals[SIGNAL_COLORS_CHANGED], 0);
}

/**
 * adw_color_theme_get_color:
 * @self: a `AdwColorTheme`
 * @color: which color to get
 *
 * Gets the [struct@GdkRGBA] representation of `color`
 *
 * Returns: (transfer none): A [struct@GdkRGBA] color value
 *
 * Since: 1.0
 */
GdkRGBA *
adw_color_theme_get_color (AdwColorTheme *self,
                           AdwColor       color)
{
  g_return_val_if_fail (ADW_IS_COLOR_THEME (self), NULL);

  return (GdkRGBA *) g_hash_table_lookup (self->colors, color_to_string (color));
}

/**
 * adw_color_theme_get_css:
 * @self: a `AdwColorTheme`
 *
 * Gets the css generated by the `AdwColorTheme`
 *
 * Returns: (transfer none): the css generated by the `AdwColorTheme`
 *
 * Since: 1.0
 */
char *
adw_color_theme_get_css (AdwColorTheme *self)
{
  GHashTableIter iter;
  const char *name;
  GdkRGBA *rgba;
  GString *str = g_string_new ("");

  g_return_val_if_fail (ADW_IS_COLOR_THEME (self), NULL);

  g_hash_table_iter_init (&iter, self->colors);

  while (g_hash_table_iter_next (&iter, (gpointer *) &name, (gpointer *) &rgba)) {
    g_autofree char *rgba_string = gdk_rgba_to_string (rgba);
    g_string_append_printf (str, "@define-color %s %s;\n", name, rgba_string);
  }

  return g_string_free (str, FALSE);
}
