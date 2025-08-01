/*
 * Copyright (C) 2023 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-multi-layout-view-private.h"

#include "adw-layout-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwMultiLayoutView:
 *
 * A widget for switching between different layouts.
 *
 * `AdwMultiLayoutView` contains layouts and children. Each child has
 * an ID, each layout has slots inside it, each slot also has an ID. When
 * switching layouts, children are inserted into slots with matching IDs. The
 * [property@Gtk.Widget:visible] property of each slot is updated to match
 * that of the inserted child.
 *
 * This can be useful for rearranging children when it's difficult to do so
 * otherwise, for example to move a child from a sidebar to a bottom bar.
 *
 * The currently used layout can be switched using the
 * [property@MultiLayoutView:layout] or [property@MultiLayoutView:layout-name]
 * properties. For example, it can be done via a [class@Adw.Breakpoint] setter
 * to change layouts depending on the window size.
 *
 * ## AdwMultiLayoutView as GtkBuildable
 *
 * The `AdwMultiLayoutView` implementation of the [iface@Gtk.Buildable]
 * interface supports adding layouts via `<child>` element with the `type`
 * attribute omitted.
 *
 * It also supports setting children via `<child type="ID">`.
 *
 * Example of an `AdwMultiLayoutView` UI definition that can display a secondary
 * child as either a sidebar or a bottom sheet.
 *
 * ```xml
 * <object class="AdwMultiLayoutView">
 *   <child>
 *     <object class="AdwLayout">
 *       <property name="name">sidebar</property>
 *       <property name="content">
 *         <object class="AdwOverlaySplitView">
 *           <property name="sidebar">
 *             <object class="AdwLayoutSlot">
 *               <property name="id">secondary</property>
 *             </object>
 *           </property>
 *           <property name="content">
 *             <object class="AdwLayoutSlot">
 *               <property name="id">primary</property>
 *             </object>
 *           </property>
 *         </object>
 *       </property>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="AdwLayout">
 *       <property name="name">bottom-sheet</property>
 *       <property name="content">
 *         <object class="AdwBottomSheet">
 *           <property name="open">True</property>
 *           <property name="content">
 *             <object class="AdwLayoutSlot">
 *               <property name="id">primary</property>
 *             </object>
 *           </property>
 *           <property name="sheet">
 *             <object class="AdwLayoutSlot">
 *               <property name="id">secondary</property>
 *             </object>
 *           </property>
 *         </object>
 *       </property>
 *     </object>
 *   </child>
 *   <child type="primary">
 *     <!-- ... -->
 *   </child>
 *   <child type="secondary">
 *     <!-- ... -->
 *   </child>
 * </object>
 * ```
 *
 * ## CSS nodes
 *
 * `AdwMultiLayoutView` has a single CSS node with name `multi-layout-view`.
 *
 * Since: 1.6
 */

struct _AdwMultiLayoutView
{
  GtkWidget parent_instance;

  GList *layouts;
  GHashTable *children;
  GHashTable *child_visible_bindings;

  AdwLayout *current_layout;
  GtkWidget *content;
  GHashTable *slots;

  gboolean accepting_slots;
};

static void adw_multi_layout_view_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwMultiLayoutView, adw_multi_layout_view, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_multi_layout_view_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_LAYOUT,
  PROP_LAYOUT_NAME,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
parent_child (AdwMultiLayoutView *self,
              const char         *id)
{
  GtkWidget *slot = g_hash_table_lookup (self->slots, id);
  GtkWidget *child;
  GBinding *child_visible_binding;

  if (!slot)
    return;

  child = g_hash_table_lookup (self->children, id);

  if (gtk_widget_get_parent (child) == GTK_WIDGET (slot))
    return;

  child_visible_binding = g_object_bind_property (child, "visible",
                                                  slot, "visible",
                                                  G_BINDING_SYNC_CREATE);

  g_hash_table_insert (self->child_visible_bindings,
                       child,
                       g_object_ref (child_visible_binding));

  gtk_widget_set_parent (child, GTK_WIDGET (slot));
}

static void
parent_child_func (const char         *id,
                   GtkWidget          *child,
                   AdwMultiLayoutView *self)
{
  parent_child (self, id);
}

static void
unparent_child (const char         *id,
                GtkWidget          *child,
                AdwMultiLayoutView *self)
{
  if (!g_hash_table_contains (self->slots, id))
    return;

  g_hash_table_remove (self->child_visible_bindings, child);
  gtk_widget_unparent (child);
}

