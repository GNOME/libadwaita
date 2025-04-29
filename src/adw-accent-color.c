/*
 * Copyright (C) 2023 Christopher Davis <christopherdavis@gnome.org>
 * Copyright (C) 2024 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-accent-color-private.h"

#include "adw-color-utils-private.h"

/**
 * AdwAccentColor:
 * @ADW_ACCENT_COLOR_BLUE: Use a blue color (`#3584e4`). This is the default value.
 * @ADW_ACCENT_COLOR_TEAL: Use a teal color (`#2190a4`).
 * @ADW_ACCENT_COLOR_GREEN: Use a green color (`#3a944a`).
 * @ADW_ACCENT_COLOR_YELLOW: Use a yellow color (`#c88800`).
 * @ADW_ACCENT_COLOR_ORANGE: Use a orange color (`#ed5b00`).
 * @ADW_ACCENT_COLOR_RED: Use a red color (`#e62d42`).
 * @ADW_ACCENT_COLOR_PINK: Use a pink color (`#d56199`).
 * @ADW_ACCENT_COLOR_PURPLE: Use a purple color (`#9141ac`).
 * @ADW_ACCENT_COLOR_SLATE: Use a slate color (`#6f8396`).
 *
 * Describes the available system accent colors.
 *
 * Since: 1.6
 */

/**
 * adw_accent_color_to_rgba:
 * @self: an accent color
 * @rgba: (out): return location for the color
 *
 * Converts @self to a `GdkRGBA` representing its background color.
 *
 * The matching foreground color is white.
 *
 * Since: 1.6
 */
void
adw_accent_color_to_rgba (AdwAccentColor  self,
                          GdkRGBA        *rgba)
{
  const char *hex = NULL;

  g_return_if_fail (self >= ADW_ACCENT_COLOR_BLUE);
  g_return_if_fail (self <= ADW_ACCENT_COLOR_SLATE);
  g_return_if_fail (rgba != NULL);

  switch (self) {
  case ADW_ACCENT_COLOR_BLUE:
    hex = "#3584e4";
    break;
  case ADW_ACCENT_COLOR_TEAL:
    hex = "#2190a4";
    break;
  case ADW_ACCENT_COLOR_GREEN:
    hex = "#3a944a";
    break;
  case ADW_ACCENT_COLOR_YELLOW:
    hex = "#c88800";
    break;
  case ADW_ACCENT_COLOR_ORANGE:
    hex = "#ed5b00";
    break;
  case ADW_ACCENT_COLOR_RED:
    hex = "#e62d42";
    break;
  case ADW_ACCENT_COLOR_PINK:
    hex = "#d56199";
    break;
  case ADW_ACCENT_COLOR_PURPLE:
    hex = "#9141ac";
    break;
  case ADW_ACCENT_COLOR_SLATE:
    hex = "#6f8396";
    break;
  default:
    g_assert_not_reached ();
  }

  if (rgba)
    gdk_rgba_parse (rgba, hex);
}

/**
 * adw_accent_color_to_standalone_rgba:
 * @self: an accent color
 * @dark: Whether to calculate standalone color for light or dark background
 * @rgba: (out): return location for the color
 *
 * Converts @self to a `GdkRGBA` representing its standalone color.
 *
 * It will typically be darker for light background, and lighter for dark
 * background, ensuring contrast.
 *
 * Since: 1.6
 */
void
adw_accent_color_to_standalone_rgba (AdwAccentColor  self,
                                     gboolean        dark,
                                     GdkRGBA        *rgba)
{
  g_return_if_fail (self >= ADW_ACCENT_COLOR_BLUE);
  g_return_if_fail (self <= ADW_ACCENT_COLOR_SLATE);
  g_return_if_fail (rgba != NULL);

  dark = !!dark;

  adw_accent_color_to_rgba (self, rgba);
  adw_rgba_to_standalone (rgba, dark, rgba);
}

/**
 * adw_rgba_to_standalone:
 * @rgba: a background color
 * @dark: Whether to calculate standalone color for light or dark background
 * @standalone_rgba: (out): return location for the standalone color
 *
 * Adjusts @rgba to be suitable as a standalone color.
 *
 * It will typically be darker for light background, and lighter for dark
 * background, ensuring contrast.
 *
 * Since: 1.6
 */
void
adw_rgba_to_standalone (const GdkRGBA *rgba,
                        gboolean       dark,
                        GdkRGBA       *standalone_rgba)
{
  float L, a, b;

  g_return_if_fail (rgba != NULL);
  g_return_if_fail (standalone_rgba != NULL);

  dark = !!dark;

  adw_rgb_to_oklab (rgba->red, rgba->green, rgba->blue, &L, &a, &b);

  if (dark)
    L = MAX (L, 0.85);
  else
    L = MIN (L, 0.5);

  adw_oklab_to_rgb (L, a, b,
                    &standalone_rgba->red,
                    &standalone_rgba->green,
                    &standalone_rgba->blue);

  standalone_rgba->red =   CLAMP (standalone_rgba->red,   0, 1);
  standalone_rgba->green = CLAMP (standalone_rgba->green, 0, 1);
  standalone_rgba->blue =  CLAMP (standalone_rgba->blue,  0, 1);
  standalone_rgba->alpha = rgba->alpha;
}

AdwAccentColor
adw_accent_color_nearest_from_rgba (GdkRGBA *original_color)
{
  float L, c, h;

  g_return_val_if_fail (original_color != NULL, ADW_ACCENT_COLOR_BLUE);

  adw_rgb_to_oklch (original_color->red,
                    original_color->green,
                    original_color->blue,
                    &L, &c, &h);

  if (c < 0.04)
    return ADW_ACCENT_COLOR_SLATE;

  if (h > 345)
    return ADW_ACCENT_COLOR_PINK;

  if (h > 280)
    return ADW_ACCENT_COLOR_PURPLE;

  if (h > 230)
    return ADW_ACCENT_COLOR_BLUE;

  if (h > 175)
    return ADW_ACCENT_COLOR_TEAL;

  if (h > 115)
    return ADW_ACCENT_COLOR_GREEN;

  if (h > 75.5)
    return ADW_ACCENT_COLOR_YELLOW;

  if (h > 35)
    return ADW_ACCENT_COLOR_ORANGE;

  if (h > 10)
    return ADW_ACCENT_COLOR_RED;

  return ADW_ACCENT_COLOR_PINK;
}
