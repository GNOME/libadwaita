/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

int notified, pushed, popped, replaced, showing, hiding, shown, hidden;

static void
notify_cb (void)
{
  notified++;
}

static void
pushed_cb (void)
{
  pushed++;
}

static void
popped_cb (void)
{
  popped++;
}

static void
replaced_cb (void)
{
  replaced++;
}

static void
showing_cb (void)
{
  showing++;
}

static void
hiding_cb (void)
{
  hiding++;
}

static void
shown_cb (void)
{
  shown++;
}

static void
hidden_cb (void)
{
  hidden++;
}

static void
check_navigation_stack (AdwNavigationView *view,
                        int                n_pages,
                        ...)
{
  GListModel *stack = adw_navigation_view_get_navigation_stack (view);
  va_list args;
  guint i;

  va_start (args, n_pages);

  g_assert_cmpint (g_list_model_get_n_items (stack), ==, n_pages);

  for (i = 0; i < n_pages; i++) {
    AdwNavigationPage *page = g_list_model_get_item (stack, i);
    const char *tag = va_arg (args, const char *);

    g_assert_cmpstr (adw_navigation_page_get_tag (page), ==, tag);

    g_object_unref (page);
  }

  va_end (args);

  g_assert_finalize_object (stack);
}

static void
test_adw_navigation_view_add_remove (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  AdwNavigationPage *page_1 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdwNavigationPage *page_2 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdwNavigationPage *page_3 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 2 again", "page-2"));

  g_assert_nonnull (view);
  g_assert_nonnull (page_1);
  g_assert_nonnull (page_2);
  g_assert_nonnull (page_3);

  notified = pushed = popped = 0;
  g_signal_connect (view, "pushed", G_CALLBACK (pushed_cb), NULL);
  g_signal_connect (view, "popped", G_CALLBACK (popped_cb), NULL);
  g_signal_connect (view, "notify::visible-page", G_CALLBACK (notify_cb), NULL);

  g_assert_null (adw_navigation_view_get_visible_page (view));
  check_navigation_stack (view, 0);
  g_assert_cmpint (pushed, ==, 0);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_view_add (view, page_1);
  adw_navigation_view_add (view, page_2);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*Duplicate page tag*");
  adw_navigation_view_add (view, page_3);
  g_test_assert_expected_messages ();

  g_assert_true (adw_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);

  adw_navigation_view_remove (view, page_1);
  adw_navigation_view_remove (view, page_2);

  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_null (adw_navigation_view_find_page (view, "page-2"));

  g_assert_true (adw_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");

  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 1);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
}

