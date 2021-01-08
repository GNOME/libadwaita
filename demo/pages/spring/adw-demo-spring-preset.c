#include "adw-demo-spring-preset.h"

#include <glib/gi18n.h>

struct _AdwDemoSpringPreset
{
  GObject parent_instance;

  gchar *name;
  gdouble damping;
  gdouble mass;
  gdouble stiffness;
  gdouble precision;
};

G_DEFINE_TYPE (AdwDemoSpringPreset, adw_demo_spring_preset, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_DAMPING,
  PROP_MASS,
  PROP_STIFFNESS,
  PROP_PRECISION,
  PROP_NAME,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static inline void
set_string (gchar       **dest,
            const gchar  *source)
{
  if (*dest)
    g_free (*dest);

  *dest = g_strdup (source);
}

static void
adw_demo_spring_preset_finalize (GObject *object)
{
  AdwDemoSpringPreset *self = ADW_DEMO_SPRING_PRESET (object);

  g_clear_pointer (&self->name, g_free);

  G_OBJECT_CLASS (adw_demo_spring_preset_parent_class)->finalize (object);
}

static void
adw_demo_spring_preset_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AdwDemoSpringPreset *self = ADW_DEMO_SPRING_PRESET (object);

  switch (prop_id) {
  case PROP_DAMPING:
    g_value_set_double (value, self->damping);
    break;
  case PROP_MASS:
    g_value_set_double (value, self->mass);
    break;
  case PROP_STIFFNESS:
    g_value_set_double (value, self->stiffness);
    break;
  case PROP_PRECISION:
    g_value_set_double (value, self->precision);
    break;
  case PROP_NAME:
    g_value_set_string (value, self->name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_spring_preset_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AdwDemoSpringPreset *self = ADW_DEMO_SPRING_PRESET (object);

  switch (prop_id) {
  case PROP_DAMPING:
    self->damping = g_value_get_double (value);
    break;
  case PROP_MASS:
    self->mass = g_value_get_double (value);
    break;
  case PROP_STIFFNESS:
    self->stiffness = g_value_get_double (value);
    break;
  case PROP_PRECISION:
    self->precision = g_value_get_double (value);
    break;
  case PROP_NAME:
    set_string (&self->name, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_spring_preset_class_init (AdwDemoSpringPresetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = adw_demo_spring_preset_finalize;
  object_class->get_property = adw_demo_spring_preset_get_property;
  object_class->set_property = adw_demo_spring_preset_set_property;

  props[PROP_DAMPING] =
    g_param_spec_double ("damping",
                         _("Damping"),
                         _("Damping"),
                         0, G_MAXDOUBLE, 0,
                         G_PARAM_READWRITE);

  props[PROP_MASS] =
    g_param_spec_double ("mass",
                         _("Mass"),
                         _("Mass"),
                         0, G_MAXDOUBLE, 0,
                         G_PARAM_READWRITE);

  props[PROP_STIFFNESS] =
    g_param_spec_double ("stiffness",
                         _("Stiffness"),
                         _("Stiffness"),
                         0, G_MAXDOUBLE, 0,
                         G_PARAM_READWRITE);

  props[PROP_PRECISION] =
    g_param_spec_double ("precision",
                         _("Precision"),
                         _("Precision"),
                         0, 1, 0,
                         G_PARAM_READWRITE);

  props[PROP_NAME] =
    g_param_spec_string ("name",
                         _("Name"),
                         _("Name"),
                         NULL,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_demo_spring_preset_init (AdwDemoSpringPreset *self)
{
}

AdwDemoSpringPreset *
adw_demo_spring_preset_new (gdouble      damping,
                            gdouble      mass,
                            gdouble      stiffness,
                            gdouble      precision,
                            const gchar *name)
{
  return g_object_new (ADW_TYPE_DEMO_SPRING_PRESET,
                       "damping", damping,
                       "mass", mass,
                       "stiffness", stiffness,
                       "precision", precision,
                       "name", name,
                       NULL);
}
