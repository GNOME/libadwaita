#include "adw-style-demo-window.h"

#include <glib/gi18n.h>

struct _AdwStyleDemoWindow
{
  AdwWindow parent_instance;

  gboolean progress;

  GtkWindow *header_bar_window;
  GtkWindow *status_page_window;
  GtkWindow *sidebar_window;
  AdwLeaflet *sidebar_leaflet;
};

G_DEFINE_FINAL_TYPE (AdwStyleDemoWindow, adw_style_demo_window, ADW_TYPE_WINDOW)

enum {
  PROP_0,
  PROP_DEVEL,
  PROP_PROGRESS,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
header_bar_cb (GtkWidget  *sender,
               const char *name,
               GVariant   *param)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (sender);

  gtk_window_present (self->header_bar_window);
}

static void
status_page_cb (GtkWidget  *sender,
                const char *name,
                GVariant   *param)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (sender);

  gtk_window_present (self->status_page_window);
}

static void
sidebar_cb (GtkWidget  *sender,
            const char *name,
            GVariant   *param)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (sender);

  gtk_window_present (self->sidebar_window);
}

static void
dummy_cb (GtkWidget  *sender,
          const char *name,
          GVariant   *param)
{
}

static void
set_devel_style (AdwStyleDemoWindow *self,
                 gboolean            devel)
{
  if (devel) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "devel");
    gtk_widget_add_css_class (GTK_WIDGET (self->header_bar_window), "devel");
    gtk_widget_add_css_class (GTK_WIDGET (self->status_page_window), "devel");
  } else {
    gtk_widget_remove_css_class (GTK_WIDGET (self), "devel");
    gtk_widget_remove_css_class (GTK_WIDGET (self->header_bar_window), "devel");
    gtk_widget_remove_css_class (GTK_WIDGET (self->status_page_window), "devel");
  }
}

static void
sidebar_back_cb (AdwStyleDemoWindow *self)
{
  adw_leaflet_navigate (self->sidebar_leaflet, ADW_NAVIGATION_DIRECTION_BACK);
}

static void
sidebar_forward_cb (AdwStyleDemoWindow *self)
{
  adw_leaflet_navigate (self->sidebar_leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
}

static GtkSelectionMode
selection_mode_for_folded (gpointer data,
                           gboolean folded)
{
  return folded ? GTK_SELECTION_NONE : GTK_SELECTION_BROWSE;
}

static void
adw_style_demo_window_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (object);

  switch (prop_id) {
  case PROP_DEVEL:
    g_value_set_boolean (value, gtk_widget_has_css_class (GTK_WIDGET (self), "devel"));
    break;
  case PROP_PROGRESS:
    g_value_set_boolean (value, self->progress);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_demo_window_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (object);

  switch (prop_id) {
  case PROP_DEVEL:
    set_devel_style (self, g_value_get_boolean (value));
    break;
  case PROP_PROGRESS:
    self->progress = g_value_get_boolean (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_demo_window_dispose (GObject *object)
{
  AdwStyleDemoWindow *self = ADW_STYLE_DEMO_WINDOW (object);

  if (self->header_bar_window)
    gtk_window_destroy (self->header_bar_window);

  if (self->status_page_window)
    gtk_window_destroy (self->status_page_window);

  if (self->sidebar_window)
    gtk_window_destroy (self->sidebar_window);

  G_OBJECT_CLASS (adw_style_demo_window_parent_class)->dispose (object);
}

static void
adw_style_demo_window_class_init (AdwStyleDemoWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_style_demo_window_get_property;
  object_class->set_property = adw_style_demo_window_set_property;
  object_class->dispose = adw_style_demo_window_dispose;

  props[PROP_DEVEL] =
    g_param_spec_boolean ("devel", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  props[PROP_PROGRESS] =
    g_param_spec_boolean ("progress", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/styles/adw-style-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoWindow, header_bar_window);
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoWindow, status_page_window);
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoWindow, sidebar_window);
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoWindow, sidebar_leaflet);
  gtk_widget_class_bind_template_callback (widget_class, selection_mode_for_folded);
  gtk_widget_class_bind_template_callback (widget_class, sidebar_back_cb);
  gtk_widget_class_bind_template_callback (widget_class, sidebar_forward_cb);

  gtk_widget_class_install_property_action (widget_class, "style.devel", "devel");
  gtk_widget_class_install_property_action (widget_class, "style.progress", "progress");
  gtk_widget_class_install_action (widget_class, "style.header-bar", NULL, header_bar_cb);
  gtk_widget_class_install_action (widget_class, "style.status-page", NULL, status_page_cb);
  gtk_widget_class_install_action (widget_class, "style.sidebar", NULL, sidebar_cb);
  gtk_widget_class_install_action (widget_class, "style.dummy", NULL, dummy_cb);
}

static void
adw_style_demo_window_init (AdwStyleDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwStyleDemoWindow *
adw_style_demo_window_new (void)
{
  return g_object_new (ADW_TYPE_STYLE_DEMO_WINDOW, NULL);
}
