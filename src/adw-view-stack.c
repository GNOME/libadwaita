/*
 * Copyright (C) 2013 Red Hat, Inc.
 * Copyright (C) 2021 Frederick Schenk
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Based on gtkstack.c
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/ba44668478b7184bec02609f292691b85a2c6cdd/gtk/gtkstack.c
 */

#include "config.h"

#include "adw-view-stack.h"

#include "adw-animation-util.h"
#include "adw-gizmo-private.h"
#include "adw-timed-animation.h"
#include "adw-widget-utils-private.h"

/**
 * AdwViewStack:
 *
 * A view container for [class@ViewSwitcher].
 *
 * `AdwViewStack` is a container which only shows one page at a time.
 * It is typically used to hold an application's main views.
 *
 * It doesn't provide a way to transition between pages.
 * Instead, a separate widget such as [class@ViewSwitcher],
 * [class@InlineViewSwitcher] or [class@ViewSwitcherSidebar] can be used with
 * `AdwViewStack` to provide this functionality.
 *
 * `AdwViewStack` pages can have a title, an icon, an attention request, and a
 * numbered badge that [class@ViewSwitcher] will use to let users identify which
 * page is which. Set them using the [property@ViewStackPage:title],
 * [property@ViewStackPage:icon-name],
 * [property@ViewStackPage:needs-attention], and
 * [property@ViewStackPage:badge-number] properties.
 *
 * `AdwViewStack` pages can also be grouped into sections, using the
 * [property@ViewStackPage:starts-section] and
 * [property@ViewStackPage:section-title] properties. Currently, only
 * [class@ViewSwitcherSidebar] displays groups.
 *
 * Unlike [class@Gtk.Stack], transitions between views can only be animated via
 * a crossfade and size changes are always interpolated. Animations are disabled
 * by default. Use [property@ViewStack:enable-transitions] to enable them.
 *
 * `AdwViewStack` maintains a [class@ViewStackPage] object for each added child,
 * which holds additional per-child properties. You obtain the
 * [class@ViewStackPage] for a child with [method@ViewStack.get_page] and you
 * can obtain a [iface@Gtk.SelectionModel] containing all the pages with
 * [method@ViewStack.get_pages].
 *
 * ## AdwViewStack as GtkBuildable
 *
 * To set child-specific properties in a .ui file, create
 * [class@ViewStackPage] objects explicitly, and set the child widget as a
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
 *   </object>
 * ```
 *
 * ## CSS nodes
 *
 * `AdwViewStack` has a single CSS node named `stack`.
 *
 * ## Accessibility
 *
 * `AdwViewStack` uses the `GTK_ACCESSIBLE_ROLE_TAB_PANEL` for the stack pages
 * which are the accessible parent objects of the child widgets.
 */

/**
 * AdwViewStackPage:
 *
 * An auxiliary class used by [class@ViewStack].
 */

/**
 * AdwViewStackPages:
 *
 * An auxiliary class used by [class@ViewStack].
 *
 * See [property@ViewStack:pages].
 *
 * Since: 1.4
 */

#define OPPOSITE_ORIENTATION(_orientation) (1 - (_orientation))

enum {
  PROP_0,
  PROP_HHOMOGENEOUS,
  PROP_VHOMOGENEOUS,
  PROP_VISIBLE_CHILD,
  PROP_VISIBLE_CHILD_NAME,
  PROP_ENABLE_TRANSITIONS,
  PROP_TRANSITION_DURATION,
  PROP_TRANSITION_RUNNING,
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

  GtkATContext *at_context;
  AdwViewStackPage *next_page;

  gboolean needs_attention;
  gboolean visible;
  gboolean use_underline;
  gboolean in_destruction;

  gboolean starts_section;
  char *section_title;
};

static void adw_view_stack_page_accessible_init (GtkAccessibleInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwViewStackPage, adw_view_stack_page, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE, adw_view_stack_page_accessible_init))

enum {
  PAGE_PROP_0,
  PAGE_PROP_CHILD,
  PAGE_PROP_NAME,
  PAGE_PROP_TITLE,
  PAGE_PROP_USE_UNDERLINE,
  PAGE_PROP_ICON_NAME,
  PAGE_PROP_NEEDS_ATTENTION,
  PAGE_PROP_BADGE_NUMBER,
  PAGE_PROP_VISIBLE,
  PAGE_PROP_STARTS_SECTION,
  PAGE_PROP_SECTION_TITLE,
  LAST_PAGE_PROP,
  PAGE_PROP_ACCESSIBLE_ROLE
};

static GParamSpec *page_props[LAST_PAGE_PROP];

struct _AdwViewStackPages
{
  GObject parent_instance;
  AdwViewStack *stack;
};

enum {
  PAGES_PROP_0,
  PAGES_PROP_SELECTED_PAGE,
  N_PAGES_PROPS,
};

static GParamSpec *pages_props[N_PAGES_PROPS];

struct _AdwViewStack {
  GtkWidget parent_instance;

  GList *children;

  AdwViewStackPage *visible_child;

  gboolean homogeneous[2];

  gboolean enable_transitions;
  guint transition_duration;
  AdwViewStackPage *last_visible_child;
  gboolean transition_running;
  AdwAnimation *animation;

  int last_visible_widget_width;
  int last_visible_widget_height;

  GtkSelectionModel *pages;
};

static GParamSpec *props[LAST_PROP];

