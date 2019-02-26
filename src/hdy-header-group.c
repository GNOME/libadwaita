/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-header-group.h"

typedef struct
{
  GSList *header_bars;
  GtkHeaderBar *focus;

} HdyHeaderGroupPrivate;

static void hdy_header_group_buildable_init (GtkBuildableIface *iface);
static gboolean hdy_header_group_buildable_custom_tag_start (GtkBuildable  *buildable,
                                                             GtkBuilder    *builder,
                                                             GObject       *child,
                                                             const gchar   *tagname,
                                                             GMarkupParser *parser,
                                                             gpointer      *data);
static void hdy_header_group_buildable_custom_finished (GtkBuildable *buildable,
                                                        GtkBuilder   *builder,
                                                        GObject      *child,
                                                        const gchar  *tagname,
                                                        gpointer      user_data);

G_DEFINE_TYPE_WITH_CODE (HdyHeaderGroup, hdy_header_group, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (HdyHeaderGroup)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         hdy_header_group_buildable_init))

enum {
  PROP_0,
  PROP_FOCUS,
  N_PROPS
};

static GParamSpec *props [N_PROPS];

static gboolean
contains (HdyHeaderGroup *self,
          GtkHeaderBar   *header_bar)
{
  HdyHeaderGroupPrivate *priv;
  GSList *header_bars;

  priv = hdy_header_group_get_instance_private (self);

  for (header_bars = priv->header_bars; header_bars != NULL; header_bars = header_bars->next)
    if (header_bars->data == header_bar)
      return TRUE;

  return FALSE;
}


static void
update_decoration_layouts (HdyHeaderGroup *self)
{
  HdyHeaderGroupPrivate *priv;
  GSList *header_bars;
  GtkSettings *settings;
  GtkHeaderBar *start_headerbar = NULL, *end_headerbar = NULL;
  g_autofree gchar *layout = NULL;
  g_autofree gchar *start_layout = NULL;
  g_autofree gchar *end_layout = NULL;
  g_auto(GStrv) ends = NULL;

  g_return_if_fail (HDY_IS_HEADER_GROUP (self));

  priv = hdy_header_group_get_instance_private (self);
  header_bars = priv->header_bars;

  if (header_bars == NULL)
    return;

  settings = gtk_settings_get_default ();
  g_object_get (G_OBJECT (settings), "gtk-decoration-layout", &layout, NULL);
  if (layout == NULL)
    layout = g_strdup (":");

  if (priv->focus != NULL) {
    for (; header_bars != NULL; header_bars = header_bars->next)
      if (header_bars->data == priv->focus &&
          gtk_widget_get_mapped (header_bars->data))
        gtk_header_bar_set_decoration_layout (header_bars->data, layout);
      else
        gtk_header_bar_set_decoration_layout (header_bars->data, ":");

    return;
  }

  for (; header_bars != NULL; header_bars = header_bars->next) {
    gtk_header_bar_set_decoration_layout (header_bars->data, ":");

    if (!gtk_widget_get_mapped (header_bars->data))
      continue;

    /* The headerbars are in reverse order in the list.  */
    start_headerbar = header_bars->data;
    if (end_headerbar == NULL)
      end_headerbar = header_bars->data;
  }

  if (start_headerbar == NULL || end_headerbar == NULL)
    return;

  if (start_headerbar == end_headerbar) {
    gtk_header_bar_set_decoration_layout (start_headerbar, layout);

    return;
  }

  ends = g_strsplit (layout, ":", 2);
  if (g_strv_length (ends) >= 2) {
    start_layout = g_strdup_printf ("%s:", ends[0]);
    end_layout = g_strdup_printf (":%s", ends[1]);
  } else {
    start_layout = g_strdup (":");
    end_layout = g_strdup (":");
  }
  gtk_header_bar_set_decoration_layout (start_headerbar, start_layout);
  gtk_header_bar_set_decoration_layout (end_headerbar, end_layout);
}

static void
header_bar_destroyed (HdyHeaderGroup *self, GtkHeaderBar *header_bar)
{
  HdyHeaderGroupPrivate *priv;

  g_return_if_fail (HDY_IS_HEADER_GROUP (self));

  priv = hdy_header_group_get_instance_private (self);
  priv->header_bars = g_slist_remove (priv->header_bars, header_bar);

  g_object_unref (self);
}

HdyHeaderGroup *
hdy_header_group_new (void)
{
  return g_object_new (HDY_TYPE_HEADER_GROUP, NULL);
}

