/*
 * Copyright (C) 2022 Jamie Murphy <hello@itsjamie.dev>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_banner_revealed (void)
{
  AdwBanner *banner = g_object_ref_sink (ADW_BANNER (adw_banner_new ("")));

  g_assert_nonnull (banner);

  g_assert_false (adw_banner_get_revealed (banner));

  adw_banner_set_revealed (banner, TRUE);
  g_assert_true (adw_banner_get_revealed (banner));

  adw_banner_set_revealed (banner, FALSE);
  g_assert_false (adw_banner_get_revealed (banner));

  g_assert_finalize_object (banner);
}

static void
test_adw_banner_title (void)
{
  AdwBanner *banner = g_object_ref_sink (ADW_BANNER (adw_banner_new ("")));

  g_assert_nonnull (banner);

  g_assert_cmpstr (adw_banner_get_title (banner), ==, "");

  adw_banner_set_title (banner, "Dummy title");
  g_assert_cmpstr (adw_banner_get_title (banner), ==, "Dummy title");

  adw_banner_set_use_markup (banner, FALSE);
  adw_banner_set_title (banner, "Invalid <b>markup");
  g_assert_cmpstr (adw_banner_get_title (banner), ==, "Invalid <b>markup");

  g_assert_finalize_object (banner);
}

static void
test_adw_banner_button_label (void)
{
  AdwBanner *banner = g_object_ref_sink (ADW_BANNER (adw_banner_new ("")));
  char *button_label;

  g_assert_nonnull (banner);

  g_object_get (banner, "button-label", &button_label, NULL);
  g_assert_null (button_label);

  adw_banner_set_button_label (banner, "Dummy label");
  g_assert_cmpstr (adw_banner_get_button_label (banner), ==, "Dummy label");

  adw_banner_set_button_label (banner, NULL);
  g_assert_cmpstr (adw_banner_get_button_label (banner), ==, "");

  g_object_set (banner, "button-label", "Button 2", NULL);
  g_assert_cmpstr (adw_banner_get_button_label (banner), ==, "Button 2");

  g_assert_finalize_object (banner);
}

static void
test_adw_banner_button_style (void)
{
  AdwBanner *banner = g_object_ref_sink (ADW_BANNER (adw_banner_new ("")));
  AdwBannerButtonStyle button_style;
  int notified = 0;

  g_assert_nonnull (banner);

  g_signal_connect_swapped (banner, "notify::button-style", G_CALLBACK (increment), &notified);

  g_object_get (banner, "button-style", &button_style, NULL);
  g_assert_cmpint (button_style, ==, ADW_BANNER_BUTTON_DEFAULT);

  adw_banner_set_button_style (banner, ADW_BANNER_BUTTON_DEFAULT);
  g_assert_cmpint (notified, ==, 0);

  adw_banner_set_button_style (banner, ADW_BANNER_BUTTON_SUGGESTED);
  g_assert_cmpint (adw_banner_get_button_style (banner), ==, ADW_BANNER_BUTTON_SUGGESTED);
  g_assert_cmpint (notified, ==, 1);

  g_assert_finalize_object (banner);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/Banner/revealed", test_adw_banner_revealed);
  g_test_add_func ("/Adwaita/Banner/title", test_adw_banner_title);
  g_test_add_func ("/Adwaita/Banner/button_label", test_adw_banner_button_label);
  g_test_add_func ("/Adwaita/Banner/button_style", test_adw_banner_button_style);

  return g_test_run ();
}