static void adw_view_stack_buildable_init (GtkBuildableIface *iface);
static void adw_view_stack_accessible_init (GtkAccessibleInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwViewStack, adw_view_stack, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_view_stack_buildable_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE, adw_view_stack_accessible_init))

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
  case PAGE_PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adw_view_stack_page_get_use_underline (self));
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
  case PAGE_PROP_STARTS_SECTION:
    g_value_set_boolean (value, adw_view_stack_page_get_starts_section (self));
    break;
  case PAGE_PROP_SECTION_TITLE:
    g_value_set_string (value, adw_view_stack_page_get_section_title (self));
    break;
  case PAGE_PROP_ACCESSIBLE_ROLE:
    g_value_set_enum (value, GTK_ACCESSIBLE_ROLE_TAB_PANEL);
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
    if (self->widget)
      gtk_accessible_set_accessible_parent (GTK_ACCESSIBLE (self->widget),
                                            GTK_ACCESSIBLE (self), NULL);
    break;
  case PAGE_PROP_NAME:
    adw_view_stack_page_set_name (self, g_value_get_string (value));
    break;
  case PAGE_PROP_TITLE:
    adw_view_stack_page_set_title (self, g_value_get_string (value));
    break;
  case PAGE_PROP_USE_UNDERLINE:
    adw_view_stack_page_set_use_underline (self, g_value_get_boolean (value));
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
  case PAGE_PROP_STARTS_SECTION:
    adw_view_stack_page_set_starts_section (self, g_value_get_boolean (value));
    break;
  case PAGE_PROP_SECTION_TITLE:
    adw_view_stack_page_set_section_title (self, g_value_get_string (value));
    break;
  case PAGE_PROP_ACCESSIBLE_ROLE:
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_view_stack_page_dispose (GObject *object)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (object);

  self->in_destruction = TRUE;

  g_clear_object (&self->at_context);

  G_OBJECT_CLASS (adw_view_stack_page_parent_class)->dispose (object);
}

static void
adw_view_stack_page_finalize (GObject *object)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (object);

  g_clear_object (&self->widget);
  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->icon_name, g_free);
  g_clear_pointer (&self->section_title, g_free);
  g_clear_weak_pointer (&self->last_focus);

  G_OBJECT_CLASS (adw_view_stack_page_parent_class)->finalize (object);
}

