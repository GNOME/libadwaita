/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <handy.h>

gint notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_hdy_flap_flap (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  GtkWidget *widget = NULL;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  notified = 0;
  g_signal_connect (flap, "notify::flap", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "flap", &widget, NULL);
  g_assert_null (widget);

  hdy_flap_set_flap (flap, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  hdy_flap_set_flap (flap, widget);
  g_assert_true (hdy_flap_get_flap (flap) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "flap", NULL, NULL);
  g_assert_null (hdy_flap_get_flap (flap));
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_separator (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  GtkWidget *widget = NULL;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  notified = 0;
  g_signal_connect (flap, "notify::separator", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "separator", &widget, NULL);
  g_assert_null (widget);

  hdy_flap_set_separator (flap, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  hdy_flap_set_separator (flap, widget);
  g_assert_true (hdy_flap_get_separator (flap) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "separator", NULL, NULL);
  g_assert_null (hdy_flap_get_separator (flap));
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_flap_position (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  GtkPackType position;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  notified = 0;
  g_signal_connect (flap, "notify::flap-position", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "flap-position", &position, NULL);
  g_assert_cmpint (position, ==, GTK_PACK_START);

  hdy_flap_set_flap_position (flap, GTK_PACK_START);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_flap_position (flap, GTK_PACK_END);
  g_assert_cmpint (hdy_flap_get_flap_position (flap), ==, GTK_PACK_END);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "flap-position", GTK_PACK_START, NULL);
  g_assert_cmpint (hdy_flap_get_flap_position (flap), ==, GTK_PACK_START);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_reveal_flap (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  gboolean reveal;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  hdy_flap_set_flap (flap, gtk_button_new ());

  notified = 0;
  g_signal_connect (flap, "notify::reveal-flap", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "reveal-flap", &reveal, NULL);
  g_assert_true (reveal);

  hdy_flap_set_reveal_flap (flap, TRUE);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_reveal_flap (flap, FALSE);
  g_assert_false (hdy_flap_get_reveal_flap (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "reveal-flap", TRUE, NULL);
  g_assert_true (hdy_flap_get_reveal_flap (flap));
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_reveal_duration (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  guint duration;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  notified = 0;
  g_signal_connect (flap, "notify::reveal-duration", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "reveal-duration", &duration, NULL);
  g_assert_cmpint (duration, ==, 250);

  hdy_flap_set_reveal_duration (flap, 250);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_reveal_duration (flap, 500);
  g_assert_cmpint (hdy_flap_get_reveal_duration (flap), ==, 500);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "reveal-duration", 100, NULL);
  g_assert_cmpint (hdy_flap_get_reveal_duration (flap), ==, 100);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_reveal_progress (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  gdouble progress;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  notified = 0;
  g_signal_connect (flap, "notify::reveal-progress", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "reveal-progress", &progress, NULL);
  g_assert_cmpint (progress, ==, 1.0);

  hdy_flap_set_reveal_flap (flap, FALSE);
  g_assert_cmpint (hdy_flap_get_reveal_progress (flap), ==, 0.0);
  g_assert_cmpint (notified, ==, 1);

  hdy_flap_set_reveal_flap (flap, TRUE);
  g_assert_cmpint (hdy_flap_get_reveal_progress (flap), ==, 1.0);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_fold_policy (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  HdyFlapFoldPolicy policy;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  notified = 0;
  g_signal_connect (flap, "notify::fold-policy", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "fold-policy", &policy, NULL);
  g_assert_cmpint (policy, ==, HDY_FLAP_FOLD_POLICY_AUTO);

  hdy_flap_set_fold_policy (flap, HDY_FLAP_FOLD_POLICY_AUTO);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_fold_policy (flap, HDY_FLAP_FOLD_POLICY_NEVER);
  g_assert_cmpint (hdy_flap_get_fold_policy (flap), ==, HDY_FLAP_FOLD_POLICY_NEVER);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "fold-policy", HDY_FLAP_FOLD_POLICY_ALWAYS, NULL);
  g_assert_cmpint (hdy_flap_get_fold_policy (flap), ==, HDY_FLAP_FOLD_POLICY_ALWAYS);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_fold_duration (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  guint duration;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  notified = 0;
  g_signal_connect (flap, "notify::fold-duration", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "fold-duration", &duration, NULL);
  g_assert_cmpint (duration, ==, 250);

  hdy_flap_set_fold_duration (flap, 250);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_fold_duration (flap, 500);
  g_assert_cmpint (hdy_flap_get_fold_duration (flap), ==, 500);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "fold-duration", 100, NULL);
  g_assert_cmpint (hdy_flap_get_fold_duration (flap), ==, 100);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_folded (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  gboolean folded;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  hdy_flap_set_flap (flap, gtk_button_new ());
  hdy_flap_set_fold_policy (flap, HDY_FLAP_FOLD_POLICY_NEVER);

  notified = 0;
  g_signal_connect (flap, "notify::folded", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "folded", &folded, NULL);
  g_assert_false (folded);

  hdy_flap_set_fold_policy (flap, HDY_FLAP_FOLD_POLICY_ALWAYS);
  g_assert_true (hdy_flap_get_folded (flap));
  g_assert_cmpint (notified, ==, 1);
}

static void
test_hdy_flap_locked (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  gboolean locked;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  hdy_flap_set_flap (flap, gtk_button_new ());

  notified = 0;
  g_signal_connect (flap, "notify::locked", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "locked", &locked, NULL);
  g_assert_false (locked);

  hdy_flap_set_locked (flap, FALSE);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_locked (flap, TRUE);
  g_assert_true (hdy_flap_get_locked (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "locked", FALSE, NULL);
  g_assert_false (hdy_flap_get_locked (flap));
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_transition_type (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  HdyFlapTransitionType policy;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  notified = 0;
  g_signal_connect (flap, "notify::transition-type", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "transition-type", &policy, NULL);
  g_assert_cmpint (policy, ==, HDY_FLAP_TRANSITION_TYPE_OVER);

  hdy_flap_set_transition_type (flap, HDY_FLAP_TRANSITION_TYPE_OVER);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_transition_type (flap, HDY_FLAP_TRANSITION_TYPE_SLIDE);
  g_assert_cmpint (hdy_flap_get_transition_type (flap), ==, HDY_FLAP_TRANSITION_TYPE_SLIDE);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "transition-type", HDY_FLAP_TRANSITION_TYPE_UNDER, NULL);
  g_assert_cmpint (hdy_flap_get_transition_type (flap), ==, HDY_FLAP_TRANSITION_TYPE_UNDER);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_modal (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  gboolean modal;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  hdy_flap_set_flap (flap, gtk_button_new ());

  notified = 0;
  g_signal_connect (flap, "notify::modal", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "modal", &modal, NULL);
  g_assert_true (modal);

  hdy_flap_set_modal (flap, TRUE);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_modal (flap, FALSE);
  g_assert_false (hdy_flap_get_modal (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "modal", TRUE, NULL);
  g_assert_true (hdy_flap_get_modal (flap));
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_swipe_to_open (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  gboolean swipe_to_open;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  hdy_flap_set_flap (flap, gtk_button_new ());

  notified = 0;
  g_signal_connect (flap, "notify::swipe-to-open", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "swipe-to-open", &swipe_to_open, NULL);
  g_assert_true (swipe_to_open);

  hdy_flap_set_swipe_to_open (flap, TRUE);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_swipe_to_open (flap, FALSE);
  g_assert_false (hdy_flap_get_swipe_to_open (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "swipe-to-open", TRUE, NULL);
  g_assert_true (hdy_flap_get_swipe_to_open (flap));
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_flap_swipe_to_close (void)
{
  g_autoptr (HdyFlap) flap = NULL;
  gboolean swipe_to_close;

  flap = g_object_ref_sink (HDY_FLAP (hdy_flap_new ()));
  g_assert_nonnull (flap);

  hdy_flap_set_flap (flap, gtk_button_new ());

  notified = 0;
  g_signal_connect (flap, "notify::swipe-to-close", G_CALLBACK (notify_cb), NULL);

  g_object_get (flap, "swipe-to-close", &swipe_to_close, NULL);
  g_assert_true (swipe_to_close);

  hdy_flap_set_swipe_to_close (flap, TRUE);
  g_assert_cmpint (notified, ==, 0);

  hdy_flap_set_swipe_to_close (flap, FALSE);
  g_assert_false (hdy_flap_get_swipe_to_close (flap));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (flap, "swipe-to-close", TRUE, NULL);
  g_assert_true (hdy_flap_get_swipe_to_close (flap));
  g_assert_cmpint (notified, ==, 2);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func ("/Handy/Flap/flap", test_hdy_flap_flap);
  g_test_add_func ("/Handy/Flap/separator", test_hdy_flap_separator);
  g_test_add_func ("/Handy/Flap/flap_position", test_hdy_flap_flap_position);
  g_test_add_func ("/Handy/Flap/reveal_flap", test_hdy_flap_reveal_flap);
  g_test_add_func ("/Handy/Flap/reveal_duration", test_hdy_flap_reveal_duration);
  g_test_add_func ("/Handy/Flap/reveal_progress", test_hdy_flap_reveal_progress);
  g_test_add_func ("/Handy/Flap/fold_policy", test_hdy_flap_fold_policy);
  g_test_add_func ("/Handy/Flap/fold_duration", test_hdy_flap_fold_duration);
  g_test_add_func ("/Handy/Flap/folded", test_hdy_flap_folded);
  g_test_add_func ("/Handy/Flap/locked", test_hdy_flap_locked);
  g_test_add_func ("/Handy/Flap/transition_type", test_hdy_flap_transition_type);
  g_test_add_func ("/Handy/Flap/modal", test_hdy_flap_modal);
  g_test_add_func ("/Handy/Flap/swipe_to_open", test_hdy_flap_swipe_to_open);
  g_test_add_func ("/Handy/Flap/swipe_to_close", test_hdy_flap_swipe_to_close);

  return g_test_run ();
}
