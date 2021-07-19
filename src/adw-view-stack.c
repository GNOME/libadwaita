/*
 * Copyright (C) 2013 Red Hat, Inc.
 * Copyright (C) 2021 Frederick Schenk
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on gtkstack.c
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/ba44668478b7184bec02609f292691b85a2c6cdd/gtk/gtkstack.c
 */

#include "config.h"

#include "adw-view-stack.h"

#include "gtkprogresstrackerprivate.h"
#include "adw-animation-private.h"
#include "adw-enums.h"
#include "adw-focus-private.h"
#include "adw-gizmo-private.h"
#include "adw-macros-private.h"

/**
 * AdwViewStack:
 *
 * A view container for [class@Adw.ViewSwitcher].
 *
 * `AdwViewStack` is a container which only shows one page at a time.
 * It is typically used to hold an application's main views.
 *
 * It doesn't provide a way to transition between pages.
 * Instead, a separate widget such as [class@Adw.ViewSwitcher] can be used with
 * `AdwViewStack` to provide this functionality.
 *
 * `AdwViewStack` pages can have a title, an icon, an attention request, and a
 * numbered badge that [class@Adw.ViewSwitcher] will use to let users identify
 * which page is which.
 * Set them using the [property@Adw.ViewStackPage:title],
 * [property@Adw.ViewStackPage:icon-name],
 * [property@Adw.ViewStackPage:needs-attention], and
 * [property@Adw.ViewStackPage:badge-number] properties.
 *
 * Transitions between views are animated by crossfading.
 * These animations respect the [property@Gtk.Settings:gtk-enable-animations]
 * setting.
 *
 * `AdwViewStack` maintains a [class@Adw.ViewStackPage] object for each added
 * child, which holds additional per-child properties.
 * You obtain the [class@Adw.ViewStackPage] for a child with
 * [method@Adw.ViewStack.get_page] and you can obtain a
 * [iface@Gtk.SelectionModel] containing all the pages with
 * [method@Adw.ViewStack.get_pages].
 *
 * ## AdwViewStack as GtkBuildable
 *
 * To set child-specific properties in a .ui file, create
 * [class@Adw.ViewStackPage] objects explicitly, and set the child widget as a
 * property on it:
 *
 * ```xml
 *   <object class="AdwViewStack" id="stack">
 *     <child>
 *       <object class="AdwViewStackPage">
 *         <property name="name">overview</property>
 *         <property name="title">Overview</property>
 *         <property name="child">
 *           <object class="AdwStatusPage">
 *             <property name="title">Welcome!</property>
 *           </object>
 *         </property>
 *       </object>
 *     </child>
 * ```
 *
 * ## CSS nodes
 *
 * `AdwViewStack` has a single CSS node named `stack`.
 *
 * Since: 1.0
 */

/**
 * AdwViewStackPage:
 *
 * An auxiliary class used by [class@Adw.ViewStack].
 *
 * Since: 1.0
 */

#define TRANSITION_DURATION 200

enum {
  PROP_0,
  PROP_HHOMOGENEOUS,
  PROP_VHOMOGENEOUS,
  PROP_VISIBLE_CHILD,
  PROP_VISIBLE_CHILD_NAME,
  PROP_TRANSITION_RUNNING,
  PROP_INTERPOLATE_SIZE,
  PROP_PAGES,
  LAST_PROP
};

struct _AdwViewStackPage {
  GObject parent_instance;

  GtkWidget *widget;
  char *name;
  char *title;
  char *icon_name;
  guint badge_number;
  GtkWidget *last_focus;

  bool needs_attention;
  bool visible;
  bool use_underline;
};

G_DEFINE_TYPE (AdwViewStackPage, adw_view_stack_page, G_TYPE_OBJECT)

enum {
  PAGE_PROP_0,
  PAGE_PROP_CHILD,
  PAGE_PROP_NAME,
  PAGE_PROP_TITLE,
  PAGE_PROP_ICON_NAME,
  PAGE_PROP_NEEDS_ATTENTION,
  PAGE_PROP_BADGE_NUMBER,
  PAGE_PROP_VISIBLE,
  PAGE_PROP_USE_UNDERLINE,
  LAST_PAGE_PROP
};

static GParamSpec *page_props[LAST_PAGE_PROP];

struct _AdwViewStack {
  GtkWidget parent_instance;

  GList *children;

  AdwViewStackPage *visible_child;

  gboolean hhomogeneous;
  gboolean vhomogeneous;

  AdwViewStackPage *last_visible_child;
  guint tick_id;
  GtkProgressTracker tracker;
  gboolean first_frame_skipped;

  int last_visible_widget_width;
  int last_visible_widget_height;

  gboolean interpolate_size;

  GtkSelectionModel *pages;
};

static GParamSpec *props[LAST_PROP];