static void
adw_view_stack_page_class_init (AdwViewStackPageClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->dispose = adw_view_stack_page_dispose;
  object_class->finalize = adw_view_stack_page_finalize;
  object_class->get_property = adw_view_stack_page_get_property;
  object_class->set_property = adw_view_stack_page_set_property;

  /**
   * AdwViewStackPage:child:
   *
   * The stack child to which the page belongs.
   */
  page_props[PAGE_PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * AdwViewStackPage:name:
   *
   * The name of the child page.
   */
  page_props[PAGE_PROP_NAME] =
    g_param_spec_string ("name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:title:
   *
   * The title of the child page.
   */
  page_props[PAGE_PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:use-underline:
   *
   * Whether an embedded underline in the title indicates a mnemonic.
   */
  page_props[PAGE_PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:icon-name:
   *
   * The icon name of the child page.
   */
  page_props[PAGE_PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:needs-attention:
   *
   * Whether the page requires the user attention.
   *
   * [class@ViewSwitcher] will display it as a dot next to the page icon.
   */
  page_props[PAGE_PROP_NEEDS_ATTENTION] =
    g_param_spec_boolean ("needs-attention", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:badge-number:
   *
   * The badge number for this page.
   *
   * [class@ViewSwitcher] can display it as a badge next to the page icon. It is
   * commonly used to display a number of unread items within the page.
   *
   * It can be used together with [property@ViewStack{age}:needs-attention].
   */
  page_props[PAGE_PROP_BADGE_NUMBER] =
    g_param_spec_uint ("badge-number", NULL, NULL,
                       0, G_MAXUINT, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:visible:
   *
   * Whether this page is visible.
   *
   * This is independent from the [property@Gtk.Widget:visible] property of
   * [property@ViewStackPage:child].
   */
  page_props[PAGE_PROP_VISIBLE] =
    g_param_spec_boolean ("visible", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:starts-section:
   *
   * Whether this page starts a section.
   *
   * If set to `TRUE`, [property@ViewStack:pages] will have a section starting
   * from this page.
   *
   * If [property@ViewStackPage:section-title] is set, it should be used as a
   * title for the section.
   *
   * Since: 1.9
   */
  page_props[PAGE_PROP_STARTS_SECTION] =
    g_param_spec_boolean ("starts-section", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStackPage:section-title:
   *
   * Section title for this page.
   *
   * Does nothing unless [property@ViewStackPage:starts-section] is set.
   *
   * Since: 1.9
   */
  page_props[PAGE_PROP_SECTION_TITLE] =
    g_param_spec_string ("section-title", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PAGE_PROP, page_props);

  g_object_class_override_property (object_class, PAGE_PROP_ACCESSIBLE_ROLE, "accessible-role");
}

static void
adw_view_stack_page_init (AdwViewStackPage *self)
{
  self->visible = TRUE;
}

static GtkATContext *
adw_view_stack_page_accessible_get_at_context (GtkAccessible *accessible)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (accessible);

  if (self->in_destruction)
    return NULL;

  if (self->at_context == NULL) {
    GtkAccessibleRole role = GTK_ACCESSIBLE_ROLE_TAB_PANEL;
    GdkDisplay *display;

    if (self->widget != NULL)
      display = gtk_widget_get_display (self->widget);
    else
      display = gdk_display_get_default ();

    self->at_context = gtk_at_context_create (role, accessible, display);

    if (self->at_context == NULL)
      return NULL;
  }

  return g_object_ref (self->at_context);
}

static gboolean
adw_view_stack_page_accessible_get_platform_state (GtkAccessible              *self,
                                                   GtkAccessiblePlatformState  state)
{
  return FALSE;
}

static GtkAccessible *
adw_view_stack_page_accessible_get_accessible_parent (GtkAccessible *accessible)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (accessible);
  GtkWidget *parent;

  if (!self->widget)
    return NULL;

  parent = gtk_widget_get_parent (self->widget);

  return GTK_ACCESSIBLE (g_object_ref (parent));
}

static GtkAccessible *
adw_view_stack_page_accessible_get_first_accessible_child (GtkAccessible *accessible)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (accessible);

  if (!self->widget)
    return NULL;

  return GTK_ACCESSIBLE (g_object_ref (self->widget));
}

static GtkAccessible *
adw_view_stack_page_accessible_get_next_accessible_sibling (GtkAccessible *accessible)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (accessible);

  if (!self->next_page)
    return NULL;

  return GTK_ACCESSIBLE (g_object_ref (self->next_page));
}

static gboolean
adw_view_stack_page_accessible_get_bounds (GtkAccessible *accessible,
                                           int           *x,
                                           int           *y,
                                           int           *width,
                                           int           *height)
{
  AdwViewStackPage *self = ADW_VIEW_STACK_PAGE (accessible);

  if (self->widget)
    return gtk_accessible_get_bounds (GTK_ACCESSIBLE (self->widget), x, y, width, height);

  return FALSE;
}

static void
adw_view_stack_page_accessible_init (GtkAccessibleInterface *iface)
{
  iface->get_at_context = adw_view_stack_page_accessible_get_at_context;
  iface->get_platform_state = adw_view_stack_page_accessible_get_platform_state;
  iface->get_accessible_parent = adw_view_stack_page_accessible_get_accessible_parent;
  iface->get_first_accessible_child = adw_view_stack_page_accessible_get_first_accessible_child;
  iface->get_next_accessible_sibling = adw_view_stack_page_accessible_get_next_accessible_sibling;
  iface->get_bounds = adw_view_stack_page_accessible_get_bounds;
}

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

static void
adw_view_stack_pages_get_section (GtkSectionModel *model,
                                  guint            position,
                                  guint           *out_start,
                                  guint           *out_end)
{
  AdwViewStackPages *self = ADW_VIEW_STACK_PAGES (model);
  GList *l = g_list_nth (self->stack->children, position);
  guint start, end;

  if (!l->data) {
    start = 0;
    end = g_list_model_get_n_items (G_LIST_MODEL (model));
  } else {
    GList *start_iter = l, *end_iter = l->next;

    start = position;
    end = position + 1;

    while (start > 0 && start_iter && start_iter->data) {
      AdwViewStackPage *page = start_iter->data;

      if (adw_view_stack_page_get_starts_section (page))
        break;

      start--;
      start_iter = start_iter->prev;
    }

    while (end_iter && end_iter->data) {
      AdwViewStackPage *page = end_iter->data;

      if (adw_view_stack_page_get_starts_section (page))
        break;

      end++;
      end_iter = end_iter->next;
    }
  }

  if (out_start)
    *out_start = start;
  if (out_end)
    *out_end = end;
}

static void
adw_view_stack_pages_section_model_init (GtkSectionModelInterface *iface)
{
  iface->get_section = adw_view_stack_pages_get_section;
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

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwViewStackPages, adw_view_stack_pages, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_view_stack_pages_list_model_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SECTION_MODEL, adw_view_stack_pages_section_model_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, adw_view_stack_pages_selection_model_init))

/**
 * adw_view_stack_pages_get_selected_page:
 * @self: a [class@ViewStackPages]
 *
 * Gets the [class@ViewStackPage] for the visible child of a view stack
 *
 * Gets the [class@ViewStackPage] for the visible child of the associated stack.
 *
 * Returns `NULL` if there's no selected page.
 *
 * Returns: (transfer none) (nullable): the stack page
 *
 * Since: 1.4
 */
AdwViewStackPage *
adw_view_stack_pages_get_selected_page (AdwViewStackPages *self)
{
  GtkWidget *child;

  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGES (self), NULL);

  if (self->stack == NULL)
    return NULL;

  if ((child = adw_view_stack_get_visible_child (self->stack)))
    return adw_view_stack_get_page (self->stack, child);

  return NULL;
}

/**
 * adw_view_stack_pages_set_selected_page:
 * @self: a [class@ViewStackPages]
 * @page: a stack page within the associated stack
 *
 * Sets the visible child in the associated [class@ViewStack].
 *
 * See [property@ViewStack:visible-child].
 *
 * Since: 1.4
 */
void
adw_view_stack_pages_set_selected_page (AdwViewStackPages *self,
                                        AdwViewStackPage  *page)
{
  GtkWidget *child = NULL;

  g_return_if_fail (ADW_IS_VIEW_STACK_PAGES (self));
  g_return_if_fail (!page || ADW_IS_VIEW_STACK_PAGE (page));

  if (self->stack == NULL)
    return;

  if (page == adw_view_stack_pages_get_selected_page (self))
    return;

  if (page != NULL)
    child = adw_view_stack_page_get_child (page);

  adw_view_stack_set_visible_child (self->stack, child);
}

static void
adw_view_stack_pages_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  AdwViewStackPages *self = ADW_VIEW_STACK_PAGES (object);

  switch (prop_id) {
  case PAGES_PROP_SELECTED_PAGE:
    g_value_set_object (value, adw_view_stack_pages_get_selected_page (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_view_stack_pages_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  AdwViewStackPages *self = ADW_VIEW_STACK_PAGES (object);

  switch (prop_id) {
  case PAGES_PROP_SELECTED_PAGE:
    adw_view_stack_pages_set_selected_page (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_view_stack_pages_init (AdwViewStackPages *pages)
{
}

static void
adw_view_stack_pages_class_init (AdwViewStackPagesClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->get_property = adw_view_stack_pages_get_property;
  object_class->set_property = adw_view_stack_pages_set_property;

  /**
   * AdwViewStackPages:selected-page:
   *
   * The selected [class@ViewStackPage] within the [class@ViewStackPages].
   *
   * This can be used to keep an up-to-date view of the [class@ViewStackPage] for
   * The visible [class@ViewStackPage] within the associated [class@ViewStackPages].
   *
   * This can be used to keep an up-to-date view of the visible child.
   *
   * Since: 1.4
   */
  pages_props[PAGES_PROP_SELECTED_PAGE] =
    g_param_spec_object ("selected-page", NULL, NULL,
                         ADW_TYPE_VIEW_STACK_PAGE,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PAGES_PROPS, pages_props);
}

static AdwViewStackPages *
adw_view_stack_pages_new (AdwViewStack *stack)
{
  AdwViewStackPages *pages;

  pages = g_object_new (ADW_TYPE_VIEW_STACK_PAGES, NULL);
  pages->stack = stack;

  return pages;
}

static GList *
find_link_for_widget (AdwViewStack *self,
                      GtkWidget    *child)
{
  AdwViewStackPage *page;
  GList *l;

  for (l = self->children; l; l = l->next) {
    page = l->data;

    if (page->widget == child)
      return l;
  }

  return NULL;
}

static AdwViewStackPage *
find_page_for_widget (AdwViewStack *self,
                      GtkWidget    *child)
{
  GList *l = find_link_for_widget (self, child);

  if (l)
    return l->data;

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
set_transition_running (AdwViewStack *self,
                        gboolean      running)
{
  if (self->transition_running == running)
    return;

  self->transition_running = running;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);
}

static void
transition_cb (double        value,
               AdwViewStack *self)
{
  if (self->homogeneous[GTK_ORIENTATION_HORIZONTAL] &&
      self->homogeneous[GTK_ORIENTATION_VERTICAL]) {
    gtk_widget_queue_draw (GTK_WIDGET (self));
  } else {
    gtk_widget_queue_resize (GTK_WIDGET (self));
  }
}

static void
transition_done_cb (AdwViewStack *self)
{
  if (self->last_visible_child) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }

  adw_animation_reset (self->animation);

  set_transition_running (self, FALSE);
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

      if (gtk_widget_get_visible (p->widget)) {
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

    g_set_weak_pointer (&self->visible_child->last_focus, focus);
  }

  if (self->transition_running)
    adw_animation_skip (self->animation);

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

  if (self->homogeneous[GTK_ORIENTATION_HORIZONTAL] && self->homogeneous[GTK_ORIENTATION_VERTICAL])
    gtk_widget_queue_allocate (widget);
  else
    gtk_widget_queue_resize (widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD_NAME]);

  if (self->pages != NULL)
    g_object_notify_by_pspec (G_OBJECT (self->pages), pages_props[PAGES_PROP_SELECTED_PAGE]);

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

  set_transition_running (self, TRUE);
  adw_animation_play (self->animation);
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

  gtk_accessible_update_state (GTK_ACCESSIBLE (page),
                               GTK_ACCESSIBLE_STATE_HIDDEN, !visible,
                               -1);
}

static void
section_changed (AdwViewStack     *self,
                 AdwViewStackPage *page)
{
  if (self->pages) {
    guint n_pages = g_list_model_get_n_items (G_LIST_MODEL (self->pages));

    gtk_section_model_sections_changed (GTK_SECTION_MODEL (self->pages), 0, n_pages);
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

  if (self->children) {
    AdwViewStackPage *prev_last = g_list_last (self->children)->data;

    prev_last->next_page = page;
  } else {
    page->next_page = NULL;
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

  if (self->homogeneous[GTK_ORIENTATION_HORIZONTAL] || self->homogeneous[GTK_ORIENTATION_VERTICAL] || self->visible_child == page)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static AdwViewStackPage *
add_internal (AdwViewStack *self,
              GtkWidget    *child,
              const char   *name,
              const char   *title,
              const char   *icon_name)
{
  AdwViewStackPage *page;

  g_return_val_if_fail (child != NULL, NULL);

  page = g_object_new (ADW_TYPE_VIEW_STACK_PAGE, NULL);
  page->widget = g_object_ref (child);
  page->name = g_strdup (name);
  page->title = g_strdup (title);
  page->icon_name = g_strdup (icon_name);
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
  GList *l;

  l = find_link_for_widget (self, child);
  if (!l)
    return;

  page = l->data;

  g_signal_handlers_disconnect_by_func (child,
                                        stack_child_visibility_notify_cb,
                                        self);

  was_visible = gtk_widget_get_visible (child);

  if (self->visible_child == page)
    self->visible_child = NULL;

  gtk_widget_unparent (child);

  g_clear_object (&page->widget);

  l = l->prev;

  self->children = g_list_remove (self->children, page);

  if (l) {
    AdwViewStackPage *prev_page = l->data;

    prev_page->next_page = page->next_page;
  }

  g_object_unref (page);

  if (!in_dispose &&
      (self->homogeneous[GTK_ORIENTATION_HORIZONTAL] || self->homogeneous[GTK_ORIENTATION_VERTICAL]) &&
      was_visible)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
adw_view_stack_snapshot (GtkWidget   *widget,
                         GtkSnapshot *snapshot)
{
  AdwViewStack *self = ADW_VIEW_STACK (widget);

  if (!self->visible_child)
    return;

  if (adw_animation_get_state (self->animation) == ADW_ANIMATION_PLAYING) {
    double t = adw_animation_get_value (self->animation);

    gtk_snapshot_push_cross_fade (snapshot, t);

    if (self->last_visible_child)
      gtk_widget_snapshot_child (widget, self->last_visible_child->widget, snapshot);
    gtk_snapshot_pop (snapshot);

    gtk_widget_snapshot_child (widget, self->visible_child->widget, snapshot);
    gtk_snapshot_pop (snapshot);

    return;
  }

  gtk_widget_snapshot_child (widget, self->visible_child->widget, snapshot);
}

static void
adw_view_stack_size_allocate (GtkWidget *widget,
                              int        width,
                              int        height,
                              int        baseline)
{
  AdwViewStack *self = ADW_VIEW_STACK (widget);

  if (self->last_visible_child) {
    GtkAllocation child_allocation;

    child_allocation.x = 0;
    child_allocation.y = 0;
    child_allocation.width = width;
    child_allocation.height = height;

    adw_ensure_child_allocation_size (self->last_visible_child->widget, &child_allocation);

    gtk_widget_size_allocate (self->last_visible_child->widget, &child_allocation, -1);
  }

  if (self->visible_child) {
    GtkAllocation child_allocation;

    child_allocation.x = 0;
    child_allocation.y = 0;
    child_allocation.width = width;
    child_allocation.height = height;

    adw_ensure_child_allocation_size (self->visible_child->widget, &child_allocation);

    gtk_widget_size_allocate (self->visible_child->widget, &child_allocation, -1);
  }
}

/* Given adw_lerp (a, b, t) -> r, find b. In other words: we know what size
 * we're interpolating *from*, the current size, and the current interpolation
 * progress; guess which size we're interpolating *to*.
 *
 * Note that this is different from the similarly named inverse_lerp function
 * in adw-clamp-layout.c. That one finds t given a, b, and r, while this one
 * finds b given a, r, and t.
 */
static inline double
inverse_lerp (int a,
              int r,
              double t)
{
  return (r - a * (1.0 - t)) / t;
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
  int child_min, child_nat, child_for_size;
  int last_size, last_opposite_size;
  double t;
  GList *l;

  if (self->last_visible_child != NULL && !self->homogeneous[orientation]) {
    t = adw_animation_get_value (self->animation);

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      last_size = self->last_visible_widget_width;
      last_opposite_size = self->last_visible_widget_height;
    } else {
      last_size = self->last_visible_widget_height;
      last_opposite_size = self->last_visible_widget_width;
    }

    /* Work out which for_size we want to pass for measuring our children */
    if (for_size == -1 || for_size == last_opposite_size ||
        self->homogeneous[OPPOSITE_ORIENTATION(orientation)])
      child_for_size = for_size;
    else if (t <= 0.0)  /* avoid -Wfloat-equal */
      /* We're going to return last_size anyway */
      child_for_size = -1;
    else {
      double d = inverse_lerp (last_opposite_size, for_size, t);
      /* inverse_lerp is numerically unstable due to its use of floating-point
       * division, potentially by a very small value when progress is close to
       * zero. So we sanity check the return value.
       */
      if (isnan (d) || isinf (d) || d < 0 || d >= G_MAXINT)
        child_for_size = -1;
      else
        child_for_size = floor (d);
    }
  } else {
    child_for_size = for_size;
    /* Avoid -Wmaybe-uninitialized */
    last_size = 0;
    t = 1.0;
  }

  *minimum = 0;
  *natural = 0;

  for (l = self->children; l; l = l->next) {
    AdwViewStackPage *page = l->data;
    GtkWidget *child = page->widget;

    if (!self->homogeneous[orientation] &&
        self->visible_child != page)
      continue;

    if (!gtk_widget_get_visible (child))
      continue;

    if (!self->homogeneous[OPPOSITE_ORIENTATION(orientation)] && self->visible_child != page) {
      int measure_for_size;

      if (child_for_size == -1)
        measure_for_size = -1;
      else {
        gtk_widget_measure (child, OPPOSITE_ORIENTATION (orientation),
                            -1,
                            &measure_for_size, NULL,
                            NULL, NULL);
        measure_for_size = MAX (measure_for_size, child_for_size);
      }
      gtk_widget_measure (child, orientation,
                          measure_for_size,
                          &child_min, &child_nat,
                          NULL, NULL);
    } else {
      gtk_widget_measure (child, orientation,
                          child_for_size,
                          &child_min, &child_nat,
                          NULL, NULL);
    }

    *minimum = MAX (*minimum, child_min);
    *natural = MAX (*natural, child_nat);
  }

  if (self->last_visible_child != NULL && !self->homogeneous[orientation]) {
    *minimum = ceil (adw_lerp (last_size, *minimum, t));
    *natural = ceil (adw_lerp (last_size, *natural, t));
  }
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
  case PROP_ENABLE_TRANSITIONS:
    g_value_set_boolean (value, adw_view_stack_get_enable_transitions (self));
    break;
  case PROP_TRANSITION_DURATION:
    g_value_set_uint (value, adw_view_stack_get_transition_duration (self));
    break;
  case PROP_TRANSITION_RUNNING:
    g_value_set_boolean (value, adw_view_stack_get_transition_running (self));
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
  case PROP_ENABLE_TRANSITIONS:
    adw_view_stack_set_enable_transitions (self, g_value_get_boolean (value));
    break;
  case PROP_TRANSITION_DURATION:
    adw_view_stack_set_transition_duration (self, g_value_get_uint (value));
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

  g_clear_weak_pointer (&self->pages);

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

  widget_class->snapshot = adw_view_stack_snapshot;
  widget_class->size_allocate = adw_view_stack_size_allocate;
  widget_class->measure = adw_view_stack_measure;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwViewStack:hhomogeneous:
   *
   * Whether the stack is horizontally homogeneous.
   *
   * If the stack is horizontally homogeneous, it allocates the same width for
   * all children.
   *
   * If it's `FALSE`, the stack may change width when a different child becomes
   * visible.
   */
  props[PROP_HHOMOGENEOUS] =
    g_param_spec_boolean ("hhomogeneous", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:vhomogeneous:
   *
   * Whether the stack is vertically homogeneous.
   *
   * If the stack is vertically homogeneous, it allocates the same height for
   * all children.
   *
   * If it's `FALSE`, the stack may change height when a different child becomes
   * visible.
   */
  props[PROP_VHOMOGENEOUS] =
    g_param_spec_boolean ("vhomogeneous", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:visible-child:
   *
   * The widget currently visible in the stack.
   */
  props[PROP_VISIBLE_CHILD] =
    g_param_spec_object ("visible-child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:visible-child-name:
   *
   * The name of the widget currently visible in the stack.
   *
   * See [property@ViewStack:visible-child].
   */
  props[PROP_VISIBLE_CHILD_NAME] =
    g_param_spec_string ("visible-child-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:enable-transitions:
   *
   * Whether the stack uses a crossfade transition between pages.
   *
   * Use [property@ViewStack:transition-duration] to control the duration, and
   * [property@ViewStack:transition-running] to know when the transition is
   * running.
   *
   * Since: 1.7
   */
  props[PROP_ENABLE_TRANSITIONS] =
    g_param_spec_boolean ("enable-transitions", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:transition-duration:
   *
   * The transition animation duration, in milliseconds.
   *
   * Only used when [property@ViewStack:enable-transitions] is set to `TRUE`.
   *
   * Since: 1.7
   */
  props[PROP_TRANSITION_DURATION] =
    g_param_spec_uint ("transition-duration", NULL, NULL,
                       0, G_MAXUINT, 200,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:transition-running:
   *
   * Whether a transition is currently running.
   *
   * If a transition is impossible, the property value will be set to `TRUE` and
   * then immediately to `FALSE`, so it's possible to rely on its notifications
   * to know that a transition has happened.
   *
   * Since: 1.7
   */
  props[PROP_TRANSITION_RUNNING] =
    g_param_spec_boolean ("transition-running", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwViewStack:pages:
   *
   * A selection model with the stack's pages.
   *
   * This can be used to keep an up-to-date view.
   *
   * The model implements [iface@Gtk.SectionModel] and creates sections based on
   * [property@ViewStackPage:starts-section] values.
   *
   * The model also implements [iface@Gtk.SelectionModel] and can be used to
   * track and change the visible page.
   */
  props[PROP_PAGES] =
    g_param_spec_object ("pages", NULL, NULL,
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "stack");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_view_stack_init (AdwViewStack *self)
{
  AdwAnimationTarget *target;

  self->homogeneous[GTK_ORIENTATION_VERTICAL] = TRUE;
  self->homogeneous[GTK_ORIENTATION_HORIZONTAL] = TRUE;
  self->transition_duration = 200;

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc) transition_cb,
                                              self, NULL);
  self->animation = adw_timed_animation_new (GTK_WIDGET (self), 0, 1, 0, target);

  adw_timed_animation_set_easing (ADW_TIMED_ANIMATION (self->animation), ADW_EASE);

  g_signal_connect_swapped (self->animation, "done",
                            G_CALLBACK (transition_done_cb), self);
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
    add_internal (self, GTK_WIDGET (child), NULL, NULL, NULL);
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_view_stack_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_view_stack_buildable_add_child;
}

static GtkAccessible *
adw_view_stack_accessible_get_first_accessible_child (GtkAccessible *accessible)
{
  AdwViewStack *self = ADW_VIEW_STACK (accessible);

  if (self->children && self->children->data)
    return g_object_ref (self->children->data);

  return NULL;
}

static void
adw_view_stack_accessible_init (GtkAccessibleInterface *iface)
{
  iface->get_first_accessible_child = adw_view_stack_accessible_get_first_accessible_child;
}

/**
 * adw_view_stack_page_get_child:
 * @self: a view stack page
 *
 * Gets the stack child to which @self belongs.
 *
 * Returns: (transfer none): the child to which @self belongs
 */
GtkWidget *
adw_view_stack_page_get_child (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->widget;
}

/**
 * adw_view_stack_page_get_name:
 * @self: a view stack page
 *
 * Gets the name of the page.
 *
 * Returns: (nullable): the name of the page
 */
const char *
adw_view_stack_page_get_name (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->name;
}

/**
 * adw_view_stack_page_set_name:
 * @self: a view stack page
 * @name: (nullable): the page name
 *
 * Sets the name of the page.
 */
void
adw_view_stack_page_set_name (AdwViewStackPage *self,
                              const char       *name)
{
  AdwViewStack *stack = NULL;

  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  if (self->widget &&
      gtk_widget_get_parent (self->widget) &&
      ADW_IS_VIEW_STACK (gtk_widget_get_parent (self->widget)) &&
      name) {
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

  if (!g_set_str (&self->name, name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_NAME]);

  if (stack && stack->visible_child == self)
    g_object_notify_by_pspec (G_OBJECT (stack),
                              props[PROP_VISIBLE_CHILD_NAME]);
}

/**
 * adw_view_stack_page_get_title:
 * @self: a view stack page
 *
 * Gets the page title.
 *
 * Returns: (nullable): the page title
 */
const char *
adw_view_stack_page_get_title (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->title;
}

/**
 * adw_view_stack_page_set_title:
 * @self: a view stack page
 * @title: (nullable): the page title
 *
 * Sets the page title.
 */
void
adw_view_stack_page_set_title (AdwViewStackPage *self,
                               const char       *title)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  if (!g_set_str (&self->title, title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TITLE]);

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, self->title,
                                  -1);
}

/**
 * adw_view_stack_page_get_use_underline:
 * @self: a view stack page
 *
 * Gets whether underlines in the page title indicate mnemonics.
 *
 * Returns: whether underlines in the page title indicate mnemonics
 */
gboolean
adw_view_stack_page_get_use_underline (AdwViewStackPage *self)
{
  return self->use_underline;
}

/**
 * adw_view_stack_page_set_use_underline:
 * @self: a view stack page
 * @use_underline: the new value to set
 *
 * Sets whether underlines in the page title indicate mnemonics.
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
 * adw_view_stack_page_get_icon_name:
 * @self: a view stack page
 *
 * Gets the icon name of the page.
 *
 * Returns: (nullable): the icon name of the page
 */
const char *
adw_view_stack_page_get_icon_name (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->icon_name;
}

/**
 * adw_view_stack_page_set_icon_name:
 * @self: a view stack page
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name of the page.
 */
void
adw_view_stack_page_set_icon_name (AdwViewStackPage *self,
                                   const char       *icon_name)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  if (!g_set_str (&self->icon_name, icon_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_ICON_NAME]);
}

/**
 * adw_view_stack_page_get_needs_attention:
 * @self: a view stack page
 *
 * Gets whether the page requires the user attention.
 *
 * Returns: whether the page needs attention
 */
gboolean
adw_view_stack_page_get_needs_attention (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), FALSE);

  return self->needs_attention;
}

/**
 * adw_view_stack_page_set_needs_attention:
 * @self: a view stack page
 * @needs_attention: the new value to set
 *
 * Sets whether the page requires the user attention.
 *
 * [class@ViewSwitcher] will display it as a dot next to the page icon.
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
 * adw_view_stack_page_get_badge_number:
 * @self: a view stack page
 *
 * Gets the badge number for this page.
 *
 * Returns: the badge number for this page
 */
guint
adw_view_stack_page_get_badge_number (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), 0);

  return self->badge_number;
}

/**
 * adw_view_stack_page_set_badge_number:
 * @self: a view stack page
 * @badge_number: the new value to set
 *
 * Sets the badge number for this page.
 *
 * [class@ViewSwitcher] can display it as a badge next to the page icon. It is
 * commonly used to display a number of unread items within the page.
 *
 * It can be used together with [property@ViewStack{age}:needs-attention].
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
 * adw_view_stack_page_get_visible:
 * @self: a view stack page
 *
 * Gets whether @self is visible in its `AdwViewStack`.
 *
 * This is independent from the [property@Gtk.Widget:visible]
 * property of its widget.
 *
 * Returns: whether @self is visible
 */
gboolean
adw_view_stack_page_get_visible (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), FALSE);

  return self->visible;
}

/**
 * adw_view_stack_page_set_visible:
 * @self: a view stack page
 * @visible: whether @self is visible
 *
 * Sets whether @self is visible in its `AdwViewStack`.
 *
 * This is independent from the [property@Gtk.Widget:visible] property of
 * [property@ViewStackPage:child].
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
 * adw_view_stack_page_get_starts_section:
 * @self: a view stack page
 *
 * Gets whether @self starts a section.
 *
 * Returns: whether @self starts a section
 *
 * Since: 1.9
 */
gboolean
adw_view_stack_page_get_starts_section (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), FALSE);

  return self->starts_section;
}

/**
 * adw_view_stack_page_set_starts_section:
 * @self: a view stack page
 * @starts_section: whether @self starts a section
 *
 * Sets whether @self starts a section.
 *
 * If set to `TRUE`, [property@ViewStack:pages] will have a section starting
 * from this page.
 *
 * If [property@ViewStackPage:section-title] is set, it should be used as a
 * title for the section.
 *
 * Since: 1.9
 */
void
adw_view_stack_page_set_starts_section (AdwViewStackPage *self,
                                        gboolean          starts_section)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  starts_section = !!starts_section;

  if (starts_section == self->starts_section)
    return;

  self->starts_section = starts_section;

  if (self->widget && gtk_widget_get_parent (self->widget))
    section_changed (ADW_VIEW_STACK (gtk_widget_get_parent (self->widget)), self);

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_STARTS_SECTION]);
}

/**
 * adw_view_stack_page_get_section_title:
 * @self: a view stack page
 *
 * Gets the section title for @self.
 *
 * Returns: (nullable): the section title
 *
 * Since: 1.9
 */
const char *
adw_view_stack_page_get_section_title (AdwViewStackPage *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK_PAGE (self), NULL);

  return self->section_title;
}

/**
 * adw_view_stack_page_set_section_title:
 * @self: a view stack page
 * @section_title: (nullable): the section title
 *
 * Sets the section title for @self.
 *
 * Does nothing unless [property@ViewStackPage:starts-section] is set.
 *
 * Since: 1.9
 */
void
adw_view_stack_page_set_section_title (AdwViewStackPage *self,
                                       const char       *section_title)
{
  g_return_if_fail (ADW_IS_VIEW_STACK_PAGE (self));

  if (!g_set_str (&self->section_title, section_title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_SECTION_TITLE]);
}

/**
 * adw_view_stack_new:
 *
 * Creates a new `AdwViewStack`.
 *
 * Returns: the newly created `AdwViewStack`
 */
GtkWidget *
adw_view_stack_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_STACK, NULL);
}

