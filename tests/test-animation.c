/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

static double last_value;
static int done_count;

static void
value_cb (double   value,
          gpointer user_data)
{
  last_value = value;
}

static void
done_cb (gpointer user_data)
{
  done_count++;
}

static void
test_adw_animation_general (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwAnimation *animation =
    adw_timed_animation_new (widget, 10, 20, 100, g_object_ref (target));

  last_value = 0;
  done_count = 0;

  g_signal_connect (animation, "done", G_CALLBACK (done_cb), NULL);

  g_assert_nonnull (animation);

  g_assert_true (adw_animation_get_widget (animation) == widget);
  g_assert_true (adw_animation_get_target (animation) == target);

  g_assert_cmpint (adw_animation_get_state (animation), ==, ADW_ANIMATION_IDLE);
  g_assert_cmpfloat (adw_animation_get_value (animation), ==, 10);
  g_assert_cmpfloat (last_value, ==, 0);
  g_assert_cmpint (done_count, ==, 0);

  adw_animation_play (animation);

  /* Since the widget is not mapped, the animation will immediately finish */
  g_assert_cmpint (adw_animation_get_state (animation), ==, ADW_ANIMATION_FINISHED);
  g_assert_cmpfloat (adw_animation_get_value (animation), ==, 20);
  g_assert_cmpfloat (last_value, ==, 20);
  g_assert_cmpint (done_count, ==, 1);

  adw_animation_reset (animation);

  g_assert_cmpfloat (adw_animation_get_value (animation), ==, 10);
  g_assert_cmpfloat (last_value, ==, 10);
  g_assert_cmpint (done_count, ==, 1);

  adw_animation_skip (animation);

  g_assert_cmpint (adw_animation_get_state (animation), ==, ADW_ANIMATION_FINISHED);
  g_assert_cmpfloat (adw_animation_get_value (animation), ==, 20);
  g_assert_cmpfloat (last_value, ==, 20);
  g_assert_cmpint (done_count, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);

  g_assert_cmpfloat (last_value, ==, 20);
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