static void adw_view_stack_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwViewStack, adw_view_stack, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_view_stack_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
adw_view_stack_page_get_property (GObject      *object,
                                  guint         property_id,
                                  GValue       *value,
                                  GParamSpec   *pspec)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (object);

  switch (property_id) {
  case PAGE_PROP_CHILD:
    g_value_set_object (value, self->widget);
    break;
  case PAGE_PROP_NAME:
    g_value_set_string (value, adw_view_stack_page_get_name (self));
    break;
  case PAGE_PROP_TITLE:
    g_value_set_string (value, adw_view_stack_page_get_title (self));
    break;
  case PAGE_PROP_ICON_NAME:
    g_value_set_string (value, adw_view_stack_page_get_icon_name (self));
    break;
  case PAGE_PROP_NEEDS_ATTENTION:
    g_value_set_boolean (value, adw_view_stack_page_get_needs_attention (self));
    break;
  case PAGE_PROP_BADGE_NUMBER:
    g_value_set_uint (value, adw_view_stack_page_get_badge_number (self));
    break;
  case PAGE_PROP_VISIBLE:
    g_value_set_boolean (value, adw_view_stack_page_get_visible (self));
    break;
  case PAGE_PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adw_view_stack_page_get_use_underline (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_view_stack_page_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (object);

  switch (property_id) {
  case PAGE_PROP_CHILD:
    g_set_object (&self->widget, g_value_get_object (value));
    break;
  case PAGE_PROP_NAME:
    adw_view_stack_page_set_name (self, g_value_get_string (value));
    break;
  case PAGE_PROP_TITLE:
    adw_view_stack_page_set_title (self, g_value_get_string (value));
    break;
  case PAGE_PROP_ICON_NAME:
    adw_view_stack_page_set_icon_name (self, g_value_get_string (value));
    break;
  case PAGE_PROP_NEEDS_ATTENTION:
    adw_view_stack_page_set_needs_attention (self, g_value_get_boolean (value));
    break;
  case PAGE_PROP_BADGE_NUMBER:
    adw_view_stack_page_set_badge_number (self, g_value_get_uint (value));
    break;
  case PAGE_PROP_VISIBLE:
    adw_view_stack_page_set_visible (self, g_value_get_boolean (value));
    break;
  case PAGE_PROP_USE_UNDERLINE:
    adw_view_stack_page_set_use_underline (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_view_stack_page_finalize (GObject *object)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (object);

  g_clear_object (&self->widget);
  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->icon_name, g_free);

  if (self->last_focus)
    g_object_remove_weak_pointer (G_OBJECT (self->last_focus),
                                  (gpointer *) &self->last_focus);

  G_OBJECT_CLASS (adw_view_stack_page_parent_class)->finalize (object);
}

static void
adw_view_stack_page_class_init (AdwViewStackPageClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = adw_view_stack_page_finalize;
  object_class->get_property = adw_view_stack_page_get_property;
  object_class->set_property = adw_view_stack_page_set_property;

  /**
   * AdwViewStackPage:child: (attributes org.gtk.Property.get=adw_view_stack_page_get_child)
   *
   * The child of the page.
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_CHILD] =
    g_param_spec_object ("child",
                         "Child",
                         "The child of the page",
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * AdwViewStackPage:name: (attributes org.gtk.Property.get=adw_view_stack_page_get_name org.gtk.Property.set=adw_view_stack_page_set_name)
   *
   * The name of the child page.
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "The name of the child page",
                         NULL,
                         G_PARAM_READWRITE);

  /**
   * AdwViewStackPage:title: (attributes org.gtk.Property.get=adw_view_stack_page_get_title org.gtk.Property.set=adw_view_stack_page_set_title)
   *
   * The title of the child page.
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "The title of the child page",
                         NULL,
                         G_PARAM_READWRITE);

  /**
   * AdwViewStackPage:icon-name: (attributes org.gtk.Property.get=adw_view_stack_page_get_icon_name org.gtk.Property.set=adw_view_stack_page_set_icon_name)
   *
   * The icon name of the child page.
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         "Icon name",
                         "The icon name of the child page",
                         NULL,
                         G_PARAM_READWRITE);

  /**
   * AdwViewStackPage:needs-attention: (attributes org.gtk.Property.get=adw_view_stack_page_get_needs_attention org.gtk.Property.set=adw_view_stack_page_set_needs_attention)
   *
   * Whether the page requires the user attention.
   *
   * [class@Adw.ViewSwitcher] will display it as a dot next to the page icon.
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_NEEDS_ATTENTION] =
    g_param_spec_boolean ("needs-attention",
                          "Needs Attention",
                          "Whether the page requires the user attention",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:badge-number: (attributes org.gtk.Property.get=adw_view_stack_page_get_badge_number org.gtk.Property.set=adw_view_stack_page_set_badge_number)
   *
   * A number associated with the page.
   *
   * [class@Adw.ViewSwitcher] can display it as a badge next to the page icon.
   * It is commonly used to display a number of unread items within the page.
   *
   * It can be used together with [property@Adw.ViewStack{age}:needs-attention].
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_BADGE_NUMBER] =
    g_param_spec_uint ("badge-number",
                       "Badge_number",
                       "A number associated with the page",
                       0, G_MAXUINT, 0,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:visible: (attributes org.gtk.Property.get=adw_view_stack_page_get_visible org.gtk.Property.set=adw_view_stack_page_set_visible)
   *
   * Whether this page is visible.
   *
   * This is independent from the [property@Gtk.Widget:visible] property of
   * [property@Adw.ViewStackPage:child].
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_VISIBLE] =
    g_param_spec_boolean ("visible",
                          "Visible",
                          "Whether this page is visible",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:use-underline: (attributes org.gtk.Property.get=adw_view_stack_page_get_use_underline org.gtk.Property.set=adw_view_stack_page_set_use_underline)
   *
   * Whether an embedded underline in the title indicates a mnemonic.
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline",
                          "Use underline",
                          "Whether an embedded underline in the title label indicates a mnemonic",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PAGE_PROP, page_props);
}

static void
adw_view_stack_page_init (AdwViewStackPage *self)
{
  self->visible = TRUE;
}

#define ADW_TYPE_VIEW_STACK_PAGES (adw_view_stack_pages_get_type ())

G_DECLARE_FINAL_TYPE (AdwViewStackPages, adw_view_stack_pages, ADW, VIEW_STACK_PAGES, GObject)

struct _AdwViewStackPages
{
  GObject parent_instance;
  AdwViewStack *stack;
};

static GType
adw_view_stack_pages_get_item_type (GListModel *model)
{
  return ADW_TYPE_VIEW_STACK_PAGE;
}

static guint
adw_view_stack_pages_get_n_items (GListModel *model)
{
  AdwViewStackPages *self = ADW_VIEW_STACK_PAGES (model);

  return g_list_length (self->stack->children);
}

static gpointer
adw_view_stack_pages_get_item (GListModel *model,
                               guint       position)
{
  AdwViewStackPages *self = ADW_VIEW_STACK_PAGES (model);
  AdwViewStackPage *page;

  page = g_list_nth_data (self->stack->children, position);

  if (!page)
    return NULL;

  return g_object_ref (page);
}

static void
adw_view_stack_pages_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_view_stack_pages_get_item_type;
  iface->get_n_items = adw_view_stack_pages_get_n_items;
  iface->get_item = adw_view_stack_pages_get_item;
}

static gboolean
adw_view_stack_pages_is_selected (GtkSelectionModel *model,
                                  guint              position)
{
  AdwViewStackPages *self = ADW_VIEW_STACK_PAGES (model);
  AdwViewStackPage *page;

  page = g_list_nth_data (self->stack->children, position);

  return page && page == self->stack->visible_child;
}

static gboolean
adw_view_stack_pages_select_item (GtkSelectionModel *model,
                                  guint              position,
                                  gboolean           exclusive)
{
  AdwViewStackPages *self = ADW_VIEW_STACK_PAGES (model);
  AdwViewStackPage *page;

  page = g_list_nth_data (self->stack->children, position);

  adw_view_stack_set_visible_child (self->stack, page->widget);

  return TRUE;
}

static void
adw_view_stack_pages_selection_model_init (GtkSelectionModelInterface *iface)
{
  iface->is_selected = adw_view_stack_pages_is_selected;
  iface->select_item = adw_view_stack_pages_select_item;
}

G_DEFINE_TYPE_WITH_CODE (AdwViewStackPages, adw_view_stack_pages, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_view_stack_pages_list_model_init)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, adw_view_stack_pages_selection_model_init))

static void
adw_view_stack_pages_init (AdwViewStackPages *pages)
{
}

static void
adw_view_stack_pages_class_init (AdwViewStackPagesClass *class)
{
}

static AdwViewStackPages *
adw_view_stack_pages_new (AdwViewStack *stack)
{
  AdwViewStackPages *pages;

  pages = g_object_new (ADW_TYPE_VIEW_STACK_PAGES, NULL);
  pages->stack = stack;

  return pages;
}

static AdwViewStackPage *
find_page_for_widget (AdwViewStack *self,
                      GtkWidget    *child)
{
  AdwViewStackPage *page;
  GList *l;

  for (l = self->children; l; l = l->next) {
    page = l->data;

    if (page->widget == child)
      return page;
  }

  return NULL;
}

static AdwViewStackPage *
find_page_for_name (AdwViewStack *self,
                    const char   *name)
{
  AdwViewStackPage *page;
  GList *l;

  for (l = self->children; l; l = l->next) {
    page = l->data;

    if (g_strcmp0 (page->name, name) == 0)
      return page;
  }

  return NULL;
}

static void
progress_updated (AdwViewStack *self)
{
  if (!self->vhomogeneous || !self->hhomogeneous)
    gtk_widget_queue_resize (GTK_WIDGET (self));
  else
    gtk_widget_queue_draw (GTK_WIDGET (self));

  if (gtk_progress_tracker_get_state (&self->tracker) == GTK_PROGRESS_STATE_AFTER &&
      self->last_visible_child != NULL) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }
}

static gboolean
transition_cb (GtkWidget     *widget,
               GdkFrameClock *frame_clock,
               gpointer       user_data)
{
  AdwViewStack *self = ADW_VIEW_STACK (widget);

  if (self->first_frame_skipped)
    gtk_progress_tracker_advance_frame (&self->tracker,
                                        gdk_frame_clock_get_frame_time (frame_clock));
  else
    self->first_frame_skipped = TRUE;

  /* Finish animation early if not mapped anymore */
  if (!gtk_widget_get_mapped (widget))
    gtk_progress_tracker_finish (&self->tracker);

  progress_updated (self);

  if (gtk_progress_tracker_get_state (&self->tracker) == GTK_PROGRESS_STATE_AFTER) {
    self->tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);

    return FALSE;
  }

  return TRUE;
}

static void
schedule_ticks (AdwViewStack *self)
{
  if (self->tick_id == 0) {
    self->tick_id =
      gtk_widget_add_tick_callback (GTK_WIDGET (self),
                                    transition_cb,
                                    self, NULL);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);
  }
}