/**
 * adw_view_stack_add:
 * @self: a view stack
 * @child: the widget to add
 *
 * Adds a child to @self.
 *
 * Returns: (transfer none): the [class@ViewStackPage] for @child
 */
AdwViewStackPage *
adw_view_stack_add (AdwViewStack   *self,
                    GtkWidget      *child)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return add_internal (self, child, NULL, NULL, NULL);
}

/**
 * adw_view_stack_add_named:
 * @self: a view stack
 * @child: the widget to add
 * @name: (nullable): the name for @child
 *
 * Adds a child to @self.
 *
 * The child is identified by the @name.
 *
 * Returns: (transfer none): the `AdwViewStackPage` for @child
 */
AdwViewStackPage *
adw_view_stack_add_named (AdwViewStack   *self,
                          GtkWidget      *child,
                          const char     *name)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return add_internal (self, child, name, NULL, NULL);
}

/**
 * adw_view_stack_add_titled:
 * @self: a view stack
 * @child: the widget to add
 * @name: (nullable): the name for @child
 * @title: a human-readable title for @child
 *
 * Adds a child to @self.
 *
 * The child is identified by the @name. The @title will be used by
 * [class@ViewSwitcher] to represent @child, so it should be short.
 *
 * Returns: (transfer none): the `AdwViewStackPage` for @child
 */
