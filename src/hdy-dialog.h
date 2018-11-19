/*
 * Copyright © 2018 Zander Brown <zbrown@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_DIALOG (hdy_dialog_get_type())

struct _HdyDialogClass {
  GtkDialogClass parent_class;
};

G_DECLARE_DERIVABLE_TYPE (HdyDialog, hdy_dialog, HDY, DIALOG, GtkDialog)

GtkWidget *hdy_dialog_new (GtkWindow *parent);

G_END_DECLS
