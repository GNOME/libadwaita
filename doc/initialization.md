Title: Initialization
Slug: initialization

# Initialization

Before using Libadwaita, it must be initialized. There are two ways of doing
this.

## Using `AdwApplication` (Recommended)

[class@Application] automatically initializes Libadwaita if used instead of
[class@Gtk.Application].

Example:

```c
#include <adwaita.h>

static void
activate_cb (GtkApplication *app)
{
  GtkWidget *window = gtk_application_window_new (app);
  GtkWidget *label = gtk_label_new ("Hello World");

  gtk_window_set_title (GTK_WINDOW (window), "Hello");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  gtk_window_set_child (GTK_WINDOW (window), label);
  gtk_window_present (GTK_WINDOW (window));
}

int
main (int   argc,
      char *argv[])
{
  g_autoptr (AdwApplication) app = NULL;

  app = adw_application_new ("org.example.Hello", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect (app, "activate", G_CALLBACK (activate_cb), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
```

After building and running, the application will look like this:

<picture>
  <source srcset="hello-world-dark.png" media="(prefers-color-scheme: dark)">
  <img src="hello-world.png" alt="hello-world">
</picture>

## Using `adw_init()`

If using [class@Application] is not possible, use [func@init] instead. It can be
called instead of [func@Gtk.init].

Example:

```c
#include <adwaita.h>

int
main (int   argc,
      char *argv[])
{
  GtkWidget *window, *label;

  adw_init ();

  window = gtk_window_new ();
  label = gtk_label_new ("Hello World");

  gtk_window_set_title (GTK_WINDOW (window), "Hello");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  gtk_window_set_child (GTK_WINDOW (window), label);
  gtk_window_present (GTK_WINDOW (window));

   while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
```
