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
  { N_("Desktop Shell"), 32, 0  },
  { N_("Mobile Shell"),  32, 18 },
  { N_("Phosh"),         32, 15 },
  { N_("Fullscreen"),    0,  0 },
  { N_("Custom"),        -1, -1 },
};

// Mobile shell
#define DEFAULT_SHELL_PRESET 1

typedef struct {
  const char *name;
  int width;
  int height;
  float scale;
  float corners;
  const char *notches;
} DevicePreset;

static const DevicePreset device_presets[] = {
  { N_("Small Phone"), 360,  720, 2.0, 0, NULL },
  { N_("OnePlus 6"),   360,  760, 3.0, 71, "M 357 0  A 24 24 0 0 1 381 22  A 64 64 0 0 0 445 80  L 635 80  A 64 64 0 0 0 699 22  A 24 24 0 0 1 723 0  Z" },
  { N_("OnePlus 6T"),  360,  770, 3.0, 75, "M 355,0  h 368.34  c -9.77,0.44 -19.57,0.08 -29.28,1.24  c -20.33,1.14 -41.18,5.17 -58.62,16.24  c -16.9,10.79 -29.44,26.78 -43.44,40.81  a 72.73,72.73 0 0 1 -38.29 19.58  c -16.53,2.51 -34,1 -49.09,-6.62  c -9.85,-4.62 -17.88,-12.24 -25.21,-20.18  c -10.46,-11.27 -20.9,-22.75 -33.53,-31.66  c -11.49,-8 -24.9,-12.78 -38.53,-15.42  c -17.27,-3.18 -34.86,-3.6 -52.35,-3.99  Z" },
  { N_("Tablet"),      1280, 800, 1.0, 0, NULL },
  { N_("Custom"),      -1,   -1,  1.0, 0, NULL },
};

// L5, PP
#define DEFAULT_DEVICE_PRESET 0
