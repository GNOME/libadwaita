/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <glib/gi18n-lib.h>

typedef struct {
  const char *name;
  int top_bar;
  int bottom_bar;
} ShellPreset;

static const ShellPreset shell_presets[] = {
  { NC_("Shell preset", "Desktop Shell"), 32, 0  },
  { NC_("Shell preset", "Mobile Shell"),  32, 18 },
  { NC_("Shell preset", "Phosh"),         32, 15 },
  { NC_("Shell preset", "Fullscreen"),    0,  0 },
  { NC_("Shell preset", "Custom"),        -1, -1 },
};

// Mobile shell
#define DEFAULT_SHELL_PRESET 1

typedef struct {
  const char *id;
  const char *name;
  int width;
  int height;

  /* In inches */
  float screen_diagonal;

  float scale_factor;

  /* In pixels, to match gmobile */
  float top_screen_corners;
  float bottom_screen_corners;
  const char *notches;

  /* In inches so that they can be filled in from specs */
  float top_bezel;
  float side_bezel;
  float bottom_bezel;
  float top_device_corners;
  float bottom_device_corners;
} DevicePreset;

static const DevicePreset device_presets[] = {
/* These are too similar to expose each of them
  {
    "librem5",
    NC_("Device preset", "Librem 5"),
    720, 1440,
    5.7, 2.0,
    0, 0,
    NULL,
    0.462, 0.201, 0.462, 0.4, 0.4,
  },
  {
    "pinephone",
    NC_("Device preset", "PinePhone"),
    720, 1440,
    5.95, 2.0,
    0, 0,
    NULL,
    0.498, 0.176, 0.498, 0.4, 0.4,
  },
  {
    "pinephone-pro",
    NC_("Device preset", "PinePhone Pro"),
    720, 1440,
    6.0, 2.0,
    0, 0,
    NULL,
    0.482, 0.166, 0.482, 0.4, 0.4,
  },
*/
  {
    "generic-phone",
    NC_("Device preset", "Generic Phone"),
    720, 1440,
    5.85, 2.0,
    0, 0,
    NULL,
    0.48, 0.18, 0.48, 0.4, 0.4,
  },
  {
    "generic-tablet",
    NC_("Device preset", "Generic Tablet"),
    1280, 800,
    10.0, 1.0,
    0, 0,
    NULL,
    0.25, 0.25, 0.25, 0.25, 0.25,
  },
  {
    "oneplus6",
    NC_("Device preset", "OnePlus 6"),
    1080, 2280,
    6.28, 3.0,
    80, 60,
    "M 357 0  A 24 24 0 0 1 381 22  A 64 64 0 0 0 445 80  L 635 80  A 64 64 0 0 0 699 22  A 24 24 0 0 1 723 0  Z",
    0.139, 0.139, 0.275, 0.33, 0.33,
  },
  {
    "oneplus6t",
    NC_("Device preset", "OnePlus 6T"),
    1080, 2340,
    6.41, 3.0,
    120, 120,
    "M 355,0  h 368.34  c -9.77,0.44 -19.57,0.08 -29.28,1.24  c -20.33,1.14 -41.18,5.17 -58.62,16.24  c -16.9,10.79 -29.44,26.78 -43.44,40.81  a 72.73,72.73 0 0 1 -38.29 19.58  c -16.53,2.51 -34,1 -49.09,-6.62  c -9.85,-4.62 -17.88,-12.24 -25.21,-20.18  c -10.46,-11.27 -20.9,-22.75 -33.53,-31.66  c -11.49,-8 -24.9,-12.78 -38.53,-15.42  c -17.27,-3.18 -34.86,-3.6 -52.35,-3.99  Z",
    0.129, 0.129, 0.253, 0.425, 0.425,
  },
  {
    "custom",
    NC_("Device preset", "Custom"),
    -1, -1,
    1.0, 1.0,
    0, 0, NULL,
    0, 0, 0, 0, 0,
  },
};

// Generic Phone
#define DEFAULT_DEVICE_PRESET 0
