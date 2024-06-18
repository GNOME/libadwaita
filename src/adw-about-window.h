/*
 * Copyright (C) 2021-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-window.h"

G_BEGIN_DECLS

#define ADW_TYPE_ABOUT_WINDOW (adw_about_window_get_type())

ADW_DEPRECATED_IN_1_6_FOR(AdwAboutDialog)
G_DECLARE_FINAL_TYPE (AdwAboutWindow, adw_about_window, ADW, ABOUT_WINDOW, AdwWindow)

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_new)
GtkWidget *adw_about_window_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_new_from_appdata)
GtkWidget *adw_about_window_new_from_appdata (const char *resource_path,
                                              const char *release_notes_version) G_GNUC_WARN_UNUSED_RESULT;

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_application_name)
const char *adw_about_window_get_application_name (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_application_name)
void        adw_about_window_set_application_name (AdwAboutWindow *self,
                                                   const char     *application_name);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_application_icon)
const char *adw_about_window_get_application_icon (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_application_icon)
void        adw_about_window_set_application_icon (AdwAboutWindow *self,
                                                   const char     *application_icon);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_developer_name)
const char *adw_about_window_get_developer_name (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_developer_name)
void        adw_about_window_set_developer_name (AdwAboutWindow *self,
                                                 const char     *developer_name);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_version)
const char *adw_about_window_get_version (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_version)
void        adw_about_window_set_version (AdwAboutWindow *self,
                                          const char     *version);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_release_notes_version)
const char *adw_about_window_get_release_notes_version (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_release_notes_version)
void        adw_about_window_set_release_notes_version (AdwAboutWindow *self,
                                                        const char     *version);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_release_notes)
const char *adw_about_window_get_release_notes (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_release_notes)
void        adw_about_window_set_release_notes (AdwAboutWindow *self,
                                                const char     *release_notes);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_comments)
const char *adw_about_window_get_comments (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_comments)
void        adw_about_window_set_comments (AdwAboutWindow *self,
                                           const char     *comments);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_website)
const char *adw_about_window_get_website (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_website)
void        adw_about_window_set_website (AdwAboutWindow *self,
                                          const char     *website);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_support_url)
const char *adw_about_window_get_support_url (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_support_url)
void        adw_about_window_set_support_url (AdwAboutWindow *self,
                                              const char     *support_url);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_issue_url)
const char *adw_about_window_get_issue_url (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_issue_url)
void        adw_about_window_set_issue_url (AdwAboutWindow *self,
                                            const char     *issue_url);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_add_link)
void adw_about_window_add_link (AdwAboutWindow *self,
                                const char     *title,
                                const char     *url);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_debug_info)
const char *adw_about_window_get_debug_info (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_debug_info)
void        adw_about_window_set_debug_info (AdwAboutWindow *self,
                                             const char     *debug_info);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_debug_info_filename)
const char *adw_about_window_get_debug_info_filename (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_debug_info_filename)
void        adw_about_window_set_debug_info_filename (AdwAboutWindow *self,
                                                      const char     *filename);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_developers)
const char * const *adw_about_window_get_developers (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_developers)
void                adw_about_window_set_developers (AdwAboutWindow  *self,
                                                     const char     **developers);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_designers)
const char * const *adw_about_window_get_designers (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_designers)
void                adw_about_window_set_designers (AdwAboutWindow  *self,
                                                    const char     **designers);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_artists)
const char * const *adw_about_window_get_artists (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_artists)
void                adw_about_window_set_artists (AdwAboutWindow  *self,
                                                  const char     **artists);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_documenters)
const char * const *adw_about_window_get_documenters (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_documenters)
void                adw_about_window_set_documenters (AdwAboutWindow  *self,
                                                      const char     **documenters);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_translator_credits)
const char *adw_about_window_get_translator_credits (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_translator_credits)
void        adw_about_window_set_translator_credits (AdwAboutWindow *self,
                                                     const char     *translator_credits);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_add_credit_section)
void adw_about_window_add_credit_section (AdwAboutWindow  *self,
                                          const char      *name,
                                          const char     **people);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_add_acknowledgement_section)
void adw_about_window_add_acknowledgement_section (AdwAboutWindow  *self,
                                                   const char      *name,
                                                   const char     **people);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_copyright)
const char *adw_about_window_get_copyright (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_copyright)
void        adw_about_window_set_copyright (AdwAboutWindow *self,
                                            const char     *copyright);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_license_type)
GtkLicense adw_about_window_get_license_type (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_license_type)
void       adw_about_window_set_license_type (AdwAboutWindow *self,
                                              GtkLicense      license_type);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_get_license)
const char *adw_about_window_get_license (AdwAboutWindow *self);
ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_set_license)
void        adw_about_window_set_license (AdwAboutWindow *self,
                                          const char     *license);

ADW_DEPRECATED_IN_1_6_FOR(adw_about_dialog_add_legal_section)
void adw_about_window_add_legal_section (AdwAboutWindow *self,
                                         const char     *title,
                                         const char     *copyright,
                                         GtkLicense      license_type,
                                         const char     *license);

ADW_DEPRECATED_IN_1_6_FOR(adw_show_about_dialog)
void adw_show_about_window (GtkWindow  *parent,
                            const char *first_property_name,
                            ...) G_GNUC_NULL_TERMINATED;

ADW_DEPRECATED_IN_1_6_FOR(adw_show_about_dialog_from_appdata)
void adw_show_about_window_from_appdata (GtkWindow  *parent,
                                         const char *resource_path,
                                         const char *release_notes_version,
                                         const char *first_property_name,
                                         ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS
