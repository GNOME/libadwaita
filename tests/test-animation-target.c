/*
 * Copyright (C) 2022 George Barrett <bob@bob131.so>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_property_animation_target_construct (void)
{
  GObject *widget = g_object_ref_sink (G_OBJECT (gtk_button_new ()));
  AdwPropertyAnimationTarget *named_target, *pspec_target;
  GParamSpec *target_pspec;

  named_target =
    ADW_PROPERTY_ANIMATION_TARGET (adw_property_animation_target_new (widget, "opacity"));

  target_pspec = adw_property_animation_target_get_pspec (named_target);
  g_assert_nonnull (target_pspec);
  g_assert_cmpstr (target_pspec->name, ==, "opacity");

  pspec_target =
    ADW_PROPERTY_ANIMATION_TARGET (adw_property_animation_target_new_for_pspec (widget, target_pspec));

  g_assert_true (adw_property_animation_target_get_pspec (pspec_target) == target_pspec);

  target_pspec = adw_property_animation_target_get_pspec (named_target);
  g_assert_nonnull (target_pspec);
  g_assert_cmpstr (target_pspec->name, ==, "opacity");

  g_assert_finalize_object (named_target);
  g_assert_finalize_object (pspec_target);
  g_assert_finalize_object (widget);
}

static void
test_adw_property_animation_target_basic (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_property_animation_target_new (G_OBJECT (widget), "opacity");
  AdwAnimation *animation =
    adw_timed_animation_new (widget, 1, 0, 100, g_object_ref (target));

  g_assert_true (G_APPROX_VALUE (gtk_widget_get_opacity (widget), 1, DBL_EPSILON));

  adw_animation_play (animation);

  /* Since the widget is not mapped, the animation will immediately finish */
  g_assert_true (G_APPROX_VALUE (gtk_widget_get_opacity (widget), 0, DBL_EPSILON));

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/PropertyAnimationTarget/construct",
                  test_adw_property_animation_target_construct);
  g_test_add_func("/Adwaita/PropertyAnimationTarget/basic",
                  test_adw_property_animation_target_basic);

  return g_test_run();
}