static void
test_adw_navigation_view_push_pop (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  AdwNavigationPage *page_1 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdwNavigationPage *page_2 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdwNavigationPage *page_3 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  AdwNavigationPage *page_4 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 4", "page-4"));
  AdwNavigationPage *page_5 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 4 again", "page-4"));

  g_assert_nonnull (view);
  g_assert_nonnull (page_1);
  g_assert_nonnull (page_2);
  g_assert_nonnull (page_3);
  g_assert_nonnull (page_4);

  notified = pushed = popped = 0;
  g_signal_connect (view, "pushed", G_CALLBACK (pushed_cb), NULL);
  g_signal_connect (view, "popped", G_CALLBACK (popped_cb), NULL);
  g_signal_connect (view, "notify::visible-page", G_CALLBACK (notify_cb), NULL);

  g_assert_cmpint (pushed, ==, 0);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 0);

  /* Will be autoremoved after pop */
  adw_navigation_view_push (view, page_1);
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  /* Explicitly added page - will persist after pop */
  adw_navigation_view_add (view, page_2);
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  adw_navigation_view_push (view, page_2);
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_2);
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_cmpint (pushed, ==, 2);
  g_assert_cmpint (notified, ==, 2);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in navigation stack*");
  adw_navigation_view_push (view, page_2);
  g_test_assert_expected_messages ();
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_cmpint (pushed, ==, 2);
  g_assert_cmpint (notified, ==, 2);

  adw_navigation_view_add (view, page_3);
  adw_navigation_view_push (view, page_3);
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_cmpint (pushed, ==, 3);
  g_assert_cmpint (notified, ==, 3);

  /* Removing while in navigation stack - no effect until it's popped */
  adw_navigation_view_remove (view, page_3);
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_cmpint (pushed, ==, 3);
  g_assert_cmpint (notified, ==, 3);

  adw_navigation_view_push (view, page_4);
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (pushed, ==, 4);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*Duplicate page tag*");
  adw_navigation_view_push (view, page_5);
  g_test_assert_expected_messages ();
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (pushed, ==, 4);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_assert_true (adw_navigation_view_pop (view));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_null (adw_navigation_view_find_page (view, "page-4"));
  g_assert_cmpint (popped, ==, 1);
  g_assert_cmpint (notified, ==, 5);

  g_assert_true (adw_navigation_view_pop (view));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_2);
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_null (adw_navigation_view_find_page (view, "page-3"));
  g_assert_cmpint (popped, ==, 2);
  g_assert_cmpint (notified, ==, 6);

  g_assert_true (adw_navigation_view_pop (view));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_cmpint (popped, ==, 3);
  g_assert_cmpint (notified, ==, 7);

  /* Last page - not allowed to pop */
  g_assert_false (adw_navigation_view_pop (view));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_cmpint (popped, ==, 3);
  g_assert_cmpint (notified, ==, 7);

  g_assert_cmpint (pushed, ==, 4);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
  g_assert_finalize_object (page_4);
  g_assert_finalize_object (page_5);
}

static void
test_adw_navigation_view_push_pop_by_tag (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  AdwNavigationPage *page_1 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdwNavigationPage *page_2 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdwNavigationPage *page_3 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  AdwNavigationPage *page_4 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 4", "page-4"));

  g_assert_nonnull (view);
  g_assert_nonnull (page_1);
  g_assert_nonnull (page_2);
  g_assert_nonnull (page_3);
  g_assert_nonnull (page_4);

  notified = pushed = popped = 0;
  g_signal_connect (view, "pushed", G_CALLBACK (pushed_cb), NULL);
  g_signal_connect (view, "popped", G_CALLBACK (popped_cb), NULL);
  g_signal_connect (view, "notify::visible-page", G_CALLBACK (notify_cb), NULL);

  adw_navigation_view_add (view, page_1);
  adw_navigation_view_add (view, page_2);
  adw_navigation_view_add (view, page_3);
  adw_navigation_view_add (view, page_4);

  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 1);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*with the tag*");
  adw_navigation_view_push_by_tag (view, "page-0");
  g_test_assert_expected_messages ();
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in navigation stack*");
  adw_navigation_view_push_by_tag (view, "page-1");
  g_test_assert_expected_messages ();
  g_assert_cmpint (pushed, ==, 1);
  g_assert_cmpint (notified, ==, 1);

  adw_navigation_view_push_by_tag (view, "page-2");
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_2);
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_cmpint (pushed, ==, 2);
  g_assert_cmpint (notified, ==, 2);

  adw_navigation_view_push_by_tag (view, "page-3");
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_cmpint (pushed, ==, 3);
  g_assert_cmpint (notified, ==, 3);

  adw_navigation_view_push_by_tag (view, "page-4");
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (pushed, ==, 4);
  g_assert_cmpint (notified, ==, 4);

  g_assert_cmpint (popped, ==, 0);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*with the tag*");
  g_assert_false (adw_navigation_view_pop_to_tag (view, "page-5"));
  g_test_assert_expected_messages ();
  g_assert_cmpint (notified, ==, 4);

  g_assert_false (adw_navigation_view_pop_to_tag (view, "page-4"));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_assert_true (adw_navigation_view_pop_to_tag (view, "page-2"));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_2);
  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_cmpint (popped, ==, 2);
  g_assert_cmpint (notified, ==, 5);

  g_assert_true (adw_navigation_view_pop_to_tag (view, "page-1"));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (popped, ==, 3);
  g_assert_cmpint (notified, ==, 6);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
  g_assert_finalize_object (page_4);
}