AdwViewStackPage *
adw_view_stack_add_titled (AdwViewStack   *self,
                           GtkWidget      *child,
                           const char     *name,
                           const char     *title)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return add_internal (self, child, name, title, NULL);
}

/**
 * adw_view_stack_add_titled_with_icon:
 * @self: a view stack
 * @child: the widget to add
 * @name: (nullable): the name for @child
 * @title: a human-readable title for @child
 * @icon_name: an icon name for @child
 *
 * Adds a child to @self.
 *
 * The child is identified by the @name. The @title and @icon_name will be used
 * by [class@ViewSwitcher] to represent @child.
 *
 * Returns: (transfer none): the `AdwViewStackPage` for @child
 *
 * Since: 1.2
 */
AdwViewStackPage *
adw_view_stack_add_titled_with_icon (AdwViewStack *self,
                                     GtkWidget    *child,
                                     const char   *name,
                                     const char   *title,
                                     const char   *icon_name)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return add_internal (self, child, name, title, icon_name);
}

/**
 * adw_view_stack_remove:
 * @self: a view stack
 * @child: the child to remove
 *
 * Removes a child widget from @self.
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
 * @self: a view stack
 * @child: a child of @self
 *
 * Gets the [class@ViewStackPage] object for @child.
 *
 * Returns: (transfer none): the page object for @child
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
 * @self: a view stack
 * @name: the name of the child to find
 *
 * Finds the child with @name in @self.
 *
 * Returns: (transfer none) (nullable): the requested child
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
 * adw_view_stack_get_visible_child:
 * @self: a view stack
 *
 * Gets the currently visible child of @self.
 *
 * Returns: (transfer none) (nullable): the visible child
 */
