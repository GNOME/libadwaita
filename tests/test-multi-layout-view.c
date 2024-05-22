/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_multi_layout_view_add_remove (void)
{
  AdwMultiLayoutView *view = g_object_ref_sink (ADW_MULTI_LAYOUT_VIEW (adw_multi_layout_view_new ()));
  GtkWidget *content1 = g_object_ref_sink (adw_bin_new ());
  GtkWidget *content2 = g_object_ref_sink (adw_bin_new ());
  AdwLayout *layout1 = adw_layout_new (content1);
  AdwLayout *layout2 = adw_layout_new (content2);

  g_assert_nonnull (view);

  adw_multi_layout_view_add_layout (view, g_object_ref (layout1));
  g_assert_true (adw_multi_layout_view_get_layout (view) == layout1);

  adw_multi_layout_view_add_layout (view, g_object_ref (layout2));
  g_assert_true (adw_multi_layout_view_get_layout (view) == layout1);

  adw_multi_layout_view_remove_layout (view, layout1);
  g_assert_true (adw_multi_layout_view_get_layout (view) == layout2);

  g_assert_finalize_object (view);
  g_assert_finalize_object (layout1);
  g_assert_finalize_object (layout2);
  g_assert_finalize_object (content1);
  g_assert_finalize_object (content2);
}

static void
test_adw_multi_layout_view_layout (void)
{
  AdwMultiLayoutView *view = g_object_ref_sink (ADW_MULTI_LAYOUT_VIEW (adw_multi_layout_view_new ()));
  GtkWidget *content1 = g_object_ref_sink (adw_bin_new ());
  GtkWidget *content2 = g_object_ref_sink (adw_bin_new ());
  AdwLayout *layout1 = adw_layout_new (content1);
  AdwLayout *layout2 = adw_layout_new (content2);
  int notified = 0;
  AdwLayout *layout;

  g_assert_nonnull (view);

  g_signal_connect_swapped (view, "notify::layout", G_CALLBACK (increment), &notified);

  g_object_get (view, "layout", &layout, NULL);
  g_assert_null (layout);

  g_assert_cmpint (notified, ==, 0);

  adw_multi_layout_view_add_layout (view, g_object_ref (layout1));

  g_assert_true (adw_multi_layout_view_get_layout (view) == layout1);
  g_assert_cmpint (notified, ==, 1);

  adw_multi_layout_view_add_layout (view, g_object_ref (layout2));

  g_assert_cmpint (notified, ==, 1);

  adw_multi_layout_view_set_layout (view, layout2);

  g_assert_true (adw_multi_layout_view_get_layout (view) == layout2);
  g_assert_cmpint (notified, ==, 2);

  g_object_set (view, "layout", layout1, NULL);
  g_assert_true (adw_multi_layout_view_get_layout (view) == layout1);
  g_assert_cmpint (notified, ==, 3);

  g_assert_finalize_object (view);
  g_assert_finalize_object (layout1);
  g_assert_finalize_object (layout2);
  g_assert_finalize_object (content1);
  g_assert_finalize_object (content2);
}

static void
test_adw_multi_layout_view_layout_name (void)
{
  AdwMultiLayoutView *view = g_object_ref_sink (ADW_MULTI_LAYOUT_VIEW (adw_multi_layout_view_new ()));
  GtkWidget *content1 = g_object_ref_sink (adw_bin_new ());
  GtkWidget *content2 = g_object_ref_sink (adw_bin_new ());
  AdwLayout *layout1 = adw_layout_new (content1);
  AdwLayout *layout2 = adw_layout_new (content2);
  int notified = 0;
  const char *layout_name;

  g_assert_nonnull (view);

  adw_layout_set_name (layout1, "layout1");
  adw_layout_set_name (layout2, "layout2");

  g_signal_connect_swapped (view, "notify::layout-name", G_CALLBACK (increment), &notified);

  g_object_get (view, "layout-name", &layout_name, NULL);
  g_assert_null (layout_name);

  g_assert_cmpint (notified, ==, 0);

  adw_multi_layout_view_add_layout (view, g_object_ref (layout1));

  g_assert_true (adw_multi_layout_view_get_layout (view) == layout1);
  g_assert_cmpstr (adw_multi_layout_view_get_layout_name (view), ==, "layout1");
  g_assert_cmpint (notified, ==, 1);

  adw_multi_layout_view_add_layout (view, g_object_ref (layout2));

  g_assert_cmpint (notified, ==, 1);

  adw_multi_layout_view_set_layout_name (view, "layout2");

  g_assert_true (adw_multi_layout_view_get_layout (view) == layout2);
  g_assert_cmpstr (adw_multi_layout_view_get_layout_name (view), ==, "layout2");
  g_assert_cmpint (notified, ==, 2);

  g_object_set (view, "layout-name", "layout1", NULL);
  g_assert_true (adw_multi_layout_view_get_layout (view) == layout1);
  g_assert_cmpstr (adw_multi_layout_view_get_layout_name (view), ==, "layout1");
  g_assert_cmpint (notified, ==, 3);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*not found*");
  adw_multi_layout_view_set_layout_name (view, "layout3");
  g_test_assert_expected_messages ();

  g_assert_cmpint (notified, ==, 3);

  adw_layout_set_name (layout1, "layout11");
  g_assert_cmpstr (adw_multi_layout_view_get_layout_name (view), ==, "layout11");

  g_assert_cmpint (notified, ==, 4);

  adw_layout_set_name (layout2, "layout22");
  g_assert_cmpint (notified, ==, 4);

  g_assert_true (adw_multi_layout_view_get_layout_by_name (view, "layout11") == layout1);
  g_assert_true (adw_multi_layout_view_get_layout_by_name (view, "layout22") == layout2);
  g_assert_null (adw_multi_layout_view_get_layout_by_name (view, "layout1"));
  g_assert_null (adw_multi_layout_view_get_layout_by_name (view, "layout2"));
  g_assert_null (adw_multi_layout_view_get_layout_by_name (view, "layout3"));

  g_assert_finalize_object (view);
  g_assert_finalize_object (layout1);
  g_assert_finalize_object (layout2);
  g_assert_finalize_object (content1);
  g_assert_finalize_object (content2);
}

