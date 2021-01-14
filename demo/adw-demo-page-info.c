#include "adw-demo-page-info.h"

#include <glib/gi18n.h>

#include "adw-demo-page.h"

struct _AdwDemoPageInfo
{
  GObject parent_instance;

  char *icon_name;
  char *title;
  GType gtype;
};

G_DEFINE_TYPE (AdwDemoPageInfo, adw_demo_page_info, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_TITLE,
  PROP_GTYPE,
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
adw_demo_page_info_finalize (GObject *object)
{
  AdwDemoPageInfo *self = ADW_DEMO_PAGE_INFO (object);

  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->icon_name, g_free);

  G_OBJECT_CLASS (adw_demo_page_info_parent_class)->finalize (object);
}

static void
adw_demo_page_info_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwDemoPageInfo *self = ADW_DEMO_PAGE_INFO (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, self->icon_name);
    break;
  case PROP_TITLE:
    g_value_set_string (value, self->title);
    break;
  case PROP_GTYPE:
    g_value_set_gtype (value, self->gtype);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_page_info_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwDemoPageInfo *self = ADW_DEMO_PAGE_INFO (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    set_string (&self->icon_name, g_value_get_string (value));
    break;
  case PROP_TITLE:
    set_string (&self->title, g_value_get_string (value));
    break;
  case PROP_GTYPE:
    self->gtype = g_value_get_gtype (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_page_info_class_init (AdwDemoPageInfoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = adw_demo_page_info_finalize;
  object_class->get_property = adw_demo_page_info_get_property;
  object_class->set_property = adw_demo_page_info_set_property;

  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         _("Icon Name"),
                         _("Icon Name"),
                         NULL,
                         G_PARAM_READWRITE);

  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("Title"),
                         NULL,
                         G_PARAM_READWRITE);

  props[PROP_GTYPE] =
    g_param_spec_gtype ("gtype",
                         _("Type"),
                         _("Type"),
                         ADW_TYPE_DEMO_PAGE,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_demo_page_info_init (AdwDemoPageInfo *self)
{
}

AdwDemoPageInfo *
adw_demo_page_info_new (const char *title,
                        const char *icon_name,
                        GType       type)
{
  return g_object_new (ADW_TYPE_DEMO_PAGE_INFO,
                       "title", title,
                       "icon-name", icon_name,
                       "gtype", type,
                       NULL);
}
