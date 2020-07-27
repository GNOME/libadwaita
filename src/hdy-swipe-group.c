/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-swipe-group.h"
#include <gtk/gtk.h>
#include "hdy-navigation-direction.h"
#include "hdy-swipe-tracker-private.h"

#define BUILDABLE_TAG_OBJECT "object"
#define BUILDABLE_TAG_SWIPEABLE "swipeable"
#define BUILDABLE_TAG_SWIPEABLES "swipeables"
#define BUILDABLE_TAG_TEMPLATE "template"

/**
 * SECTION:hdy-swipe-group
 * @short_description: An object for syncing swipeable widgets.
 * @title: HdySwipeGroup
 * @See_also: #HdyCarousel, #HdyDeck, #HdyLeaflet, #HdySwipeable
 *
 * The #HdySwipeGroup object can be used to sync multiple swipeable widgets
 * that implement the #HdySwipeable interface, such as #HdyCarousel, so that
 * animating one of them also animates all the other widgets in the group.
 *
 * This can be useful for syncing widgets between a window's titlebar and
 * content area.
 *
 * # #HdySwipeGroup as #GtkBuildable
 *
 * #HdySwipeGroup can be created in an UI definition. The list of swipeable
 * widgets is specified with a &lt;swipeables&gt; element containing multiple
 * &lt;swipeable&gt; elements with their ”name” attribute specifying the id of
 * the widgets.
 *
 * |[
 * <object class="HdySwipeGroup">
 *   <swipeables>
 *     <swipeable name="carousel1"/>
 *     <swipeable name="carousel2"/>
 *   </swipeables>
 * </object>
 * ]|
 *
 * Since: 0.0.12
 */

struct _HdySwipeGroup
{
  GObject parent_instance;

  GSList *swipeables;
  HdySwipeable *current;
  gboolean block;
};

static void hdy_swipe_group_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdySwipeGroup, hdy_swipe_group, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         hdy_swipe_group_buildable_init))

static gboolean
contains (HdySwipeGroup *self,
          HdySwipeable  *swipeable)
{
  GSList *swipeables;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data == swipeable)
      return TRUE;

  return FALSE;
}

static void
swipeable_destroyed (HdySwipeGroup *self,
                     HdySwipeable  *swipeable)
{
  g_return_if_fail (HDY_IS_SWIPE_GROUP (self));

  self->swipeables = g_slist_remove (self->swipeables, swipeable);

  g_object_unref (self);
}

/**
 * hdy_swipe_group_new:
 *
 * Create a new #HdySwipeGroup object.
 *
 * Returns: The newly created #HdySwipeGroup object
 *
 * Since: 0.0.12
 */
HdySwipeGroup *
hdy_swipe_group_new (void)
{
  return g_object_new (HDY_TYPE_SWIPE_GROUP, NULL);
}

static void
child_switched_cb (HdySwipeGroup *self,
                   guint          index,
                   gint64         duration,
                   HdySwipeable  *swipeable)
{
  GSList *swipeables;

  if (self->block)
    return;

  if (self->current != NULL && self->current != swipeable)
    return;

  self->block = TRUE;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data != swipeable)
      hdy_swipeable_switch_child (swipeables->data, index, duration);

  self->block = FALSE;
}

static void
begin_swipe_cb (HdySwipeGroup          *self,
                HdyNavigationDirection  direction,
                gboolean                direct,
                HdySwipeTracker        *tracker)
{
  HdySwipeable *swipeable;
  GSList *swipeables;

  if (self->block)
    return;

  swipeable = hdy_swipe_tracker_get_swipeable (tracker);

  if (self->current != NULL && self->current != swipeable)
    return;

  self->current = swipeable;

  self->block = TRUE;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data != swipeable)
      hdy_swipe_tracker_emit_begin_swipe (hdy_swipeable_get_swipe_tracker (swipeables->data),
                                          direction, FALSE);

  self->block = FALSE;
}

