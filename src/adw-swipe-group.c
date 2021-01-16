/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-swipe-group.h"
#include <gtk/gtk.h>
#include "adw-navigation-direction.h"
#include "adw-swipe-tracker-private.h"

#define BUILDABLE_TAG_OBJECT "object"
#define BUILDABLE_TAG_SWIPEABLE "swipeable"
#define BUILDABLE_TAG_SWIPEABLES "swipeables"
#define BUILDABLE_TAG_TEMPLATE "template"

/**
 * SECTION:adw-swipe-group
 * @short_description: An object for syncing swipeable widgets.
 * @title: AdwSwipeGroup
 * @See_also: #AdwCarousel, #AdwLeaflet, #AdwSwipeable
 *
 * The #AdwSwipeGroup object can be used to sync multiple swipeable widgets
 * that implement the #AdwSwipeable interface, such as #AdwCarousel, so that
 * animating one of them also animates all the other widgets in the group.
 *
 * This can be useful for syncing widgets between a window's titlebar and
 * content area.
 *
 * # #AdwSwipeGroup as #GtkBuildable
 *
 * #AdwSwipeGroup can be created in an UI definition. The list of swipeable
 * widgets is specified with a &lt;swipeables&gt; element containing multiple
 * &lt;swipeable&gt; elements with their ”name” attribute specifying the id of
 * the widgets.
 *
 * |[
 * <object class="AdwSwipeGroup">
 *   <swipeables>
 *     <swipeable name="carousel1"/>
 *     <swipeable name="carousel2"/>
 *   </swipeables>
 * </object>
 * ]|
 *
 * Since: 1.0
 */

struct _AdwSwipeGroup
{
  GObject parent_instance;

  GSList *swipeables;
  AdwSwipeable *current;
  gboolean block;
};

static void adw_swipe_group_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwSwipeGroup, adw_swipe_group, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adw_swipe_group_buildable_init))

static gboolean
contains (AdwSwipeGroup *self,
          AdwSwipeable  *swipeable)
{
  GSList *swipeables;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data == swipeable)
      return TRUE;

  return FALSE;
}

static void
swipeable_destroyed (AdwSwipeGroup *self,
                     AdwSwipeable  *swipeable)
{
  g_return_if_fail (ADW_IS_SWIPE_GROUP (self));

  self->swipeables = g_slist_remove (self->swipeables, swipeable);

  g_object_unref (self);
}

/**
 * adw_swipe_group_new:
 *
 * Create a new #AdwSwipeGroup object.
 *
 * Returns: The newly created #AdwSwipeGroup object
 *
 * Since: 1.0
 */
AdwSwipeGroup *
adw_swipe_group_new (void)
{
  return g_object_new (ADW_TYPE_SWIPE_GROUP, NULL);
}

static void
child_switched_cb (AdwSwipeGroup *self,
                   guint          index,
                   gint64         duration,
                   AdwSwipeable  *swipeable)
{
  GSList *swipeables;

  if (self->block)
    return;

  if (self->current != NULL && self->current != swipeable)
    return;

  self->block = TRUE;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data != swipeable)
      adw_swipeable_switch_child (swipeables->data, index, duration);

  self->block = FALSE;
}

static void
begin_swipe_cb (AdwSwipeGroup          *self,
                AdwNavigationDirection  direction,
                gboolean                direct,
                AdwSwipeTracker        *tracker)
{
  AdwSwipeable *swipeable;
  GSList *swipeables;

  if (self->block)
    return;

  swipeable = adw_swipe_tracker_get_swipeable (tracker);

  if (self->current != NULL && self->current != swipeable)
    return;

  self->current = swipeable;

  self->block = TRUE;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data != swipeable)
      adw_swipe_tracker_emit_begin_swipe (adw_swipeable_get_swipe_tracker (swipeables->data),
                                          direction, FALSE);

  self->block = FALSE;
}