static void
binding_unbind_and_unref (gpointer data)
{
  GBinding *binding = data;

  g_binding_unbind (binding);
  g_object_unref (binding);
}

static void
destroy_current_layout (AdwMultiLayoutView *self)
{
  g_hash_table_foreach (self->children, (GHFunc) unparent_child, self);
  g_hash_table_remove_all (self->slots);
  g_clear_pointer (&self->content, gtk_widget_unparent);
}

static void
rebuild_current_layout (AdwMultiLayoutView *self)
{
  GtkWidget *focus = NULL;
  GtkRoot *root;

  if (!self->current_layout) {
    if (self->content)
      destroy_current_layout (self);

    return;
  }

  root = gtk_widget_get_root (GTK_WIDGET (self));

  if (root) {
    focus = gtk_root_get_focus (root);

    if (focus && gtk_widget_is_ancestor (focus, GTK_WIDGET (self)))
      g_object_add_weak_pointer (G_OBJECT (focus), (gpointer *) &focus);
    else
      focus = NULL;
  }

  if (self->content)
    destroy_current_layout (self);

  self->content = adw_layout_get_content (self->current_layout);

  if (self->content) {
    int old_size;

    self->accepting_slots = TRUE;
    gtk_widget_set_parent (self->content, GTK_WIDGET (self));

    do {
      old_size = g_hash_table_size (self->slots);

      g_hash_table_foreach (self->children, (GHFunc) parent_child_func, self);
    } while (g_hash_table_size (self->slots) > old_size);

    self->accepting_slots = FALSE;
  } else {
    g_critical ("Content in AdwLayout cannot be NULL");
  }

  if (focus) {
    gtk_widget_grab_focus (focus);
    g_clear_weak_pointer (&focus);
  }
}

static void
set_layout (AdwMultiLayoutView *self,
            AdwLayout          *layout)
{
  if (layout == self->current_layout)
    return;

  g_set_object (&self->current_layout, layout);

  if (gtk_widget_get_root (GTK_WIDGET (self)) != NULL)
    rebuild_current_layout (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LAYOUT]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LAYOUT_NAME]);
}

static void
adw_multi_layout_view_root (GtkWidget *widget)
{
  AdwMultiLayoutView *self = ADW_MULTI_LAYOUT_VIEW (widget);

  GTK_WIDGET_CLASS (adw_multi_layout_view_parent_class)->root (widget);

  rebuild_current_layout (self);
}

static void
adw_multi_layout_view_unroot (GtkWidget *widget)
{
  AdwMultiLayoutView *self = ADW_MULTI_LAYOUT_VIEW (widget);

  destroy_current_layout (self);

  GTK_WIDGET_CLASS (adw_multi_layout_view_parent_class)->unroot (widget);
}

static void
adw_multi_layout_view_dispose (GObject *object)
{
  AdwMultiLayoutView *self = ADW_MULTI_LAYOUT_VIEW (object);
  GList *l;

  g_clear_object (&self->current_layout);
  g_clear_pointer (&self->children, g_hash_table_unref);
  g_clear_pointer (&self->child_visible_bindings, g_hash_table_unref);

  for (l = self->layouts; l; l = l->next)
    g_object_unref (l->data);

  g_clear_pointer (&self->layouts, g_list_free);
  g_clear_pointer (&self->content, gtk_widget_unparent);
  g_clear_pointer (&self->slots, g_hash_table_unref);

  G_OBJECT_CLASS (adw_multi_layout_view_parent_class)->dispose (object);
}

