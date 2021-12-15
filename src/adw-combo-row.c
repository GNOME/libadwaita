/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * Heavily based on GTK 3.99.3 GtkDropDown widget initially written by Matthias
 * Clasen, and heavily modified for libadwaita by Alexander Mikhaylenko on
 * behalf of Purism SPC 2020.
 */

#include "config.h"
#include "adw-combo-row.h"

/**
 * AdwComboRow:
 *
 * A [class@Gtk.ListBoxRow] used to choose from a list of items.
 *
 * The `AdwComboRow` widget allows the user to choose from a list of valid
 * choices. The row displays the selected choice. When activated, the row
 * displays a popover which allows the user to make a new choice.
 *
 * `AdwComboRow` mirrors [class@Gtk.DropDown], see that widget for details.
 *
 * `AdwComboRow` is [property@Gtk.ListBoxRow:activatable] if a model is set.
 *
 * ## CSS nodes
 *
 * `AdwComboRow` has a main CSS node with name `row` and the `.combo` style
 * class.
 *
 * Its popover has the node named `popover` with the `.menu` style class, it
 * contains a [class@Gtk.ScrolledWindow], which in turn contains a
 * [class@Gtk.ListView], both are accessible via their regular nodes.
 *
 * ## Accessibility
 *
 * `AdwComboRow` uses the `GTK_ACCESSIBLE_ROLE_COMBO_BOX` role.
 *
 * Since: 1.0
 */

/*
 * This was mostly inspired by code from the display panel from GNOME Settings.
 */

typedef struct
{
  GtkImage *image;
  GtkListView *current;
  GtkListView *list;
  GtkPopover *popover;
  gboolean use_subtitle;

  GtkListItemFactory *factory;
  GtkListItemFactory *list_factory;
  GListModel *model;
  GtkSelectionModel *selection;
  GtkSelectionModel *popup_selection;
  GtkSelectionModel *current_selection;

  GtkExpression *expression;
} AdwComboRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwComboRow, adw_combo_row, ADW_TYPE_ACTION_ROW)

enum {
  PROP_0,
  PROP_MODEL,
  PROP_SELECTED,
  PROP_SELECTED_ITEM,
  PROP_FACTORY,
  PROP_LIST_FACTORY,
  PROP_EXPRESSION,
  PROP_USE_SUBTITLE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static char *
get_item_representation (AdwComboRow *self,
                         gpointer     item)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  GValue value = G_VALUE_INIT;

  if (priv->expression &&
      gtk_expression_evaluate (priv->expression, item, &value)) {
    char *ret = g_value_dup_string (&value);

    g_value_unset (&value);

    return ret;
  } else if (GTK_IS_STRING_OBJECT (item)) {
    return g_strdup (gtk_string_object_get_string (GTK_STRING_OBJECT (item)));
  }

  return NULL;
}

static void
selection_changed (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  guint selected;

  if (!GTK_IS_SINGLE_SELECTION (priv->selection))
    return;

  selected = gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (priv->selection));

  if (priv->use_subtitle) {
    if (g_list_model_get_n_items (G_LIST_MODEL (priv->current_selection)) > 0) {
      g_autoptr (GtkListItem) item = g_list_model_get_item (G_LIST_MODEL (priv->current_selection), 0);
      g_autofree char *repr = get_item_representation (self, item);

      adw_action_row_set_subtitle (ADW_ACTION_ROW (self), repr);
    } else {
      adw_action_row_set_subtitle (ADW_ACTION_ROW (self), NULL);
    }
  }

  gtk_single_selection_set_selected (GTK_SINGLE_SELECTION (priv->popup_selection), selected);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_ITEM]);
}

static void
model_changed (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  guint n_items = priv->model ? g_list_model_get_n_items (priv->model) : 0;

  gtk_widget_set_sensitive (GTK_WIDGET (self), n_items > 0);
  gtk_widget_set_visible (GTK_WIDGET (priv->image), n_items > 1);
  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (self), n_items > 1);
}

static void
row_activated_cb (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  gtk_popover_popdown (GTK_POPOVER (priv->popover));

  adw_combo_row_set_selected (self, gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (priv->popup_selection)));
}