static void
update_swipe_cb (AdwSwipeGroup   *self,
                 gdouble          progress,
                 AdwSwipeTracker *tracker)
{
  AdwSwipeable *swipeable;
  GSList *swipeables;

  if (self->block)
    return;

  swipeable = adw_swipe_tracker_get_swipeable (tracker);

  if (swipeable != self->current)
    return;

  self->block = TRUE;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data != swipeable)
      adw_swipe_tracker_emit_update_swipe (adw_swipeable_get_swipe_tracker (swipeables->data),
                                           progress);

  self->block = FALSE;
}

static void
end_swipe_cb (AdwSwipeGroup   *self,
              gint64           duration,
              gdouble          to,
              AdwSwipeTracker *tracker)
{
  AdwSwipeable *swipeable;
  GSList *swipeables;

  if (self->block)
    return;

  swipeable = adw_swipe_tracker_get_swipeable (tracker);

  if (swipeable != self->current)
    return;

  self->block = TRUE;

  for (swipeables = self->swipeables; swipeables != NULL; swipeables = swipeables->next)
    if (swipeables->data != swipeable)
      adw_swipe_tracker_emit_end_swipe (adw_swipeable_get_swipe_tracker (swipeables->data),
                                        duration, to);

  self->current = NULL;

  self->block = FALSE;
}

/**
 * adw_swipe_group_add_swipeable:
 * @self: a #AdwSwipeGroup
 * @swipeable: the #AdwSwipeable to add
 *
 * When the widget is destroyed or no longer referenced elsewhere, it will
 * be removed from the swipe group.
 *
 * Since: 1.0
 */
void
adw_swipe_group_add_swipeable (AdwSwipeGroup *self,
                               AdwSwipeable  *swipeable)
{
  AdwSwipeTracker *tracker;

  g_return_if_fail (ADW_IS_SWIPE_GROUP (self));
  g_return_if_fail (ADW_IS_SWIPEABLE (swipeable));

  tracker = adw_swipeable_get_swipe_tracker (swipeable);

  g_return_if_fail (ADW_IS_SWIPE_TRACKER (tracker));

  g_signal_connect_swapped (swipeable, "child-switched", G_CALLBACK (child_switched_cb), self);
  g_signal_connect_swapped (tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self);
  g_signal_connect_swapped (tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self);
  g_signal_connect_swapped (tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self);

  self->swipeables = g_slist_prepend (self->swipeables, swipeable);

  g_object_ref (self);

  g_signal_connect_swapped (swipeable, "destroy", G_CALLBACK (swipeable_destroyed), self);
}


/**
 * adw_swipe_group_remove_swipeable:
 * @self: a #AdwSwipeGroup
 * @swipeable: the #AdwSwipeable to remove
 *
 * Removes a widget from a #AdwSwipeGroup.
 *
 * Since: 1.0
 **/
void
adw_swipe_group_remove_swipeable (AdwSwipeGroup *self,
                                  AdwSwipeable  *swipeable)
{
  AdwSwipeTracker *tracker;

  g_return_if_fail (ADW_IS_SWIPE_GROUP (self));
  g_return_if_fail (ADW_IS_SWIPEABLE (swipeable));
  g_return_if_fail (contains (self, swipeable));

  tracker = adw_swipeable_get_swipe_tracker (swipeable);

  self->swipeables = g_slist_remove (self->swipeables, swipeable);

  g_signal_handlers_disconnect_by_data (swipeable, self);
  g_signal_handlers_disconnect_by_data (tracker, self);

  g_object_unref (self);
}


/**
 * adw_swipe_group_get_swipeables:
 * @self: a #AdwSwipeGroup
 *
 * Returns the list of swipeables associated with @self.
 *
 * Returns:  (element-type AdwSwipeable) (transfer none): a #GSList of
 *   swipeables. The list is owned by libadwaita and should not be modified.
 *
 * Since: 1.0
 **/