static void
update_swipe_cb (HdySwipeGroup   *self,
                 gdouble          progress,
                 HdySwipeTracker *tracker)
{
  HdySwipeable *swipeable;
  GSList *swipeables;

  if (self->block)
    return;

  swipeable = hdy_swipe_tracker_get_swipeable (tracker);

  if (swipeable != self->current)
    return;

  self->block = TRUE;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data != swipeable)
      hdy_swipe_tracker_emit_update_swipe (hdy_swipeable_get_swipe_tracker (swipeables->data),
                                           progress);

  self->block = FALSE;
}

static void
end_swipe_cb (HdySwipeGroup   *self,
              gint64           duration,
              gdouble          to,
              HdySwipeTracker *tracker)
{
  HdySwipeable *swipeable;
  GSList *swipeables;

  if (self->block)
    return;

  swipeable = hdy_swipe_tracker_get_swipeable (tracker);

  if (swipeable != self->current)
    return;

  self->block = TRUE;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data != swipeable)
      hdy_swipe_tracker_emit_end_swipe (hdy_swipeable_get_swipe_tracker (swipeables->data),
                                        duration, to);

  self->current = NULL;

  self->block = FALSE;
}

/**
 * hdy_swipe_group_add_swipeable:
 * @self: a #HdySwipeGroup
 * @swipeable: the #HdySwipeable to add
 *
 * When the widget is destroyed or no longer referenced elsewhere, it will
 * be removed from the swipe group.
 *
 * Since: 0.0.12
 */
void
hdy_swipe_group_add_swipeable (HdySwipeGroup *self,
                               HdySwipeable  *swipeable)
{
  HdySwipeTracker *tracker;

  g_return_if_fail (HDY_IS_SWIPE_GROUP (self));
  g_return_if_fail (HDY_IS_SWIPEABLE (swipeable));

  tracker = hdy_swipeable_get_swipe_tracker (swipeable);

  g_return_if_fail (HDY_IS_SWIPE_TRACKER (tracker));

  g_signal_connect_swapped (swipeable, "child-switched", G_CALLBACK (child_switched_cb), self);
  g_signal_connect_swapped (tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self);
  g_signal_connect_swapped (tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self);
  g_signal_connect_swapped (tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self);

  self->swipeables = g_slist_prepend (self->swipeables, swipeable);

  g_object_ref (self);

  g_signal_connect_swapped (swipeable, "destroy", G_CALLBACK (swipeable_destroyed), self);
}


/**
 * hdy_swipe_group_remove_swipeable:
 * @self: a #HdySwipeGroup
 * @swipeable: the #HdySwipeable to remove
 *
 * Removes a widget from a #HdySwipeGroup.
 *
 * Since: 0.0.12
 **/
void
hdy_swipe_group_remove_swipeable (HdySwipeGroup *self,
                                  HdySwipeable  *swipeable)
{
  HdySwipeTracker *tracker;

  g_return_if_fail (HDY_IS_SWIPE_GROUP (self));
  g_return_if_fail (HDY_IS_SWIPEABLE (swipeable));
  g_return_if_fail (contains (self, swipeable));

  tracker = hdy_swipeable_get_swipe_tracker (swipeable);

  self->swipeables = g_slist_remove (self->swipeables, swipeable);

  g_signal_handlers_disconnect_by_data (swipeable, self);
  g_signal_handlers_disconnect_by_data (tracker, self);

  g_object_unref (self);
}


/**
 * hdy_swipe_group_get_swipeables:
 * @self: a #HdySwipeGroup
 *
 * Returns the list of swipeables associated with @self.
 *
 * Returns:  (element-type HdySwipeable) (transfer none): a #GSList of
 *   swipeables. The list is owned by libhandy and should not be modified.
 *
 * Since: 0.0.12
 **/
