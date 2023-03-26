/* Color utilities
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Simon Budig <Simon.Budig@unix-ag.org> (original code)
 *          Federico Mena-Quintero <federico@gimp.org> (cleanup for GTK+)
 *          Jonathan Blandford <jrb@redhat.com> (cleanup for GTK+)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#include "config.h"
#include "adw-color-utils-private.h"

#include <math.h>

#define DEG_TO_RAD(x) ((x) * G_PI / 180)
#define RAD_TO_DEG(x) ((x) * 180 / G_PI)

static inline void
_sincosf (float  angle,
          float *out_s,
          float *out_c)
{
#ifdef HAVE_SINCOSF
  sincosf (angle, out_s, out_c);
#else
  *out_s = sinf (angle);
  *out_c = cosf (angle);
#endif
}

static void
oklab_to_oklch (float  L,  float  a, float  b,
                float *L2, float *C, float *H)
{
  *L2 = L;
  *C = hypotf (a, b);
  *H = RAD_TO_DEG (atan2 (b, a));
  *H = fmod (*H, 360);
  if (*H < 0)
    *H += 360;
}

static void
oklch_to_oklab (float  L,  float  C, float  H,
                float *L2, float *a, float *b)
{
  *L2 = L;
  _sincosf (DEG_TO_RAD (H), b, a);
  *a *= C;
  *b *= C;
}

static float
apply_gamma (float v)
{
  if (v > 0.0031308)
    return 1.055 * pow (v, 1/2.4) - 0.055;
  else
    return 12.92 * v;
}

static float
unapply_gamma (float v)
{
  if (v >= 0.04045)
    return pow (((v + 0.055)/(1 + 0.055)), 2.4);
  else
    return v / 12.92;
}

static void
oklab_to_linear_srgb (float  L,   float  a,     float  b,
                      float *red, float *green, float *blue)
{
  float l = L + 0.3963377774f * a + 0.2158037573f * b;
  float m = L - 0.1055613458f * a - 0.0638541728f * b;
  float s = L - 0.0894841775f * a - 1.2914855480f * b;

  l = powf (l, 3);
  m = powf (m, 3);
  s = powf (s, 3);

  *red = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
  *green = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
  *blue = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;
}

static void
linear_srgb_to_oklab (float  red, float  green, float  blue,
                      float *L,   float *a,     float *b)
{
  float l = 0.4122214708f * red + 0.5363325363f * green + 0.0514459929f * blue;
  float m = 0.2119034982f * red + 0.6806995451f * green + 0.1073969566f * blue;
  float s = 0.0883024619f * red + 0.2817188376f * green + 0.6299787005f * blue;

  l = cbrtf (l);
  m = cbrtf (m);
  s = cbrtf (s);

  *L = 0.2104542553f*l + 0.7936177850f*m - 0.0040720468f*s;
  *a = 1.9779984951f*l - 2.4285922050f*m + 0.4505937099f*s;
  *b = 0.0259040371f*l + 0.7827717662f*m - 0.8086757660f*s;
}

static void
rgb_to_linear_srgb (float  red,        float  green,        float  blue,
                    float *linear_red, float *linear_green, float *linear_blue)
{
  *linear_red = unapply_gamma (red);
  *linear_green = unapply_gamma (green);
  *linear_blue = unapply_gamma (blue);
}

static void
linear_srgb_to_rgb (float  linear_red, float  linear_green, float  linear_blue,
                    float *red,        float *green,        float *blue)
{
  *red = apply_gamma (linear_red);
  *green = apply_gamma (linear_green);
  *blue = apply_gamma (linear_blue);
}

void
adw_oklab_to_rgb (float  L,   float  a,     float  b,
                  float *red, float *green, float *blue)
{
  float linear_red, linear_green, linear_blue;
  oklab_to_linear_srgb (L, a, b, &linear_red, &linear_green, &linear_blue);
  linear_srgb_to_rgb (linear_red, linear_green, linear_blue, red, green, blue);
}

void
adw_rgb_to_oklab (float  red, float  green, float  blue,
                  float *L,   float *a,     float *b)
{
  float linear_red, linear_green, linear_blue;
  rgb_to_linear_srgb (red, green, blue, &linear_red, &linear_green, &linear_blue);
  linear_srgb_to_oklab (linear_red, linear_green, linear_blue, L, a, b);
}

void
adw_oklch_to_rgb (float  L,   float  c,     float  h,
                  float *red, float *green, float *blue)
{
  float l, a, b;
  oklch_to_oklab (L, c, h, &l, &a, &b);
  adw_oklab_to_rgb (l, a, b, red, green, blue);
}

void
adw_rgb_to_oklch (float  red, float  green, float  blue,
                  float *L,   float *c,     float *h)
{
  float l, a, b;
  adw_rgb_to_oklab (red, green, blue, &l, &a, &b);
  oklab_to_oklch (l, a, b, L, c, h);
}
