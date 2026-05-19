/*
 * Copyright (C) 2026 Jamie Murphy <jmurphy@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_css_class_binding_default (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;

  gtk_widget_set_visible (source, false);

  binding = adw_bind_property_to_css_class (source, "visible",
                                            target, "test-class",
                                            G_BINDING_DEFAULT);

  GObject *tmp_source = adw_css_class_binding_get_source (binding);
  g_assert_nonnull (tmp_source);
  g_assert_true ((gpointer) tmp_source == (gpointer) source);

  GtkWidget *tmp_target = adw_css_class_binding_get_target (binding);
  g_assert_nonnull (tmp_target);
  g_assert_true ((gpointer) tmp_target == (gpointer) target);

  gtk_widget_set_visible (source, true);
  g_assert_true (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_set_visible (source, false);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  g_assert_finalize_object (binding);

  gtk_widget_set_visible (source, true);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
}

static void
test_adw_css_class_binding_multiple_bindings_same_property (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));

  gtk_widget_set_visible (source, TRUE);

  adw_bind_property_to_css_class (source, "visible",
                                  target, "test-class",
                                  G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
  adw_bind_property_to_css_class (source, "visible",
                                  target, "second-class",
                                  G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);

  g_assert_true (gtk_widget_has_css_class (target, "test-class"));
  g_assert_false (gtk_widget_has_css_class (target, "second-class"));

  gtk_widget_set_visible (source, FALSE);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));
  g_assert_true (gtk_widget_has_css_class (target, "second-class"));

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
}

static gboolean
custom_map_to (AdwCssClassBinding *self,
               const GValue       *from_value,
               gpointer            user_data)
{
  gdouble opacity;

  g_assert_true (G_VALUE_HOLDS (from_value, G_TYPE_DOUBLE));

  opacity = g_value_get_double (from_value);

  return opacity < 0.5;
}

static void
custom_map_from (AdwCssClassBinding *self,
                 gboolean            from_value,
                 GValue             *to_value,
                 gpointer            user_data)
{
  if (from_value)
    g_value_set_double (to_value, 1.0);
  else
    g_value_set_double (to_value, 0.0);
}

static void
data_free (gpointer data)
{
  gboolean *b = data;
  *b = TRUE;
}

static void
test_adw_css_class_binding_custom_map (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  gboolean unused_data = FALSE;

  adw_bind_property_to_css_class_full (source, "opacity",
                                       target, "test-class",
                                       G_BINDING_DEFAULT,
                                       custom_map_to,
                                       NULL,
                                       &unused_data,
                                       data_free);

  gtk_widget_set_opacity (source, 0.0);
  g_assert_true (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_set_opacity (source, 1.0);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
  g_assert_true (unused_data);
}

static void
test_adw_css_class_binding_custom_map_bidirectional (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  gboolean unused_data = FALSE;

  gtk_widget_set_opacity (source, 0.0);

  adw_bind_property_to_css_class_full (source, "opacity",
                                       target, "test-class",
                                       G_BINDING_DEFAULT | G_BINDING_BIDIRECTIONAL,
                                       custom_map_to,
                                       custom_map_from,
                                       &unused_data,
                                       data_free);

  
  gtk_widget_add_css_class (target, "test-class");
  g_assert_cmpfloat_with_epsilon (gtk_widget_get_opacity (source), 1.0, 0.005);

  gtk_widget_remove_css_class (target, "test-class");
  g_assert_cmpfloat_with_epsilon (gtk_widget_get_opacity (source), 0.0, 0.005);

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
  g_assert_true (unused_data);
}

static void
test_adw_css_class_binding_closures (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  gboolean unused_data_1 = FALSE, unused_data_2 = FALSE;
  GClosure *map_to, *map_from;

  map_to = g_cclosure_new (G_CALLBACK (custom_map_to), &unused_data_1, (GClosureNotify) data_free);
  map_from = g_cclosure_new (G_CALLBACK (custom_map_from), &unused_data_2, (GClosureNotify) data_free);

  adw_bind_property_to_css_class_with_closures (source, "opacity",
                                                target, "test-class",
                                                G_BINDING_DEFAULT,
                                                map_to, map_from);

  gtk_widget_set_opacity (source, 0.0);
  g_assert_true (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_set_opacity (source, 1.0);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);

  g_assert_true (unused_data_1);
  g_assert_true (unused_data_2);
}

static void
test_adw_css_class_binding_sync_create (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));

  gtk_widget_set_visible (source, TRUE);

  adw_bind_property_to_css_class (source, "visible",
                                  target, "test-class",
                                  G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

  g_assert_true (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_set_visible (source, FALSE);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
}

static void
test_adw_css_class_binding_invert_boolean (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));

  adw_bind_property_to_css_class (source, "visible",
                                  target, "test-class",
                                  G_BINDING_DEFAULT | G_BINDING_INVERT_BOOLEAN);

  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_set_visible (source, FALSE);
  g_assert_true (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_set_visible (source, TRUE);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
}

static void
test_adw_css_class_binding_bidirectional (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;

  gtk_widget_set_visible (source, FALSE);

  binding = adw_bind_property_to_css_class (source, "visible",
                                            target, "test-class",
                                            G_BINDING_DEFAULT | G_BINDING_BIDIRECTIONAL);

  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_set_visible (source, TRUE);
  g_assert_true (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_remove_css_class (target, "test-class");
  g_assert_false (gtk_widget_get_visible (source));

  gtk_widget_add_css_class (target, "test-class");
  g_assert_true (gtk_widget_get_visible (source));

  g_assert_finalize_object (binding);

  gtk_widget_remove_css_class (target, "test-class");
  g_assert_true (gtk_widget_get_visible (source));

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
}

static void
test_adw_css_class_binding_same_object (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;

  gtk_widget_set_visible (source, FALSE);

  binding = adw_bind_property_to_css_class (source, "visible",
                                            source, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);

  g_assert_false (gtk_widget_has_css_class (source, "test-class"));

  gtk_widget_set_visible (source, TRUE);
  g_assert_true (gtk_widget_has_css_class (source, "test-class"));

  gtk_widget_set_visible (source, FALSE);
  g_assert_false (gtk_widget_has_css_class (source, "test-class"));

  g_assert_finalize_object (source);
  g_assert_null (binding);
}

static void
test_adw_css_class_binding_unbind (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;
  
  gtk_widget_set_visible (source, FALSE);

  binding = adw_bind_property_to_css_class (source, "visible",
                                            target, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);

  gtk_widget_set_visible (source, TRUE);
  g_assert_true (gtk_widget_has_css_class (target, "test-class"));

  gtk_widget_set_visible (source, FALSE);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  adw_css_class_binding_unbind (binding);
  g_assert_null (binding);

  gtk_widget_set_visible (source, TRUE);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
}

static void
test_adw_css_class_binding_unbind_same_object (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;
  
  gtk_widget_set_visible (source, FALSE);

  binding = adw_bind_property_to_css_class (source, "visible",
                                            source, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);

  gtk_widget_set_visible (source, TRUE);
  g_assert_true (gtk_widget_has_css_class (source, "test-class"));

  gtk_widget_set_visible (source, FALSE);
  g_assert_false (gtk_widget_has_css_class (source, "test-class"));

  adw_css_class_binding_unbind (binding);
  g_assert_null (binding);

  g_assert_finalize_object (source);
}

static void
test_adw_css_class_binding_unbind_weak_ref (void)
{
  GtkWidget *source;
  GtkWidget *target;
  AdwCssClassBinding *binding;

  /* Source, then Target */
  source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  binding = adw_bind_property_to_css_class (source, "visible",
                                            target, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);

  g_assert_nonnull (binding);
  g_assert_finalize_object (source);
  g_assert_null (binding);
  g_assert_finalize_object (target);
  g_assert_null (binding);

  /* Target, then Source */
  source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  binding = adw_bind_property_to_css_class (source, "visible",
                                            target, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);

  g_assert_nonnull (binding);
  g_assert_finalize_object (target);
  g_assert_null (binding);
  g_assert_finalize_object (source);
  g_assert_null (binding);

  /* Same Object Source */
  source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  binding = adw_bind_property_to_css_class (source, "visible",
                                            source, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);

  g_assert_nonnull (binding);
  g_assert_finalize_object (source);
  g_assert_null (binding);
}

