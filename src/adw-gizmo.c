/*
 * Copyright (C) 2020 Purism SPC
 *
 * Based on gtkgizmo.c
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/5d5625dec839c00fdb572af82fbbe872ea684859/gtk/gtkgizmo.c
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "adw-gizmo-private.h"

#include "adw-macros-private.h"
#include "adw-widget-utils-private.h"

struct _AdwGizmo
{
  GtkWidget parent_instance;

  AdwGizmoMeasureFunc   measure_func;
  AdwGizmoAllocateFunc  allocate_func;
  AdwGizmoSnapshotFunc  snapshot_func;
  AdwGizmoContainsFunc  contains_func;
  AdwGizmoFocusFunc     focus_func;
  AdwGizmoGrabFocusFunc grab_focus_func;
};

G_DEFINE_FINAL_TYPE (AdwGizmo, adw_gizmo, GTK_TYPE_WIDGET)

static void
adw_gizmo_measure (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  AdwGizmo *self = ADW_GIZMO (widget);

  if (self->measure_func)
    self->measure_func (self, orientation, for_size,
                        minimum, natural,
                        minimum_baseline, natural_baseline);
}

static void
adw_gizmo_size_allocate (GtkWidget *widget,
                         int        width,
                         int        height,
                         int        baseline)
{
  AdwGizmo *self = ADW_GIZMO (widget);

  if (self->allocate_func)
    self->allocate_func (self, width, height, baseline);
}

static void
adw_gizmo_snapshot (GtkWidget   *widget,
                    GtkSnapshot *snapshot)
{
  AdwGizmo *self = ADW_GIZMO (widget);

  if (self->snapshot_func)
    self->snapshot_func (self, snapshot);
  else
    GTK_WIDGET_CLASS (adw_gizmo_parent_class)->snapshot (widget, snapshot);
}

static gboolean
adw_gizmo_contains (GtkWidget *widget,
                    double     x,
                    double     y)
{
  AdwGizmo *self = ADW_GIZMO (widget);

  if (self->contains_func)
    return self->contains_func (self, x, y);
  else
    return GTK_WIDGET_CLASS (adw_gizmo_parent_class)->contains (widget, x, y);
}

static gboolean
adw_gizmo_focus (GtkWidget        *widget,
                 GtkDirectionType  direction)
{
  AdwGizmo *self = ADW_GIZMO (widget);

  if (self->focus_func)
    return self->focus_func (self, direction);

  return FALSE;
}

static gboolean
adw_gizmo_grab_focus (GtkWidget *widget)
{
  AdwGizmo *self = ADW_GIZMO (widget);

  if (self->grab_focus_func)
    return self->grab_focus_func (self);

  return FALSE;
}

static void
adw_gizmo_dispose (GObject *object)
{
  AdwGizmo *self = ADW_GIZMO (object);
  GtkWidget *widget = gtk_widget_get_first_child (GTK_WIDGET (self));

  while (widget) {
    GtkWidget *next = gtk_widget_get_next_sibling (widget);

    gtk_widget_unparent (widget);

    widget = next;
  }

  G_OBJECT_CLASS (adw_gizmo_parent_class)->dispose (object);
}

static void
adw_gizmo_class_init (AdwGizmoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_gizmo_dispose;

  widget_class->measure = adw_gizmo_measure;
  widget_class->size_allocate = adw_gizmo_size_allocate;
  widget_class->snapshot = adw_gizmo_snapshot;
  widget_class->contains = adw_gizmo_contains;
  widget_class->grab_focus = adw_gizmo_grab_focus;
  widget_class->focus = adw_gizmo_focus;
  widget_class->compute_expand = adw_widget_compute_expand;
}

static void
adw_gizmo_init (AdwGizmo *self)
{
}

GtkWidget *
adw_gizmo_new (const char            *css_name,
               AdwGizmoMeasureFunc    measure_func,
               AdwGizmoAllocateFunc   allocate_func,
               AdwGizmoSnapshotFunc   snapshot_func,
               AdwGizmoContainsFunc   contains_func,
               AdwGizmoFocusFunc      focus_func,
               AdwGizmoGrabFocusFunc  grab_focus_func)
{
  AdwGizmo *gizmo = g_object_new (ADW_TYPE_GIZMO,
                                  "css-name", css_name,
                                  NULL);

  gizmo->measure_func  = measure_func;
  gizmo->allocate_func = allocate_func;
  gizmo->snapshot_func = snapshot_func;
  gizmo->contains_func = contains_func;
  gizmo->focus_func = focus_func;
  gizmo->grab_focus_func = grab_focus_func;

  return GTK_WIDGET (gizmo);
}

GtkWidget *
adw_gizmo_new_with_role (const char            *css_name,
                         GtkAccessibleRole      role,
                         AdwGizmoMeasureFunc    measure_func,
                         AdwGizmoAllocateFunc   allocate_func,
                         AdwGizmoSnapshotFunc   snapshot_func,
                         AdwGizmoContainsFunc   contains_func,
                         AdwGizmoFocusFunc      focus_func,
                         AdwGizmoGrabFocusFunc  grab_focus_func)
{
  AdwGizmo *gizmo = ADW_GIZMO (g_object_new (ADW_TYPE_GIZMO,
                                             "css-name", css_name,
                                             "accessible-role", role,
                                             NULL));

  gizmo->measure_func  = measure_func;
  gizmo->allocate_func = allocate_func;
  gizmo->snapshot_func = snapshot_func;
  gizmo->contains_func = contains_func;
  gizmo->focus_func = focus_func;
  gizmo->grab_focus_func = grab_focus_func;

  return GTK_WIDGET (gizmo);
}
