#include "adw-demo-page.h"

#include <glib/gi18n.h>

typedef struct
{
  gboolean show_back_button;
  char *title;
  GtkWidget *child;

  GtkWidget *header_bar;
  GtkWidget *child_bin;
} AdwDemoPagePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwDemoPage, adw_demo_page, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_SHOW_BACK_BUTTON,
  PROP_TITLE,
  PROP_CHILD,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static inline void
set_string (char       **dest,
            const char  *source)
{
  if (*dest)
    g_free (*dest);

  *dest = g_strdup (source);
}

static void
adw_demo_page_dispose (GObject *object)
{
  AdwDemoPage *self = ADW_DEMO_PAGE (object);
  AdwDemoPagePrivate *priv = adw_demo_page_get_instance_private (self);

  gtk_widget_unparent (priv->header_bar);
  gtk_widget_unparent (priv->child_bin);
  g_clear_object (&priv->child);

  G_OBJECT_CLASS (adw_demo_page_parent_class)->dispose (object);
}

static void
adw_demo_page_finalize (GObject *object)
{
  AdwDemoPage *self = ADW_DEMO_PAGE (object);
  AdwDemoPagePrivate *priv = adw_demo_page_get_instance_private (self);

  g_clear_pointer (&priv->title, g_free);

  G_OBJECT_CLASS (adw_demo_page_parent_class)->finalize (object);
}

static void
adw_demo_page_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  AdwDemoPage *self = ADW_DEMO_PAGE (object);
  AdwDemoPagePrivate *priv = adw_demo_page_get_instance_private (self);

  switch (prop_id) {
  case PROP_SHOW_BACK_BUTTON:
    g_value_set_boolean (value, priv->show_back_button);
    break;
  case PROP_TITLE:
    g_value_set_string (value, priv->title);
    break;
  case PROP_CHILD:
    g_value_set_object (value, priv->child);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_page_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  AdwDemoPage *self = ADW_DEMO_PAGE (object);
  AdwDemoPagePrivate *priv = adw_demo_page_get_instance_private (self);

  switch (prop_id) {
  case PROP_SHOW_BACK_BUTTON:
    priv->show_back_button = g_value_get_boolean (value);
    break;
  case PROP_TITLE:
    set_string (&priv->title, g_value_get_string (value));
    break;
  case PROP_CHILD:
    g_set_object (&priv->child, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_page_class_init (AdwDemoPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_demo_page_dispose;
  object_class->finalize = adw_demo_page_finalize;
  object_class->get_property = adw_demo_page_get_property;
  object_class->set_property = adw_demo_page_set_property;

  props[PROP_SHOW_BACK_BUTTON] =
    g_param_spec_boolean ("show-back-button",
                          _("Show Back Button"),
                          _("Show Back Button"),
                          FALSE,
                          G_PARAM_READWRITE);

  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("Title"),
                         NULL,
                         G_PARAM_READWRITE);

  props[PROP_CHILD] =
    g_param_spec_object ("child",
                         _("Child"),
                         _("Child"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/adw-demo-page.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwDemoPage, header_bar);
  gtk_widget_class_bind_template_child_private (widget_class, AdwDemoPage, child_bin);
}

static void
adw_demo_page_init (AdwDemoPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