static void
notify_popover_visible_cb (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  if (gtk_widget_get_visible (GTK_WIDGET (priv->popover)))
    gtk_widget_add_css_class (GTK_WIDGET (self), "has-open-popup");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "has-open-popup");
}

static void
adw_combo_row_activate (AdwActionRow *row)
{
  AdwComboRow *self = ADW_COMBO_ROW (row);
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  if (gtk_widget_get_visible (GTK_WIDGET (priv->image)))
    gtk_popover_popup (priv->popover);
}

static void
setup_item (GtkSignalListItemFactory *factory,
            GtkListItem              *list_item,
            AdwComboRow              *self)
{
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *icon;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  label = gtk_label_new (NULL);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars (GTK_LABEL (label), 20);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (box), label);

  icon = gtk_image_new_from_icon_name ("object-select-symbolic");
  gtk_box_append (GTK_BOX (box), icon);

  gtk_list_item_set_child (list_item, box);
}

static void
selected_item_changed (AdwComboRow *self,
                       GParamSpec  *pspec,
                       GtkListItem *list_item)
{
  GtkWidget *box;
  GtkWidget *icon;

  box = gtk_list_item_get_child (list_item);
  icon = gtk_widget_get_last_child (box);

  if (adw_combo_row_get_selected_item (self) == gtk_list_item_get_item (list_item))
    gtk_widget_set_opacity (icon, 1.0);
  else
    gtk_widget_set_opacity (icon, 0.0);
}

static void
bind_item (GtkSignalListItemFactory *factory,
           GtkListItem              *list_item,
           AdwComboRow              *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  gpointer item;
  GtkWidget *box;
  GtkWidget *icon;
  g_autofree char *repr = NULL;

  item = gtk_list_item_get_item (list_item);
  box = gtk_list_item_get_child (list_item);
  icon = gtk_widget_get_last_child (box);

  repr = get_item_representation (self, item);

  if (repr) {
    GtkWidget *label = gtk_widget_get_first_child (box);

    gtk_label_set_label (GTK_LABEL (label), repr);
  } else {
    g_critical ("Either AdwComboRow:factory or AdwComboRow:expression must be set");
  }

  if (gtk_widget_get_ancestor (box, GTK_TYPE_POPOVER) == GTK_WIDGET (priv->popover)) {
    gtk_widget_show (icon);
    g_signal_connect (self, "notify::selected-item",
                      G_CALLBACK (selected_item_changed), list_item);
    selected_item_changed (self, NULL, list_item);
  } else {
    gtk_widget_hide (icon);
  }
}

static void
unbind_item (GtkSignalListItemFactory *factory,
             GtkListItem              *list_item,
             AdwComboRow              *self)
{
  g_signal_handlers_disconnect_by_func (self, selected_item_changed, list_item);
}

static void
set_default_factory (AdwComboRow *self)
{
  g_autoptr (GtkListItemFactory) factory = gtk_signal_list_item_factory_new ();

  g_signal_connect (factory, "setup", G_CALLBACK (setup_item), self);
  g_signal_connect (factory, "bind", G_CALLBACK (bind_item), self);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind_item), self);

  adw_combo_row_set_factory (self, factory);
}