GtkWidget *
adw_view_stack_get_visible_child (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);

  return self->visible_child ? self->visible_child->widget : NULL;
}

/**
 * adw_view_stack_set_visible_child:
 * @self: a view stack
 * @child: a child of @self
 *
 * Makes @child the visible child of @self.
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
 * adw_view_stack_get_visible_child_name:
 * @self: a view stack
 *
 * Returns the name of the currently visible child of @self.
 *
 * Returns: (transfer none) (nullable): the name of the visible child
 */
const char *
adw_view_stack_get_visible_child_name (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);

  return self->visible_child ? self->visible_child->name : NULL;
}

/**
 * adw_view_stack_set_visible_child_name:
 * @self: a view stack
 * @name: the name of the child
 *
 * Makes the child with @name visible.
 *
 * See [property@ViewStack:visible-child].
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
 * adw_view_stack_get_hhomogeneous:
 * @self: a view stack
 *
 * Gets whether @self is horizontally homogeneous.
 *
 * Returns: whether @self is horizontally homogeneous
 */
gboolean
adw_view_stack_get_hhomogeneous (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), FALSE);

  return self->homogeneous[GTK_ORIENTATION_HORIZONTAL];
}

/**
 * adw_view_stack_set_hhomogeneous:
 * @self: a view stack
 * @hhomogeneous: whether to make @self horizontally homogeneous
 *
 * Sets @self to be horizontally homogeneous or not.
 *
 * If the stack is horizontally homogeneous, it allocates the same width for
 * all children.
 *
 * If it's `FALSE`, the stack may change width when a different child becomes
 * visible.
 */
