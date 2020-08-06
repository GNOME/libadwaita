/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "hdy-combo-row.h"

#include <glib/gi18n-lib.h>

/**
 * SECTION:hdy-combo-row
 * @short_description: A #GtkListBox row used to choose from a list of items.
 * @Title: HdyComboRow
 *
 * The #HdyComboRow widget allows the user to choose from a list of valid
 * choices. The row displays the selected choice. When activated, the row
 * displays a popover which allows the user to make a new choice.
 *
 * The #HdyComboRow uses the model-view pattern; the list of valid choices
 * is specified in the form of a #GListModel, and the display of the choices can
 * be adapted to the data in the model via widget creation functions.
 *
 * #HdyComboRow is #GtkListBoxRow:activatable if a model is set.
 *
 * # CSS nodes
 *
 * #HdyComboRow has a main CSS node with name row.
 *
 * Its popover has the node name popover with the .combo style class, it
 * contains a #GtkScrolledWindow, which in turn contains a #GtkListBox, both are
 * accessible via their regular nodes.
 *
 * A checkmark of node and style class image.checkmark in the popover denotes
 * the current item.
 *
 * Since: 0.0.6
 */

/*
 * This was mostly inspired by code from the display panel from GNOME Settings.
 */

typedef struct
{
  HdyComboRowGetNameFunc func;
  gpointer func_data;
  GDestroyNotify func_data_destroy;
} HdyComboRowGetName;

typedef struct
{
  GtkBox *current;
  GtkImage *image;
  GtkListBox *list;
  GtkPopover *popover;
  gint selected_index;
  gboolean use_subtitle;
  HdyComboRowGetName *get_name;

  GListModel *bound_model;
  GtkListBoxCreateWidgetFunc create_list_widget_func;
  GtkListBoxCreateWidgetFunc create_current_widget_func;
  gpointer create_widget_func_data;
  GDestroyNotify create_widget_func_data_free_func;
  /* This is owned by create_widget_func_data, which is ultimately owned by the
   * list box, and hence should not be destroyed manually.
   */
  HdyComboRowGetName *get_name_internal;
} HdyComboRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyComboRow, hdy_combo_row, HDY_TYPE_ACTION_ROW)

enum {
  PROP_0,
  PROP_SELECTED_INDEX,
  PROP_USE_SUBTITLE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static GtkWidget *
create_list_label (gpointer item,
                   gpointer user_data)
{
  HdyComboRowGetName *get_name = (HdyComboRowGetName *) user_data;
  g_autofree gchar *name = get_name->func (item, get_name->func_data);

  return g_object_new (GTK_TYPE_LABEL,
                       "ellipsize", PANGO_ELLIPSIZE_END,
                       "label", name,
                       "max-width-chars", 20,
                       "valign", GTK_ALIGN_CENTER,
                       "visible", TRUE,
                       "xalign", 0.0,
                        NULL);
}

static GtkWidget *
create_current_label (gpointer item,
                      gpointer user_data)
{
  HdyComboRowGetName *get_name = (HdyComboRowGetName *) user_data;
  g_autofree gchar *name = NULL;

  if (get_name->func)
    name = get_name->func (item, get_name->func_data);

  return g_object_new (GTK_TYPE_LABEL,
                       "ellipsize", PANGO_ELLIPSIZE_END,
                       "halign", GTK_ALIGN_END,
                       "label", name,
                       "valign", GTK_ALIGN_CENTER,
                       "visible", TRUE,
                       "xalign", 0.0,
                        NULL);
}

static void
create_list_widget_data_free (gpointer user_data)
{
  HdyComboRow *self = HDY_COMBO_ROW (user_data);
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  if (priv->create_widget_func_data_free_func)
    priv->create_widget_func_data_free_func (priv->create_widget_func_data);
}

static GtkWidget *
create_list_widget (gpointer item,
                    gpointer user_data)
{
  HdyComboRow *self = HDY_COMBO_ROW (user_data);
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);
  GtkWidget *checkmark = g_object_new (GTK_TYPE_IMAGE,
                                       "halign", GTK_ALIGN_START,
                                       "icon-name", "emblem-ok-symbolic",
                                       "valign", GTK_ALIGN_CENTER,
                                       NULL);
  GtkWidget *box = g_object_new (GTK_TYPE_BOX,
                                 "child", priv->create_list_widget_func (item, priv->create_widget_func_data),
                                 "child", checkmark,
                                 "halign", GTK_ALIGN_START,
                                 "spacing", 6,
                                 "valign", GTK_ALIGN_CENTER,
                                 "visible", TRUE,
                                 NULL);
  GtkStyleContext *checkmark_context = gtk_widget_get_style_context (checkmark);

