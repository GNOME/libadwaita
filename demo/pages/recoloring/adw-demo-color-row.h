#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_COLOR_ROW (adw_demo_color_row_get_type ())

G_DECLARE_FINAL_TYPE (AdwDemoColorRow, adw_demo_color_row, ADW, DEMO_COLOR_ROW, AdwActionRow)

AdwDemoColorRow *adw_demo_color_row_new (AdwColor  color_key);

G_END_DECLS
