/*
 * Copyright (C) 2023 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-layout-slot.h"

#include "adw-multi-layout-view-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwLayoutSlot:
 *
 * A child slot within [class@Layout].
 *
 * While it contains a layout child, the [property@Gtk.Widget:visible] property
 * of the slot is updated to match that of the layout child.
 *
 * See [class@MultiLayoutView].
 *
 * Since: 1.6
 */

struct _AdwLayoutSlot
{
  GtkWidget parent_instance;

  char *id;
};

G_DEFINE_FINAL_TYPE (AdwLayoutSlot, adw_layout_slot, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_ID,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
adw_layout_slot_root (GtkWidget *widget)
{
  AdwLayoutSlot *self = ADW_LAYOUT_SLOT (widget);
  GtkWidget *view;

  GTK_WIDGET_CLASS (adw_layout_slot_parent_class)->root (widget);

  view = gtk_widget_get_ancestor (widget, ADW_TYPE_MULTI_LAYOUT_VIEW);

  if (view)
    adw_multi_layout_view_register_slot (ADW_MULTI_LAYOUT_VIEW (view), self->id, widget);
}

static void
adw_layout_slot_constructed (GObject *object)
{
  AdwLayoutSlot *self = ADW_LAYOUT_SLOT (object);

  G_OBJECT_CLASS (adw_layout_slot_parent_class)->constructed (object);

  if (!self->id)
    g_error ("AdwLayoutSlot %p created without an ID", self);
}

static void
adw_layout_slot_finalize (GObject *object)
{
  AdwLayoutSlot *self = ADW_LAYOUT_SLOT (object);

  g_free (self->id);

  G_OBJECT_CLASS (adw_layout_slot_parent_class)->finalize (object);
}

static void
adw_layout_slot_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdwLayoutSlot *self = ADW_LAYOUT_SLOT (object);

  switch (prop_id) {
  case PROP_ID:
    g_value_set_string (value, adw_layout_slot_get_slot_id (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_layout_slot_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  AdwLayoutSlot *self = ADW_LAYOUT_SLOT (object);

  switch (prop_id) {
  case PROP_ID:
    g_set_str (&self->id, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_layout_slot_class_init (AdwLayoutSlotClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = adw_layout_slot_constructed;
  object_class->finalize = adw_layout_slot_finalize;
  object_class->get_property = adw_layout_slot_get_property;
  object_class->set_property = adw_layout_slot_set_property;

  widget_class->root = adw_layout_slot_root;
  widget_class->compute_expand = adw_widget_compute_expand;

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);

  /**
   * AdwLayoutSlot:id: (getter get_slot_id)
   *
   * The slot ID.
   *
   * See [method@MultiLayoutView.set_child].
   *
   * Since: 1.6
   */
  props[PROP_ID] =
    g_param_spec_string ("id", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_layout_slot_init (AdwLayoutSlot *self)
{
}

/**
 * adw_layout_slot_new:
 * @id: the slot ID
 *
 * Creates a new `AdwLayoutSlot` with its ID set to @id.
 *
 * Returns: a new `AdwLayoutSlot`
 *
 * Since: 1.6
 */
GtkWidget *
adw_layout_slot_new (const char *id)
{
  g_return_val_if_fail (id != NULL, NULL);

  return g_object_new (ADW_TYPE_LAYOUT_SLOT, "id", id, NULL);
}

/**
 * adw_layout_slot_get_slot_id: (get-property id)
 * @self: a layout slot
 *
 * Gets the slot id of @self.
 *
 * Returns: the slot ID
 *
 * Since: 1.6
 */
const char *
adw_layout_slot_get_slot_id (AdwLayoutSlot *self)
{
  g_return_val_if_fail (ADW_IS_LAYOUT_SLOT (self), NULL);

  return self->id;
}
