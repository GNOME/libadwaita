#include "adw-demo-page-leaflet.h"

#include <glib/gi18n.h>

struct _AdwDemoPageLeaflet
{
  AdwBin parent_instance;

  AdwLeafletTransitionType transition_type;
};

enum {
  PROP_0,
  PROP_TRANSITION_TYPE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_NEXT_PAGE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

G_DEFINE_TYPE (AdwDemoPageLeaflet, adw_demo_page_leaflet, ADW_TYPE_BIN)

static char *
get_transition_name (AdwEnumListItem *item,
                     gpointer         user_data)
{
  switch (adw_enum_list_item_get_value (item)) {
  case ADW_LEAFLET_TRANSITION_TYPE_OVER:
    return g_strdup (_("Over"));
  case ADW_LEAFLET_TRANSITION_TYPE_UNDER:
    return g_strdup (_("Under"));
  case ADW_LEAFLET_TRANSITION_TYPE_SLIDE:
    return g_strdup (_("Slide"));
  default:
    return NULL;
  }
}

static void
next_row_activated_cb (AdwDemoPageLeaflet *self)
{
  g_signal_emit (self, signals[SIGNAL_NEXT_PAGE], 0);
}

static void
adw_demo_page_leaflet_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwDemoPageLeaflet *self = ADW_DEMO_PAGE_LEAFLET (object);

  switch (prop_id) {
  case PROP_TRANSITION_TYPE:
    g_value_set_enum (value, self->transition_type);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_page_leaflet_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwDemoPageLeaflet *self = ADW_DEMO_PAGE_LEAFLET (object);

  switch (prop_id) {
  case PROP_TRANSITION_TYPE:
    self->transition_type = g_value_get_enum (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_page_leaflet_class_init (AdwDemoPageLeafletClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_demo_page_leaflet_get_property;
  object_class->set_property = adw_demo_page_leaflet_set_property;

  props[PROP_TRANSITION_TYPE] =
    g_param_spec_enum ("transition-type", NULL, NULL,
                       ADW_TYPE_LEAFLET_TRANSITION_TYPE,
                       ADW_LEAFLET_TRANSITION_TYPE_OVER,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  signals[SIGNAL_NEXT_PAGE] =
    g_signal_new ("next-page",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/leaflet/adw-demo-page-leaflet.ui");
  gtk_widget_class_bind_template_callback (widget_class, get_transition_name);
  gtk_widget_class_bind_template_callback (widget_class, next_row_activated_cb);
}

static void
adw_demo_page_leaflet_init (AdwDemoPageLeaflet *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