static void
test_adw_navigation_view_pop_to_page (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  AdwNavigationPage *page_1 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdwNavigationPage *page_2 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdwNavigationPage *page_3 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  AdwNavigationPage *page_4 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 4", "page-4"));
  AdwNavigationPage *page_5 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 5", "page-5"));

  g_assert_nonnull (view);
  g_assert_nonnull (page_1);
  g_assert_nonnull (page_2);
  g_assert_nonnull (page_3);
  g_assert_nonnull (page_4);

  notified = popped = 0;
  g_signal_connect (view, "popped", G_CALLBACK (popped_cb), NULL);
  g_signal_connect (view, "notify::visible-page", G_CALLBACK (notify_cb), NULL);

  adw_navigation_view_add (view, page_1);
  adw_navigation_view_add (view, page_3);
  adw_navigation_view_push (view, page_2);
  adw_navigation_view_push (view, page_3);
  adw_navigation_view_push (view, page_4);

  g_assert_true (adw_navigation_view_get_visible_page (view) == page_4);
  check_navigation_stack (view, 4, "page-1", "page-2", "page-3", "page-4");
  g_assert_cmpint (notified, ==, 4);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*not in the navigation stack*");
  g_assert_false (adw_navigation_view_pop_to_page (view, page_5));
  g_test_assert_expected_messages ();
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_assert_false (adw_navigation_view_pop_to_page (view, page_4));
  g_assert_cmpint (popped, ==, 0);
  g_assert_cmpint (notified, ==, 4);

  g_assert_true (adw_navigation_view_pop_to_page (view, page_3));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_3);
  check_navigation_stack (view, 3, "page-1", "page-2", "page-3");
  g_assert_cmpint (popped, ==, 1);
  g_assert_cmpint (notified, ==, 5);
  g_assert_null (adw_navigation_view_find_page (view, "page-4"));

  g_assert_true (adw_navigation_view_pop_to_page (view, page_1));
  g_assert_true (adw_navigation_view_get_visible_page (view) == page_1);
  check_navigation_stack (view, 1, "page-1");
  g_assert_cmpint (popped, ==, 3);
  g_assert_cmpint (notified, ==, 6);
  g_assert_null (adw_navigation_view_find_page (view, "page-2"));
  g_assert_true (adw_navigation_view_find_page (view, "page-3") == page_3);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
  g_assert_finalize_object (page_4);
  g_assert_finalize_object (page_5);
}