static void
adw_multi_layout_view_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwMultiLayoutView *self = ADW_MULTI_LAYOUT_VIEW (object);

  switch (prop_id) {
  case PROP_LAYOUT:
    g_value_set_object (value, adw_multi_layout_view_get_layout (self));
    break;
  case PROP_LAYOUT_NAME:
    g_value_set_string (value, adw_multi_layout_view_get_layout_name (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_multi_layout_view_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwMultiLayoutView *self = ADW_MULTI_LAYOUT_VIEW (object);

  switch (prop_id) {
  case PROP_LAYOUT:
    adw_multi_layout_view_set_layout (self, g_value_get_object (value));
    break;
  case PROP_LAYOUT_NAME:
    adw_multi_layout_view_set_layout_name (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_multi_layout_view_class_init (AdwMultiLayoutViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_multi_layout_view_dispose;
  object_class->get_property = adw_multi_layout_view_get_property;
  object_class->set_property = adw_multi_layout_view_set_property;

  widget_class->root = adw_multi_layout_view_root;
  widget_class->unroot = adw_multi_layout_view_unroot;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwMultiLayoutView:layout:
   *
   * The currently used layout.
   *
   * Since: 1.6
   */
  props[PROP_LAYOUT] =
    g_param_spec_object ("layout", NULL, NULL,
                         ADW_TYPE_LAYOUT,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwMultiLayoutView:layout-name:
   *
   * The name of the currently used layout.
   *
   * See [property@Layout:name].
   *
   * Since: 1.6
   */
  props[PROP_LAYOUT_NAME] =
    g_param_spec_string ("layout-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "multi-layout-view");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_multi_layout_view_init (AdwMultiLayoutView *self)
{
  self->children = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  self->child_visible_bindings = g_hash_table_new_full (NULL, NULL, NULL, binding_unbind_and_unref);
  self->slots = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

static void
adw_multi_layout_view_buildable_add_child (GtkBuildable *buildable,
                                           GtkBuilder   *builder,
                                           GObject      *child,
                                           const char   *type)
{
  AdwMultiLayoutView *self = ADW_MULTI_LAYOUT_VIEW (buildable);

  if (ADW_IS_LAYOUT (child))
    adw_multi_layout_view_add_layout (self, g_object_ref (ADW_LAYOUT (child)));
  else if (type && GTK_IS_WIDGET (child))
    adw_multi_layout_view_set_child (self, type, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_multi_layout_view_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_multi_layout_view_buildable_add_child;
}

/**
 * adw_multi_layout_view_new:
 *
 * Creates a new `AdwMultiLayoutView`.
 *
 * Returns: the newly created `AdwMultiLayoutView`
 *
 * Since: 1.6
 */
GtkWidget *
adw_multi_layout_view_new (void)
{
  return g_object_new (ADW_TYPE_MULTI_LAYOUT_VIEW, NULL);
}

/**
 * adw_multi_layout_view_get_layout:
 * @self: a multi-layout view
 *
 * Gets the currently used layout of @self.
 *
 * Returns: (transfer none) (nullable): the current layout
 *
 * Since: 1.6
 */
AdwLayout *
adw_multi_layout_view_get_layout (AdwMultiLayoutView *self)
{
  g_return_val_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self), NULL);

  return self->current_layout;
}

/**
 * adw_multi_layout_view_set_layout:
 * @self: a multi-layout view
 * @layout: a layout in @self
 *
 * Makes @layout the current layout of @self.
 *
 * Since: 1.6
 */
void
adw_multi_layout_view_set_layout (AdwMultiLayoutView *self,
                                  AdwLayout          *layout)
{
  g_return_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self));
  g_return_if_fail (ADW_IS_LAYOUT (layout));

  set_layout (self, layout);
}

/**
 * adw_multi_layout_view_get_layout_name:
 * @self: a multi-layout view
 *
 * Returns the name of the currently used layout of @self.
 *
 * Returns: (transfer none) (nullable): the name of the current layout
 *
 * Since: 1.6
 */
const char *
adw_multi_layout_view_get_layout_name (AdwMultiLayoutView *self)
{
  g_return_val_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self), NULL);

  if (self->current_layout)
    return adw_layout_get_name (self->current_layout);

  return NULL;
}

/**
 * adw_multi_layout_view_set_layout_name:
 * @self: a multi-layout view
 * @name: the name of the layout
 *
 * Makes the layout with @name the current layout of @self.
 *
 * See [property@Layout:name].
 *
 * Since: 1.6
 */
void
adw_multi_layout_view_set_layout_name (AdwMultiLayoutView *self,
                                       const char         *name)
{
  AdwLayout *layout;

  g_return_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self));

  if (name == NULL)
    return;

  layout = adw_multi_layout_view_get_layout_by_name (self, name);

  if (layout == NULL) {
    g_critical ("Layout name '%s' not found in AdwMultiLayoutView", name);

    return;
  }

  adw_multi_layout_view_set_layout (self, layout);
}

/**
 * adw_multi_layout_view_add_layout:
 * @self: a multi-layout view
 * @layout: (transfer full): the layout to add
 *
 * Adds @layout to @self.
 *
 * Since: 1.6
 */
void
adw_multi_layout_view_add_layout (AdwMultiLayoutView *self,
                                  AdwLayout          *layout)
{
  const char *name;

  g_return_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self));
  g_return_if_fail (ADW_IS_LAYOUT (layout));

  name = adw_layout_get_name (layout);

  if (name && adw_multi_layout_view_get_layout_by_name (self, name)) {
    g_warning ("While adding layout: duplicate layout name in AdwMultiLayoutView: %s",
               name);
  }

  if (!self->layouts)
    adw_multi_layout_view_set_layout (self, layout);

  self->layouts = g_list_append (self->layouts, layout);

  adw_layout_set_view (layout, self);
}