static void
test_adw_multi_layout_view_children (void)
{
  AdwMultiLayoutView *view = g_object_ref_sink (ADW_MULTI_LAYOUT_VIEW (adw_multi_layout_view_new ()));
  GtkWidget *slot11 = g_object_ref_sink (adw_layout_slot_new ("slot1"));
  GtkWidget *slot12 = g_object_ref_sink (adw_layout_slot_new ("slot1"));
  GtkWidget *slot21 = g_object_ref_sink (adw_layout_slot_new ("slot2"));
  GtkWidget *slot22 = g_object_ref_sink (adw_layout_slot_new ("slot2"));
  GtkWidget *content1 = g_object_ref_sink (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
  GtkWidget *content2 = g_object_ref_sink (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
  GtkWidget *child1 = g_object_ref_sink (adw_bin_new ());
  GtkWidget *child2 = g_object_ref_sink (adw_bin_new ());
  AdwLayout *layout1, *layout2;
  GtkWidget *window = adw_window_new ();

  gtk_box_append (GTK_BOX (content1), slot11);
  gtk_box_append (GTK_BOX (content1), slot21);
  gtk_box_append (GTK_BOX (content2), slot12);
  gtk_box_append (GTK_BOX (content2), slot22);

  layout1 = adw_layout_new (content1);
  layout2 = adw_layout_new (content2);

  adw_window_set_content (ADW_WINDOW (window), GTK_WIDGET (view));
  adw_multi_layout_view_set_child (view, "slot1", child1);

  g_assert_true (adw_multi_layout_view_get_child (view, "slot1") == child1);
  g_assert_null (gtk_widget_get_parent (child1));
  g_assert_null (adw_multi_layout_view_get_child (view, "slot2"));

  adw_multi_layout_view_add_layout (view, g_object_ref (layout1));
  adw_multi_layout_view_add_layout (view, g_object_ref (layout2));
  g_assert_true (gtk_widget_get_parent (child1) == slot11);

  adw_multi_layout_view_set_child (view, "slot2", child2);
  g_assert_true (gtk_widget_get_parent (child2) == slot21);

  adw_multi_layout_view_set_layout (view, layout2);
  g_assert_true (gtk_widget_get_parent (child1) == slot12);
  g_assert_true (gtk_widget_get_parent (child2) == slot22);

  g_assert_true (adw_multi_layout_view_get_child (view, "slot1") == child1);
  g_assert_true (adw_multi_layout_view_get_child (view, "slot2") == child2);

  g_assert_finalize_object (window);
  g_assert_finalize_object (view);
  g_assert_finalize_object (layout1);
  g_assert_finalize_object (layout2);
  g_assert_finalize_object (content1);
  g_assert_finalize_object (content2);
  g_assert_finalize_object (slot11);
  g_assert_finalize_object (slot12);
  g_assert_finalize_object (slot21);
  g_assert_finalize_object (slot22);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/MultiLayoutView/add-remove", test_adw_multi_layout_view_add_remove);
  g_test_add_func ("/Adwaita/MultiLayoutView/layout", test_adw_multi_layout_view_layout);
  g_test_add_func ("/Adwaita/MultiLayoutView/layout-name", test_adw_multi_layout_view_layout_name);
  g_test_add_func ("/Adwaita/MultiLayoutView/children", test_adw_multi_layout_view_children);

  return g_test_run ();
}