  gtk_style_context_add_class (checkmark_context, "checkmark");

  g_object_set_data (G_OBJECT (box), "checkmark", checkmark);

  return box;
}

static void
get_name_free (HdyComboRowGetName *get_name)
{
  if (get_name == NULL)
    return;

  if (get_name->func_data_destroy)
    get_name->func_data_destroy (get_name->func_data);
  get_name->func = NULL;
  get_name->func_data = NULL;
  get_name->func_data_destroy = NULL;

  g_free (get_name);
}

static void
update (HdyComboRow *self)
{
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);
  g_autoptr(GObject) item = NULL;
  g_autofree gchar *name = NULL;
  GtkWidget *widget;
  guint n_items = priv->bound_model ? g_list_model_get_n_items (priv->bound_model) : 0;

  gtk_widget_set_visible (GTK_WIDGET (priv->current), !priv->use_subtitle);
  gtk_container_foreach (GTK_CONTAINER (priv->current), (GtkCallback) gtk_widget_destroy, NULL);

  gtk_widget_set_sensitive (GTK_WIDGET (self), n_items > 0);
  gtk_widget_set_visible (GTK_WIDGET (priv->image), n_items > 1);
  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (self), n_items > 1);

  if (n_items == 0) {
    g_assert (priv->selected_index == -1);

    return;
  }

  g_assert (priv->selected_index >= 0 && priv->selected_index <= n_items);

  {
    g_autoptr (GList) rows = gtk_container_get_children (GTK_CONTAINER (priv->list));
    GList *l;
    int i = 0;

    for (l = rows; l; l = l->next) {
      GtkWidget *row = GTK_WIDGET (l->data);
      GtkWidget *box = gtk_bin_get_child (GTK_BIN (row));

      gtk_widget_set_visible (GTK_WIDGET (g_object_get_data (G_OBJECT (box), "checkmark")),
                              priv->selected_index == i++);
    }
  }

  item = g_list_model_get_item (priv->bound_model, priv->selected_index);
  if (priv->use_subtitle) {
    if (priv->get_name != NULL && priv->get_name->func)
      name = priv->get_name->func (item, priv->get_name->func_data);
    else if (priv->get_name_internal != NULL && priv->get_name_internal->func)
      name = priv->get_name_internal->func (item, priv->get_name_internal->func_data);
    hdy_action_row_set_subtitle (HDY_ACTION_ROW (self), name);
  }
  else {
    widget = priv->create_current_widget_func (item, priv->create_widget_func_data);
    gtk_container_add (GTK_CONTAINER (priv->current), widget);
  }
}

static void
bound_model_changed (GListModel *list,
                     guint       index,
                     guint       removed,
                     guint       added,
                     gpointer    user_data)
{
  gint new_idx;
  HdyComboRow *self = HDY_COMBO_ROW (user_data);
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  /* Selection is in front of insertion/removal point, nothing to do */
  if (priv->selected_index > 0 && priv->selected_index < index)
    return;

  if (priv->selected_index < index + removed) {
    /* The item selected item was removed (or none is selected) */
    new_idx = -1;
  } else {
    /* The item selected item was behind the insertion/removal */
    new_idx = priv->selected_index + added - removed;
  }

  /* Select the first item if none is selected. */
  if (new_idx == -1 && g_list_model_get_n_items (list) > 0)
    new_idx = 0;

  hdy_combo_row_set_selected_index (self, new_idx);
}

static void
row_activated_cb (HdyComboRow   *self,
                  GtkListBoxRow *row)
{
  hdy_combo_row_set_selected_index (self, gtk_list_box_row_get_index (row));
}