static void
test_adw_navigation_view_replace (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  AdwNavigationPage *page_1 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdwNavigationPage *page_2 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdwNavigationPage *page_3 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));

  notified = pushed = popped = replaced = 0;
  g_signal_connect (view, "pushed", G_CALLBACK (pushed_cb), NULL);
  g_signal_connect (view, "popped", G_CALLBACK (popped_cb), NULL);
  g_signal_connect (view, "replaced", G_CALLBACK (replaced_cb), NULL);
  g_signal_connect (view, "notify::visible-page", G_CALLBACK (notify_cb), NULL);

  check_navigation_stack (view, 0);

  adw_navigation_view_replace (view, NULL, 0);

  check_navigation_stack (view, 0);
  g_assert_cmpint (replaced, ==, 1);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_view_replace (view, &page_1, 1);

  check_navigation_stack (view, 1, "page-1");
  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_null (adw_navigation_view_find_page (view, "page-2"));
  g_assert_cmpint (replaced, ==, 2);
  g_assert_cmpint (notified, ==, 1);

  adw_navigation_view_replace (view, (AdwNavigationPage *[2]) {
    page_2,
    page_1,
  }, 2);

  check_navigation_stack (view, 2, "page-2", "page-1");
  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_cmpint (replaced, ==, 3);
  g_assert_cmpint (notified, ==, 1);

  adw_navigation_view_replace (view, (AdwNavigationPage *[2]) {
    page_1,
    page_2,
  }, 2);

  check_navigation_stack (view, 2, "page-1", "page-2");
  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_cmpint (replaced, ==, 4);
  g_assert_cmpint (notified, ==, 2);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in navigation stack*");
  adw_navigation_view_replace (view, (AdwNavigationPage *[2]) {
    page_1,
    page_1,
  }, 2);
  g_test_assert_expected_messages ();
  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_null (adw_navigation_view_find_page (view, "page-2"));
  g_assert_cmpint (replaced, ==, 5);
  g_assert_cmpint (notified, ==, 3);

  adw_navigation_view_replace (view, (AdwNavigationPage *[2]) {
    page_1,
    page_2,
  }, 2);
  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_cmpint (replaced, ==, 6);
  g_assert_cmpint (notified, ==, 4);

  adw_navigation_view_add (view, page_2);
  adw_navigation_view_add (view, page_3);

  adw_navigation_view_replace_with_tags (view, (const char *[2]) {
    "page-2",
    "page-3",
  }, 2);
  g_assert_null (adw_navigation_view_find_page (view, "page-1"));
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_true (adw_navigation_view_find_page (view, "page-3") == page_3);
  g_assert_cmpint (replaced, ==, 7);
  g_assert_cmpint (notified, ==, 5);

  adw_navigation_view_remove (view, page_3);

  adw_navigation_view_replace_with_tags (view, NULL, 0);
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_null (adw_navigation_view_find_page (view, "page-3"));
  g_assert_cmpint (replaced, ==, 8);
  g_assert_cmpint (notified, ==, 6);

  g_assert_cmpint (pushed, ==, 0);
  g_assert_cmpint (popped, ==, 0);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
}

static void
test_adw_navigation_view_previous_page (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  AdwNavigationPage *page_1 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdwNavigationPage *page_2 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  AdwNavigationPage *page_3 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 3", "page-3"));
  AdwNavigationPage *page_4 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 4", "page-4"));

  adw_navigation_view_add (view, page_1);
  adw_navigation_view_add (view, page_2);
  adw_navigation_view_add (view, page_4);

  adw_navigation_view_push (view, page_2);
  adw_navigation_view_push (view, page_3);

  g_assert_null (adw_navigation_view_get_previous_page (view, page_1));
  g_assert_true (adw_navigation_view_get_previous_page (view, page_2) == page_1);
  g_assert_true (adw_navigation_view_get_previous_page (view, page_3) == page_2);
  g_assert_null (adw_navigation_view_get_previous_page (view, page_4));

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
  g_assert_finalize_object (page_4);
}

static void
test_adw_navigation_view_find_page (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  AdwNavigationPage *page_1 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 1", "page-1"));
  AdwNavigationPage *page_2 = g_object_ref_sink (adw_navigation_page_new_with_tag (gtk_button_new (), "Page 2", "page-2"));
  g_assert_null (adw_navigation_view_find_page (view, "page-1"));
  g_assert_null (adw_navigation_view_find_page (view, "page-2"));

  adw_navigation_view_add (view, page_1);
  adw_navigation_view_add (view, page_2);
  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*Duplicate page tag*");
  adw_navigation_page_set_tag (page_1, "page-2");
  g_test_assert_expected_messages ();
  g_assert_true (adw_navigation_view_find_page (view, "page-1") == page_1);
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);

  adw_navigation_page_set_tag (page_1, "page-3");
  g_assert_null (adw_navigation_view_find_page (view, "page-1"));
  g_assert_true (adw_navigation_view_find_page (view, "page-2") == page_2);
  g_assert_true (adw_navigation_view_find_page (view, "page-3") == page_1);

  adw_navigation_view_replace (view, NULL, 0);
  adw_navigation_view_remove (view, page_1);
  adw_navigation_view_remove (view, page_2);
  g_assert_null (adw_navigation_view_find_page (view, "page-1"));
  g_assert_null (adw_navigation_view_find_page (view, "page-2"));
  g_assert_null (adw_navigation_view_find_page (view, "page-3"));

  g_assert_finalize_object (view);
  g_assert_finalize_object (page_1);
  g_assert_finalize_object (page_2);
}

