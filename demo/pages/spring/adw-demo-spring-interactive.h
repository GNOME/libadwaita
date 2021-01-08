#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_SPRING_INTERACTIVE (adw_demo_spring_interactive_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoSpringInteractive, adw_demo_spring_interactive, ADW, DEMO_SPRING_INTERACTIVE, AdwBin)

void adw_demo_spring_interactive_reset (AdwDemoSpringInteractive *self);

G_END_DECLS