static void
hdy_combo_row_activate (HdyActionRow *row)
{
  HdyComboRow *self = HDY_COMBO_ROW (row);
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  if (gtk_widget_get_visible (GTK_WIDGET (priv->image)))
    gtk_popover_popup (priv->popover);
}

static void
destroy_model (HdyComboRow *self)
{
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  if (!priv->bound_model)
    return;

  /* Disconnect the bound model *before* releasing it. */
  g_signal_handlers_disconnect_by_func (priv->bound_model, bound_model_changed, self);

  /* Destroy the model and the user data. */
  if (priv->list)
    gtk_list_box_bind_model (priv->list, NULL, NULL, NULL, NULL);

  priv->bound_model = NULL;
  priv->create_list_widget_func = NULL;
  priv->create_current_widget_func = NULL;
  priv->create_widget_func_data = NULL;
  priv->create_widget_func_data_free_func = NULL;
}

static void
hdy_combo_row_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  HdyComboRow *self = HDY_COMBO_ROW (object);

  switch (prop_id) {
  case PROP_SELECTED_INDEX:
    g_value_set_int (value, hdy_combo_row_get_selected_index (self));
    break;
  case PROP_USE_SUBTITLE:
    g_value_set_boolean (value, hdy_combo_row_get_use_subtitle (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_combo_row_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  HdyComboRow *self = HDY_COMBO_ROW (object);

  switch (prop_id) {
  case PROP_SELECTED_INDEX:
    hdy_combo_row_set_selected_index (self, g_value_get_int (value));
    break;
  case PROP_USE_SUBTITLE:
    hdy_combo_row_set_use_subtitle (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_combo_row_dispose (GObject *object)
{
  HdyComboRow *self = HDY_COMBO_ROW (object);
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  destroy_model (self);
  g_clear_pointer (&priv->get_name, get_name_free);

  G_OBJECT_CLASS (hdy_combo_row_parent_class)->dispose (object);
}

typedef struct {
  HdyComboRow *row;
  GtkCallback callback;
  gpointer callback_data;
} ForallData;

static void
for_non_internal_child (GtkWidget *widget,
                        gpointer   callback_data)
{
  ForallData *data = callback_data;
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (data->row);

  if (widget != (GtkWidget *) priv->current &&
      widget != (GtkWidget *) priv->image)
    data->callback (widget, data->callback_data);
}

static void
hdy_combo_row_forall (GtkContainer *container,
                      gboolean      include_internals,
                      GtkCallback   callback,
                      gpointer      callback_data)
{
  HdyComboRow *self = HDY_COMBO_ROW (container);
  ForallData data;

  if (include_internals) {
    GTK_CONTAINER_CLASS (hdy_combo_row_parent_class)->forall (GTK_CONTAINER (self), include_internals, callback, callback_data);

    return;
  }

  data.row = self;
  data.callback = callback;
  data.callback_data = callback_data;

  GTK_CONTAINER_CLASS (hdy_combo_row_parent_class)->forall (GTK_CONTAINER (self), include_internals, for_non_internal_child, &data);
}

static void
hdy_combo_row_class_init (HdyComboRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  HdyActionRowClass *row_class = HDY_ACTION_ROW_CLASS (klass);

  object_class->get_property = hdy_combo_row_get_property;
  object_class->set_property = hdy_combo_row_set_property;
  object_class->dispose = hdy_combo_row_dispose;

  container_class->forall = hdy_combo_row_forall;

  row_class->activate = hdy_combo_row_activate;

  /**
   * HdyComboRow:selected-index:
   *
   * The index of the selected item in its #GListModel.
   *
   * Since: 0.0.7
   */
  props[PROP_SELECTED_INDEX] =
      g_param_spec_int ("selected-index",
                        _("Selected index"),
                        _("The index of the selected item"),
                        -1, G_MAXINT, -1,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyComboRow:use-subtitle:
   *
   * %TRUE to set the current value as the subtitle.
   *
   * If you use a custom widget creation function, you will need to give the row
   * a name conversion closure with hdy_combo_row_set_get_name_func().
   *
   * If %TRUE, you should not access HdyActionRow:subtitle.
   *
   * Since: 0.0.10
   */
  props[PROP_USE_SUBTITLE] =
    g_param_spec_boolean ("use-subtitle",
                          _("Use subtitle"),
                          _("Set the current value as the subtitle"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-combo-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyComboRow, current);
  gtk_widget_class_bind_template_child_private (widget_class, HdyComboRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, HdyComboRow, list);
  gtk_widget_class_bind_template_child_private (widget_class, HdyComboRow, popover);
}

static void
hdy_combo_row_init (HdyComboRow *self)
{
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  priv->selected_index = -1;

  g_signal_connect_object (priv->list, "row-activated", G_CALLBACK (gtk_widget_hide),
                           priv->popover, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->list, "row-activated", G_CALLBACK (row_activated_cb),
                           self, G_CONNECT_SWAPPED);

  update (self);
}

/**
 * hdy_combo_row_new:
 *
 * Creates a new #HdyComboRow.
 *
 * Returns: a new #HdyComboRow
 *
 * Since: 0.0.6
 */
GtkWidget *
hdy_combo_row_new (void)
{
  return g_object_new (HDY_TYPE_COMBO_ROW, NULL);
}

/**
 * hdy_combo_row_get_model:
 * @self: a #HdyComboRow
 *
 * Gets the model bound to @self, or %NULL if none is bound.
 *
 * Returns: (transfer none) (nullable): the #GListModel bound to @self or %NULL
 *
 * Since: 0.0.6
 */
GListModel *
hdy_combo_row_get_model (HdyComboRow *self)
{
  HdyComboRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_COMBO_ROW (self), NULL);

  priv = hdy_combo_row_get_instance_private (self);

  return priv->bound_model;
}

/**
 * hdy_combo_row_bind_model:
 * @self: a #HdyComboRow
 * @model: (nullable): the #GListModel to be bound to @self
 * @create_list_widget_func: (nullable) (scope call): a function that creates
 *   widgets for items to display in the list, or %NULL in case you also passed
 *   %NULL as @model
 * @create_current_widget_func: (nullable) (scope call): a function that creates
 *   widgets for items to display as the selected item, or %NULL in case you
 *   also passed %NULL as @model
 * @user_data: user data passed to @create_list_widget_func and
 *   @create_current_widget_func
 * @user_data_free_func: function for freeing @user_data
 *
 * Binds @model to @self.
 *
 * If @self was already bound to a model, that previous binding is destroyed.
 *
 * The contents of @self are cleared and then filled with widgets that represent
 * items from @model. @self is updated whenever @model changes. If @model is
 * %NULL, @self is left empty.
 *
 * Since: 0.0.6
 */
void
hdy_combo_row_bind_model (HdyComboRow                *self,
                          GListModel                 *model,
                          GtkListBoxCreateWidgetFunc  create_list_widget_func,
                          GtkListBoxCreateWidgetFunc  create_current_widget_func,
                          gpointer                    user_data,
                          GDestroyNotify              user_data_free_func)
{
  HdyComboRowPrivate *priv;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));
  g_return_if_fail (model == NULL || G_IS_LIST_MODEL (model));
  g_return_if_fail (model == NULL || create_list_widget_func != NULL);
  g_return_if_fail (model == NULL || create_current_widget_func != NULL);

  priv = hdy_combo_row_get_instance_private (self);

  destroy_model (self);

  gtk_container_foreach (GTK_CONTAINER (priv->current), (GtkCallback) gtk_widget_destroy, NULL);
  priv->selected_index = -1;

  if (model == NULL) {
    update (self);

    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_INDEX]);
    return;
  }

  /* We don't need take a reference as the list box holds one for us. */
  priv->bound_model = model;
  priv->create_list_widget_func = create_list_widget_func;
  priv->create_current_widget_func = create_current_widget_func;
  priv->create_widget_func_data = user_data;
  priv->create_widget_func_data_free_func = user_data_free_func;

  g_signal_connect (priv->bound_model, "items-changed", G_CALLBACK (bound_model_changed), self);

  if (g_list_model_get_n_items (priv->bound_model) > 0)
    priv->selected_index = 0;

  gtk_list_box_bind_model (priv->list, model, create_list_widget, self, create_list_widget_data_free);

  update (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_INDEX]);
}

/**
 * hdy_combo_row_bind_name_model:
 * @self: a #HdyComboRow
 * @model: (nullable): the #GListModel to be bound to @self
 * @get_name_func: (nullable): a function that creates names for items, or %NULL
 *   in case you also passed %NULL as @model
 * @user_data: user data passed to @get_name_func
 * @user_data_free_func: function for freeing @user_data
 *
 * Binds @model to @self.
 *
 * If @self was already bound to a model, that previous binding is destroyed.
 *
 * The contents of @self are cleared and then filled with widgets that represent
 * items from @model. @self is updated whenever @model changes. If @model is
 * %NULL, @self is left empty.
 *
 * This is more convenient to use than hdy_combo_row_bind_model() if you want to
 * represent items of the model with names.
 *
 * Since: 0.0.6
 */
void
hdy_combo_row_bind_name_model (HdyComboRow            *self,
                               GListModel             *model,
                               HdyComboRowGetNameFunc  get_name_func,
                               gpointer                user_data,
                               GDestroyNotify          user_data_free_func)
{
  HdyComboRowPrivate *priv = hdy_combo_row_get_instance_private (self);

  g_return_if_fail (HDY_IS_COMBO_ROW (self));
  g_return_if_fail (model == NULL || G_IS_LIST_MODEL (model));
  g_return_if_fail (model == NULL || get_name_func != NULL);

  priv->get_name_internal = g_new0 (HdyComboRowGetName, 1);
  priv->get_name_internal->func = get_name_func;
  priv->get_name_internal->func_data = user_data;
  priv->get_name_internal->func_data_destroy = user_data_free_func;

  hdy_combo_row_bind_model (self, model, create_list_label, create_current_label, priv->get_name_internal, (GDestroyNotify) get_name_free);
}

/**
 * hdy_combo_row_set_for_enum:
 * @self: a #HdyComboRow
 * @enum_type: the enumeration #GType to be bound to @self
 * @get_name_func: (nullable): a function that creates names for items, or %NULL
 *   in case you also passed %NULL as @model
 * @user_data: user data passed to @get_name_func
 * @user_data_free_func: function for freeing @user_data
 *
 * Creates a model for @enum_type and binds it to @self. The items of the model
 * will be #HdyEnumValueObject objects.
 *
 * If @self was already bound to a model, that previous binding is destroyed.
 *
 * The contents of @self are cleared and then filled with widgets that represent
 * items from @model. @self is updated whenever @model changes. If @model is
 * %NULL, @self is left empty.
 *
 * This is more convenient to use than hdy_combo_row_bind_name_model() if you
 * want to represent values of an enumeration with names.
 *
 * See hdy_enum_value_row_name().
 *
 * Since: 0.0.6
 */
void
hdy_combo_row_set_for_enum (HdyComboRow                     *self,
                            GType                            enum_type,
                            HdyComboRowGetEnumValueNameFunc  get_name_func,
                            gpointer                         user_data,
                            GDestroyNotify                   user_data_free_func)
{
  g_autoptr (GListStore) store = g_list_store_new (HDY_TYPE_ENUM_VALUE_OBJECT);
  /* g_autoptr for GEnumClass would require glib > 2.56 */
  GEnumClass *enum_class = NULL;
  gsize i;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));

  enum_class = g_type_class_ref (enum_type);
  for (i = 0; i < enum_class->n_values; i++)
    {
      g_autoptr(HdyEnumValueObject) obj = hdy_enum_value_object_new (&enum_class->values[i]);

      g_list_store_append (store, obj);
    }

  hdy_combo_row_bind_name_model (self, G_LIST_MODEL (store), (HdyComboRowGetNameFunc) get_name_func, user_data, user_data_free_func);
  g_type_class_unref (enum_class);
}