GSList *
hdy_swipe_group_get_swipeables (HdySwipeGroup *self)
{
  g_return_val_if_fail (HDY_IS_SWIPE_GROUP (self), NULL);

  return self->swipeables;
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
hdy_swipe_group_dispose (GObject *object)
{
  HdySwipeGroup *self = (HdySwipeGroup *)object;

  g_slist_free_full (self->swipeables, (GDestroyNotify) g_object_unref);
  self->swipeables = NULL;

  G_OBJECT_CLASS (hdy_swipe_group_parent_class)->dispose (object);
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
      (g_str_equal (parent_name, BUILDABLE_TAG_OBJECT) &&
       g_str_equal (parent, BUILDABLE_TAG_TEMPLATE)))
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
swipe_group_start_element (GMarkupParseContext  *context,
                           const gchar          *element_name,
                           const gchar         **names,
                           const gchar         **values,
                           gpointer              user_data,
                           GError              **error)
{
  GSListSubParserData *data = (GSListSubParserData*)user_data;

  if (strcmp (element_name, BUILDABLE_TAG_SWIPEABLE) == 0)
    {
      const gchar *name;
      ItemData *item_data;

      if (!_gtk_builder_check_parent (data->builder, context, BUILDABLE_TAG_SWIPEABLES, error))
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
  else if (strcmp (element_name, BUILDABLE_TAG_SWIPEABLES) == 0)
    {
      if (!_gtk_builder_check_parent (data->builder, context, BUILDABLE_TAG_OBJECT, error))
        return;

      if (!g_markup_collect_attributes (element_name, names, values, error,
                                        G_MARKUP_COLLECT_INVALID, NULL, NULL,
                                        G_MARKUP_COLLECT_INVALID))
        _gtk_builder_prefix_error (data->builder, context, error);
    }
  else
    {
      _gtk_builder_error_unhandled_tag (data->builder, context,
                                        "HdySwipeGroup", element_name,
                                        error);
    }
}


/* This has been copied and modified from gtksizegroup.c. */
static const GMarkupParser swipe_group_parser =
  {
    swipe_group_start_element
  };

/* This has been copied and modified from gtksizegroup.c. */
static gboolean
hdy_swipe_group_buildable_custom_tag_start (GtkBuildable  *buildable,
                                            GtkBuilder    *builder,
                                            GObject       *child,
                                            const gchar   *tagname,
                                            GMarkupParser *parser,
                                            gpointer      *parser_data)
{
  GSListSubParserData *data;

  if (child)
    return FALSE;

  if (strcmp (tagname, BUILDABLE_TAG_SWIPEABLES) == 0)
    {
      data = g_slice_new0 (GSListSubParserData);
      data->items = NULL;
      data->object = G_OBJECT (buildable);
      data->builder = builder;

      *parser = swipe_group_parser;
      *parser_data = data;

      return TRUE;
    }

  return FALSE;
}

/* This has been copied and modified from gtksizegroup.c. */
static void
hdy_swipe_group_buildable_custom_finished (GtkBuildable *buildable,
                                           GtkBuilder   *builder,
                                           GObject      *child,
                                           const gchar  *tagname,
                                           gpointer      user_data)
{
  GSList *l;
  GSListSubParserData *data;
  GObject *object;

  if (strcmp (tagname, BUILDABLE_TAG_SWIPEABLES) != 0)
    return;

  data = (GSListSubParserData*)user_data;
  data->items = g_slist_reverse (data->items);

  for (l = data->items; l; l = l->next)
    {
      ItemData *item_data = l->data;
      object = gtk_builder_get_object (builder, item_data->name);
      if (!object)
        continue;
      hdy_swipe_group_add_swipeable (HDY_SWIPE_GROUP (data->object), HDY_SWIPEABLE (object));
    }
  g_slist_free_full (data->items, item_data_free);
  g_slice_free (GSListSubParserData, data);
}

static void
hdy_swipe_group_class_init (HdySwipeGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = hdy_swipe_group_dispose;
}

static void
hdy_swipe_group_init (HdySwipeGroup *self)
{
}

static void
hdy_swipe_group_buildable_init (GtkBuildableIface *iface)
{
  iface->custom_tag_start = hdy_swipe_group_buildable_custom_tag_start;
  iface->custom_finished = hdy_swipe_group_buildable_custom_finished;
}