static void
adw_combo_row_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  AdwComboRow *self = ADW_COMBO_ROW (object);

  switch (prop_id) {
  case PROP_MODEL:
    g_value_set_object (value, adw_combo_row_get_model (self));
    break;
  case PROP_SELECTED:
    g_value_set_uint (value, adw_combo_row_get_selected (self));
    break;
  case PROP_SELECTED_ITEM:
    g_value_set_object (value, adw_combo_row_get_selected_item (self));
    break;
  case PROP_FACTORY:
    g_value_set_object (value, adw_combo_row_get_factory (self));
    break;
  case PROP_LIST_FACTORY:
    g_value_set_object (value, adw_combo_row_get_list_factory (self));
    break;
  case PROP_EXPRESSION:
    gtk_value_set_expression (value, adw_combo_row_get_expression (self));
    break;
  case PROP_USE_SUBTITLE:
    g_value_set_boolean (value, adw_combo_row_get_use_subtitle (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_combo_row_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  AdwComboRow *self = ADW_COMBO_ROW (object);

  switch (prop_id) {
  case PROP_MODEL:
    adw_combo_row_set_model (self, g_value_get_object (value));
    break;
  case PROP_SELECTED:
    adw_combo_row_set_selected (self, g_value_get_uint (value));
    break;
  case PROP_FACTORY:
    adw_combo_row_set_factory (self, g_value_get_object (value));
    break;
  case PROP_LIST_FACTORY:
    adw_combo_row_set_list_factory (self, g_value_get_object (value));
    break;
  case PROP_EXPRESSION:
    adw_combo_row_set_expression (self, gtk_value_get_expression (value));
    break;
  case PROP_USE_SUBTITLE:
    adw_combo_row_set_use_subtitle (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_combo_row_dispose (GObject *object)
{
  AdwComboRow *self = ADW_COMBO_ROW (object);
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  gtk_list_view_set_model (priv->list, NULL);
  gtk_list_view_set_model (priv->current, NULL);

  if (priv->selection) {
    g_signal_handlers_disconnect_by_func (priv->selection, selection_changed, self);
    g_signal_handlers_disconnect_by_func (priv->selection, model_changed, self);
  }

  g_clear_pointer (&priv->expression, gtk_expression_unref);
  g_clear_object (&priv->selection);
  g_clear_object (&priv->popup_selection);
  g_clear_object (&priv->current_selection);
  g_clear_object (&priv->factory);
  g_clear_object (&priv->list_factory);

  g_clear_object (&priv->model);

  G_OBJECT_CLASS (adw_combo_row_parent_class)->dispose (object);
}

static void
adw_combo_row_size_allocate (GtkWidget *widget,
                             int        width,
                             int        height,
                             int        baseline)
{
  AdwComboRow *self = ADW_COMBO_ROW (widget);
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  GTK_WIDGET_CLASS (adw_combo_row_parent_class)->size_allocate (widget, width, height, baseline);

  gtk_popover_present (priv->popover);
}

static gboolean
adw_combo_row_focus (GtkWidget        *widget,
                     GtkDirectionType  direction)
{
  AdwComboRow *self = ADW_COMBO_ROW (widget);
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  if (priv->popover && gtk_widget_get_visible (GTK_WIDGET (priv->popover)))
    return gtk_widget_child_focus (GTK_WIDGET (priv->popover), direction);
  else
    return GTK_WIDGET_CLASS (adw_combo_row_parent_class)->focus (widget, direction);
}

static void
adw_combo_row_class_init (AdwComboRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  AdwActionRowClass *row_class = ADW_ACTION_ROW_CLASS (klass);

  object_class->get_property = adw_combo_row_get_property;
  object_class->set_property = adw_combo_row_set_property;
  object_class->dispose = adw_combo_row_dispose;

  widget_class->size_allocate = adw_combo_row_size_allocate;
  widget_class->focus = adw_combo_row_focus;

  row_class->activate = adw_combo_row_activate;

  /**
   * AdwComboRow:model: (attributes org.gtk.Property.get=adw_combo_row_get_model org.gtk.Property.set=adw_combo_row_set_model)
   *
   * Model for the displayed items.
   *
   * Since: 1.0
   */
  props[PROP_MODEL] =
    g_param_spec_object ("model",
                         "Model",
                         "Model for the displayed items",
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:selected: (attributes org.gtk.Property.get=adw_combo_row_get_selected org.gtk.Property.set=adw_combo_row_set_selected)
   *
   * The position of the selected item.
   *
   * If no item is selected, the property has the value
   * `GTK_INVALID_LIST_POSITION`.
   *
   * Since: 1.0
   */
  props[PROP_SELECTED] =
    g_param_spec_uint ("selected",
                       "Selected",
                       "Position of the selected item",
                       0, G_MAXUINT, GTK_INVALID_LIST_POSITION,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:selected-item: (attributes org.gtk.Property.get=adw_combo_row_get_selected_item)
   *
   * The selected item.
   *
   * Since: 1.0
   */
  props[PROP_SELECTED_ITEM] =
    g_param_spec_object ("selected-item",
                         "Selected Item",
                         "The selected item",
                         G_TYPE_OBJECT,
                         G_PARAM_READABLE);

  /**
   * AdwComboRow:factory: (attributes org.gtk.Property.get=adw_combo_row_get_factory org.gtk.Property.set=adw_combo_row_set_factory)
   *
   * Factory for populating list items.
   *
   * This factory is always used for the item in the row. It is also used for
   * items in the popup unless [property@Adw.ComboRow:list-factory] is set.
   *
   * Since: 1.0
   */
  props[PROP_FACTORY] =
    g_param_spec_object ("factory",
                         "Factory",
                         "Factory for populating list items",
                         GTK_TYPE_LIST_ITEM_FACTORY,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:list-factory: (attributes org.gtk.Property.get=adw_combo_row_get_list_factory org.gtk.Property.set=adw_combo_row_set_list_factory)
   *
   * The factory for populating list items in the popup.
   *
   * If this is not set, [property@Adw.ComboRow:factory] is used.
   *
   * Since: 1.0
   */
  props[PROP_LIST_FACTORY] =
    g_param_spec_object ("list-factory",
                         "List Factory",
                         "Factory for populating list items",
                         GTK_TYPE_LIST_ITEM_FACTORY,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:expression: (type GtkExpression) (attributes org.gtk.Property.get=adw_combo_row_get_expression org.gtk.Property.set=adw_combo_row_set_expression)
   *
   * An expression used to obtain strings from items.
   *
   * It's used to bind strings to labels produced by the default factory.
   *
   * If [property@Adw.ComboRow:factory] is not set, the expression is also
   * used to bind strings to labels produced by a default factory.
   *
   * Since: 1.0
   */
  props[PROP_EXPRESSION] =
    gtk_param_spec_expression ("expression",
                               "Expression",
                               "Expression to determine strings to search for",
                               G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:use-subtitle: (attributes org.gtk.Property.get=adw_combo_row_get_use_subtitle org.gtk.Property.set=adw_combo_row_set_use_subtitle)
   *
   * Whether to use the current value as the subtitle.
   *
   * If you use a custom list item factory, you will need to give the row a
   * name conversion expression with [property@Adw.ComboRow:expression].
   *
   * If `TRUE`, you should not access [property@Adw.ActionRow:subtitle].
   *
   * Since: 1.0
   */
  props[PROP_USE_SUBTITLE] =
    g_param_spec_boolean ("use-subtitle",
                          "Use subtitle",
                          "Whether to use the current value as the subtitle",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-combo-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, current);
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, list);
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, popover);
  gtk_widget_class_bind_template_callback (widget_class, row_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_popover_visible_cb);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_COMBO_BOX);
}

static void
adw_combo_row_init (AdwComboRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  set_default_factory (self);
  model_changed (self);
}

/**
 * adw_combo_row_new:
 *
 * Creates a new `AdwComboRow`.
 *
 * Returns: the newly created `AdwComboRow`
 *
 * Since: 1.0
 */
GtkWidget *
adw_combo_row_new (void)
{
  return g_object_new (ADW_TYPE_COMBO_ROW, NULL);
}

/**
 * adw_combo_row_set_selected: (attributes org.gtk.Method.set_property=selected)
 * @self: a `AdwComboRow`
 * @position: the position of the item to select, or `GTK_INVALID_LIST_POSITION`
 *
 * Selects the item at the given position.
 *
 * Since: 1.0
 */
void
adw_combo_row_set_selected (AdwComboRow *self,
                            guint        position)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));

  priv = adw_combo_row_get_instance_private (self);

  if (priv->selection == NULL)
    return;

  if (gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (priv->selection)) == position)
    return;

  gtk_single_selection_set_selected (GTK_SINGLE_SELECTION (priv->selection), position);
}

/**
 * adw_combo_row_get_selected: (attributes org.gtk.Method.get_property=selected)
 * @self: a `AdwComboRow`
 *
 * Gets the position of the selected item.
 *
 * Returns: the position of the selected item, or `GTK_INVALID_LIST_POSITION`
 *   if no item is selected
 *
 * Since: 1.0
 */
guint
adw_combo_row_get_selected (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), GTK_INVALID_LIST_POSITION);

  priv = adw_combo_row_get_instance_private (self);

  if (priv->selection == NULL)
    return GTK_INVALID_LIST_POSITION;

  return gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (priv->selection));
}

