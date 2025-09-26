#include "adw-demo-sidebar-item.h"

struct _AdwDemoSidebarItem
{
  AdwSidebarItem parent_instance;

  GType page_type;
};

G_DEFINE_FINAL_TYPE (AdwDemoSidebarItem, adw_demo_sidebar_item, ADW_TYPE_SIDEBAR_ITEM)

enum {
  PROP_0,
  PROP_PAGE_TYPE,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
adw_demo_sidebar_item_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwDemoSidebarItem *self = ADW_DEMO_SIDEBAR_ITEM (object);

  switch (prop_id) {
  case PROP_PAGE_TYPE:
    g_value_set_gtype (value, self->page_type);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_sidebar_item_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwDemoSidebarItem *self = ADW_DEMO_SIDEBAR_ITEM (object);

  switch (prop_id) {
  case PROP_PAGE_TYPE:
    self->page_type = g_value_get_gtype (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_sidebar_item_class_init (AdwDemoSidebarItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = adw_demo_sidebar_item_get_property;
  object_class->set_property = adw_demo_sidebar_item_set_property;

  props[PROP_PAGE_TYPE] =
    g_param_spec_gtype ("page-type", "", "",
                        GTK_TYPE_WIDGET,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_demo_sidebar_item_init (AdwDemoSidebarItem *self)
{
}

GType
adw_demo_sidebar_item_get_page_type (AdwDemoSidebarItem *self)
{
  g_return_val_if_fail (ADW_IS_DEMO_SIDEBAR_ITEM (self), G_TYPE_NONE);

  return self->page_type;
}