static void
unschedule_ticks (AdwViewStack *self)
{
  if (self->tick_id != 0) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->tick_id);
    self->tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);
  }
}

static void
start_transition (AdwViewStack *self)
{
  GtkWidget *widget = GTK_WIDGET (self);

  if (gtk_widget_get_mapped (widget) &&
      adw_get_enable_animations (widget) &&
      self->last_visible_child != NULL) {
    self->first_frame_skipped = FALSE;
    schedule_ticks (self);
    gtk_progress_tracker_start (&self->tracker,
                                TRANSITION_DURATION * 1000,
                                0,
                                1.0);
  } else {
    unschedule_ticks (self);
    gtk_progress_tracker_finish (&self->tracker);
  }

  progress_updated (self);
}

static void
set_visible_child (AdwViewStack     *self,
                   AdwViewStackPage *page)
{
  GtkWidget *widget = GTK_WIDGET (self);
  GtkRoot *root;
  GtkWidget *focus;
  gboolean contains_focus = FALSE;
  guint old_pos = GTK_INVALID_LIST_POSITION;
  guint new_pos = GTK_INVALID_LIST_POSITION;

  /* if we are being destroyed, do not bother with transitions
   * and notifications
   */
  if (gtk_widget_in_destruction (widget))
    return;

  /* If none, pick first visible */
  if (!page) {
    GList *l;

    for (l = self->children; l; l = l->next) {
      AdwViewStackPage *p = l->data;

      if (gtk_widget_get_visible (page->widget)) {
        page = p;

        break;
      }
    }
  }

  if (page == self->visible_child)
    return;

  if (self->pages) {
    guint position;
    GList *l;

    for (l = self->children, position = 0; l; l = l->next, position++) {
      AdwViewStackPage *p = l->data;
      if (p == self->visible_child)
        old_pos = position;
      else if (p == page)
        new_pos = position;
    }
  }

  root = gtk_widget_get_root (widget);
  if (root)
    focus = gtk_root_get_focus (root);
  else
    focus = NULL;

  if (focus &&
      self->visible_child &&
      self->visible_child->widget &&
      gtk_widget_is_ancestor (focus, self->visible_child->widget)) {
    contains_focus = TRUE;

    if (self->visible_child->last_focus)
      g_object_remove_weak_pointer (G_OBJECT (self->visible_child->last_focus),
                                    (gpointer *)&self->visible_child->last_focus);
    self->visible_child->last_focus = focus;
    g_object_add_weak_pointer (G_OBJECT (self->visible_child->last_focus),
                               (gpointer *)&self->visible_child->last_focus);
  }

  if (self->last_visible_child)
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
  self->last_visible_child = NULL;

  if (self->visible_child && self->visible_child->widget) {
    if (gtk_widget_is_visible (widget)) {
      self->last_visible_child = self->visible_child;
      self->last_visible_widget_width = gtk_widget_get_width (widget);
      self->last_visible_widget_height = gtk_widget_get_height (widget);
    } else {
      gtk_widget_set_child_visible (self->visible_child->widget, FALSE);
    }
  }

  self->visible_child = page;

  if (page) {
    gtk_widget_set_child_visible (page->widget, TRUE);

    if (contains_focus) {
      if (page->last_focus)
        gtk_widget_grab_focus (page->last_focus);
      else
        gtk_widget_child_focus (page->widget, GTK_DIR_TAB_FORWARD);
    }
  }

  if (self->hhomogeneous && self->vhomogeneous)
    gtk_widget_queue_allocate (widget);
  else
    gtk_widget_queue_resize (widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD_NAME]);

  if (self->pages) {
    if (old_pos == GTK_INVALID_LIST_POSITION && new_pos == GTK_INVALID_LIST_POSITION)
      ; /* nothing to do */
    else if (old_pos == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, new_pos, 1);
    else if (new_pos == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, old_pos, 1);
    else
      gtk_selection_model_selection_changed (self->pages,
                                             MIN (old_pos, new_pos),
                                             MAX (old_pos, new_pos) - MIN (old_pos, new_pos) + 1);
  }

  start_transition (self);
}

static void
update_child_visible (AdwViewStack     *self,
                      AdwViewStackPage *page)
{
  gboolean visible;

  visible = page->visible && gtk_widget_get_visible (page->widget);

  if (self->visible_child == NULL && visible)
    set_visible_child (self, page);
  else if (self->visible_child == page && !visible)
    set_visible_child (self, NULL);

  if (page == self->last_visible_child) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }
}

static void
stack_child_visibility_notify_cb (GObject    *obj,
                                  GParamSpec *pspec,
                                  gpointer    user_data)
{
  AdwViewStack *self = ADW_VIEW_STACK (user_data);
  AdwViewStackPage *page;

  page = find_page_for_widget (self, GTK_WIDGET (obj));
  g_return_if_fail (page != NULL);

  update_child_visible (self, page);
}

