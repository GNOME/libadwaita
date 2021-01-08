#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_TRANSFORM_LAYOUT (adw_demo_transform_layout_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoTransformLayout, adw_demo_transform_layout, ADW, DEMO_TRANSFORM_LAYOUT, GtkLayoutManager)

GskTransform *adw_demo_transform_layout_get_transform (AdwDemoTransformLayout *self);
void          adw_demo_transform_layout_set_transform (AdwDemoTransformLayout *self,
                                                       GskTransform           *transform);

void          adw_demo_transform_layout_take_transform (AdwDemoTransformLayout *self,
                                                        GskTransform           *transform);

G_END_DECLS
