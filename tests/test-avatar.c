/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>

#define TEST_ICON_NAME "avatar-default-symbolic"
#define TEST_STRING "Mario Rossi"
#define TEST_SIZE 128


gint load_image_func_count;

static gboolean
is_surface_empty (cairo_surface_t *surface)
{
  unsigned char * data;
  guint length;

  cairo_surface_flush (surface);
  data = cairo_image_surface_get_data (surface);
  length = cairo_image_surface_get_width (surface) * cairo_image_surface_get_height (surface);

  for (int i = 0; i < length; i++) {
    if (data[i] != 0)
      return FALSE;
  }
  return TRUE;
}

static GdkPixbuf *
load_null_image_func (gint size,
                      gpointer data)
{
  load_image_func_count++;
  return NULL;
}

static GdkPixbuf *
load_image_func (gint size,
                 GdkRGBA *color)
{
  GdkPixbuf *pixbuf;
  cairo_surface_t *surface;
  cairo_t *cr;

  load_image_func_count++;

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, size, size);
  cr = cairo_create (surface);
  if (color != NULL) {
    gdk_cairo_set_source_rgba (cr, color);
    cairo_paint (cr);
  }
  pixbuf = gdk_pixbuf_get_from_surface (surface, 0, 0, size, size);

  cairo_surface_destroy (surface);
  cairo_destroy (cr);
  return pixbuf;
}


static void
map_event_cb (GtkWidget *widget, GdkEvent *event, cairo_surface_t **surface)
{
  cairo_t *cr;

  g_assert (surface != NULL);

  *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, TEST_SIZE, TEST_SIZE);
  cr = cairo_create (*surface);
  gtk_widget_draw (widget, cr);
  cairo_destroy (cr);
  gtk_main_quit ();
}


static gboolean
did_draw_something (GtkWidget *widget)
{
  GtkWidget *window;
  gboolean empty;
  cairo_surface_t *surface;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_widget_set_events (widget, GDK_STRUCTURE_MASK);
  g_signal_connect (widget, "map-event", G_CALLBACK (map_event_cb), &surface);

  gtk_window_resize (GTK_WINDOW (window), TEST_SIZE, TEST_SIZE);
  gtk_container_add (GTK_CONTAINER (window), widget);

  gtk_widget_show (widget);
  gtk_widget_show (window);

  gtk_main ();

  g_assert (surface);
  g_assert (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS);
  empty =  is_surface_empty (surface);

  cairo_surface_destroy (surface);
  gtk_widget_destroy (window);

  return !empty;
}


static void
test_hdy_avatar_generate (void)
{
  GtkWidget *avatar = hdy_avatar_new (TEST_SIZE, "", TRUE);
  g_assert (HDY_IS_AVATAR (avatar));

  g_assert_true (did_draw_something (GTK_WIDGET (avatar)));
}


static void
test_hdy_avatar_icon_name (void)
{
  HdyAvatar *avatar = HDY_AVATAR (hdy_avatar_new (128, NULL, TRUE));

  g_assert_null (hdy_avatar_get_icon_name (avatar));
  hdy_avatar_set_icon_name (avatar, TEST_ICON_NAME);
  g_assert_cmpstr (hdy_avatar_get_icon_name (avatar), ==, TEST_ICON_NAME);

  g_assert_true (did_draw_something (GTK_WIDGET (avatar)));
}

static void
test_hdy_avatar_text (void)
{
  HdyAvatar *avatar = HDY_AVATAR (hdy_avatar_new (128, NULL, TRUE));

  g_assert_null (hdy_avatar_get_text (avatar));
  hdy_avatar_set_text (avatar, TEST_STRING);
  g_assert_cmpstr (hdy_avatar_get_text (avatar), ==, TEST_STRING);

  g_assert_true (did_draw_something (GTK_WIDGET (avatar)));
}

static void
test_hdy_avatar_size (void)
{
  HdyAvatar *avatar = HDY_AVATAR (hdy_avatar_new (TEST_SIZE, NULL, TRUE));

  g_assert_cmpint (hdy_avatar_get_size (avatar), ==, TEST_SIZE);
  hdy_avatar_set_size (avatar, TEST_SIZE / 2);
  g_assert_cmpint (hdy_avatar_get_size (avatar), ==, TEST_SIZE / 2);

  g_assert_true (did_draw_something (GTK_WIDGET (avatar)));
}

static void
test_hdy_avatar_custom_image (void)
{
  GtkWidget *avatar;
  GdkRGBA color;

  avatar = hdy_avatar_new (TEST_SIZE, NULL, TRUE);

  g_assert (HDY_IS_AVATAR (avatar));

  load_image_func_count = 0;

  hdy_avatar_set_image_load_func (HDY_AVATAR (avatar),
                                  (HdyAvatarImageLoadFunc) load_image_func,
                                  NULL,
                                  NULL);

  g_object_ref (avatar);
  g_assert_false (did_draw_something (avatar));

  hdy_avatar_set_image_load_func (HDY_AVATAR (avatar),
                                  NULL,
                                  NULL,
                                  NULL);

  g_assert_true (did_draw_something (avatar));

  gdk_rgba_parse (&color, "#F00");
  hdy_avatar_set_image_load_func (HDY_AVATAR (avatar),
                                  (HdyAvatarImageLoadFunc) load_image_func,
                                  &color,
                                  NULL);

  g_assert_true (did_draw_something (avatar));

  hdy_avatar_set_image_load_func (HDY_AVATAR (avatar),
                                  (HdyAvatarImageLoadFunc) load_null_image_func,
                                  NULL,
                                  NULL);

  g_assert_true (did_draw_something (avatar));

  g_assert_cmpint (load_image_func_count, ==, 3);

  g_object_unref (avatar);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func ("/Handy/Avatar/generate", test_hdy_avatar_generate);
  g_test_add_func ("/Handy/Avatar/custom_image", test_hdy_avatar_custom_image);
  g_test_add_func ("/Handy/Avatar/icon_name", test_hdy_avatar_icon_name);
  g_test_add_func ("/Handy/Avatar/text", test_hdy_avatar_text);
  g_test_add_func ("/Handy/Avatar/size", test_hdy_avatar_size);

  return g_test_run ();
}
