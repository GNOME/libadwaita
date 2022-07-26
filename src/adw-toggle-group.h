/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-button-content.h"
#include "adw-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADW_TOGGLE_DISPLAY_MODE_LABEL,
  ADW_TOGGLE_DISPLAY_MODE_ICON,
  ADW_TOGGLE_DISPLAY_MODE_BOTH,
} AdwToggleDisplayMode;

#define ADW_TYPE_TOGGLE_GROUP (adw_toggle_group_get_type ())

ADW_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (AdwToggleGroup, adw_toggle_group, ADW, TOGGLE_GROUP, GtkWidget)

ADW_AVAILABLE_IN_1_4
GtkWidget *adw_toggle_group_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_4
void adw_toggle_group_add_toggle (AdwToggleGroup  *self,
                                  const char      *toggle,
                                  const char      *label,
                                  const char      *icon_name);

ADW_AVAILABLE_IN_1_4
void adw_toggle_group_remove_toggle (AdwToggleGroup *self,
                                     const char     *toggle);

ADW_AVAILABLE_IN_1_4
const char *adw_toggle_group_get_active (AdwToggleGroup *self);
ADW_AVAILABLE_IN_1_4
void        adw_toggle_group_set_active (AdwToggleGroup *self,
                                         const char     *toggle);

ADW_AVAILABLE_IN_1_4
AdwToggleDisplayMode adw_toggle_group_get_display_mode (AdwToggleGroup       *self);
ADW_AVAILABLE_IN_1_4
void               adw_toggle_group_set_display_mode (AdwToggleGroup       *self,
                                                      AdwToggleDisplayMode  display_mode);

ADW_AVAILABLE_IN_1_4
gboolean adw_toggle_group_has_toggle (AdwToggleGroup *self,
                                      const char     *toggle);

ADW_AVAILABLE_IN_1_4
const char *adw_toggle_group_get_label (AdwToggleGroup *self,
                                        const char     *toggle);
ADW_AVAILABLE_IN_1_4
void        adw_toggle_group_set_label (AdwToggleGroup *self,
                                        const char     *toggle,
                                        const char     *label);

ADW_AVAILABLE_IN_1_4
const char *adw_toggle_group_get_icon_name (AdwToggleGroup *self,
                                            const char     *toggle);
ADW_AVAILABLE_IN_1_4
void        adw_toggle_group_set_icon_name (AdwToggleGroup *self,
                                            const char     *toggle,
                                            const char     *icon_name);

ADW_AVAILABLE_IN_1_4
gboolean adw_toggle_group_get_use_underline (AdwToggleGroup *self,
                                             const char     *toggle);
ADW_AVAILABLE_IN_1_4
void     adw_toggle_group_set_use_underline (AdwToggleGroup *self,
                                             const char     *toggle,
                                             gboolean        use_underline);

G_END_DECLS