static void
add_page (AdwViewStack     *self,
          AdwViewStackPage *page)
{
  GList *l;

  g_return_if_fail (page->widget != NULL);

  if (page->name) {
    for (l = self->children; l; l = l->next) {
      AdwViewStackPage *p = l->data;

      if (p->name && g_strcmp0 (p->name, page->name) == 0) {
        g_warning ("While adding page: duplicate child name in AdwViewStack: %s", page->name);
        break;
      }
    }
  }

  self->children = g_list_append (self->children, g_object_ref (page));

  gtk_widget_set_child_visible (page->widget, FALSE);
  gtk_widget_set_parent (page->widget, GTK_WIDGET (self));

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), g_list_length (self->children) - 1, 0, 1);

  g_signal_connect (page->widget, "notify::visible",
                    G_CALLBACK (stack_child_visibility_notify_cb), self);

  if (self->visible_child == NULL &&
      gtk_widget_get_visible (page->widget))
    set_visible_child (self, page);

  if (self->hhomogeneous || self->vhomogeneous || self->visible_child == page)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static AdwViewStackPage *
add_internal (AdwViewStack *self,
              GtkWidget    *child,
              const char   *name,
              const char   *title)
{
  AdwViewStackPage *page;

  g_return_val_if_fail (child != NULL, NULL);

  page = g_object_new (ADW_TYPE_VIEW_STACK_PAGE, NULL);
  page->widget = g_object_ref (child);
  page->name = g_strdup (name);
  page->title = g_strdup (title);
  page->icon_name = NULL;
  page->needs_attention = FALSE;
  page->last_focus = NULL;

  add_page (self, page);

  g_object_unref (page);

  return page;
}

static void
stack_remove (AdwViewStack  *self,
              GtkWidget     *child,
              gboolean       in_dispose)
{
  AdwViewStackPage *page;
  gboolean was_visible;

  page = find_page_for_widget (self, child);
  if (!page)
    return;

  g_signal_handlers_disconnect_by_func (child,
                                        stack_child_visibility_notify_cb,
                                        self);

  was_visible = gtk_widget_get_visible (child);

  if (self->visible_child == page)
    self->visible_child = NULL;

  if (self->last_visible_child == page)
    self->last_visible_child = NULL;

  gtk_widget_unparent (child);

  g_clear_object (&page->widget);

  self->children = g_list_remove (self->children, page);

  g_object_unref (page);

  if (!in_dispose &&
      (self->hhomogeneous || self->vhomogeneous) &&
      was_visible)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
adw_view_stack_compute_expand (GtkWidget *widget,
                               gboolean  *hexpand_p,
                               gboolean  *vexpand_p)
{
  AdwViewStack *self = ADW_VIEW_STACK (widget);
  gboolean hexpand, vexpand;
  GList *l;

  hexpand = FALSE;
  vexpand = FALSE;

  for (l = self->children; l; l = l->next) {
    AdwViewStackPage *page = l->data;
    GtkWidget *child = page->widget;

    if (!hexpand &&
        gtk_widget_compute_expand (child, GTK_ORIENTATION_HORIZONTAL))
      hexpand = TRUE;

    if (!vexpand &&
        gtk_widget_compute_expand (child, GTK_ORIENTATION_VERTICAL))
      vexpand = TRUE;

    if (hexpand && vexpand)
      break;
  }

  *hexpand_p = hexpand;
  *vexpand_p = vexpand;
}

static GtkSizeRequestMode
adw_view_stack_get_request_mode (GtkWidget *widget)
{
  GtkWidget *child;
  int wfh = 0, hfw = 0;

  for (child = gtk_widget_get_first_child (widget);
       child;
       child = gtk_widget_get_next_sibling (child)) {
    GtkSizeRequestMode mode = gtk_widget_get_request_mode (child);

    switch (mode) {
    case GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH:
      hfw++;
      break;
    case GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT:
      wfh++;
      break;
    case GTK_SIZE_REQUEST_CONSTANT_SIZE:
    default:
      break;
    }
  }

  if (hfw == 0 && wfh == 0)
    return GTK_SIZE_REQUEST_CONSTANT_SIZE;
  else
    return wfh > hfw ?
        GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT :
        GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}


static void
adw_view_stack_size_allocate (GtkWidget *widget,
                              int        width,
                              int        height,
                              int        baseline)
{
  AdwViewStack *self = ADW_VIEW_STACK (widget);
  GtkAllocation child_allocation;

  if (self->last_visible_child) {
    int child_width, child_height;
    int min, nat;

    gtk_widget_measure (self->last_visible_child->widget,
                        GTK_ORIENTATION_HORIZONTAL, -1,
                        &min, &nat, NULL, NULL);
    child_width = MAX (min, width);
    gtk_widget_measure (self->last_visible_child->widget,
                        GTK_ORIENTATION_VERTICAL, child_width,
                        &min, &nat, NULL, NULL);
    child_height = MAX (min, height);

    gtk_widget_allocate (self->last_visible_child->widget,
                         child_width, child_height, -1, NULL);
  }

  child_allocation.x = 0;
  child_allocation.y = 0;
  child_allocation.width = width;
  child_allocation.height = height;

  if (self->visible_child) {
    int min_width;
    int min_height;

    gtk_widget_measure (self->visible_child->widget, GTK_ORIENTATION_HORIZONTAL,
                        height, &min_width, NULL, NULL, NULL);
    child_allocation.width = MAX (child_allocation.width, min_width);

    gtk_widget_measure (self->visible_child->widget, GTK_ORIENTATION_VERTICAL,
                        child_allocation.width, &min_height, NULL, NULL, NULL);
    child_allocation.height = MAX (child_allocation.height, min_height);

    if (child_allocation.width > width) {
      GtkAlign halign = gtk_widget_get_halign (self->visible_child->widget);

      if (halign == GTK_ALIGN_CENTER || halign == GTK_ALIGN_FILL)
        child_allocation.x = (width - child_allocation.width) / 2;
      else if (halign == GTK_ALIGN_END)
        child_allocation.x = (width - child_allocation.width);
    }

    if (child_allocation.height > height) {
      GtkAlign valign = gtk_widget_get_valign (self->visible_child->widget);

      if (valign == GTK_ALIGN_CENTER || valign == GTK_ALIGN_FILL)
        child_allocation.y = (height - child_allocation.height) / 2;
      else if (valign == GTK_ALIGN_END)
        child_allocation.y = (height - child_allocation.height);
    }

    gtk_widget_size_allocate (self->visible_child->widget, &child_allocation, -1);
  }
}

static void
adw_view_stack_measure (GtkWidget      *widget,
                        GtkOrientation  orientation,
                        int             for_size,
                        int            *minimum,
                        int            *natural,
                        int            *minimum_baseline,
                        int            *natural_baseline)
{
  AdwViewStack *self = ADW_VIEW_STACK (widget);
  int child_min, child_nat;
  GList *l;

  *minimum = 0;
  *natural = 0;

  for (l = self->children; l; l = l->next) {
    AdwViewStackPage *page = l->data;
    GtkWidget *child = page->widget;

    if (((orientation == GTK_ORIENTATION_VERTICAL && !self->vhomogeneous) ||
         (orientation == GTK_ORIENTATION_HORIZONTAL && !self->hhomogeneous)) &&
         self->visible_child != page)
      continue;

    if (gtk_widget_get_visible (child)) {
      gtk_widget_measure (child, orientation, for_size, &child_min, &child_nat, NULL, NULL);

      *minimum = MAX (*minimum, child_min);
      *natural = MAX (*natural, child_nat);
    }
  }

  if (self->last_visible_child) {
    if (orientation == GTK_ORIENTATION_VERTICAL && !self->vhomogeneous) {
      double t = self->interpolate_size ? gtk_progress_tracker_get_ease_out_cubic (&self->tracker, FALSE) : 0.0;
      *minimum = adw_lerp (*minimum, self->last_visible_widget_height, 1.0 - t);
      *natural = adw_lerp (*natural, self->last_visible_widget_height, 1.0 - t);
    }

    if (orientation == GTK_ORIENTATION_HORIZONTAL && !self->hhomogeneous) {
      double t = self->interpolate_size ? gtk_progress_tracker_get_ease_out_cubic (&self->tracker, FALSE) : 0.0;
      *minimum = adw_lerp (*minimum, self->last_visible_widget_width, 1.0 - t);
      *natural = adw_lerp (*natural, self->last_visible_widget_width, 1.0 - t);
    }
  }
}

static void
adw_view_stack_snapshot (GtkWidget   *widget,
                         GtkSnapshot *snapshot)
{
  AdwViewStack *self = ADW_VIEW_STACK (widget);
  double progress;

  if (!self->visible_child)
    return;

  if (gtk_progress_tracker_get_state (&self->tracker) == GTK_PROGRESS_STATE_AFTER) {
    gtk_widget_snapshot_child (widget,
                               self->visible_child->widget,
                               snapshot);

    return;
  }

  progress = gtk_progress_tracker_get_progress (&self->tracker, FALSE);

  gtk_snapshot_push_clip (snapshot,
                          &GRAPHENE_RECT_INIT(
                              0, 0,
                              gtk_widget_get_width (widget),
                              gtk_widget_get_height (widget)
                          ));
  gtk_snapshot_push_cross_fade (snapshot, progress);

  if (self->last_visible_child)
    gtk_widget_snapshot_child (widget,
                               self->last_visible_child->widget,
                               snapshot);
  gtk_snapshot_pop (snapshot);

  gtk_widget_snapshot_child (widget,
                             self->visible_child->widget,
                             snapshot);
  gtk_snapshot_pop (snapshot);

  gtk_snapshot_pop (snapshot);
}

static void
adw_view_stack_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdwViewStack *self = ADW_VIEW_STACK (object);

  switch (property_id) {
  case PROP_HHOMOGENEOUS:
    g_value_set_boolean (value, adw_view_stack_get_hhomogeneous (self));
    break;
  case PROP_VHOMOGENEOUS:
    g_value_set_boolean (value, adw_view_stack_get_vhomogeneous (self));
    break;
  case PROP_VISIBLE_CHILD:
    g_value_set_object (value, adw_view_stack_get_visible_child (self));
    break;
  case PROP_VISIBLE_CHILD_NAME:
    g_value_set_string (value, adw_view_stack_get_visible_child_name (self));
    break;
  case PROP_TRANSITION_RUNNING:
    g_value_set_boolean (value, adw_view_stack_get_transition_running (self));
    break;
  case PROP_INTERPOLATE_SIZE:
    g_value_set_boolean (value, adw_view_stack_get_interpolate_size (self));
    break;
  case PROP_PAGES:
    g_value_take_object (value, adw_view_stack_get_pages (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_view_stack_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdwViewStack *self = ADW_VIEW_STACK (object);

  switch (property_id) {
  case PROP_HHOMOGENEOUS:
    adw_view_stack_set_hhomogeneous (self, g_value_get_boolean (value));
    break;
  case PROP_VHOMOGENEOUS:
    adw_view_stack_set_vhomogeneous (self, g_value_get_boolean (value));
    break;
  case PROP_VISIBLE_CHILD:
    adw_view_stack_set_visible_child (self, g_value_get_object (value));
    break;
  case PROP_VISIBLE_CHILD_NAME:
    adw_view_stack_set_visible_child_name (self, g_value_get_string (value));
    break;
  case PROP_INTERPOLATE_SIZE:
    adw_view_stack_set_interpolate_size (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_view_stack_dispose (GObject *object)
{
  AdwViewStack *self = ADW_VIEW_STACK (object);
  GtkWidget *child;

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), 0,
                                g_list_length (self->children), 0);

  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self))))
    stack_remove (self, child, TRUE);

  G_OBJECT_CLASS (adw_view_stack_parent_class)->dispose (object);
}