GSList *
adw_swipe_group_get_swipeables (AdwSwipeGroup *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_GROUP (self), NULL);

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
adw_swipe_group_dispose (GObject *object)
{
  AdwSwipeGroup *self = (AdwSwipeGroup *)object;

  g_slist_free_full (self->swipeables, (GDestroyNotify) g_object_unref);
  self->swipeables = NULL;

  G_OBJECT_CLASS (adw_swipe_group_parent_class)->dispose (object);
}

/*< private >
 * @builder: a #GtkBuilder
 * @context: the #GtkBuildableParseContext
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
_gtk_builder_check_parent (GtkBuilder                *builder,
                           GtkBuildableParseContext  *context,
                           const gchar               *parent_name,
                           GError                   **error)
{
  GPtrArray *stack;
  int line, col;
  const char *parent;
  const char *element;

  stack = gtk_buildable_parse_context_get_element_stack (context);

  element = g_ptr_array_index (stack, stack->len - 1);
  parent = stack->len > 1 ? g_ptr_array_index (stack, stack->len - 2) : "";

  if (g_str_equal (parent_name, parent) ||
      (g_str_equal (parent_name, "object") && g_str_equal (parent, "template")))
    return TRUE;

  gtk_buildable_parse_context_get_position (context, &line, &col);
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
 * @context: the #GtkBuildableParseContext
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
_gtk_builder_prefix_error (GtkBuilder                *builder,
                           GtkBuildableParseContext  *context,
                           GError                   **error)
{
  gint line, col;

  gtk_buildable_parse_context_get_position (context, &line, &col);
  g_prefix_error (error, ".:%d:%d ", line, col);
}

/*< private >
 * _gtk_builder_error_unhandled_tag:
 * @builder: a #GtkBuilder
 * @context: the #GtkBuildableParseContext
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
_gtk_builder_error_unhandled_tag (GtkBuilder                *builder,
                                  GtkBuildableParseContext  *context,
                                  const gchar               *object,
                                  const gchar               *element_name,
                                  GError                   **error)
{
  gint line, col;

  gtk_buildable_parse_context_get_position (context, &line, &col);
  g_set_error (error,
               GTK_BUILDER_ERROR,
               GTK_BUILDER_ERROR_UNHANDLED_TAG,
               ".:%d:%d Unsupported tag for %s: <%s>",
               line, col,
               object, element_name);
}

/* This has been copied and modified from gtksizegroup.c. */
static void
swipe_group_start_element (GtkBuildableParseContext  *context,
                           const gchar               *element_name,
                           const gchar              **names,
                           const gchar              **values,
                           gpointer                   user_data,
                           GError                   **error)
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
      gtk_buildable_parse_context_get_position (context, &item_data->line, &item_data->col);
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
                                        "AdwSwipeGroup", element_name,
                                        error);
    }
}


/* This has been copied and modified from gtksizegroup.c. */
static const GtkBuildableParser swipe_group_parser =
  {
    swipe_group_start_element
  };

/* This has been copied and modified from gtksizegroup.c. */
static gboolean
adw_swipe_group_buildable_custom_tag_start (GtkBuildable       *buildable,
                                            GtkBuilder         *builder,
                                            GObject            *child,
                                            const gchar        *tagname,
                                            GtkBuildableParser *parser,
                                            gpointer           *parser_data)
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
adw_swipe_group_buildable_custom_finished (GtkBuildable *buildable,
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
      adw_swipe_group_add_swipeable (ADW_SWIPE_GROUP (data->object), ADW_SWIPEABLE (object));
    }
  g_slist_free_full (data->items, item_data_free);
  g_slice_free (GSListSubParserData, data);
}

static void
adw_swipe_group_class_init (AdwSwipeGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_swipe_group_dispose;
}

static void
adw_swipe_group_init (AdwSwipeGroup *self)
{
}

static void
adw_swipe_group_buildable_init (GtkBuildableIface *iface)
{
  iface->custom_tag_start = adw_swipe_group_buildable_custom_tag_start;
  iface->custom_finished = adw_swipe_group_buildable_custom_finished;
}
