/* adw-tag-widget-private.h
 *
 * SPDX-FileCopyrightText: 2022 Emmanuele Bassi
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "adw-tag.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_TAG_WIDGET (adw_tag_widget_get_type())

G_DECLARE_FINAL_TYPE (AdwTagWidget, adw_tag_widget, ADW, TAG_WIDGET, GtkWidget)

AdwTag *adw_tag_widget_get_tag (AdwTagWidget *self);

G_END_DECLS
