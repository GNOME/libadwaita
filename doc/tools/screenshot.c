#include <adwaita.h>

#define RESOURCE_PATH "/org/gnome/Adwaita/Screenshot/"

static GMainLoop *loop;

typedef struct {
  GtkWidget *widget;
  GtkWidget *hover_widget;
  GdkPaintable *paintable;
  char *name;
  GtkCssProvider *provider;
} ScreenshotData;

static void
screenshot_data_free (ScreenshotData *data)
{
  g_object_unref (data->paintable);
  gtk_window_destroy (GTK_WINDOW (gtk_widget_get_root (data->widget)));
  g_object_unref (data->provider);
  g_free (data->name);
  g_free (data);
}

static void
draw_paintable (ScreenshotData *data)
{
  GtkSnapshot *snapshot;
  GskRenderer *renderer;
  g_autoptr (GdkTexture) texture = NULL;
  g_autoptr (GskRenderNode) node = NULL;
  int x, y, width, height;
  int widget_width, widget_height;

  g_signal_handlers_disconnect_by_func (data->paintable,
                                        G_CALLBACK (draw_paintable),
                                        data);

  widget_width = gtk_widget_get_allocated_width (data->widget);
  widget_height = gtk_widget_get_allocated_height (data->widget);

  if (GTK_IS_NATIVE (data->widget)) {
    GdkSurface *surface;
    double transform_x, transform_y;

    surface = gtk_native_get_surface (GTK_NATIVE (data->widget));
    gtk_native_get_surface_transform (GTK_NATIVE (data->widget),
                                      &transform_x, &transform_y);
    x = floor (transform_x);
    y = floor (transform_y);
    width = gdk_surface_get_width (surface);
    height = gdk_surface_get_height (surface);
  } else {
    x = gtk_widget_get_margin_start (data->widget);
    y = gtk_widget_get_margin_top (data->widget);
    width = widget_width + x + gtk_widget_get_margin_end (data->widget);
    height = widget_height + y + gtk_widget_get_margin_bottom (data->widget);
  }


  snapshot = gtk_snapshot_new ();

  gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (x, y));
  gdk_paintable_snapshot (data->paintable, snapshot, widget_width, widget_height);
  node = gtk_snapshot_free_to_node (snapshot);

  if (gsk_render_node_get_node_type (node) == GSK_CLIP_NODE &&
      (x > 0 || y > 0 || widget_width < width || widget_height < height)) {
    g_autoptr (GskRenderNode) original_node = g_steal_pointer (&node);

    node = gsk_render_node_ref (gsk_clip_node_get_child (original_node));
  }

  renderer = gtk_native_get_renderer (gtk_widget_get_native (data->widget));

  texture = gsk_renderer_render_texture (renderer, node,
                                         &GRAPHENE_RECT_INIT (0, 0, width, height));

  gdk_texture_save_to_png (texture, data->name);

  screenshot_data_free (data);

  g_main_loop_quit (loop);
}

static GtkCssProvider *
load_css (const char *name)
{
  GtkCssProvider *provider = gtk_css_provider_new ();
  g_autofree char *path = g_strdup_printf (RESOURCE_PATH "%s.css", name);

  gtk_css_provider_load_from_resource (provider, path);

  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  return provider;
}

static gboolean
take_screenshot_cb (ScreenshotData *data)
{
  if (GTK_IS_POPOVER (data->widget))
    gtk_popover_popup (GTK_POPOVER (data->widget));

  if (data->hover_widget) {
    GtkStateFlags flags = gtk_widget_get_state_flags (data->hover_widget);

    gtk_widget_set_state_flags (data->hover_widget,
                                flags | GTK_STATE_FLAG_PRELIGHT, FALSE);
  }

  g_signal_connect_swapped (data->paintable, "invalidate-contents",
                            G_CALLBACK (draw_paintable), data);

  gtk_widget_queue_draw (data->widget);

  return G_SOURCE_REMOVE;
}

