/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

int notified;

static void
value_cb (double   value,
          gpointer user_data)
{
}

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_animation_value_from (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwTimedAnimation *animation =
    ADW_TIMED_ANIMATION (adw_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  double value;

  g_assert_nonnull (animation);

  notified = 0;

  g_signal_connect (animation, "notify::value-from", G_CALLBACK (notify_cb), NULL);

  g_object_get (animation, "value-from", &value, NULL);
  g_assert_cmpfloat (value, ==, 10);
  g_assert_cmpint (notified, ==, 0);

  adw_timed_animation_set_value_from (animation, 20);
  g_object_get (animation, "value-from", &value, NULL);
  g_assert_cmpfloat (value, ==, 20);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "value-from", 30.0, NULL);
  g_assert_cmpfloat (adw_timed_animation_get_value_from (animation), ==, 30);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adw_animation_value_to (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwTimedAnimation *animation =
    ADW_TIMED_ANIMATION (adw_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  double value;

  g_assert_nonnull (animation);

  notified = 0;

  adw_animation_skip (ADW_ANIMATION (animation));

  g_signal_connect (animation, "notify::value-to", G_CALLBACK (notify_cb), NULL);

  g_object_get (animation, "value-to", &value, NULL);
  g_assert_cmpfloat (value, ==, 20);
  g_assert_cmpint (notified, ==, 0);

  adw_timed_animation_set_value_to (animation, 10);
  g_object_get (animation, "value-to", &value, NULL);
  g_assert_cmpfloat (value, ==, 10);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "value-to", 30.0, NULL);
  g_assert_cmpfloat (adw_timed_animation_get_value_to (animation), ==, 30);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adw_animation_duration (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwTimedAnimation *animation =
    ADW_TIMED_ANIMATION (adw_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  guint duration;

  g_assert_nonnull (animation);

  notified = 0;

  g_signal_connect (animation, "notify::duration", G_CALLBACK (notify_cb), NULL);

  g_object_get (animation, "duration", &duration, NULL);
  g_assert_cmpint (duration, ==, 100);
  g_assert_cmpint (notified, ==, 0);

  adw_timed_animation_set_duration (animation, 200);
  g_object_get (animation, "duration", &duration, NULL);
  g_assert_cmpint (duration, ==, 200);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "duration", 300u, NULL);
  g_assert_cmpint (adw_timed_animation_get_duration (animation), ==, 300);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adw_animation_easing (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwTimedAnimation *animation =
    ADW_TIMED_ANIMATION (adw_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  AdwEasing easing;

  g_assert_nonnull (animation);

  notified = 0;

  g_signal_connect (animation, "notify::easing", G_CALLBACK (notify_cb), NULL);

  g_object_get (animation, "easing", &easing, NULL);
  g_assert_cmpint (easing, ==, ADW_EASE_OUT_CUBIC);
  g_assert_cmpint (notified, ==, 0);

  adw_timed_animation_set_easing (animation, ADW_EASE_IN_CUBIC);
  g_object_get (animation, "easing", &easing, NULL);
  g_assert_cmpint (easing, ==, ADW_EASE_IN_CUBIC);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "easing", ADW_EASE_IN_OUT_CUBIC, NULL);
  g_assert_cmpint (adw_timed_animation_get_easing (animation), ==, ADW_EASE_IN_OUT_CUBIC);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adw_animation_repeat_count (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwTimedAnimation *animation =
    ADW_TIMED_ANIMATION (adw_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  guint repeat_count;

  g_assert_nonnull (animation);

  notified = 0;

  g_signal_connect (animation, "notify::repeat-count", G_CALLBACK (notify_cb), NULL);

  g_object_get (animation, "repeat-count", &repeat_count, NULL);
  g_assert_cmpint (repeat_count, ==, 1);
  g_assert_cmpint (notified, ==, 0);

  adw_timed_animation_set_repeat_count (animation, 2);
  g_object_get (animation, "repeat-count", &repeat_count, NULL);
  g_assert_cmpint (repeat_count, ==, 2);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "repeat-count", 3u, NULL);
  g_assert_cmpint (adw_timed_animation_get_repeat_count (animation), ==, 3);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adw_animation_reverse (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwTimedAnimation *animation =
    ADW_TIMED_ANIMATION (adw_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  gboolean reverse;

  g_assert_nonnull (animation);

  notified = 0;

  g_signal_connect (animation, "notify::reverse", G_CALLBACK (notify_cb), NULL);

  g_object_get (animation, "reverse", &reverse, NULL);
  g_assert_false (reverse);
  g_assert_cmpint (notified, ==, 0);

  adw_timed_animation_set_reverse (animation, TRUE);
  g_object_get (animation, "reverse", &reverse, NULL);
  g_assert_true (reverse);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "reverse", FALSE, NULL);
  g_assert_false (adw_timed_animation_get_reverse (animation));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (animation);
  g_assert_finalize_object (target);
  g_assert_finalize_object (widget);
}

static void
test_adw_animation_alternate (void)
{
  GtkWidget *widget = g_object_ref_sink (gtk_button_new ());
  AdwAnimationTarget *target =
    adw_callback_animation_target_new (value_cb, NULL, NULL);
  AdwTimedAnimation *animation =
    ADW_TIMED_ANIMATION (adw_timed_animation_new (widget, 10, 20, 100,
                                                  g_object_ref (target)));
  gboolean alternate;

  g_assert_nonnull (animation);

  notified = 0;

  g_signal_connect (animation, "notify::alternate", G_CALLBACK (notify_cb), NULL);

  g_object_get (animation, "alternate", &alternate, NULL);
  g_assert_false (alternate);
  g_assert_cmpint (notified, ==, 0);

  adw_timed_animation_set_alternate (animation, TRUE);
  g_object_get (animation, "alternate", &alternate, NULL);
  g_assert_true (alternate);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (animation, "alternate", FALSE, NULL);
  g_assert_false (adw_timed_animation_get_alternate (animation));
  g_assert_cmpint (notified, ==, 2);

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

  g_test_add_func("/Adwaita/TimedAnimation/value_from", test_adw_animation_value_from);
  g_test_add_func("/Adwaita/TimedAnimation/value_to", test_adw_animation_value_to);
  g_test_add_func("/Adwaita/TimedAnimation/duration", test_adw_animation_duration);
  g_test_add_func("/Adwaita/TimedAnimation/easing", test_adw_animation_easing);
  g_test_add_func("/Adwaita/TimedAnimation/repeat_count", test_adw_animation_repeat_count);
  g_test_add_func("/Adwaita/TimedAnimation/reverse", test_adw_animation_reverse);
  g_test_add_func("/Adwaita/TimedAnimation/alternate", test_adw_animation_alternate);

  return g_test_run();
}