static void
test_adw_css_class_binding_unbind_multiple_calls (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;
  guint i;

  gtk_widget_set_visible (source, FALSE);

  binding = adw_bind_property_to_css_class (source, "visible",
                                            target, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_ref (binding);
  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);
  g_assert_nonnull (binding);

  for (i = 0; i < 50; i++)
    {
      adw_css_class_binding_unbind (binding);
      g_assert_nonnull (binding);
    }

  gtk_widget_set_visible (source, TRUE);
  g_assert_false (gtk_widget_has_css_class (target, "test-class"));

  adw_css_class_binding_unbind (binding);
  g_object_unref (binding);
  g_assert_null (binding);

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
}

static void
test_adw_css_class_binding_dispose_source (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;

  binding = adw_bind_property_to_css_class (source, "visible",
                                            target, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);

  g_object_run_dispose (G_OBJECT (source));
  g_assert_null (binding);

  g_assert_finalize_object (target);
  g_assert_finalize_object (source);
}

static void
test_adw_css_class_binding_dispose_target (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;

  binding = adw_bind_property_to_css_class (source, "visible",
                                            target, "test-class",
                                            G_BINDING_DEFAULT);

  g_object_add_weak_pointer (G_OBJECT (binding), (gpointer *) &binding);

  g_object_run_dispose (G_OBJECT (target));
  g_assert_null (binding);

  g_assert_finalize_object (target);
  g_assert_finalize_object (source);
}