/**
 * adw_combo_row_get_selected_item: (attributes org.gtk.Method.get_property=selected-item)
 * @self: a `AdwComboRow`
 *
 * Gets the selected item.
 *
 * Returns: (transfer none) (type GObject) (nullable): the selected item
 *
 * Since: 1.0
 */
gpointer
adw_combo_row_get_selected_item (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  if (priv->selection == NULL)
    return NULL;

  return gtk_single_selection_get_selected_item (GTK_SINGLE_SELECTION (priv->selection));
}

/**
 * adw_combo_row_get_model: (attributes org.gtk.Method.get_property=model)
 * @self: a `AdwComboRow`
 *
 * Gets the model that provides the displayed items.
 *
 * Returns: (nullable) (transfer none): The model in use
 *
 * Since: 1.0
 */
GListModel *
adw_combo_row_get_model (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  return priv->model;
}

/**
 * adw_combo_row_set_model: (attributes org.gtk.Method.set_property=model)
 * @self: a `AdwComboRow`
 * @model: (nullable) (transfer none): the model to use
 *
 * Sets the [iface@Gio.ListModel] to use.
 *
 * Since: 1.0
 */
void
adw_combo_row_set_model (AdwComboRow *self,
                         GListModel  *model)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));
  g_return_if_fail (model == NULL || G_IS_LIST_MODEL (model));

  priv = adw_combo_row_get_instance_private (self);

  if (!g_set_object (&priv->model, model))
    return;

  if (model == NULL) {
    gtk_list_view_set_model (priv->list, NULL);
    gtk_list_view_set_model (priv->current, NULL);

    if (priv->selection) {
      g_signal_handlers_disconnect_by_func (priv->selection, selection_changed, self);
      g_signal_handlers_disconnect_by_func (priv->selection, model_changed, self);
    }

    g_clear_object (&priv->selection);
    g_clear_object (&priv->popup_selection);
    g_clear_object (&priv->current_selection);
  } else {
    GtkSelectionModel *selection;
    GListModel *current_model;

    selection = GTK_SELECTION_MODEL (gtk_single_selection_new (g_object_ref (model)));
    g_set_object (&priv->popup_selection, selection);
    gtk_list_view_set_model (priv->list, selection);
    g_object_unref (selection);

    selection = GTK_SELECTION_MODEL (gtk_single_selection_new (g_object_ref (model)));
    g_set_object (&priv->selection, selection);
    g_object_unref (selection);

    current_model = G_LIST_MODEL (gtk_selection_filter_model_new (priv->selection));
    selection = GTK_SELECTION_MODEL (gtk_no_selection_new (current_model));
    g_set_object (&priv->current_selection, selection);
    gtk_list_view_set_model (priv->current, selection);
    g_object_unref (selection);

    g_signal_connect_swapped (priv->selection, "notify::selected", G_CALLBACK (selection_changed), self);
    g_signal_connect_swapped (priv->selection, "items-changed", G_CALLBACK (model_changed), self);

    selection_changed (self);
    model_changed (self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODEL]);
}

