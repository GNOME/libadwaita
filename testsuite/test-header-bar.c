/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>


static void
test_adw_header_bar_pack (void)
{
  AdwHeaderBar *bar = g_object_ref_sink (ADW_HEADER_BAR (adw_header_bar_new ()));
  GtkWidget *widget;

  g_assert_nonnull (bar);

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);

  adw_header_bar_pack_start (bar, widget);

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);

  adw_header_bar_pack_end (bar, widget);

  g_assert_finalize_object (bar);
}


static void
test_adw_header_bar_title_widget (void)
{
  AdwHeaderBar *bar = g_object_ref_sink (ADW_HEADER_BAR (adw_header_bar_new ()));
  GtkWidget *widget;

  g_assert_nonnull (bar);

  g_assert_null (adw_header_bar_get_title_widget (bar));

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);
  adw_header_bar_set_title_widget (bar, widget);
  g_assert (adw_header_bar_get_title_widget  (bar) == widget);

  adw_header_bar_set_title_widget (bar, NULL);
  g_assert_null (adw_header_bar_get_title_widget (bar));

  g_assert_finalize_object (bar);
}


static void
test_adw_header_bar_show_start_title_buttons (void)
{
  AdwHeaderBar *bar = g_object_ref_sink (ADW_HEADER_BAR (adw_header_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_true (adw_header_bar_get_show_start_title_buttons (bar));

  adw_header_bar_set_show_start_title_buttons (bar, FALSE);
  g_assert_false (adw_header_bar_get_show_start_title_buttons (bar));

  adw_header_bar_set_show_start_title_buttons (bar, TRUE);
  g_assert_true (adw_header_bar_get_show_start_title_buttons (bar));

  g_assert_finalize_object (bar);
}


static void
test_adw_header_bar_show_end_title_buttons (void)
{
  AdwHeaderBar *bar = g_object_ref_sink (ADW_HEADER_BAR (adw_header_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_true (adw_header_bar_get_show_end_title_buttons (bar));

  adw_header_bar_set_show_end_title_buttons (bar, FALSE);
  g_assert_false (adw_header_bar_get_show_end_title_buttons (bar));

  adw_header_bar_set_show_end_title_buttons (bar, TRUE);
  g_assert_true (adw_header_bar_get_show_end_title_buttons (bar));

  g_assert_finalize_object (bar);
}


static void
test_adw_header_bar_decoration_layout (void)
{
  AdwHeaderBar *bar = g_object_ref_sink (ADW_HEADER_BAR (adw_header_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_null (adw_header_bar_get_decoration_layout (bar));

  adw_header_bar_set_decoration_layout (bar, ":");
  g_assert_cmpstr (adw_header_bar_get_decoration_layout (bar), ==, ":");

  adw_header_bar_set_decoration_layout (bar, NULL);
  g_assert_null (adw_header_bar_get_decoration_layout (bar));

  g_assert_finalize_object (bar);
}


static void
test_adw_header_bar_centering_policy (void)
{
  AdwHeaderBar *bar = g_object_ref_sink (ADW_HEADER_BAR (adw_header_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_cmpint (adw_header_bar_get_centering_policy (bar), ==, ADW_CENTERING_POLICY_LOOSE);

  adw_header_bar_set_centering_policy (bar, ADW_CENTERING_POLICY_STRICT);
  g_assert_cmpint (adw_header_bar_get_centering_policy (bar), ==, ADW_CENTERING_POLICY_STRICT);

  adw_header_bar_set_centering_policy (bar, ADW_CENTERING_POLICY_LOOSE);
  g_assert_cmpint (adw_header_bar_get_centering_policy (bar), ==, ADW_CENTERING_POLICY_LOOSE);

  g_assert_finalize_object (bar);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/HeaderBar/pack", test_adw_header_bar_pack);
  g_test_add_func("/Adwaita/HeaderBar/title_widget", test_adw_header_bar_title_widget);
  g_test_add_func("/Adwaita/HeaderBar/show_start_title_buttons", test_adw_header_bar_show_start_title_buttons);
  g_test_add_func("/Adwaita/HeaderBar/show_end_title_buttons", test_adw_header_bar_show_end_title_buttons);
  g_test_add_func("/Adwaita/HeaderBar/decoration_layout", test_adw_header_bar_decoration_layout);
  g_test_add_func("/Adwaita/HeaderBar/centering_policy", test_adw_header_bar_centering_policy);

  return g_test_run();
}