/**
 * hdy_header_group_add_header_bar:
 * @self: a #HdyHeaderGroup
 * @header_bar: the #GtkHeaderBar to add
 *
 * Adds a header bar to a #HdyHeaderGroup. The decoration layout of the
 * widgets will be edited depending on their position in the composite header
 * bar, the start widget displaying only the start of the user's decoration
 * layout and the end widget displaying only its end while widgets in the middle
 * won't display anything. A header bar can be set as having the focus to
 * display all the decorations. See gtk_header_bar_set_decoration_layout().
 *
 * When the widget is destroyed or no longer referenced elsewhere, it will
 * be removed from the header group.
 */
void
hdy_header_group_add_header_bar (HdyHeaderGroup *self,
                                 GtkHeaderBar   *header_bar)
{
  HdyHeaderGroupPrivate *priv;

  g_return_if_fail (HDY_IS_HEADER_GROUP (self));
  g_return_if_fail (GTK_IS_HEADER_BAR (header_bar));

  priv = hdy_header_group_get_instance_private (self);

  g_signal_connect_swapped (header_bar, "map", G_CALLBACK (update_decoration_layouts), self);
  g_signal_connect_swapped (header_bar, "unmap", G_CALLBACK (update_decoration_layouts), self);
  priv->header_bars = g_slist_prepend (priv->header_bars, header_bar);

  g_object_ref (self);

  g_signal_connect_swapped (header_bar, "destroy", G_CALLBACK (header_bar_destroyed), self);

  update_decoration_layouts (self);
}


/**
 * hdy_header_group_remove_header_bar:
 * @self: a #HdyHeaderGroup
 * @header_bar: the #GtkHeaderBar to remove
 *
 * Removes a widget from a #HdyHeaderGroup
 **/
void
hdy_header_group_remove_header_bar (HdyHeaderGroup *self,
                                    GtkHeaderBar *header_bar)
{
  HdyHeaderGroupPrivate *priv;

  g_return_if_fail (HDY_IS_HEADER_GROUP (self));
  g_return_if_fail (GTK_IS_HEADER_BAR (header_bar));
  g_return_if_fail (contains (self, header_bar));

  priv = hdy_header_group_get_instance_private (self);
  priv->header_bars = g_slist_remove (priv->header_bars, header_bar);

  if (priv->focus == header_bar)
    hdy_header_group_set_focus (self, NULL);

  g_signal_handlers_disconnect_by_data (header_bar, self);

  g_object_unref (self);
}


/**
 * hdy_header_group_get_header_bars:
 * @self: a #HdyHeaderGroup
 *
 * Returns the list of headerbars associated with @self.
 *
 * Returns:  (element-type GtkHeaderBar) (transfer none): a #GSList of
 *   headerbars. The list is owned by libhandy and should not be modified.
 **/
GSList *
hdy_header_group_get_header_bars (HdyHeaderGroup *self)
{
  HdyHeaderGroupPrivate *priv;

  g_return_val_if_fail (HDY_IS_HEADER_GROUP (self), NULL);
  priv = hdy_header_group_get_instance_private (self);
  return priv->header_bars;
}


/**
 * hdy_header_group_set_focus:
 * @self: a #HdyHeaderGroup
 * @header_bar: (nullable): a #GtkHeaderBar of @self, or %NULL
 *
 * Sets the the currently focused header bar. If @header_bar is %NULL, the
 * decoration will be spread as if the header bars of the group were only one,
 * otherwise @header_bar will be the only one to receive the decoration.
 */
void
hdy_header_group_set_focus (HdyHeaderGroup *self,
                            GtkHeaderBar   *header_bar)
{
  HdyHeaderGroupPrivate *priv;

  g_return_if_fail (HDY_IS_HEADER_GROUP (self));
  g_return_if_fail (header_bar == NULL || GTK_IS_HEADER_BAR (header_bar));
  g_return_if_fail (header_bar == NULL || contains (self, header_bar));

  priv = hdy_header_group_get_instance_private (self);

  priv->focus = header_bar;

  update_decoration_layouts (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FOCUS]);
}

/**
 * hdy_header_group_get_focus:
 * @self: a #HdyHeaderGroup
 *
 * Returns: (nullable) (transfer none): The currently focused header bar
 */
GtkHeaderBar *
hdy_header_group_get_focus (HdyHeaderGroup *self)
{
  HdyHeaderGroupPrivate *priv;

  g_return_val_if_fail (HDY_IS_HEADER_GROUP (self), FALSE);

  priv = hdy_header_group_get_instance_private (self);

  return priv->focus;
}