static void
test_adw_navigation_view_animate_transitions (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  gboolean animate_transitions;

  g_assert_nonnull (view);

  notified = 0;
  g_signal_connect (view, "notify::animate-transitions", G_CALLBACK (notify_cb), NULL);

  g_object_get (view, "animate-transitions", &animate_transitions, NULL);
  g_assert_true (animate_transitions);

  adw_navigation_view_set_animate_transitions (view, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_view_set_animate_transitions (view, FALSE);
  g_assert_false (adw_navigation_view_get_animate_transitions (view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (view, "animate-transitions", TRUE, NULL);
  g_assert_true (adw_navigation_view_get_animate_transitions (view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (view);
}

static void
test_adw_navigation_page_child (void)
{
  GtkWidget *button = g_object_ref_sink (gtk_button_new ());
  AdwNavigationPage *page = g_object_ref_sink (adw_navigation_page_new (button, "Title"));
  GtkWidget *widget;

  g_assert_nonnull (page);

  notified = 0;
  g_signal_connect (page, "notify::child", G_CALLBACK (notify_cb), NULL);

  g_object_get (page, "child", &widget, NULL);
  g_assert_true (widget == button);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_page_set_child (page, NULL);
  g_assert_null (adw_navigation_page_get_child (page));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "child", button, NULL);
  g_assert_true (adw_navigation_page_get_child (page) == button);
  g_assert_cmpint (notified, ==, 2);

  g_object_unref (widget);
  g_assert_finalize_object (page);
  g_assert_finalize_object (button);
}

static void
test_adw_navigation_page_title (void)
{
  AdwNavigationPage *page = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Title"));
  char *title;

  g_assert_nonnull (page);

  notified = 0;
  g_signal_connect (page, "notify::title", G_CALLBACK (notify_cb), NULL);

  g_object_get (page, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title");
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_page_set_title (page, "Some title");
  g_assert_cmpstr (adw_navigation_page_get_title (page), ==, "Some title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "title", "Some other title", NULL);
  g_assert_cmpstr (adw_navigation_page_get_title (page), ==, "Some other title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (page);
}

static void
test_adw_navigation_page_tag (void)
{
  AdwNavigationPage *page = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Title"));
  char *tag;

  g_assert_nonnull (page);

  notified = 0;
  g_signal_connect (page, "notify::tag", G_CALLBACK (notify_cb), NULL);

  g_object_get (page, "tag", &tag, NULL);
  g_assert_null (tag);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_page_set_tag (page, "tag");
  g_assert_cmpstr (adw_navigation_page_get_tag (page), ==, "tag");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "tag", "other-tag", NULL);
  g_assert_cmpstr (adw_navigation_page_get_tag (page), ==, "other-tag");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (page);
}

static void
test_adw_navigation_page_can_pop (void)
{
  AdwNavigationPage *page = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Title"));
  gboolean can_pop;

  g_assert_nonnull (page);

  notified = 0;
  g_signal_connect (page, "notify::can-pop", G_CALLBACK (notify_cb), NULL);

  g_object_get (page, "can-pop", &can_pop, NULL);
  g_assert_true (can_pop);

  adw_navigation_page_set_can_pop (page, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_page_set_can_pop (page, FALSE);
  g_assert_false (adw_navigation_page_get_can_pop (page));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (page, "can-pop", TRUE, NULL);
  g_assert_true (adw_navigation_page_get_can_pop (page));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (page);
}

static void
test_adw_navigation_page_signals (void)
{
  AdwNavigationView *view = g_object_ref_sink (ADW_NAVIGATION_VIEW (adw_navigation_view_new ()));
  AdwNavigationPage *page = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Title"));
  AdwNavigationPage *page_2 = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Title"));
  AdwNavigationPage *page_3 = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Title"));

  showing = shown = hiding = hidden = 0;
  g_signal_connect (page, "showing", G_CALLBACK (showing_cb), NULL);
  g_signal_connect (page, "shown", G_CALLBACK (shown_cb), NULL);
  g_signal_connect (page, "hiding", G_CALLBACK (hiding_cb), NULL);
  g_signal_connect (page, "hidden", G_CALLBACK (hidden_cb), NULL);

  g_assert_cmpint (showing, ==, 0);
  g_assert_cmpint (shown, ==, 0);
  g_assert_cmpint (hiding, ==, 0);
  g_assert_cmpint (hidden, ==, 0);

  adw_navigation_view_add (view, page);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 0);
  g_assert_cmpint (hidden, ==, 0);

  adw_navigation_view_push (view, page_2);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 1);
  g_assert_cmpint (hidden, ==, 1);

  adw_navigation_view_pop (view);
  g_assert_cmpint (showing, ==, 2);
  g_assert_cmpint (shown, ==, 2);
  g_assert_cmpint (hiding, ==, 1);
  g_assert_cmpint (hidden, ==, 1);

  adw_navigation_view_replace (view, &page_2, 1);
  g_assert_cmpint (showing, ==, 2);
  g_assert_cmpint (shown, ==, 2);
  g_assert_cmpint (hiding, ==, 2);
  g_assert_cmpint (hidden, ==, 2);

  adw_navigation_view_push (view, page);
  g_assert_cmpint (showing, ==, 3);
  g_assert_cmpint (shown, ==, 3);
  g_assert_cmpint (hiding, ==, 2);
  g_assert_cmpint (hidden, ==, 2);

  adw_navigation_view_pop (view);
  g_assert_cmpint (showing, ==, 3);
  g_assert_cmpint (shown, ==, 3);
  g_assert_cmpint (hiding, ==, 3);
  g_assert_cmpint (hidden, ==, 3);

  adw_navigation_view_replace (view, (AdwNavigationPage *[3]) {
    page_2,
    page,
    page_3,
  }, 3);
  g_assert_cmpint (showing, ==, 3);
  g_assert_cmpint (shown, ==, 3);
  g_assert_cmpint (hiding, ==, 3);
  g_assert_cmpint (hidden, ==, 3);

  adw_navigation_view_pop_to_page (view, page_2);
  g_assert_cmpint (showing, ==, 3);
  g_assert_cmpint (shown, ==, 3);
  g_assert_cmpint (hiding, ==, 3);
  g_assert_cmpint (hidden, ==, 3);

  g_assert_finalize_object (view);
  g_assert_finalize_object (page);
  g_assert_finalize_object (page_2);
  g_assert_finalize_object (page_3);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/NavigationView/add_remove", test_adw_navigation_view_add_remove);
  g_test_add_func ("/Adwaita/NavigationView/push_pop", test_adw_navigation_view_push_pop);
  g_test_add_func ("/Adwaita/NavigationView/push_pop_by_tag", test_adw_navigation_view_push_pop_by_tag);
  g_test_add_func ("/Adwaita/NavigationView/pop_to_page", test_adw_navigation_view_pop_to_page);
  g_test_add_func ("/Adwaita/NavigationView/replace", test_adw_navigation_view_replace);
  g_test_add_func ("/Adwaita/NavigationView/previous_page", test_adw_navigation_view_previous_page);
  g_test_add_func ("/Adwaita/NavigationView/find_page", test_adw_navigation_view_find_page);
  g_test_add_func ("/Adwaita/NavigationView/animate_transitions", test_adw_navigation_view_animate_transitions);
  g_test_add_func ("/Adwaita/NavigationPage/child", test_adw_navigation_page_child);
  g_test_add_func ("/Adwaita/NavigationPage/title", test_adw_navigation_page_title);
  g_test_add_func ("/Adwaita/NavigationPage/tag", test_adw_navigation_page_tag);
  g_test_add_func ("/Adwaita/NavigationPage/can-pop", test_adw_navigation_page_can_pop);
  g_test_add_func ("/Adwaita/NavigationPage/signals", test_adw_navigation_page_signals);

  return g_test_run ();
}