static void
test_adw_css_class_binding_invalid_property_type (void)
{
  GtkWidget *source = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  GtkWidget *target = g_object_ref_sink (GTK_WIDGET (adw_bin_new ()));
  AdwCssClassBinding *binding;

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                         "*default mapping functions*boolean*double*");

  binding = adw_bind_property_to_css_class (source, "opacity",
                                            target, "test-class",
                                            G_BINDING_DEFAULT);

  g_assert_null (binding);

  g_test_assert_expected_messages ();

  g_assert_finalize_object (source);
  g_assert_finalize_object (target);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/CssClassBinding/default", test_adw_css_class_binding_default);
  g_test_add_func ("/Adwaita/CssClassBinding/multiple-bindings-same-property", test_adw_css_class_binding_multiple_bindings_same_property);
  g_test_add_func ("/Adwaita/CssClassBinding/custom-map", test_adw_css_class_binding_custom_map);
  g_test_add_func ("/Adwaita/CssClassBinding/custom-map-bidirectional", test_adw_css_class_binding_custom_map_bidirectional);
  g_test_add_func ("/Adwaita/CssClassBinding/closures", test_adw_css_class_binding_closures);
  g_test_add_func ("/Adwaita/CssClassBinding/sync-create", test_adw_css_class_binding_sync_create);
  g_test_add_func ("/Adwaita/CssClassBinding/invert-boolean", test_adw_css_class_binding_invert_boolean);
  g_test_add_func ("/Adwaita/CssClassBinding/bidirectional", test_adw_css_class_binding_bidirectional);
  g_test_add_func ("/Adwaita/CssClassBinding/same-object", test_adw_css_class_binding_same_object);
  g_test_add_func ("/Adwaita/CssClassBinding/unbind", test_adw_css_class_binding_unbind);
  g_test_add_func ("/Adwaita/CssClassBinding/unbind-same-object", test_adw_css_class_binding_unbind_same_object);
  g_test_add_func ("/Adwaita/CssClassBinding/unbind-weak-ref", test_adw_css_class_binding_unbind_weak_ref);
  g_test_add_func ("/Adwaita/CssClassBinding/unbind-multiple-calls", test_adw_css_class_binding_unbind_multiple_calls);
  g_test_add_func ("/Adwaita/CssClassBinding/dispose-source", test_adw_css_class_binding_dispose_source);
  g_test_add_func ("/Adwaita/CssClassBinding/dispose-target", test_adw_css_class_binding_dispose_target);
  g_test_add_func ("/Adwaita/CssClassBinding/invalid-property-type", test_adw_css_class_binding_invalid_property_type);

  return g_test_run ();
}
