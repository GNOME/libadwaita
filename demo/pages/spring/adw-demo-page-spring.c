#include "adw-demo-page-spring.h"

#include <glib/gi18n.h>

#include "adw-demo-adjustment-row.h"
#include "adw-demo-spring-basic.h"
#include "adw-demo-spring-interactive.h"
#include "adw-demo-spring-preset.h"
#include "adw-spring-animation-private.h"

struct _AdwDemoPageSpring
{
  AdwDemoPage parent_instance;

  AdwDemoSpringBasic *basic_view;
  AdwDemoSpringInteractive *interactive_view;

  GListStore *presets;
  AdwComboRow *presets_row;

  gdouble damping;
  gdouble mass;
  gdouble stiffness;
  gdouble precision;
};

G_DEFINE_TYPE (AdwDemoPageSpring, adw_demo_page_spring, ADW_TYPE_DEMO_PAGE)

enum {
  PROP_0,
  PROP_DAMPING,
  PROP_MASS,
  PROP_STIFFNESS,
  PROP_PRECISION,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
reset (AdwDemoPageSpring *self)
{
  adw_demo_spring_basic_reset (self->basic_view);
  adw_demo_spring_interactive_reset (self->interactive_view);
}

static void
apply_preset (AdwDemoPageSpring   *self,
              AdwDemoSpringPreset *preset)
{
  gdouble damping, mass, stiffness, precision;

  g_object_get (preset,
                "damping", &damping,
                "mass", &mass,
                "stiffness", &stiffness,
                "precision", &precision,
                NULL);

  g_object_set (self,
                "damping", damping,
                "mass", mass,
                "stiffness", stiffness,
                "precision", precision,
                NULL);
}

static void
preset_cb (AdwDemoPageSpring *self)
{
  apply_preset (self, adw_combo_row_get_selected_item (self->presets_row));
}

static void
adw_demo_page_spring_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  AdwDemoPageSpring *self = ADW_DEMO_PAGE_SPRING (object);

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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_page_spring_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  AdwDemoPageSpring *self = ADW_DEMO_PAGE_SPRING (object);

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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_page_spring_class_init (AdwDemoPageSpringClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_demo_page_spring_get_property;
  object_class->set_property = adw_demo_page_spring_set_property;

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

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/pages/spring/adw-demo-page-spring.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageSpring, basic_view);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageSpring, interactive_view);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageSpring, presets);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageSpring, presets_row);
  gtk_widget_class_bind_template_callback (widget_class, reset);
  gtk_widget_class_bind_template_callback (widget_class, preset_cb);
}

static void
adw_demo_page_spring_init (AdwDemoPageSpring *self)
{
  g_type_ensure (ADW_TYPE_DEMO_ADJUSTMENT_ROW);
  g_type_ensure (ADW_TYPE_DEMO_SPRING_BASIC);
  g_type_ensure (ADW_TYPE_DEMO_SPRING_INTERACTIVE);
  g_type_ensure (ADW_TYPE_DEMO_SPRING_PRESET);

  gtk_widget_init_template (GTK_WIDGET (self));

  g_list_store_append (self->presets, adw_demo_spring_preset_new (10,  1, 100, 0.001, _("Default (Core Animation)")));
  g_list_store_append (self->presets, adw_demo_spring_preset_new (26,  1, 170, 0.001, _("Default (react-spring)")));
  g_list_store_append (self->presets, adw_demo_spring_preset_new (14,  1, 120, 0.001, _("Gentle")));
  g_list_store_append (self->presets, adw_demo_spring_preset_new (12,  1, 180, 0.001, _("Wobbly")));
  g_list_store_append (self->presets, adw_demo_spring_preset_new (20,  1, 210, 0.001, _("Stiff")));
  g_list_store_append (self->presets, adw_demo_spring_preset_new (60,  1, 280, 0.001, _("Slow")));
  g_list_store_append (self->presets, adw_demo_spring_preset_new (120, 1, 280, 0.001, _("Molasses")));

  preset_cb (self);
}