/**
 * adw_combo_row_get_factory: (attributes org.gtk.Method.get_property=factory)
 * @self: a `AdwComboRow`
 *
 * Gets the factory that's currently used to populate list items.
 *
 * Returns: (nullable) (transfer none): the factory in use
 *
 * Since: 1.0
 */
GtkListItemFactory *
adw_combo_row_get_factory (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  return priv->factory;
}

/**
 * adw_combo_row_set_factory: (attributes org.gtk.Method.set_property=factory)
 * @self: a `AdwComboRow`
 * @factory: (nullable) (transfer none): the factory to use
 *
 * Sets the `GtkListItemFactory` to use for populating list items.
 *
 * Since: 1.0
 */
void
adw_combo_row_set_factory (AdwComboRow        *self,
                           GtkListItemFactory *factory)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));
  g_return_if_fail (factory == NULL || GTK_LIST_ITEM_FACTORY (factory));

  priv = adw_combo_row_get_instance_private (self);

  if (!g_set_object (&priv->factory, factory))
    return;

  gtk_list_view_set_factory (priv->current, factory);

  if (priv->list_factory == NULL)
    gtk_list_view_set_factory (priv->list, factory);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FACTORY]);
}

/**
 * adw_combo_row_get_list_factory: (attributes org.gtk.Method.get_property=list-factory)
 * @self: a `AdwComboRow`
 *
 * Gets the factory that's currently used to populate list items in the popup.
 *
 * Returns: (nullable) (transfer none): the factory in use
 *
 * Since: 1.0
 */