static void
adw_view_stack_finalize (GObject *object)
{
  AdwViewStack *self = ADW_VIEW_STACK (object);

  if (self->pages)
    g_object_remove_weak_pointer (G_OBJECT (self->pages),
                                  (gpointer *) &self->pages);

  unschedule_ticks (self);

  G_OBJECT_CLASS (adw_view_stack_parent_class)->finalize (object);
}

static void
adw_view_stack_class_init (AdwViewStackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_view_stack_get_property;
  object_class->set_property = adw_view_stack_set_property;
  object_class->dispose = adw_view_stack_dispose;
  object_class->finalize = adw_view_stack_finalize;

  widget_class->size_allocate = adw_view_stack_size_allocate;
  widget_class->snapshot = adw_view_stack_snapshot;
  widget_class->measure = adw_view_stack_measure;
  widget_class->compute_expand = adw_view_stack_compute_expand;
  widget_class->get_request_mode = adw_view_stack_get_request_mode;

  /**
   * AdwViewStack:hhomogeneous: (attributes org.gtk.Property.get=adw_view_stack_get_hhomogeneous org.gtk.Property.set=adw_view_stack_set_hhomogeneous)
   *
   * Whether the stack allocates the same width for all children.
   *
   * If it's `FALSE`, the stack may change width when a different child becomes
   * visible.
   *
   * Since: 1.0
   */
  props[PROP_HHOMOGENEOUS] =
      g_param_spec_boolean ("hhomogeneous",
                            "Horizontally homogeneous",
                            "Whether the stack allocates the same width for all children",
                            TRUE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:vhomogeneous: (attributes org.gtk.Property.get=adw_view_stack_get_vhomogeneous org.gtk.Property.set=adw_view_stack_set_vhomogeneous)
   *
   * Whether the stack allocates the same height for all children.
   *
   * If it's `FALSE`, the stack may change height when a different child becomes
   * visible.
   *
   * Since: 1.0
   */
  props[PROP_VHOMOGENEOUS] =
      g_param_spec_boolean ("vhomogeneous",
                            "Vertically homogeneous",
                            "Whether the stack allocates the same height for all children",
                            TRUE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:visible-child: (attributes org.gtk.Property.get=adw_view_stack_get_visible_child org.gtk.Property.set=adw_view_stack_set_visible_child)
   *
   * The widget currently visible in the stack.
   *
   * Since: 1.0
   */
  props[PROP_VISIBLE_CHILD] =
      g_param_spec_object ("visible-child",
                           "Visible child",
                           "The widget currently visible in the stack",
                           GTK_TYPE_WIDGET,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:visible-child-name: (attributes org.gtk.Property.get=adw_view_stack_get_visible_child_name org.gtk.Poperty.set=adw_view_stack_set_visible_child_name)
   *
   * The name of the widget currently visible in the stack.
   *
   * See [property@Adw.ViewStack:visible-child].
   *
   * Since: 1.0
   */
  props[PROP_VISIBLE_CHILD_NAME] =
      g_param_spec_string ("visible-child-name",
                           "Name of visible child",
                           "The name of the widget currently visible in the stack",
                           NULL,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:transition-running: (attributes org.gtk.Property.get=adw_view_stack_get_transition_running)
   *
   * Whether a transition is currently running.
   *
   * Since: 1.0
   */
  props[PROP_TRANSITION_RUNNING] =
      g_param_spec_boolean ("transition-running",
                            "Transition running",
                            "Whether a transition is currently running",
                            FALSE,
                            G_PARAM_READABLE);

  /**
   * AdwViewStack:interpolate-size: (attributes org.gtk.Property.get=adw_view_stack_get_interpolate_size org.gtk.Property.set=adw_view_stack_set_interpolate_size)
   *
   * Whether the stack interpolates its size when changing the visible child.
   *
   * Since: 1.0
   */
  props[PROP_INTERPOLATE_SIZE] =
      g_param_spec_boolean ("interpolate-size",
                            "Interpolate size",
                            "Whether the stack interpolates its size when changing the visible child",
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:pages: (attributes org.gtk.Property.get=adw_view_stack_get_pages)
   *
   * A selection model with the stack's pages.
   *
   * This can be used to keep an up-to-date view. The model also implements
   * [iface@Gtk.SelectionModel] and can be used to track and change the visible
   * page.
   */
  props[PROP_PAGES] =
      g_param_spec_object ("pages",
                           "Pages",
                           "A selection model with the stack's pages",
                           GTK_TYPE_SELECTION_MODEL,
                           G_PARAM_READABLE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "stack");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_view_stack_init (AdwViewStack *self)
{
  self->vhomogeneous = TRUE;
  self->hhomogeneous = TRUE;
}

static void
adw_view_stack_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder   *builder,
                                    GObject      *child,
                                    const char   *type)
{
  AdwViewStack *self = ADW_VIEW_STACK (buildable);

  if (ADW_IS_VIEW_STACK_PAGE (child))
    add_page (self, ADW_VIEW_STACK_PAGE (child));
  else if (GTK_IS_WIDGET (child))
    add_internal (self, GTK_WIDGET (child), NULL, NULL);
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_view_stack_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_view_stack_buildable_add_child;
}

/**
 * adw_view_stack_page_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a `AdwViewStackPage`
 *
 * Gets the stack child to which @self belongs.
 *
 * Returns: (transfer none): the child to which @self belongs
 *
 * Since: 1.0
 */
GtkWidget *
adw_view_stack_page_get_child (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->widget;
}

/**
 * adw_view_stack_page_get_visible: (attributes org.gtk.Method.get_property=visible)
 * @self: a `AdwViewStackPage`
 *
 * Gets whether @self is visible in its `AdwViewStack`.
 *
 * This is independent from the [property@Gtk.Widget:visible]
 * property of its widget.
 *
 * Returns: whether @self is visible
 *
 * Since: 1.0
 */
gboolean
adw_view_stack_page_get_visible (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), FALSE);

  return self->visible;
}

/**
 * adw_view_stack_page_set_visible: (attributes org.gtk.Method.set_property=visible)
 * @self: a `AdwViewStackPage`
 * @visible: whether @self is visible
 *
 * Sets whether @page is visible in its `AdwViewStack`.
 *
 * Since: 1.0
 */
void
adw_view_stack_page_set_visible (AdwViewStackPage *self,
                                 gboolean          visible)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  visible = !!visible;

  if (visible == self->visible)
    return;

  self->visible = visible;

  if (self->widget && gtk_widget_get_parent (self->widget))
    update_child_visible (ADW_VIEW_STACK (gtk_widget_get_parent (self->widget)), self);

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_VISIBLE]);
}