/**
 * hdy_combo_row_get_selected_index:
 * @self: a #GtkListBoxRow
 *
 * Gets the index of the selected item in its #GListModel.
 *
 * Returns: the index of the selected item, or -1 if no item is selected
 *
 * Since: 0.0.7
 */
gint
hdy_combo_row_get_selected_index (HdyComboRow *self)
{
  HdyComboRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_COMBO_ROW (self), -1);

  priv = hdy_combo_row_get_instance_private (self);

  return priv->selected_index;
}

/**
 * hdy_combo_row_set_selected_index:
 * @self: a #HdyComboRow
 * @selected_index: the index of the selected item
 *
 * Sets the index of the selected item in its #GListModel.
 *
 * Since: 0.0.7
 */
void
hdy_combo_row_set_selected_index (HdyComboRow *self,
                                  gint         selected_index)
{
  HdyComboRowPrivate *priv;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));
  g_return_if_fail (selected_index >= -1);

  priv = hdy_combo_row_get_instance_private (self);

  g_return_if_fail (selected_index >= 0 || priv->bound_model == NULL || g_list_model_get_n_items (priv->bound_model) == 0);
  g_return_if_fail (selected_index == -1 || (priv->bound_model != NULL && selected_index < g_list_model_get_n_items (priv->bound_model)));

  if (priv->selected_index == selected_index)
    return;

  priv->selected_index = selected_index;
  update (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_INDEX]);
}