typedef struct {
  gchar *name;
  gint line;
  gint col;
} ItemData;

static void
item_data_free (gpointer data)
{
  ItemData *item_data = data;

  g_free (item_data->name);
  g_free (item_data);
}

typedef struct {
  GObject *object;
  GtkBuilder *builder;
  GSList *items;
} GSListSubParserData;

static void
hdy_header_group_dispose (GObject *object)
{
  HdyHeaderGroup *self = (HdyHeaderGroup *)object;
  HdyHeaderGroupPrivate *priv = hdy_header_group_get_instance_private (self);

  g_slist_free_full (priv->header_bars, (GDestroyNotify) g_object_unref);
  priv->header_bars = NULL;
  priv->focus = NULL;

  G_OBJECT_CLASS (hdy_header_group_parent_class)->dispose (object);
}

static void
hdy_header_group_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  HdyHeaderGroup *self = HDY_HEADER_GROUP (object);

  switch (prop_id) {
  case PROP_FOCUS:
    g_value_set_object (value, hdy_header_group_get_focus (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_header_group_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  HdyHeaderGroup *self = HDY_HEADER_GROUP (object);

  switch (prop_id) {
  case PROP_FOCUS:
    hdy_header_group_set_focus (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

/*< private >
 * @builder: a #GtkBuilder
 * @context: the #GMarkupParseContext
 * @parent_name: the name of the expected parent element
 * @error: return location for an error
 *
 * Checks that the parent element of the currently handled
 * start tag is @parent_name and set @error if it isn't.
 *
 * This is intended to be called in start_element vfuncs to
 * ensure that element nesting is as intended.
 *
 * Returns: %TRUE if @parent_name is the parent element
 */
/* This has been copied and modified from gtkbuilder.c. */
static gboolean
_gtk_builder_check_parent (GtkBuilder           *builder,
                           GMarkupParseContext  *context,
                           const gchar          *parent_name,
                           GError              **error)
{
  const GSList *stack;
  gint line, col;
  const gchar *parent;
  const gchar *element;

  stack = g_markup_parse_context_get_element_stack (context);

  element = (const gchar *)stack->data;
  parent = stack->next ? (const gchar *)stack->next->data : "";

  if (g_str_equal (parent_name, parent) ||
      (g_str_equal (parent_name, "object") && g_str_equal (parent, "template")))
    return TRUE;

  g_markup_parse_context_get_position (context, &line, &col);
  g_set_error (error,
               GTK_BUILDER_ERROR,
               GTK_BUILDER_ERROR_INVALID_TAG,
               ".:%d:%d Can't use <%s> here",
               line, col, element);

  return FALSE;
}

/*< private >
 * _gtk_builder_prefix_error:
 * @builder: a #GtkBuilder
 * @context: the #GMarkupParseContext
 * @error: an error
 *
 * Calls g_prefix_error() to prepend a filename:line:column marker
 * to the given error. The filename is taken from @builder, and
 * the line and column are obtained by calling
 * g_markup_parse_context_get_position().
 *
 * This is intended to be called on errors returned by
 * g_markup_collect_attributes() in a start_element vfunc.
 */
/* This has been copied and modified from gtkbuilder.c. */
static void
_gtk_builder_prefix_error (GtkBuilder           *builder,
                           GMarkupParseContext  *context,
                           GError              **error)
{
  gint line, col;

  g_markup_parse_context_get_position (context, &line, &col);
  g_prefix_error (error, ".:%d:%d ", line, col);
}

/*< private >
 * _gtk_builder_error_unhandled_tag:
 * @builder: a #GtkBuilder
 * @context: the #GMarkupParseContext
 * @object: name of the object that is being handled
 * @element_name: name of the element whose start tag is being handled
 * @error: return location for the error
 *
 * Sets @error to a suitable error indicating that an @element_name
 * tag is not expected in the custom markup for @object.
 *
 * This is intended to be called in a start_element vfunc.
 */
/* This has been copied and modified from gtkbuilder.c. */
static void
_gtk_builder_error_unhandled_tag (GtkBuilder           *builder,
                                  GMarkupParseContext  *context,
                                  const gchar          *object,
                                  const gchar          *element_name,
                                  GError              **error)
{
  gint line, col;

  g_markup_parse_context_get_position (context, &line, &col);
  g_set_error (error,
               GTK_BUILDER_ERROR,
               GTK_BUILDER_ERROR_UNHANDLED_TAG,
               ".:%d:%d Unsupported tag for %s: <%s>",
               line, col,
               object, element_name);
}

/* This has been copied and modified from gtksizegroup.c. */
static void
header_group_start_element (GMarkupParseContext  *context,
                          const gchar          *element_name,
                          const gchar         **names,
                          const gchar         **values,
                          gpointer              user_data,
                          GError              **error)
{
  GSListSubParserData *data = (GSListSubParserData*)user_data;

  if (strcmp (element_name, "headerbar") == 0)
    {
      const gchar *name;
      ItemData *item_data;

      if (!_gtk_builder_check_parent (data->builder, context, "headerbars", error))
        return;

      if (!g_markup_collect_attributes (element_name, names, values, error,
                                        G_MARKUP_COLLECT_STRING, "name", &name,
                                        G_MARKUP_COLLECT_INVALID))
        {
          _gtk_builder_prefix_error (data->builder, context, error);
          return;
        }

      item_data = g_new (ItemData, 1);
      item_data->name = g_strdup (name);
      g_markup_parse_context_get_position (context, &item_data->line, &item_data->col);
      data->items = g_slist_prepend (data->items, item_data);
    }
  else if (strcmp (element_name, "headerbars") == 0)
    {
      if (!_gtk_builder_check_parent (data->builder, context, "object", error))
        return;

      if (!g_markup_collect_attributes (element_name, names, values, error,
                                        G_MARKUP_COLLECT_INVALID, NULL, NULL,
                                        G_MARKUP_COLLECT_INVALID))
        _gtk_builder_prefix_error (data->builder, context, error);
    }
  else
    {
      _gtk_builder_error_unhandled_tag (data->builder, context,
                                        "GtkSizeGroup", element_name,
                                        error);
    }
}


/* This has been copied and modified from gtksizegroup.c. */
static const GMarkupParser header_group_parser =
  {
    header_group_start_element
  };

/* This has been copied and modified from gtksizegroup.c. */
static gboolean
hdy_header_group_buildable_custom_tag_start (GtkBuildable  *buildable,
                                             GtkBuilder    *builder,
                                             GObject       *child,
                                             const gchar   *tagname,
                                             GMarkupParser *parser,
                                             gpointer      *parser_data)
{
  GSListSubParserData *data;

  if (child)
    return FALSE;

  if (strcmp (tagname, "headerbars") == 0)
    {
      data = g_slice_new0 (GSListSubParserData);
      data->items = NULL;
      data->object = G_OBJECT (buildable);
      data->builder = builder;

      *parser = header_group_parser;
      *parser_data = data;

      return TRUE;
    }

  return FALSE;
}

/* This has been copied and modified from gtksizegroup.c. */
static void
hdy_header_group_buildable_custom_finished (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const gchar  *tagname,
                                            gpointer      user_data)
{
  GSList *l;
  GSListSubParserData *data;
  GObject *object;

  if (strcmp (tagname, "headerbars") != 0)
    return;

  data = (GSListSubParserData*)user_data;
  data->items = g_slist_reverse (data->items);

  for (l = data->items; l; l = l->next)
    {
      ItemData *item_data = l->data;
      object = gtk_builder_get_object (builder, item_data->name);
      if (!object)
        continue;
      hdy_header_group_add_header_bar (HDY_HEADER_GROUP (data->object), GTK_HEADER_BAR (object));
    }
  g_slist_free_full (data->items, item_data_free);
  g_slice_free (GSListSubParserData, data);
}

static void
hdy_header_group_class_init (HdyHeaderGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = hdy_header_group_dispose;
  object_class->get_property = hdy_header_group_get_property;
  object_class->set_property = hdy_header_group_set_property;

  /**
   * HdyHeaderGroup:focus:
   *
   * The the currently focused header bar. If %NULL, the decoration will be
   * spread as if the header bars of the group were only one, otherwise the
   * focused header bar will be the only one to receive the decoration.
   */
  props[PROP_FOCUS] =
    g_param_spec_object ("focus",
                         _("Focus"),
                         _("The header bar that should have the focus"),
                         GTK_TYPE_HEADER_BAR,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, N_PROPS, props);
}

static void
hdy_header_group_init (HdyHeaderGroup *self)
{
  GtkSettings *settings = gtk_settings_get_default ();

  g_signal_connect_swapped (settings, "notify::gtk-decoration-layout", G_CALLBACK (update_decoration_layouts), self);
}

static void
hdy_header_group_buildable_init (GtkBuildableIface *iface)
{
  iface->custom_tag_start = hdy_header_group_buildable_custom_tag_start;
  iface->custom_finished = hdy_header_group_buildable_custom_finished;
}