/**
 * adw_view_stack_page_get_needs_attention: (attributes org.gtk.Method.get_property=needs-attention)
 * @self: a `AdwViewStackPage`
 *
 * Gets whether the page is marked as “needs attention”.
 *
 * Returns: whether the page needs attention
 *
 * Since: 1.0
 */
gboolean
adw_view_stack_page_get_needs_attention (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), FALSE);

  return self->needs_attention;
}

/**
 * adw_view_stack_page_set_needs_attention: (attributes org.gtk.Method.set_property=needs-attention)
 * @self: a `AdwViewStackPage`
 * @needs_attention: the new value to set
 *
 * Sets whether the page is marked as “needs attention”.
 *
 * Since: 1.0
 */
void
adw_view_stack_page_set_needs_attention (AdwViewStackPage *self,
                                         gboolean          needs_attention)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  needs_attention = !!needs_attention;

  if (needs_attention == self->needs_attention)
    return;

  self->needs_attention = needs_attention;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_NEEDS_ATTENTION]);
}

/**
 * adw_view_stack_page_get_badge_number: (attributes org.gtk.Method.get_property=badge-number)
 * @self: a `AdwViewStackPage`
 *
 * Gets the badge number for this page.
 *
 * Returns: the badge number for this page
 *
 * Since: 1.0
 */
guint
adw_view_stack_page_get_badge_number (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), 0);

  return self->badge_number;
}

/**
 * adw_view_stack_page_set_badge_number: (attributes org.gtk.Method.set_property=badge-number)
 * @self: a `AdwViewStackPage`
 * @badge_number: the new value to set
 *
 * Sets the badge number for this page.
 *
 * Since: 1.0
 */
void
adw_view_stack_page_set_badge_number (AdwViewStackPage *self,
                                      guint             badge_number)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  if (badge_number == self->badge_number)
    return;

  self->badge_number = badge_number;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_BADGE_NUMBER]);
}

/**
 * adw_view_stack_page_get_use_underline: (attributes org.gtk.Method.get_property=use-underline)
 * @self: a `AdwViewStackPage`
 *
 * Gets whether underlines in the page title indicate mnemonics.
 *
 * Returns: whether underlines in the page title indicate mnemonics
 *
 * Since: 1.0
 */
gboolean
adw_view_stack_page_get_use_underline (AdwViewStackPage *self)
{
  return self->use_underline;
}

/**
 * adw_view_stack_page_set_use_underline: (attributes org.gtk.Method.set_property=use-underline)
 * @self: a `AdwViewStackPage`
 * @use_underline: the new value to set
 *
 * Sets whether underlines in the page title indicate mnemonics.
 *
 * Since: 1.0
 */