void
adw_view_stack_set_hhomogeneous (AdwViewStack *self,
                                 gboolean      hhomogeneous)
{
  g_return_if_fail (ADW_IS_VIEW_STACK (self));

  hhomogeneous = !!hhomogeneous;

  if (self->homogeneous[GTK_ORIENTATION_HORIZONTAL] == hhomogeneous)
    return;

  self->homogeneous[GTK_ORIENTATION_HORIZONTAL] = hhomogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET (self)))
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HHOMOGENEOUS]);
}

/**
 * adw_view_stack_get_vhomogeneous:
 * @self: a view stack
 *
 * Gets whether @self is vertically homogeneous.
 *
 * Returns: whether @self is vertically homogeneous
 */
gboolean
adw_view_stack_get_vhomogeneous (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), FALSE);

  return self->homogeneous[GTK_ORIENTATION_VERTICAL];
}

/**
 * adw_view_stack_set_vhomogeneous:
 * @self: a view stack
 * @vhomogeneous: whether to make @self vertically homogeneous
 *
 * Sets @self to be vertically homogeneous or not.
 *
 * If the stack is vertically homogeneous, it allocates the same height for
 * all children.
 *
 * If it's `FALSE`, the stack may change height when a different child becomes
 * visible.
 */
void
adw_view_stack_set_vhomogeneous (AdwViewStack *self,
                                 gboolean      vhomogeneous)
{
  g_return_if_fail (ADW_IS_VIEW_STACK (self));

  vhomogeneous = !!vhomogeneous;

  if (self->homogeneous[GTK_ORIENTATION_VERTICAL] == vhomogeneous)
    return;

  self->homogeneous[GTK_ORIENTATION_VERTICAL] = vhomogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET (self)))
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VHOMOGENEOUS]);
}