static void
take_screenshot (const char *name,
                 gboolean    dark,
                 GFile      *output_dir)
{
  g_autofree char *ui_path = NULL;
  g_autofree char *output_name = NULL;
  g_autoptr (GtkBuilder) builder = NULL;
  g_autoptr (GFile) output_file = NULL;
  ScreenshotData *data;
  GObject *widget;
  GObject *hover_widget;
  GtkWidget *window;
  gboolean wait = FALSE;

  ui_path = g_strdup_printf (RESOURCE_PATH "data/%s.ui", name);

  if (dark)
    output_name = g_strdup_printf ("%s-dark.png", name);
  else
    output_name = g_strdup_printf ("%s.png", name);

  output_file = g_file_get_child (output_dir, output_name);

  loop = g_main_loop_new (NULL, FALSE);

  if (dark)
    adw_style_manager_set_color_scheme (adw_style_manager_get_default (),
                                        ADW_COLOR_SCHEME_FORCE_DARK);
  else
    adw_style_manager_set_color_scheme (adw_style_manager_get_default (),
                                        ADW_COLOR_SCHEME_FORCE_LIGHT);

  builder = gtk_builder_new_from_resource (ui_path);
  widget = gtk_builder_get_object (builder, "widget");
  hover_widget = gtk_builder_get_object (builder, "hover");

  g_assert (GTK_IS_WIDGET (widget));

  data = g_new0 (ScreenshotData, 1);
  data->widget = GTK_WIDGET (widget);

  if (GTK_IS_WINDOW (widget)) {
    window = GTK_WIDGET (widget);
  } else if (GTK_IS_POPOVER (widget)) {
    GtkWidget *button;

    gtk_popover_set_autohide (GTK_POPOVER (widget), FALSE);

    button = gtk_menu_button_new ();
    gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), GTK_WIDGET (widget));

    window = gtk_window_new ();
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_child (GTK_WINDOW (window), button);

    wait = TRUE;
 } else {
    window = gtk_window_new ();
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_child (GTK_WINDOW (window), data->widget);
  }

  data->paintable = gtk_widget_paintable_new (data->widget);
  data->name = g_file_get_path (output_file);
  data->provider = load_css ("style");
  if (hover_widget)
    data->hover_widget = GTK_WIDGET (hover_widget);

  if (wait)
    g_timeout_add_seconds (1, G_SOURCE_FUNC (take_screenshot_cb), data);

  gtk_window_present (GTK_WINDOW (window));

  if (!wait)
    take_screenshot_cb (data);

  g_main_loop_run (loop);
  g_main_loop_unref (loop);
}

static inline char *
get_shortname (const char *basename)
{
  const char *first_period = strstr (basename, ".");

  if (first_period)
    return g_strndup (basename, first_period - basename);

  return g_strdup (basename);
}

static void
init_libadwaita (void)
{
  adw_init ();

  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_for_display (gdk_display_get_default ()),
                                    RESOURCE_PATH "icons/");

  g_object_set (gtk_settings_get_default (),
                "gtk-font-name", "Cantarell 11",
                "gtk-icon-theme-name", "Adwaita",
                "gtk-decoration-layout", ":close",
                "gtk-hint-font-metrics", TRUE,
                NULL);
}

static void
run_screenshot (GFile *output_dir)
{
  g_autoptr (GError) error = NULL;
  g_auto (GStrv) children = NULL;
  int i = -1;

  children =
    g_resources_enumerate_children (RESOURCE_PATH "data",
                                    G_RESOURCE_LOOKUP_FLAGS_NONE,
                                    &error);
  if (error) {
    g_critical ("Couldn't enumerate children: %s", error->message);
    return;
  }

  while (children[++i]) {
    g_autofree char *shortname = get_shortname (children[i]);

    g_print ("Processing %s\n", shortname);
    take_screenshot (shortname, FALSE, output_dir);
    take_screenshot (shortname, TRUE, output_dir);
  }
}

int
main (int    argc,
      char **argv)
{
  GOptionContext *context = g_option_context_new ("PATH");
  g_autoptr (GFile) output_dir = NULL;
  g_autoptr (GError) error = NULL;

  g_option_context_parse (context, &argc, &argv, NULL);

  if (argc < 2 || !argv[1]) {
    g_printerr ("%s\n", g_option_context_get_help (context, FALSE, NULL));

    return 1;
  }

  output_dir = g_file_new_for_path (argv[1]);

  if (!g_file_query_exists (output_dir, NULL)) {
    g_file_make_directory_with_parents (output_dir, NULL, &error);
    if (G_UNLIKELY (error != NULL)) {
      g_critical ("Failed to create output directory: %s", error->message);

      return 1;
    }
  }

  init_libadwaita ();
  run_screenshot (output_dir);

  return 0;
}
