/*
 * Copyright (C) 2020 Purism SPC
 *
 * Based on gtkgizmoprivate.h
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/5d5625dec839c00fdb572af82fbbe872ea684859/gtk/gtkgizmoprivate.h
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_GIZMO (hdy_gizmo_get_type())

G_DECLARE_FINAL_TYPE (HdyGizmo, hdy_gizmo, HDY, GIZMO, GtkWidget)

typedef void     (* HdyGizmoMeasureFunc)  (HdyGizmo       *self,
                                           GtkOrientation  orientation,
                                           gint            for_size,
                                           gint           *minimum,
                                           gint           *natural,
                                           gint           *minimum_baseline,
                                           gint           *natural_baseline);
typedef void     (* HdyGizmoAllocateFunc) (HdyGizmo *self,
                                           gint      width,
                                           gint      height,
                                           gint      baseline);
typedef void     (* HdyGizmoSnapshotFunc) (HdyGizmo    *self,
                                           GtkSnapshot *snapshot);
typedef gboolean (* HdyGizmoContainsFunc) (HdyGizmo *self,
                                           gdouble   x,
                                           gdouble   y);
typedef gboolean (* HdyGizmoFocusFunc)    (HdyGizmo         *self,
                                           GtkDirectionType  direction);
typedef gboolean (* HdyGizmoGrabFocusFunc)(HdyGizmo         *self);

GtkWidget *hdy_gizmo_new (const char            *css_name,
                          HdyGizmoMeasureFunc    measure_func,
                          HdyGizmoAllocateFunc   allocate_func,
                          HdyGizmoSnapshotFunc   snapshot_func,
                          HdyGizmoContainsFunc   contains_func,
                          HdyGizmoFocusFunc      focus_func,
                          HdyGizmoGrabFocusFunc  grab_focus_func);

G_END_DECLS
