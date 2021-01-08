#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_SPRING_PRESET (adw_demo_spring_preset_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoSpringPreset, adw_demo_spring_preset, ADW, DEMO_SPRING_PRESET, GObject)

AdwDemoSpringPreset *adw_demo_spring_preset_new (gdouble      damping,
                                                 gdouble      mass,
                                                 gdouble      stiffness,
                                                 gdouble      precision,
                                                 const gchar *name);

G_END_DECLS
