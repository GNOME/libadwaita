/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_header_bar_pack (void)
{
  g_autoptr (HdyHeaderBar) bar = NULL;
  GtkWidget *widget;

  bar = g_object_ref_sink (HDY_HEADER_BAR (hdy_header_bar_new ()));
  g_assert_nonnull (bar);

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);

  hdy_header_bar_pack_start (bar, widget);

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);

  hdy_header_bar_pack_end (bar, widget);
}


static void
test_hdy_header_bar_title_widget (void)
{
  g_autoptr (HdyHeaderBar) bar = NULL;
  GtkWidget *widget;

  bar = g_object_ref_sink (HDY_HEADER_BAR (hdy_header_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_null (hdy_header_bar_get_title_widget (bar));

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);
  hdy_header_bar_set_title_widget (bar, widget);
  g_assert (hdy_header_bar_get_title_widget  (bar) == widget);

  hdy_header_bar_set_title_widget (bar, NULL);
  g_assert_null (hdy_header_bar_get_title_widget (bar));
}


static void
test_hdy_header_bar_show_start_title_buttons (void)
{
  g_autoptr (HdyHeaderBar) bar = NULL;

  bar = g_object_ref_sink (HDY_HEADER_BAR (hdy_header_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_true (hdy_header_bar_get_show_start_title_buttons (bar));

  hdy_header_bar_set_show_start_title_buttons (bar, FALSE);
  g_assert_false (hdy_header_bar_get_show_start_title_buttons (bar));

  hdy_header_bar_set_show_start_title_buttons (bar, TRUE);
  g_assert_true (hdy_header_bar_get_show_start_title_buttons (bar));
}


static void
test_hdy_header_bar_show_end_title_buttons (void)
{
  g_autoptr (HdyHeaderBar) bar = NULL;

  bar = g_object_ref_sink (HDY_HEADER_BAR (hdy_header_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_true (hdy_header_bar_get_show_end_title_buttons (bar));

  hdy_header_bar_set_show_end_title_buttons (bar, FALSE);
  g_assert_false (hdy_header_bar_get_show_end_title_buttons (bar));

  hdy_header_bar_set_show_end_title_buttons (bar, TRUE);
  g_assert_true (hdy_header_bar_get_show_end_title_buttons (bar));
}


static void
test_hdy_header_bar_decoration_layout (void)
{
  g_autoptr (HdyHeaderBar) bar = NULL;

  bar = g_object_ref_sink (HDY_HEADER_BAR (hdy_header_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_null (hdy_header_bar_get_decoration_layout (bar));

  hdy_header_bar_set_decoration_layout (bar, ":");
  g_assert_cmpstr (hdy_header_bar_get_decoration_layout (bar), ==, ":");

  hdy_header_bar_set_decoration_layout (bar, NULL);
  g_assert_null (hdy_header_bar_get_decoration_layout (bar));
}


static void
test_hdy_header_bar_centering_policy (void)
{
  g_autoptr (HdyHeaderBar) bar = NULL;

  bar = g_object_ref_sink (HDY_HEADER_BAR (hdy_header_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_cmpint (hdy_header_bar_get_centering_policy (bar), ==, HDY_CENTERING_POLICY_LOOSE);

  hdy_header_bar_set_centering_policy (bar, HDY_CENTERING_POLICY_STRICT);
  g_assert_cmpint (hdy_header_bar_get_centering_policy (bar), ==, HDY_CENTERING_POLICY_STRICT);

  hdy_header_bar_set_centering_policy (bar, HDY_CENTERING_POLICY_LOOSE);
  g_assert_cmpint (hdy_header_bar_get_centering_policy (bar), ==, HDY_CENTERING_POLICY_LOOSE);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/HeaderBar/pack", test_hdy_header_bar_pack);
  g_test_add_func("/Handy/HeaderBar/title_widget", test_hdy_header_bar_title_widget);
  g_test_add_func("/Handy/HeaderBar/show_start_title_buttons", test_hdy_header_bar_show_start_title_buttons);
  g_test_add_func("/Handy/HeaderBar/show_end_title_buttons", test_hdy_header_bar_show_end_title_buttons);
  g_test_add_func("/Handy/HeaderBar/decoration_layout", test_hdy_header_bar_decoration_layout);
  g_test_add_func("/Handy/HeaderBar/centering_policy", test_hdy_header_bar_centering_policy);

  return g_test_run();
}