/**
 * hdy_combo_row_get_use_subtitle:
 * @self: a #GtkListBoxRow
 *
 * Gets whether the current value of @self should be displayed as its subtitle.
 *
 * Returns: whether the current value of @self should be displayed as its subtitle
 *
 * Since: 0.0.10
 */
gboolean
hdy_combo_row_get_use_subtitle (HdyComboRow *self)
{
  HdyComboRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_COMBO_ROW (self), FALSE);

  priv = hdy_combo_row_get_instance_private (self);

  return priv->use_subtitle;
}

/**
 * hdy_combo_row_set_use_subtitle:
 * @self: a #HdyComboRow
 * @use_subtitle: %TRUE to set the current value as the subtitle
 *
 * Sets whether the current value of @self should be displayed as its subtitle.
 *
 * If %TRUE, you should not access HdyActionRow:subtitle.
 *
 * Since: 0.0.10
 */
void
hdy_combo_row_set_use_subtitle (HdyComboRow *self,
                                gboolean     use_subtitle)
{
  HdyComboRowPrivate *priv;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));

  priv = hdy_combo_row_get_instance_private (self);

  use_subtitle = !!use_subtitle;

  if (priv->use_subtitle == use_subtitle)
    return;

  priv->use_subtitle = use_subtitle;
  update (self);
  if (!use_subtitle)
    hdy_action_row_set_subtitle (HDY_ACTION_ROW (self), NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_SUBTITLE]);
}