GtkListItemFactory *
adw_combo_row_get_list_factory (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  return priv->list_factory;
}

/**
 * adw_combo_row_set_list_factory: (attributes org.gtk.Method.set_property=list-factory)
 * @self: a `AdwComboRow`
 * @factory: (nullable) (transfer none): the factory to use
 *
 * Sets the `GtkListItemFactory` to use for populating list items in the popup.
 *
 * Since: 1.0
 */
void
adw_combo_row_set_list_factory (AdwComboRow        *self,
                                GtkListItemFactory *factory)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));
  g_return_if_fail (factory == NULL || GTK_LIST_ITEM_FACTORY (factory));

  priv = adw_combo_row_get_instance_private (self);

  if (!g_set_object (&priv->list_factory, factory))
    return;

  if (priv->list_factory != NULL)
    gtk_list_view_set_factory (priv->list, priv->list_factory);
  else
    gtk_list_view_set_factory (priv->list, priv->factory);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LIST_FACTORY]);
}

/**
 * adw_combo_row_get_expression: (attributes org.gtk.Method.get_property=expression)
 * @self: a `AdwComboRow`
 *
 * Gets the expression used to obtain strings from items.
 *
 * Returns: (nullable) (transfer none): the expression used to obtain strings from items
 *
 * Since: 1.0
 */
GtkExpression *
adw_combo_row_get_expression (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  return priv->expression;
}

/**
 * adw_combo_row_set_expression: (attributes org.gtk.Method.set_property=expression)
 * @self: a `AdwComboRow`
 * @expression: (nullable): an expression
 *
 * Sets the expression used to obtain strings from items.
 *
 * The expression must have a value type of `G_TYPE_STRING`.
 *
 * Since: 1.0
 */
void
adw_combo_row_set_expression (AdwComboRow   *self,
                              GtkExpression *expression)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));
  g_return_if_fail (expression == NULL ||
                    gtk_expression_get_value_type (expression) == G_TYPE_STRING);

  priv = adw_combo_row_get_instance_private (self);

  if (priv->expression == expression)
    return;

  if (priv->expression)
    gtk_expression_unref (priv->expression);
  priv->expression = expression;
  if (priv->expression)
    gtk_expression_ref (priv->expression);

  selection_changed (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXPRESSION]);
}

/**
 * adw_combo_row_get_use_subtitle: (attributes org.gtk.Method.get_property=use-subtitle)
 * @self: a `AdwComboRow`
 *
 * Gets whether to use the current value as the subtitle.
 *
 * Returns: whether to use the current value as the subtitle
 *
 * Since: 1.0
 */
gboolean
adw_combo_row_get_use_subtitle (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), FALSE);

  priv = adw_combo_row_get_instance_private (self);

  return priv->use_subtitle;
}

/**
 * adw_combo_row_set_use_subtitle: (attributes org.gtk.Method.set_property=use-subtitle)
 * @self: a `AdwComboRow`
 * @use_subtitle: whether to use the current value as the subtitle
 *
 * Sets whether to use the current value as the subtitle.
 *
 * Since: 1.0
 */
void
adw_combo_row_set_use_subtitle (AdwComboRow *self,
                                gboolean     use_subtitle)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));

  priv = adw_combo_row_get_instance_private (self);

  use_subtitle = !!use_subtitle;

  if (priv->use_subtitle == use_subtitle)
    return;

  priv->use_subtitle = use_subtitle;
  selection_changed (self);
  if (!use_subtitle)
    adw_action_row_set_subtitle (ADW_ACTION_ROW (self), NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_SUBTITLE]);
}