/**
 * adw_view_stack_get_enable_transitions:
 * @self: a view stack
 *
 * Gets whether @self uses a crossfade transition between pages.
 *
 * Use [property@ViewStack:transition-duration] to control the duration, and
 * [property@ViewStack:transition-running] to know when the transition is
 * running.
 *
 * Returns: whether to enable page transitions
 *
 * Since: 1.7
 */
gboolean
adw_view_stack_get_enable_transitions (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), FALSE);

  return self->enable_transitions;
}

/**
 * adw_view_stack_set_enable_transitions:
 * @self: a view stack
 * @enable_transitions: whether to enable page transitions
 *
 * Sets whether @self uses a crossfade transition between pages.
 *
 * Since: 1.7
 */
void
adw_view_stack_set_enable_transitions (AdwViewStack *self,
                                       gboolean      enable_transitions)
{
  g_return_if_fail (ADW_IS_VIEW_STACK (self));

  enable_transitions = !!enable_transitions;

  if (enable_transitions == self->enable_transitions)
    return;

  self->enable_transitions = enable_transitions;

  if (self->enable_transitions) {
    adw_timed_animation_set_duration (ADW_TIMED_ANIMATION (self->animation),
                                      self->transition_duration);
  } else {
    adw_timed_animation_set_duration (ADW_TIMED_ANIMATION (self->animation), 0);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_TRANSITIONS]);
}

/**
 * adw_view_stack_get_transition_duration:
 * @self: a view stack
 *
 * Gets the transition animation duration for @self.
 *
 * Returns: the transition duration, in milliseconds
 *
 * Since: 1.7
 */
guint
adw_view_stack_get_transition_duration (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), 0);

  return self->transition_duration;
}

/**
 * adw_view_stack_set_transition_duration:
 * @self: a view stack
 * @duration: the new duration, in milliseconds
 *
 * Sets the transition animation duration for @self.
 *
 * Only used when [property@ViewStack:enable-transitions] is set to `TRUE`.
 *
 * Since: 1.7
 */
void
adw_view_stack_set_transition_duration (AdwViewStack *self,
                                        guint         duration)
{
  g_return_if_fail (ADW_IS_VIEW_STACK (self));

  if (self->transition_duration == duration)
    return;

  self->transition_duration = duration;

  if (self->enable_transitions) {
    adw_timed_animation_set_duration (ADW_TIMED_ANIMATION (self->animation),
                                      self->transition_duration);
  } else {
    adw_timed_animation_set_duration (ADW_TIMED_ANIMATION (self->animation), 0);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_DURATION]);
}

/**
 * adw_view_stack_get_transition_running:
 * @self: a view stack
 *
 * Gets whether a transition is currently running for @self.
 *
 * If a transition is impossible, the property value will be set to `TRUE` and
 * then immediately to `FALSE`, so it's possible to rely on its notifications
 * to know that a transition has happened.
 *
 * Returns: whether a transition is currently running
 *
 * Since: 1.7
 */
gboolean
adw_view_stack_get_transition_running (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), FALSE);

  return self->transition_running;
}

/**
 * adw_view_stack_get_pages:
 * @self: a view stack
 *
 * Returns a [iface@Gio.ListModel] that contains the pages of the stack.
 *
 * This can be used to keep an up-to-date view.
 *
 * The model implements [iface@Gtk.SectionModel] and creates sections based on
 * [property@ViewStackPage:starts-section] values.
 *
 * The model also implements [iface@Gtk.SelectionModel] and can be used to track
 * and change the visible page.
 *
 * Returns: (transfer full): a `GtkSelectionModel` for the stack's children
 */
GtkSelectionModel *
adw_view_stack_get_pages (AdwViewStack *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_STACK (self), NULL);

  if (self->pages)
    return g_object_ref (self->pages);

  g_set_weak_pointer (&self->pages,
                      GTK_SELECTION_MODEL (adw_view_stack_pages_new (self)));

  return self->pages;
}

