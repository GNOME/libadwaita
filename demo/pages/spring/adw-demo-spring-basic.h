#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_SPRING_BASIC (adw_demo_spring_basic_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoSpringBasic, adw_demo_spring_basic, ADW, DEMO_SPRING_BASIC, AdwBin)

void adw_demo_spring_basic_reset (AdwDemoSpringBasic *self);

G_END_DECLS