/**
 * adw_multi_layout_view_remove_layout:
 * @self: a multi-layout view
 * @layout: the layout to add
 *
 * Removes @layout from @self.
 *
 * Since: 1.6
 */
void
adw_multi_layout_view_remove_layout (AdwMultiLayoutView *self,
                                     AdwLayout          *layout)
{
  g_return_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self));
  g_return_if_fail (ADW_IS_LAYOUT (layout));

  self->layouts = g_list_remove (self->layouts, layout);

  if (layout == self->current_layout) {
    if (self->layouts)
      set_layout (self, self->layouts->data);
    else
      set_layout (self, NULL);
  }

  g_object_unref (layout);
}

/**
 * adw_multi_layout_view_get_layout_by_name:
 * @self: a multi-layout view
 * @name: the name of the layout
 *
 * Gets layout with the name @name from @self, or `NULL` if it doesn't exist.
 *
 * See [property@Layout:name].
 *
 * Returns: (transfer none) (nullable): the layout with @name
 *
 * Since: 1.6
 */
AdwLayout *
adw_multi_layout_view_get_layout_by_name (AdwMultiLayoutView *self,
                                          const char         *name)
{
  GList *l;

  g_return_val_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  for (l = self->layouts; l; l = l->next) {
    AdwLayout *layout = l->data;

    if (g_strcmp0 (adw_layout_get_name (layout), name) == 0)
      return layout;
  }

  return NULL;
}

/**
 * adw_multi_layout_view_get_child:
 * @self: a multi-layout view
 * @id: the id of the child
 *
 * Gets the child for @id to @self.
 *
 * Returns: (transfer none) (nullable): the child for @id
 *
 * Since: 1.6
 */
GtkWidget *
adw_multi_layout_view_get_child (AdwMultiLayoutView *self,
                                 const char         *id)
{
  g_return_val_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self), NULL);
  g_return_val_if_fail (id != NULL, NULL);

  return g_hash_table_lookup (self->children, id);
}

/**
 * adw_multi_layout_view_set_child:
 * @self: a multi-layout view
 * @id: the id of the child
 * @child: the child to set
 *
 * Sets @child as the child for @id in @self.
 *
 * When changing layouts, it will be inserted into the slot with @id.
 *
 * Since: 1.6
 */
void
adw_multi_layout_view_set_child (AdwMultiLayoutView *self,
                                 const char         *id,
                                 GtkWidget          *child)
{
  GtkWidget *prev_child;

  g_return_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self));
  g_return_if_fail (id != NULL);
  g_return_if_fail (GTK_IS_WIDGET (child));

  prev_child = adw_multi_layout_view_get_child (self, id);

  if (prev_child == child)
    return;

  if (prev_child && self->current_layout)
    unparent_child (id, prev_child, self);

  g_hash_table_insert (self->children, g_strdup (id), g_object_ref_sink (child));

  if (self->current_layout)
    parent_child (self, id);
}

void
adw_multi_layout_view_register_slot (AdwMultiLayoutView *self,
                                     const char         *id,
                                     GtkWidget          *slot)
{
  g_return_if_fail (ADW_IS_MULTI_LAYOUT_VIEW (self));
  g_return_if_fail (id != NULL);
  g_return_if_fail (GTK_IS_WIDGET (slot));

  if (!self->accepting_slots)
    return;

  if (g_hash_table_contains (self->slots, id)) {
    g_warning ("Duplicate slot ID in AdwLayout %p: %s", self->current_layout, id);
    return;
  }

  g_hash_table_insert (self->slots, g_strdup (id), slot);
}
