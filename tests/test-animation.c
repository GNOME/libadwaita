/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

static double last_value;

static void
value_cb (double   value,
          gpointer user_data)
{
  last_value = value;
}

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_animation_general (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwAnimationTarget *target2 =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwAnimation *animation =
    adw_timed_animation_new (widget, 10, 20, 100, g_object_ref (target));
  int done_count = 0;

  last_value = 0;

  g_signal_connect_swapped (animation, "done", G_CALLBACK (increment), &done_count);

  g_assert_nonnull (animation);

  g_assert_true (adw_animation_get_widget (animation) == widget);
  g_assert_true (adw_animation_get_target (animation) == target);

  g_assert_cmpint (adw_animation_get_state (animation), ==, ADW_ANIMATION_IDLE);
  g_assert_true (G_APPROX_VALUE (adw_animation_get_value (animation), 10, DBL_EPSILON));
  g_assert_true (G_APPROX_VALUE (last_value, 0, DBL_EPSILON));
  g_assert_cmpint (done_count, ==, 0);

  adw_animation_play (animation);

  /* Since the widget is not mapped, the animation will immediately finish */
  g_assert_cmpint (adw_animation_get_state (animation), ==, ADW_ANIMATION_FINISHED);
  g_assert_true (G_APPROX_VALUE (adw_animation_get_value (animation), 20, DBL_EPSILON));
  g_assert_true (G_APPROX_VALUE (last_value, 20, DBL_EPSILON));
  g_assert_cmpint (done_count, ==, 1);

  adw_animation_reset (animation);

  g_assert_true (G_APPROX_VALUE (adw_animation_get_value (animation), 10, DBL_EPSILON));
  g_assert_true (G_APPROX_VALUE (last_value, 10, DBL_EPSILON));
  g_assert_cmpint (done_count, ==, 1);

  adw_animation_skip (animation);

  g_assert_cmpint (adw_animation_get_state (animation), ==, ADW_ANIMATION_FINISHED);
  g_assert_true (G_APPROX_VALUE (adw_animation_get_value (animation), 20, DBL_EPSILON));
  g_assert_true (G_APPROX_VALUE (last_value, 20, DBL_EPSILON));
  g_assert_cmpint (done_count, ==, 2);

  adw_animation_set_target (animation, target2);
  g_assert_true (adw_animation_get_target (animation) == target2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (target2);
  g_assert_finalize_object (widget);

  g_assert_true (G_APPROX_VALUE (last_value, 20, DBL_EPSILON));
  g_assert_cmpint (done_count, ==, 2);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/Animation/general", test_adw_animation_general);

  return g_test_run();
}