void
adw_view_stack_page_set_use_underline (AdwViewStackPage *self,
                                       gboolean          use_underline)
{
  use_underline = !!use_underline;

  if (use_underline == self->use_underline)
    return;

  self->use_underline = use_underline;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_USE_UNDERLINE]);
}

/**
 * adw_view_stack_page_get_name: (attributes org.gtk.Method.get_property=name)
 * @self: a `AdwViewStackPage`
 *
 * Gets the name of the page.
 *
 * Returns: (nullable): the name of the page
 *
 * Since: 1.0
 */
const char *
adw_view_stack_page_get_name (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->name;
}

/**
 * adw_view_stack_page_set_name: (attributes org.gtk.Method.set_property=name)
 * @self: a `AdwViewStackPage`
 * @name: (nullable): the page name
 *
 * Sets the name of the page.
 *
 * Since: 1.0
 */
void
adw_view_stack_page_set_name (AdwViewStackPage *self,
                              const char       *name)
{
  AdwViewStack *stack = NULL;

  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  if (self->widget &&
      gtk_widget_get_parent (self->widget) &&
      ADW_IS_VIEW_STACK (gtk_widget_get_parent (self->widget))) {
    GList *l;

    stack = ADW_VIEW_STACK (gtk_widget_get_parent (self->widget));

    for (l = stack->children; l; l = l->next) {
      AdwViewStackPage *p = l->data;
      if (self == p)
        continue;

      if (g_strcmp0 (p->name, name) == 0) {
        g_warning ("Duplicate child name in AdwViewStack: %s", name);
        break;
      }
    }
  }

  if (name == self->name)
    return;

  g_free (self->name);
  self->name = g_strdup (name);

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_NAME]);

  if (stack && stack->visible_child == self)
    g_object_notify_by_pspec (G_OBJECT (stack),
                              props[PROP_VISIBLE_CHILD_NAME]);
}

/**
 * adw_view_stack_page_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a `AdwViewStackPage`
 *
 * Gets the page title.
 *
 * Returns: (nullable): the page title
 *
 * Since: 1.0
 */
const char *
adw_view_stack_page_get_title (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->title;
}

/**
 * adw_view_stack_page_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a `AdwViewStackPage`
 * @title: (nullable): the page title
 *
 * Sets the page title.
 *
 * Since: 1.0
 */
void
adw_view_stack_page_set_title (AdwViewStackPage *self,
                               const char       *title)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  if (title == self->title)
    return;

  g_free (self->title);
  self->title = g_strdup (title);

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TITLE]);
}

/**
 * adw_view_stack_page_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: a `AdwViewStackPage`
 *
 * Gets the icon name of the page.
 *
 * Returns: (nullable): the icon name of the page
 *
 * Since: 1.0
 */
const char *
adw_view_stack_page_get_icon_name (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->icon_name;
}

/**
 * adw_view_stack_page_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: a `AdwViewStackPage`
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name of the page.
 *
 * Since: 1.0
 */
void
adw_view_stack_page_set_icon_name (AdwViewStackPage *self,
                                   const char       *icon_name)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  if (icon_name == self->icon_name)
    return;

  g_free (self->icon_name);
  self->icon_name = g_strdup (icon_name);

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_ICON_NAME]);
}

/**
 * adw_view_stack_new:
 *
 * Creates a new `AdwViewStack`.
 *
 * Returns: the newly created `AdwViewStack`
 *
 * Since: 1.0
 */
GtkWidget *
adw_view_stack_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_STACK, NULL);
}

/**
 * adw_view_stack_add:
 * @self: a `AdwViewStack`
 * @child: the widget to add
 *
 * Adds a child to @self.
 *
 * Returns: (transfer none): the [class@Adw.ViewStackPage] for @child
 *
 * Since: 1.0
 */
AdwViewStackPage *
adw_view_stack_add (AdwViewStack   *self,
                    GtkWidget      *child)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  return add_internal (self, child, NULL, NULL);
}

/**
 * adw_view_stack_add_named:
 * @self: a `AdwViewStack`
 * @child: the widget to add
 * @name: (nullable): the name for @child
 *
 * Adds a child to @self.
 *
 * The child is identified by the @name.
 *
 * Returns: (transfer none): the `AdwViewStackPage` for @child
 *
 * Since: 1.0
 */
AdwViewStackPage *
adw_view_stack_add_named (AdwViewStack   *self,
                          GtkWidget      *child,
                          const char     *name)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  return add_internal (self, child, name, NULL);
}

/**
 * adw_view_stack_add_titled:
 * @self: a `AdwViewStack`
 * @child: the widget to add
 * @name: (nullable): the name for @child
 * @title: a human-readable title for @child
 *
 * Adds a child to @self.
 *
 * The child is identified by the @name. The @title will be used by
 * [class@Adw.ViewSwitcher] to represent @child, so it should be short.
 *
 * Returns: (transfer none): the `AdwViewStackPage` for @child
 *
 * Since: 1.0
 */
AdwViewStackPage *
adw_view_stack_add_titled (AdwViewStack   *self,
                           GtkWidget      *child,
                           const char     *name,
                           const char     *title)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  return add_internal (self, child, name, title);
}

/**
 * adw_view_stack_remove:
 * @self: a `AdwViewStack`
 * @child: the child to remove
 *
 * Removes a child widget from @self.
 *
 * Since: 1.0
 */
void
adw_view_stack_remove (AdwViewStack  *self,
                       GtkWidget     *child)
{
  GList *l;
  guint position;

  g_return_if_fail (ADW_IS_VIEW_STACK (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (self));

  for (l = self->children, position = 0; l; l = l->next, position++) {
    AdwViewStackPage *page = l->data;

    if (page->widget == child)
      break;
  }

  stack_remove (self, child, FALSE);

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 1, 0);
}

/**
 * adw_view_stack_get_page:
 * @self: a `AdwViewStack`
 * @child: a child of @self
 *
 * Gets the [class@Adw.ViewStackPage] object for @child.
 *
 * Returns: (transfer none): the page object for @child
 *
 * Since: 1.0
 */
AdwViewStackPage *
adw_view_stack_get_page (AdwViewStack  *self,
                         GtkWidget     *child)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  return find_page_for_widget (self, child);
}

/**
 * adw_view_stack_get_child_by_name:
 * @self: a `AdwViewStack`
 * @name: the name of the child to find
 *
 * Finds the child with @name in @self.
 *
 * Returns: (transfer none) (nullable): the requested child
 *
 * Since: 1.0
 */
GtkWidget *
adw_view_stack_get_child_by_name (AdwViewStack *self,
                                  const char   *name)
{
  AdwViewStackPage *page;

  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  page = find_page_for_name (self, name);

  return page ? page->widget : NULL;
}

