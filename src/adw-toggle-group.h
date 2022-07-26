/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 * Copyright (C) 2024 GNOME Foundation Inc.
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

G_BEGIN_DECLS

#define ADW_TYPE_TOGGLE (adw_toggle_get_type ())

ADW_AVAILABLE_IN_1_7
G_DECLARE_FINAL_TYPE (AdwToggle, adw_toggle, ADW, TOGGLE, GObject)

ADW_AVAILABLE_IN_1_7
AdwToggle *adw_toggle_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_7
const char *adw_toggle_get_name (AdwToggle  *self);
ADW_AVAILABLE_IN_1_7
void        adw_toggle_set_name (AdwToggle  *self,
                                 const char *name);

ADW_AVAILABLE_IN_1_7
const char *adw_toggle_get_label (AdwToggle  *self);
ADW_AVAILABLE_IN_1_7
void        adw_toggle_set_label (AdwToggle  *self,
                                  const char *label);

ADW_AVAILABLE_IN_1_7
gboolean adw_toggle_get_use_underline (AdwToggle *self);
ADW_AVAILABLE_IN_1_7
void     adw_toggle_set_use_underline (AdwToggle *self,
                                       gboolean   use_underline);

ADW_AVAILABLE_IN_1_7
const char *adw_toggle_get_icon_name (AdwToggle  *self);
ADW_AVAILABLE_IN_1_7
void        adw_toggle_set_icon_name (AdwToggle  *self,
                                      const char *icon_name);

ADW_AVAILABLE_IN_1_7
const char *adw_toggle_get_tooltip (AdwToggle  *self);
ADW_AVAILABLE_IN_1_7
void        adw_toggle_set_tooltip (AdwToggle  *self,
                                    const char *tooltip);

ADW_AVAILABLE_IN_1_7
GtkWidget *adw_toggle_get_child (AdwToggle *self);
ADW_AVAILABLE_IN_1_7
void       adw_toggle_set_child (AdwToggle *self,
                                 GtkWidget *child);

ADW_AVAILABLE_IN_1_7
gboolean adw_toggle_get_enabled (AdwToggle *self);
ADW_AVAILABLE_IN_1_7
void     adw_toggle_set_enabled (AdwToggle *self,
                                 gboolean   enabled);

ADW_AVAILABLE_IN_1_7
guint adw_toggle_get_index (AdwToggle *self);

#define ADW_TYPE_TOGGLE_GROUP (adw_toggle_group_get_type ())

ADW_AVAILABLE_IN_1_7
G_DECLARE_FINAL_TYPE (AdwToggleGroup, adw_toggle_group, ADW, TOGGLE_GROUP, GtkWidget)

ADW_AVAILABLE_IN_1_7
GtkWidget *adw_toggle_group_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_7
void adw_toggle_group_add (AdwToggleGroup *self,
                           AdwToggle      *toggle);
ADW_AVAILABLE_IN_1_7
void adw_toggle_group_remove (AdwToggleGroup *self,
                              AdwToggle      *toggle);

ADW_AVAILABLE_IN_1_7
void adw_toggle_group_remove_all (AdwToggleGroup *self);

ADW_AVAILABLE_IN_1_7
AdwToggle *adw_toggle_group_get_toggle         (AdwToggleGroup *self,
                                                guint           index);
ADW_AVAILABLE_IN_1_7
AdwToggle *adw_toggle_group_get_toggle_by_name (AdwToggleGroup *self,
                                                const char     *name);

ADW_AVAILABLE_IN_1_7
guint adw_toggle_group_get_n_toggles (AdwToggleGroup *self);

ADW_AVAILABLE_IN_1_7
guint adw_toggle_group_get_active (AdwToggleGroup *self);
ADW_AVAILABLE_IN_1_7
void  adw_toggle_group_set_active (AdwToggleGroup *self,
                                   guint           active);

ADW_AVAILABLE_IN_1_7
const char *adw_toggle_group_get_active_name (AdwToggleGroup *self);
ADW_AVAILABLE_IN_1_7
void        adw_toggle_group_set_active_name (AdwToggleGroup *self,
                                              const char     *name);

ADW_AVAILABLE_IN_1_7
gboolean adw_toggle_group_get_homogeneous (AdwToggleGroup *self);
ADW_AVAILABLE_IN_1_7
void     adw_toggle_group_set_homogeneous (AdwToggleGroup *self,
                                           gboolean        homogeneous);

ADW_AVAILABLE_IN_1_7
gboolean adw_toggle_group_get_can_shrink (AdwToggleGroup *self);
ADW_AVAILABLE_IN_1_7
void     adw_toggle_group_set_can_shrink (AdwToggleGroup *self,
                                          gboolean        can_shrink);

ADW_AVAILABLE_IN_1_7
GtkSelectionModel *adw_toggle_group_get_toggles (AdwToggleGroup *self) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