/**
 * hdy_combo_row_set_get_name_func:
 * @self: a #HdyComboRow
 * @get_name_func: (nullable): a function that creates names for items, or %NULL
 *   in case you also passed %NULL as @model
 * @user_data: user data passed to @get_name_func
 * @user_data_free_func: function for freeing @user_data
 *
 * Sets a closure to convert items into names. See HdyComboRow:use-subtitle.
 *
 * Since: 0.0.10
 */
void
hdy_combo_row_set_get_name_func (HdyComboRow            *self,
                                 HdyComboRowGetNameFunc  get_name_func,
                                 gpointer                user_data,
                                 GDestroyNotify          user_data_free_func)
{
  HdyComboRowPrivate *priv;

  g_return_if_fail (HDY_IS_COMBO_ROW (self));

  priv = hdy_combo_row_get_instance_private (self);

  get_name_free (priv->get_name);
  priv->get_name = g_new0 (HdyComboRowGetName, 1);
  priv->get_name->func = get_name_func;
  priv->get_name->func_data = user_data;
  priv->get_name->func_data_destroy = user_data_free_func;
}

/**
 * hdy_enum_value_row_name:
 * @value: the value from the enum from which to get a name
 * @user_data: (closure): unused user data
 *
 * This is a default implementation of #HdyComboRowGetEnumValueNameFunc to be
 * used with hdy_combo_row_set_for_enum(). If the enumeration has a nickname, it
 * will return it, otherwise it will return its name.
 *
 * Returns: (transfer full): a newly allocated displayable name that represents @value
 *
 * Since: 0.0.6
 */
gchar *
hdy_enum_value_row_name (HdyEnumValueObject *value,
                         gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  return g_strdup (hdy_enum_value_object_get_nick (value) != NULL ?
                   hdy_enum_value_object_get_nick (value) :
                   hdy_enum_value_object_get_name (value));
}