/**
 * adw_view_stack_get_visible_child: (attributes org.gtk.Method.get_property=visible-child)
 * @self: a `AdwViewStack`
 *
 * Gets the currently visible child of @self, .
 *
 * Returns: (transfer none) (nullable): the visible child
 *
 * Since: 1.0
 */
GtkWidget *
adw_view_stack_get_visible_child (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);

  return self->visible_child ? self->visible_child->widget : NULL;
}

/**
 * adw_view_stack_set_visible_child: (attributes org.gtk.Method.set_property=visible-child)
 * @self: a `AdwViewStack`
 * @child: a child of @self
 *
 * Makes @child the visible child of @self.
 *
 * Since: 1.0
 */
void
adw_view_stack_set_visible_child (AdwViewStack *self,
                                  GtkWidget    *child)
{
  AdwViewStackPage *page;

  g_return_if_fail (ADW_IS_VIEW_STACK (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  page = find_page_for_widget (self, child);
  if (!page) {
    g_warning ("Given child of type '%s' not found in AdwViewStack",
               G_OBJECT_TYPE_NAME (child));

    return;
  }

  if (gtk_widget_get_visible (page->widget))
    set_visible_child (self, page);
}

/**
 * adw_view_stack_get_visible_child_name: (attributes org.gtk.Method.get_property=visible-child-name)
 * @self: a `AdwViewStack`
 *
 * Returns the name of the currently visible child of @self.
 *
 * Returns: (transfer none) (nullable): the name of the visible child
 *
 * Since: 1.0
 */
const char *
adw_view_stack_get_visible_child_name (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);

  return self->visible_child ? self->visible_child->name : NULL;
}

/**
 * adw_view_stack_set_visible_child_name: (attributes org.gtk.Method.set_property=visible-child-name)
 * @self: a `AdwViewStack`
 * @name: the name of the child
 *
 * Makes the child with @name visible.
 *
 * Since: 1.0
 */
void
adw_view_stack_set_visible_child_name (AdwViewStack *self,
                                       const char   *name)
{
  AdwViewStackPage *page;

  g_return_if_fail (ADW_IS_VIEW_STACK (self));

  if (name == NULL)
    return;

  page = find_page_for_name (self, name);

  if (page == NULL) {
    g_warning ("Child name '%s' not found in AdwViewStack", name);

    return;
  }

  if (gtk_widget_get_visible (page->widget))
    set_visible_child (self, page);
}

/**
 * adw_view_stack_set_hhomogeneous: (attributes org.gtk.Method.set_property=hhomogeneous)
 * @self: a `AdwViewStack`
 * @hhomogeneous: whether to make @self horizontally homogeneous
 *
 * Sets @self to be horizontally homogeneous or not.
 *
 * Since: 1.0
 */
void
adw_view_stack_set_hhomogeneous (AdwViewStack *self,
                                 gboolean      hhomogeneous)
{
  g_return_if_fail (ADW_IS_VIEW_STACK (self));

  hhomogeneous = !!hhomogeneous;

  if (self->hhomogeneous == hhomogeneous)
    return;

  self->hhomogeneous = hhomogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET (self)))
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HHOMOGENEOUS]);
}

/**
 * adw_view_stack_get_hhomogeneous: (attributes org.gtk.Method.get_property=hhomogeneous)
 * @self: a `AdwViewStack`
 *
 * Gets whether @self is horizontally homogeneous.
 *
 * Returns: whether @self is horizontally homogeneous
 *
 * Since: 1.0
 */
gboolean
adw_view_stack_get_hhomogeneous (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), FALSE);

  return self->hhomogeneous;
}

/**
 * adw_view_stack_set_vhomogeneous: (attributes org.gtk.Method.set_property=vhomogeneous)
 * @self: a `AdwViewStack`
 * @vhomogeneous: whether to make @self vertically homogeneous
 *
 * Sets @self to be vertically homogeneous or not.
 *
 * Since: 1.0
 */
void
adw_view_stack_set_vhomogeneous (AdwViewStack *self,
                                 gboolean      vhomogeneous)
{
  g_return_if_fail (ADW_IS_VIEW_STACK (self));

  vhomogeneous = !!vhomogeneous;

  if (self->vhomogeneous == vhomogeneous)
    return;

  self->vhomogeneous = vhomogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET (self)))
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VHOMOGENEOUS]);
}

/**
 * adw_view_stack_get_vhomogeneous: (attributes org.gtk.Method.get_property=vhomogeneous)
 * @self: a `AdwViewStack`
 *
 * Gets whether @self is vertically homogeneous.
 *
 * Returns: whether @self is vertically homogeneous
 *
 * Since: 1.0
 */
gboolean
adw_view_stack_get_vhomogeneous (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), FALSE);

  return self->vhomogeneous;
}

/**
 * adw_view_stack_get_transition_running: (attributes org.gtk.Method.get_property=transition-running)
 * @self: a `AdwViewStack`
 *
 * Gets whether the @self is currently in a transition from one page to another.
 *
 * Returns: whether a transition is currently running
 *
 * Since: 1.0
 */
gboolean
adw_view_stack_get_transition_running (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), FALSE);

  return (self->tick_id != 0);
}

/**
 * adw_view_stack_set_interpolate_size: (attributes org.gtk.Method.set_property=interpolate-size)
 * @self: A `AdwViewStack`
 * @interpolate_size: the new value
 *
 * Sets whether @self will interpolate its size when changing the visible child.
 *
 * Since: 1.0
 */
void
adw_view_stack_set_interpolate_size (AdwViewStack *self,
                                     gboolean      interpolate_size)
{
  g_return_if_fail (ADW_IS_VIEW_STACK (self));

  interpolate_size = !!interpolate_size;

  if (self->interpolate_size == interpolate_size)
    return;

  self->interpolate_size = interpolate_size;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_INTERPOLATE_SIZE]);
}

/**
 * adw_view_stack_get_interpolate_size: (attributes org.gtk.Method.get_property=interpolate-size)
 * @self: A `AdwViewStack`
 *
 * Gets whether @self will interpolate its size when changing the visible child.
 *
 * Returns: whether child sizes are interpolated
 *
 * Since: 1.0
 */
gboolean
adw_view_stack_get_interpolate_size (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), FALSE);

  return self->interpolate_size;
}

/**
 * adw_view_stack_get_pages: (attributes org.gtk.Method.get_property=pages)
 * @self: a `AdwViewStack`
 *
 * Returns a `GListModel` that contains the pages of the stack.
 *
 * This can be used to keep an up-to-date view. The model also implements
 * [iface@Gtk.SelectionModel] and can be used to track and change the visible
 * page.
 *
 * Returns: (transfer full): a `GtkSelectionModel` for the stack's children
 *
 * Since: 1.0
 */
GtkSelectionModel *
adw_view_stack_get_pages (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);

  if (self->pages)
    return g_object_ref (self->pages);

  self->pages = GTK_SELECTION_MODEL (adw_view_stack_pages_new (self));
  g_object_add_weak_pointer (G_OBJECT (self->pages), (gpointer *) &self->pages);

  return self->pages;
}
