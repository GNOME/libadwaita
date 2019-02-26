/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "hdy-list-box.h"

#include <glib/gi18n-lib.h>

/**
 * SECTION:hdy-list-box
 * @short_description: Helper functions for #GtkListBox
 * @Title: GtkListBox helpers
 *
 * Since: 0.0.6
 */

/**
 * hdy_list_box_separator_header:
 * @row: the row to update
 * @before: (allow-none): the row before @row, or %NULL if it is first
 * @unused_user_data: (closure): unused user data
 *
 * Separates rows by using #GtkSeparator as headers. The first row doesn't have
 * a separator as there is no row above it.
 *
 * Since: 0.0.6
 */
void
hdy_list_box_separator_header (GtkListBoxRow *row,
                               GtkListBoxRow *before,
                               gpointer       unused_user_data)
{
  GtkWidget *header;

  g_return_if_fail (GTK_IS_LIST_BOX_ROW (row));
  g_return_if_fail (before == NULL || GTK_IS_LIST_BOX_ROW (before));

  /* No header for the first row. */
  if (before == NULL) {
    gtk_list_box_row_set_header (row, NULL);

    return;
  }

  if (gtk_list_box_row_get_header (row) != NULL)
    return;

  header = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_show (header);
  gtk_list_box_row_set_header (row, header);
}

