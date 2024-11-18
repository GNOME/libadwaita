/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-alert-dialog.h"

G_BEGIN_DECLS

#define ADW_TYPE_MESSAGE_DIALOG (adw_message_dialog_get_type())

ADW_DEPRECATED_IN_1_6_FOR(AdwAlertDialog)
G_DECLARE_DERIVABLE_TYPE (AdwMessageDialog, adw_message_dialog, ADW, MESSAGE_DIALOG, GtkWindow)

struct _AdwMessageDialogClass
{
  GtkWindowClass parent_class;

  void (* response) (AdwMessageDialog *self,
                     const char       *response);

  /*< private >*/
  gpointer padding[4];
};

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_new)
GtkWidget *adw_message_dialog_new (GtkWindow  *parent,
                                   const char *heading,
                                   const char *body);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_heading)
const char *adw_message_dialog_get_heading (AdwMessageDialog *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_heading)
void        adw_message_dialog_set_heading (AdwMessageDialog *self,
                                            const char       *heading);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_heading_use_markup)
gboolean adw_message_dialog_get_heading_use_markup (AdwMessageDialog *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_heading_use_markup)
void     adw_message_dialog_set_heading_use_markup (AdwMessageDialog *self,
                                                    gboolean          use_markup);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_format_heading)
void adw_message_dialog_format_heading (AdwMessageDialog *self,
                                        const char       *format,
                                        ...) G_GNUC_PRINTF (2, 3);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_format_heading_markup)
void adw_message_dialog_format_heading_markup (AdwMessageDialog *self,
                                               const char       *format,
                                               ...) G_GNUC_PRINTF (2, 3);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_body)
const char *adw_message_dialog_get_body (AdwMessageDialog *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_body)
void        adw_message_dialog_set_body (AdwMessageDialog *self,
                                         const char       *body);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_body_use_markup)
gboolean adw_message_dialog_get_body_use_markup (AdwMessageDialog *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_body_use_markup)
void     adw_message_dialog_set_body_use_markup (AdwMessageDialog *self,
                                                 gboolean          use_markup);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_format_body)
void adw_message_dialog_format_body (AdwMessageDialog *self,
                                     const char       *format,
                                     ...) G_GNUC_PRINTF (2, 3);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_format_body_markup)
void adw_message_dialog_format_body_markup (AdwMessageDialog *self,
                                            const char       *format,
                                            ...) G_GNUC_PRINTF (2, 3);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_extra_child)
GtkWidget *adw_message_dialog_get_extra_child (AdwMessageDialog *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_extra_child)
void       adw_message_dialog_set_extra_child (AdwMessageDialog *self,
                                               GtkWidget        *child);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_add_response)
void adw_message_dialog_add_response (AdwMessageDialog *self,
                                      const char       *id,
                                      const char       *label);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_remove_response)
void adw_message_dialog_remove_response (AdwMessageDialog *self,
                                         const char       *id);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_add_responses)
void adw_message_dialog_add_responses (AdwMessageDialog *self,
                                       const char       *first_id,
                                       ...) G_GNUC_NULL_TERMINATED;

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_response_label)
const char *adw_message_dialog_get_response_label (AdwMessageDialog *self,
                                                   const char       *response);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_response_label)
void        adw_message_dialog_set_response_label (AdwMessageDialog *self,
                                                   const char       *response,
                                                   const char       *label);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_response_appearance)
AdwResponseAppearance adw_message_dialog_get_response_appearance (AdwMessageDialog      *self,
                                                                  const char            *response);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_response_appearance)
void                  adw_message_dialog_set_response_appearance (AdwMessageDialog      *self,
                                                                  const char            *response,
                                                                  AdwResponseAppearance  appearance);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_response_enabled)
gboolean adw_message_dialog_get_response_enabled (AdwMessageDialog *self,
                                                  const char       *response);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_response_enabled)
void     adw_message_dialog_set_response_enabled (AdwMessageDialog *self,
                                                  const char       *response,
                                                  gboolean          enabled);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_default_response)
const char *adw_message_dialog_get_default_response (AdwMessageDialog *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_default_response)
void        adw_message_dialog_set_default_response (AdwMessageDialog *self,
                                                     const char       *response);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_get_close_response)
const char *adw_message_dialog_get_close_response (AdwMessageDialog *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_set_close_response)
void        adw_message_dialog_set_close_response (AdwMessageDialog *self,
                                                   const char       *response);

ADW_DEPRECATED_IN_1_6
void adw_message_dialog_response (AdwMessageDialog *self,
                                  const char       *response);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_has_response)
gboolean adw_message_dialog_has_response (AdwMessageDialog *self,
                                          const char       *response);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_choose)
void        adw_message_dialog_choose        (AdwMessageDialog    *self,
                                              GCancellable        *cancellable,
                                              GAsyncReadyCallback  callback,
                                              gpointer             user_data);

ADW_DEPRECATED_IN_1_6_FOR(adw_alert_dialog_choose_finish)
const char *adw_message_dialog_choose_finish (AdwMessageDialog    *self,
                                              GAsyncResult        *result);

G_END_DECLS
