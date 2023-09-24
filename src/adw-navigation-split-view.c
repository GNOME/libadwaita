/*
 * Copyright (C) 2022-2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adw-navigation-split-view.h"

#include <math.h>

#include "adw-bin.h"
#include "adw-enums.h"
#include "adw-length-unit.h"
#include "adw-navigation-view-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwNavigationSplitView:
 *
 * A widget presenting sidebar and content side by side or as a navigation view.
 *
 * <picture>
 *   <source srcset="navigation-split-view-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="navigation-split-view.png" alt="navigation-split-view">
 * </picture>
 * <picture>
 *   <source srcset="navigation-split-view-collapsed-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="navigation-split-view-collapsed.png" alt="navigation-split-view-collapsed">
 * </picture>
 *
 * `AdwNavigationSplitView` has two [class@NavigationPage] children: sidebar and
 * content, and displays them side by side.
 *
 * When [property@NavigationSplitView:collapsed] is set to `TRUE`, it instead
 * puts both children inside an [class@NavigationView]. The
 * [property@NavigationSplitView:show-content] controls which child is visible
 * while collapsed.
 *
 * See also [class@OverlaySplitView].
 *
 * `AdwNavigationSplitView` is typically used together with an [class@Breakpoint]
 * setting the `collapsed` property to `TRUE` on small widths, as follows:
 *
 * ```xml
 * <object class="AdwWindow">
 *   <property name="width-request">280</property>
 *   <property name="height-request">200</property>
 *   <property name="default-width">800</property>
 *   <property name="default-height">800</property>
 *   <child>
 *     <object class="AdwBreakpoint">
 *       <condition>max-width: 400sp</condition>
 *       <setter object="split_view" property="collapsed">True</setter>
 *     </object>
 *   </child>
 *   <property name="content">
 *     <object class="AdwNavigationSplitView" id="split_view">
 *       <property name="sidebar">
 *         <object class="AdwNavigationPage">
 *           <property name="title" translatable="yes">Sidebar</property>
 *           <property name="child">
 *             <!-- ... -->
 *           </property>
 *         </object>
 *       </property>
 *       <property name="content">
 *         <object class="AdwNavigationPage">
 *           <property name="title" translatable="yes">Content</property>
 *           <property name="child">
 *             <!-- ... -->
 *           </property>
 *         </object>
 *       </property>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * ## Sizing
 *
 * When not collapsed, `AdwNavigationSplitView` changes the sidebar width
 * depending on its own width.
 *
 * If possible, it tries to allocate a fraction of the total width, controlled
 * with the [property@NavigationSplitView:sidebar-width-fraction] property.
 *
 * The sidebar also has minimum and maximum sizes, controlled with the
 * [property@NavigationSplitView:min-sidebar-width] and
 * [property@NavigationSplitView:max-sidebar-width] properties.
 *
 * The minimum and maximum sizes are using the length unit specified with the
 * [property@NavigationSplitView:sidebar-width-unit].
 *
 * By default, sidebar is using 25% of the total width, with 180sp as the
 * minimum size and 280sp as the maximum size.
 *
 * ## Header Bar Integration
 *
 * When used inside `AdwNavigationSplitView`, [class@HeaderBar] will
 * automatically hide the window buttons in the middle.
 *
 * When collapsed, it also displays a back button for the content widget, as
 * well as the page titles. See [class@NavigationView] documentation for details.
 *
 * ## Actions
 *
 * `AdwNavigationSplitView` defines the same actions as `AdwNavigationView`, but
 * they can be used even when the split view is not collapsed:
 *
 * - `navigation.push` takes a string parameter specifying the tag of the page
 * to push. If it matches the tag of the content widget, it sets
 * [property@NavigationSplitView:show-content] to `TRUE`.
 *
 * - `navigation.pop` doesn't take any parameters and sets
 * [property@NavigationSplitView:show-content] to `FALSE`.
 *
 * ## `AdwNavigationSplitView` as `GtkBuildable`
 *
 * The `AdwNavigationSplitView` implementation of the [iface@Gtk.Buildable]
 * interface supports setting the sidebar widget by specifying “sidebar” as the
 * “type” attribute of a `<child>` element, Specifying “content” child type or
 * omitting it results in setting the content widget.
 *
 * ## CSS nodes
 *
 * `AdwNavigationSplitView` has a single CSS node with the name
 * `navigation-split-view`.
 *
 * When collapsed, it contains a child node with the name `navigation-view`
 * containing both children.
 *
 * ```
 * navigation-split-view
 * ╰── navigation-view
 *     ├── [sidebar child]
 *     ╰── [content child]
 * ```
 *
 * When not collapsed, it contains two nodes with the name `widget`, one with
 * the `.sidebar-pane` style class, the other one with `.content-view` style
 * class, containing the sidebar and content children respectively.
 *
 * ```
 * navigation-split-view
 * ├── widget.sidebar-pane
 * │   ╰── [sidebar child]
 * ╰── widget.content-pane
 *     ╰── [content child]
 * ```
 *
 * ## Accessibility
 *
 * `AdwNavigationSplitView` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 *
 * Since: 1.4
 */

struct _AdwNavigationSplitView
{
  GtkWidget parent_instance;

  AdwNavigationPage *sidebar;
  AdwNavigationPage *content;

  GtkWidget *sidebar_bin;
  GtkWidget *content_bin;
  GtkWidget *navigation_view;

  gboolean collapsed;
  gboolean show_content;
  gboolean changing_page;

  double min_sidebar_width;
  double max_sidebar_width;
  double sidebar_width_fraction;
  AdwLengthUnit sidebar_width_unit;
};

static void adw_navigation_split_view_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwNavigationSplitView, adw_navigation_split_view, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_navigation_split_view_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_SIDEBAR,
  PROP_CONTENT,
  PROP_COLLAPSED,
  PROP_SHOW_CONTENT,
  PROP_MIN_SIDEBAR_WIDTH,
  PROP_MAX_SIDEBAR_WIDTH,
  PROP_SIDEBAR_WIDTH_FRACTION,
  PROP_SIDEBAR_WIDTH_UNIT,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
measure_uncollapsed (GtkWidget      *widget,
                     GtkOrientation  orientation,
                     int             for_size,
                     int            *minimum,
                     int            *natural,
                     int            *minimum_baseline,
                     int            *natural_baseline)
{
  AdwNavigationSplitView *self = ADW_NAVIGATION_SPLIT_VIEW (widget);
  int sidebar_min = 0, sidebar_nat = 0;
  int content_min = 0, content_nat = 0;

  gtk_widget_measure (self->sidebar_bin, orientation, -1,
                      &sidebar_min, &sidebar_nat, NULL, NULL);
  gtk_widget_measure (self->content_bin, orientation, -1,
                      &content_min, &content_nat, NULL, NULL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    GtkSettings *settings = gtk_widget_get_settings (widget);
    int sidebar_max;

    sidebar_min = MAX (sidebar_min, adw_length_unit_to_px (self->sidebar_width_unit,
                                                           self->min_sidebar_width,
                                                           settings));
    sidebar_max = MAX (sidebar_min, adw_length_unit_to_px (self->sidebar_width_unit,
                                                           self->max_sidebar_width,
                                                           settings));

    /* Ignore sidebar's own natural width and instead estimate it from content
     * and fraction */
    sidebar_nat = ceil (content_nat * self->sidebar_width_fraction /
                        (1.0 - self->sidebar_width_fraction));
    sidebar_nat = CLAMP (sidebar_nat, sidebar_min, sidebar_max);

    if (minimum)
      *minimum = sidebar_min + content_min;
    if (natural)
      *natural = sidebar_nat + content_nat;
  } else {
    if (minimum)
      *minimum = MAX (sidebar_min, content_min);
    if (natural)
      *natural = MAX (sidebar_nat, content_nat);
  }
}

static void
allocate_uncollapsed (GtkWidget *widget,
                      int        width,
                      int        height,
                      int        baseline)
{
  AdwNavigationSplitView *self = ADW_NAVIGATION_SPLIT_VIEW (widget);
  GtkSettings *settings = gtk_widget_get_settings (widget);
  int sidebar_min, content_min, sidebar_max, sidebar_width;
  GskTransform *transform;

  gtk_widget_measure (self->sidebar_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &sidebar_min, NULL, NULL, NULL);
  gtk_widget_measure (self->content_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &content_min, NULL, NULL, NULL);

  sidebar_min = MAX (sidebar_min,
                     ceil (adw_length_unit_to_px (self->sidebar_width_unit,
                                                  self->min_sidebar_width,
                                                  settings)));

  sidebar_max = MAX (sidebar_min,
                     ceil (adw_length_unit_to_px (self->sidebar_width_unit,
                                                  self->max_sidebar_width,
                                                  settings)));
  sidebar_max = MIN (sidebar_max, width - content_min);

  sidebar_width = CLAMP ((int) (width * self->sidebar_width_fraction),
                         sidebar_min, sidebar_max);

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) {
    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (width - sidebar_width, 0));

    gtk_widget_allocate (self->sidebar_bin, sidebar_width, height, baseline, transform);
    gtk_widget_allocate (self->content_bin, width - sidebar_width, height, baseline, NULL);
  } else {
    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (sidebar_width, 0));

    gtk_widget_allocate (self->sidebar_bin, sidebar_width, height, baseline, NULL);
    gtk_widget_allocate (self->content_bin, width - sidebar_width, height, baseline, transform);
  }
}

static void
notify_visible_page_cb (AdwNavigationSplitView *self)
{
  AdwNavigationPage *visible_page;
  gboolean show_content = FALSE;

  g_assert (self->navigation_view);
  g_assert (self->sidebar);
  g_assert (self->content);

  visible_page = adw_navigation_view_get_visible_page (ADW_NAVIGATION_VIEW (self->navigation_view));

  if (visible_page)
    show_content = visible_page == self->content;

  if (show_content == self->show_content)
    return;

  self->show_content = show_content;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_CONTENT]);
}

static void
update_navigation_stack (AdwNavigationSplitView *self)
{
  AdwNavigationPage *stack[2] = {0};
  int i = 0;

  if (!self->navigation_view)
    return;

  if (self->changing_page && self->sidebar && self->content) {
    if (self->show_content) {
      stack[i++] = self->sidebar;

      adw_navigation_view_replace (ADW_NAVIGATION_VIEW (self->navigation_view),
                                   stack, i);
      adw_navigation_view_push (ADW_NAVIGATION_VIEW (self->navigation_view),
                                self->content);
    } else {
      stack[i++] = self->sidebar;
      stack[i++] = self->content;

      adw_navigation_view_replace (ADW_NAVIGATION_VIEW (self->navigation_view),
                                   stack, i);
      adw_navigation_view_pop (ADW_NAVIGATION_VIEW (self->navigation_view));
    }

    return;
  }

  if (self->sidebar)
    stack[i++] = self->sidebar;

  if (self->content && (self->show_content || !self->sidebar))
    stack[i++] = self->content;

  adw_navigation_view_replace (ADW_NAVIGATION_VIEW (self->navigation_view),
                               stack, i);
}

static void
changing_page_done_cb (AdwNavigationSplitView *self)
{
  self->changing_page = FALSE;
}

static gboolean
tags_equal (AdwNavigationPage *sidebar,
            AdwNavigationPage *content)
{
  const char *sidebar_tag, *content_tag;

  if (!sidebar || !content)
    return FALSE;

  sidebar_tag = adw_navigation_page_get_tag (sidebar);
  content_tag = adw_navigation_page_get_tag (content);

  if (!sidebar_tag || !content_tag)
    return FALSE;

  return !strcmp (sidebar_tag, content_tag);
}

static void
check_tags_cb (AdwNavigationSplitView *self,
               GParamSpec             *pspec,
               AdwNavigationPage      *page)
{
  if (!tags_equal (self->sidebar, self->content))
    return;

  if (page == self->sidebar) {
    g_critical ("Trying to set the sidebar's tag to '%s', but the content "
                "already has the same tag",
                adw_navigation_page_get_tag (self->sidebar));

    adw_navigation_page_set_tag (self->sidebar, NULL);

    return;
  }

  if (page == self->content) {
    g_critical ("Trying to set the content's tag to '%s', but the sidebar "
                "already has the same tag",
                adw_navigation_page_get_tag (self->content));

    adw_navigation_page_set_tag (self->content, NULL);

    return;
  }

  g_assert_not_reached ();
}

static void
update_collapsed (AdwNavigationSplitView *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  GtkWidget *focus = NULL;

  if (root) {
    focus = gtk_root_get_focus (root);

    if (focus && !gtk_widget_is_ancestor (focus, GTK_WIDGET (self)))
      focus = NULL;

    if (focus)
      g_object_add_weak_pointer (G_OBJECT (focus), (gpointer *) &focus);
  }

  /* Keep the widgets alive as they are unparented */
  if (self->sidebar)
    g_object_ref (self->sidebar);
  if (self->content)
    g_object_ref (self->content);

  if (self->sidebar_bin && self->sidebar) {
    if (self->show_content && self->sidebar && self->content) {
      adw_navigation_page_hiding (self->sidebar);
      adw_navigation_page_hidden (self->sidebar);
    }

    g_signal_handlers_disconnect_by_func (self->sidebar, check_tags_cb, self);

    adw_bin_set_child (ADW_BIN (self->sidebar_bin), NULL);
  }

  if (self->content_bin && self->content) {
    if (!self->show_content && self->sidebar && self->content) {
      adw_navigation_page_hiding (self->content);
      adw_navigation_page_hidden (self->content);
    }

    g_signal_handlers_disconnect_by_func (self->content, check_tags_cb, self);

    adw_bin_set_child (ADW_BIN (self->content_bin), NULL);
  }

  if (self->navigation_view) {
    if (self->sidebar)
      adw_navigation_page_block_signals (self->sidebar);
    if (self->content)
      adw_navigation_page_block_signals (self->content);

    if (self->sidebar && self->content)
      g_signal_handlers_disconnect_by_func (self->navigation_view,
                                            notify_visible_page_cb, self);

    adw_navigation_view_replace (ADW_NAVIGATION_VIEW (self->navigation_view),
                                 NULL, 0);

    if (self->sidebar)
      adw_navigation_view_remove (ADW_NAVIGATION_VIEW (self->navigation_view),
                                  self->sidebar);

    if (self->content)
      adw_navigation_view_remove (ADW_NAVIGATION_VIEW (self->navigation_view),
                                  self->content);

    if (self->sidebar)
      adw_navigation_page_unblock_signals (self->sidebar);
    if (self->content)
      adw_navigation_page_unblock_signals (self->content);
  }

  g_clear_pointer (&self->sidebar_bin, gtk_widget_unparent);
  g_clear_pointer (&self->content_bin, gtk_widget_unparent);
  g_clear_pointer (&self->navigation_view, gtk_widget_unparent);

  if (self->collapsed) {
    gtk_widget_set_layout_manager (GTK_WIDGET (self), gtk_bin_layout_new ());

    self->navigation_view = adw_navigation_view_new ();
    gtk_widget_set_parent (self->navigation_view, GTK_WIDGET (self));

    if (self->sidebar) {
      adw_navigation_page_block_signals (self->sidebar);
      adw_navigation_view_add (ADW_NAVIGATION_VIEW (self->navigation_view),
                               self->sidebar);
    }

    if (self->content) {
      adw_navigation_page_block_signals (self->content);
      adw_navigation_view_add (ADW_NAVIGATION_VIEW (self->navigation_view),
                               self->content);
    }

    update_navigation_stack (self);

    if (self->sidebar)
      adw_navigation_page_unblock_signals (self->sidebar);
    if (self->content)
      adw_navigation_page_unblock_signals (self->content);

    if (self->sidebar && self->content)
      g_signal_connect_swapped (self->navigation_view, "notify::visible-page",
                                G_CALLBACK (notify_visible_page_cb), self);
  } else {
    gtk_widget_set_layout_manager (GTK_WIDGET (self),
                                   gtk_custom_layout_new (adw_widget_get_request_mode,
                                                          measure_uncollapsed,
                                                          allocate_uncollapsed));

    self->sidebar_bin = adw_bin_new ();
    gtk_widget_add_css_class (self->sidebar_bin, "sidebar-pane");
    gtk_widget_set_parent (self->sidebar_bin, GTK_WIDGET (self));

    if (self->sidebar) {
      adw_bin_set_child (ADW_BIN (self->sidebar_bin), GTK_WIDGET (self->sidebar));

      g_signal_connect_swapped (self->sidebar, "notify::tag",
                                G_CALLBACK (check_tags_cb), self);

      if (self->show_content && self->sidebar && self->content) {
        adw_navigation_page_showing (self->sidebar);
        adw_navigation_page_shown (self->sidebar);
      }
    }

    self->content_bin = adw_bin_new ();
    gtk_widget_add_css_class (self->content_bin, "content-pane");
    gtk_widget_set_parent (self->content_bin, GTK_WIDGET (self));

    if (self->content) {
      adw_bin_set_child (ADW_BIN (self->content_bin), GTK_WIDGET (self->content));

      g_signal_connect_swapped (self->content, "notify::tag",
                                G_CALLBACK (check_tags_cb), self);

      if (!self->show_content && self->sidebar && self->content) {
        adw_navigation_page_showing (self->content);
        adw_navigation_page_shown (self->content);
      }
    }
  }

  if (self->sidebar)
    g_object_unref (self->sidebar);
  if (self->content)
    g_object_unref (self->content);

  if (focus) {
    gboolean should_focus = TRUE;

    if (self->collapsed && self->content) {
      gboolean is_content = gtk_widget_is_ancestor (focus, GTK_WIDGET (self->content));

      should_focus = is_content == self->show_content;
    }

    if (should_focus)
      gtk_widget_grab_focus (focus);

    g_object_remove_weak_pointer (G_OBJECT (focus), (gpointer *) &focus);
  }
}

static void
navigation_push_cb (AdwNavigationSplitView *self,
                    const char             *action_name,
                    GVariant               *params)
{
  const char *tag = g_variant_get_string (params, NULL);
  GtkWidget *parent;

  if (self->content) {
    const char *content_tag = adw_navigation_page_get_tag (self->content);

    if (!g_strcmp0 (tag, content_tag)) {
      if (self->show_content && self->collapsed) {
        g_critical ("Page with the tag '%s' is already in the navigation stack",
                    tag);

        return;
      }

      adw_navigation_split_view_set_show_content (self, TRUE);
      return;
    }
  }

  if (self->sidebar) {
    const char *sidebar_tag = adw_navigation_page_get_tag (self->sidebar);

    if (!g_strcmp0 (tag, sidebar_tag)) {
      g_critical ("Page with the tag '%s' is already in the navigation stack",
                  tag);
      return;
    }
  }

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent && gtk_widget_activate_action_variant (parent, "navigation.push", params))
    return;

  g_critical ("No page with the tag '%s' found in AdwNavigationSplitView %p",
              tag, self);
}

static void
navigation_pop_cb (AdwNavigationSplitView *self)
{
  GtkWidget *parent;

  if (self->show_content && self->sidebar && self->content) {
    adw_navigation_split_view_set_show_content (self, FALSE);

    return;
  }

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent)
    gtk_widget_activate_action (parent, "navigation.pop", NULL);
}

static void
adw_navigation_split_view_dispose (GObject *object)
{
  AdwNavigationSplitView *self = ADW_NAVIGATION_SPLIT_VIEW (object);

  g_clear_pointer (&self->sidebar_bin, gtk_widget_unparent);
  g_clear_pointer (&self->content_bin, gtk_widget_unparent);
  g_clear_pointer (&self->navigation_view, gtk_widget_unparent);
  self->sidebar = NULL;
  self->content = NULL;

  G_OBJECT_CLASS (adw_navigation_split_view_parent_class)->dispose (object);
}

static void
adw_navigation_split_view_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  AdwNavigationSplitView *self = ADW_NAVIGATION_SPLIT_VIEW (object);

  switch (prop_id) {
  case PROP_SIDEBAR:
    g_value_set_object (value, adw_navigation_split_view_get_sidebar (self));
    break;
  case PROP_CONTENT:
    g_value_set_object (value, adw_navigation_split_view_get_content (self));
    break;
  case PROP_COLLAPSED:
    g_value_set_boolean (value, adw_navigation_split_view_get_collapsed (self));
    break;
  case PROP_SHOW_CONTENT:
    g_value_set_boolean (value, adw_navigation_split_view_get_show_content (self));
    break;
  case PROP_MIN_SIDEBAR_WIDTH:
    g_value_set_double (value, adw_navigation_split_view_get_min_sidebar_width (self));
    break;
  case PROP_MAX_SIDEBAR_WIDTH:
    g_value_set_double (value, adw_navigation_split_view_get_max_sidebar_width (self));
    break;
  case PROP_SIDEBAR_WIDTH_FRACTION:
    g_value_set_double (value, adw_navigation_split_view_get_sidebar_width_fraction (self));
    break;
  case PROP_SIDEBAR_WIDTH_UNIT:
    g_value_set_enum (value, adw_navigation_split_view_get_sidebar_width_unit (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_navigation_split_view_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  AdwNavigationSplitView *self = ADW_NAVIGATION_SPLIT_VIEW (object);

  switch (prop_id) {
  case PROP_SIDEBAR:
    adw_navigation_split_view_set_sidebar (self, g_value_get_object (value));
    break;
  case PROP_CONTENT:
    adw_navigation_split_view_set_content (self, g_value_get_object (value));
    break;
  case PROP_COLLAPSED:
    adw_navigation_split_view_set_collapsed (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_CONTENT:
    adw_navigation_split_view_set_show_content (self, g_value_get_boolean (value));
    break;
  case PROP_MIN_SIDEBAR_WIDTH:
    adw_navigation_split_view_set_min_sidebar_width (self, g_value_get_double (value));
    break;
  case PROP_MAX_SIDEBAR_WIDTH:
    adw_navigation_split_view_set_max_sidebar_width (self, g_value_get_double (value));
    break;
  case PROP_SIDEBAR_WIDTH_FRACTION:
    adw_navigation_split_view_set_sidebar_width_fraction (self, g_value_get_double (value));
    break;
  case PROP_SIDEBAR_WIDTH_UNIT:
    adw_navigation_split_view_set_sidebar_width_unit (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_navigation_split_view_class_init (AdwNavigationSplitViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_navigation_split_view_dispose;
  object_class->get_property = adw_navigation_split_view_get_property;
  object_class->set_property = adw_navigation_split_view_set_property;

  /**
   * AdwNavigationSplitView:sidebar: (attributes org.gtk.Property.get=adw_navigation_split_view_get_sidebar org.gtk.Property.set=adw_navigation_split_view_set_sidebar)
   *
   * The sidebar widget.
   *
   * Since: 1.4
   */
  props[PROP_SIDEBAR] =
    g_param_spec_object ("sidebar", NULL, NULL,
                         ADW_TYPE_NAVIGATION_PAGE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationSplitView:content: (attributes org.gtk.Property.get=adw_navigation_split_view_get_content org.gtk.Property.set=adw_navigation_split_view_set_content)
   *
   * The content widget.
   *
   * Since: 1.4
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         ADW_TYPE_NAVIGATION_PAGE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationSplitView:collapsed: (attributes org.gtk.Property.get=adw_navigation_split_view_get_collapsed org.gtk.Property.set=adw_navigation_split_view_set_collapsed)
   *
   * Whether the split view is collapsed.
   *
   * When collapsed, the children are put inside an [class@NavigationView],
   * otherwise they are displayed side by side.
   *
   * The [property@NavigationSplitView:show-content] controls which child is
   * visible while collapsed.
   *
   * Since: 1.4
   */
  props[PROP_COLLAPSED] =
    g_param_spec_boolean ("collapsed", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationSplitView:show-content: (attributes org.gtk.Property.get=adw_navigation_split_view_get_show_content org.gtk.Property.set=adw_navigation_split_view_set_show_content)
   *
   * Determines the visible page when collapsed.
   *
   * If set to `TRUE`, the content widget will be the visible page when
   * [property@NavigationSplitView:collapsed] is `TRUE`; otherwise the sidebar
   * widget will be visible.
   *
   * If the split view is already collapsed, the visible page changes
   * immediately.
   *
   * Since: 1.4
   */
  props[PROP_SHOW_CONTENT] =
    g_param_spec_boolean ("show-content", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationSplitView:min-sidebar-width: (attributes org.gtk.Property.get=adw_navigation_split_view_get_min_sidebar_width org.gtk.Property.set=adw_navigation_split_view_set_min_sidebar_width)
   *
   * The minimum sidebar width.
   *
   * Minimum width is affected by
   * [property@NavigationSplitView:sidebar-width-unit].
   *
   * The sidebar widget can still be allocated with larger width if its own
   * minimum width exceeds it.
   *
   * Since: 1.4
   */
  props[PROP_MIN_SIDEBAR_WIDTH] =
    g_param_spec_double ("min-sidebar-width", NULL, NULL,
                         0, G_MAXDOUBLE, 180,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationSplitView:max-sidebar-width: (attributes org.gtk.Property.get=adw_navigation_split_view_get_max_sidebar_width org.gtk.Property.set=adw_navigation_split_view_set_max_sidebar_width)
   *
   * The maximum sidebar width.
   *
   * Maximum width is affected by
   * [property@NavigationSplitView:sidebar-width-unit].
   *
   * The sidebar widget can still be allocated with larger width if its own
   * minimum width exceeds it.
   *
   * Since: 1.4
   */
  props[PROP_MAX_SIDEBAR_WIDTH] =
    g_param_spec_double ("max-sidebar-width", NULL, NULL,
                         0, G_MAXDOUBLE, 280,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationSplitView:sidebar-width-fraction: (attributes org.gtk.Property.get=adw_navigation_split_view_get_sidebar_width_fraction org.gtk.Property.set=adw_navigation_split_view_set_sidebar_width_fraction)
   *
   * The preferred sidebar width as a fraction of the total width.
   *
   * The preferred width is additionally limited by
   * [property@NavigationSplitView:min-sidebar-width] and
   * [property@NavigationSplitView:max-sidebar-width].
   *
   * The sidebar widget can be allocated with larger width if its own minimum
   * width exceeds the preferred width.
   *
   * Since: 1.4
   */
  props[PROP_SIDEBAR_WIDTH_FRACTION] =
    g_param_spec_double ("sidebar-width-fraction", NULL, NULL,
                         0, 1, 0.25,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwNavigationSplitView:sidebar-width-unit: (attributes org.gtk.Property.get=adw_navigation_split_view_get_sidebar_width_unit org.gtk.Property.set=adw_navigation_split_view_set_sidebar_width_unit)
   *
   * The length unit for minimum and maximum sidebar widths.
   *
   * See [property@NavigationSplitView:min-sidebar-width] and
   * [property@NavigationSplitView:max-sidebar-width].
   *
   * Since: 1.4
   */
  props[PROP_SIDEBAR_WIDTH_UNIT] =
    g_param_spec_enum ("sidebar-width-unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_SP,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_install_action (widget_class, "navigation.push", "s",
                                   (GtkWidgetActionActivateFunc) navigation_push_cb);
  gtk_widget_class_install_action (widget_class, "navigation.pop", NULL,
                                   (GtkWidgetActionActivateFunc) navigation_pop_cb);

  gtk_widget_class_set_css_name (widget_class, "navigation-split-view");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_navigation_split_view_init (AdwNavigationSplitView *self)
{
  self->min_sidebar_width = 180;
  self->max_sidebar_width = 280;
  self->sidebar_width_fraction = 0.25;
  self->sidebar_width_unit = ADW_LENGTH_UNIT_SP;

  update_collapsed (self);
}

static void
adw_navigation_split_view_add_child (GtkBuildable *buildable,
                                     GtkBuilder   *builder,
                                     GObject      *child,
                                     const char   *type)
{
  if (!ADW_IS_NAVIGATION_PAGE (child) && GTK_IS_WIDGET (child))
    g_warning ("Cannot add an object of type %s to AdwNavigationSplitView",
               g_type_name (G_OBJECT_TYPE (child)));
  else if (!g_strcmp0 (type, "content"))
    adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (buildable),
                                           ADW_NAVIGATION_PAGE (child));
  else if (!g_strcmp0 (type, "sidebar"))
    adw_navigation_split_view_set_sidebar (ADW_NAVIGATION_SPLIT_VIEW (buildable),
                                           ADW_NAVIGATION_PAGE (child));
  else if (!type && ADW_IS_NAVIGATION_PAGE (child))
    adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (buildable),
                                           ADW_NAVIGATION_PAGE (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_navigation_split_view_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_navigation_split_view_add_child;
}

/**
 * adw_navigation_split_view_new:
 *
 * Creates a new `AdwNavigationSplitView`.
 *
 * Returns: the newly created `AdwNavigationSplitView`
 *
 * Since: 1.4
 */
GtkWidget *
adw_navigation_split_view_new (void)
{
  return g_object_new (ADW_TYPE_NAVIGATION_SPLIT_VIEW, NULL);
}

/**
 * adw_navigation_split_view_get_sidebar: (attributes org.gtk.Method.get_property=sidebar)
 * @self: a navigation split view
 *
 * Gets the sidebar widget for @self.
 *
 * Returns: (transfer none) (nullable): the sidebar widget
 *
 * Since: 1.4
 */
AdwNavigationPage *
adw_navigation_split_view_get_sidebar (AdwNavigationSplitView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self), NULL);

  return self->sidebar;
}

/**
 * adw_navigation_split_view_set_sidebar: (attributes org.gtk.Method.set_property=sidebar)
 * @self: a navigation split view
 * @sidebar: (nullable): the sidebar widget
 *
 * Sets the sidebar widget for @self.
 *
 * Since: 1.4
 */
void
adw_navigation_split_view_set_sidebar (AdwNavigationSplitView *self,
                                       AdwNavigationPage      *sidebar)
{
  g_return_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self));
  g_return_if_fail (sidebar == NULL || ADW_IS_NAVIGATION_PAGE (sidebar));

  if (sidebar)
    g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (sidebar)) == NULL);

  if (sidebar == self->sidebar)
    return;

  if (tags_equal (sidebar, self->content)) {
    g_critical ("Trying to add sidebar with the tag '%s' to "
                "AdwNavigationSplitView, but content already has the same tag",
                adw_navigation_page_get_tag (sidebar));

    return;
  }

  if (self->navigation_view && self->sidebar && self->content)
    g_signal_handlers_disconnect_by_func (self->navigation_view,
                                          notify_visible_page_cb, self);

  if (self->sidebar) {
    if (self->sidebar_bin) {
      adw_navigation_page_hiding (self->sidebar);
      adw_navigation_page_hidden (self->sidebar);

      g_signal_handlers_disconnect_by_func (self->sidebar, check_tags_cb, self);

      adw_bin_set_child (ADW_BIN (self->sidebar_bin), NULL);
    } else if (self->navigation_view) {
      adw_navigation_view_remove (ADW_NAVIGATION_VIEW (self->navigation_view),
                                  self->sidebar);
    }
  }

  self->sidebar = sidebar;

  if (self->sidebar) {
    if (self->sidebar_bin) {
      adw_bin_set_child (ADW_BIN (self->sidebar_bin),
                         GTK_WIDGET (self->sidebar));

      g_signal_connect_swapped (self->sidebar, "notify::tag",
                                G_CALLBACK (check_tags_cb), self);

      adw_navigation_page_showing (self->sidebar);
      adw_navigation_page_shown (self->sidebar);
    } else if (self->navigation_view) {
      adw_navigation_view_add (ADW_NAVIGATION_VIEW (self->navigation_view),
                               self->sidebar);
    }
  }

  update_navigation_stack (self);

  if (self->navigation_view && self->sidebar && self->content)
    g_signal_connect_swapped (self->navigation_view, "notify::visible-page",
                              G_CALLBACK (notify_visible_page_cb), self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR]);
}

/**
 * adw_navigation_split_view_get_content: (attributes org.gtk.Method.get_property=content)
 * @self: a navigation split view
 *
 * Sets the content widget for @self.
 *
 * Returns: (transfer none) (nullable): the content widget
 *
 * Since: 1.4
 */
AdwNavigationPage *
adw_navigation_split_view_get_content (AdwNavigationSplitView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self), NULL);

  return self->content;
}

/**
 * adw_navigation_split_view_set_content: (attributes org.gtk.Method.set_property=content)
 * @self: a navigation split view
 * @content: (nullable): the content widget
 *
 * Sets the content widget for @self.
 *
 * Since: 1.4
 */
void
adw_navigation_split_view_set_content (AdwNavigationSplitView *self,
                                       AdwNavigationPage      *content)
{
  g_return_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self));
  g_return_if_fail (content == NULL || ADW_IS_NAVIGATION_PAGE (content));

  if (content)
    g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (content)) == NULL);

  if (content == self->content)
    return;

  if (tags_equal (self->sidebar, content)) {
    g_critical ("Trying to add content with the tag '%s' to "
                "AdwNavigationSplitView, but sidebar already has the same tag",
                adw_navigation_page_get_tag (content));

    return;
  }

  if (self->navigation_view && self->sidebar && self->content)
    g_signal_handlers_disconnect_by_func (self->navigation_view,
                                          notify_visible_page_cb, self);

  if (self->content) {
    if (self->content_bin) {
      adw_navigation_page_hiding (self->content);
      adw_navigation_page_hidden (self->content);

      g_signal_handlers_disconnect_by_func (self->content, check_tags_cb, self);

      adw_bin_set_child (ADW_BIN (self->content_bin), NULL);
    } else if (self->navigation_view) {
      adw_navigation_view_remove (ADW_NAVIGATION_VIEW (self->navigation_view),
                                  self->content);
    }
  }

  self->content = content;

  if (self->content) {
    if (self->content_bin) {
      adw_bin_set_child (ADW_BIN (self->content_bin),
                         GTK_WIDGET (self->content));

      g_signal_connect_swapped (self->content, "notify::tag",
                                G_CALLBACK (check_tags_cb), self);

      adw_navigation_page_showing (self->content);
      adw_navigation_page_shown (self->content);
    } else if (self->navigation_view) {
      adw_navigation_view_add (ADW_NAVIGATION_VIEW (self->navigation_view),
                               self->content);
    }
  }

  update_navigation_stack (self);

  if (self->navigation_view && self->sidebar && self->content)
    g_signal_connect_swapped (self->navigation_view, "notify::visible-page",
                              G_CALLBACK (notify_visible_page_cb), self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_navigation_split_view_get_collapsed: (attributes org.gtk.Method.get_property=collapsed)
 * @self: a navigation split view
 *
 * Gets whether @self is collapsed.
 *
 * Returns: whether @self is collapsed
 *
 * Since: 1.4
 */
gboolean
adw_navigation_split_view_get_collapsed (AdwNavigationSplitView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self), FALSE);

  return self->collapsed;
}

/**
 * adw_navigation_split_view_set_collapsed: (attributes org.gtk.Method.set_property=collapsed)
 * @self: a navigation split view
 * @collapsed: whether @self is collapsed
 *
 * Sets whether @self is collapsed.
 *
 * When collapsed, the children are put inside an [class@NavigationView],
 * otherwise they are displayed side by side.
 *
 * The [property@NavigationSplitView:show-content] controls which child is
 * visible while collapsed.
 *
 * Since: 1.4
 */
void
adw_navigation_split_view_set_collapsed (AdwNavigationSplitView *self,
                                         gboolean                collapsed)
{
  g_return_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self));

  collapsed = !!collapsed;

  if (collapsed == self->collapsed)
    return;

  self->collapsed = collapsed;

  update_collapsed (self);
  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLLAPSED]);
}

/**
 * adw_navigation_split_view_get_show_content: (attributes org.gtk.Method.get_property=show-content)
 * @self: a navigation split view
 *
 * Gets which page is visible when @self is collapsed.
 *
 * Returns: whether to show content when collapsed
 *
 * Since: 1.4
 */
gboolean
adw_navigation_split_view_get_show_content (AdwNavigationSplitView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self), FALSE);

  return self->show_content;
}

/**
 * adw_navigation_split_view_set_show_content: (attributes org.gtk.Method.set_property=show-content)
 * @self: a navigation split view
 * @show_content: whether to show content when collapsed
 *
 * Sets which page is visible when @self is collapsed.
 *
 * If set to `TRUE`, the content widget will be the visible page when
 * [property@NavigationSplitView:collapsed] is `TRUE`; otherwise the sidebar
 * widget will be visible.
 *
 * If the split view is already collapsed, the visible page changes immediately.
 *
 * Since: 1.4
 */
void
adw_navigation_split_view_set_show_content (AdwNavigationSplitView *self,
                                            gboolean                show_content)
{
  g_return_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self));

  show_content = !!show_content;

  if (show_content == self->show_content)
    return;

  if (!self->navigation_view || !self->content || !self->sidebar) {
    self->show_content = show_content;

    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_CONTENT]);

    return;
  }

  self->changing_page = TRUE;
  g_idle_add_once ((GSourceOnceFunc) changing_page_done_cb, self);

  if (show_content)
    adw_navigation_view_push (ADW_NAVIGATION_VIEW (self->navigation_view),
                              self->content);
  else
    adw_navigation_view_pop_to_page (ADW_NAVIGATION_VIEW (self->navigation_view),
                                     self->sidebar);
}

/**
 * adw_navigation_split_view_get_min_sidebar_width: (attributes org.gtk.Method.get_property=min-sidebar-width)
 * @self: a navigation split view
 *
 * Gets the minimum sidebar width for @self.
 *
 * Returns: the minimum width
 *
 * Since: 1.4
 */
double
adw_navigation_split_view_get_min_sidebar_width (AdwNavigationSplitView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self), 0.0);

  return self->min_sidebar_width;
}

/**
 * adw_navigation_split_view_set_min_sidebar_width: (attributes org.gtk.Method.set_property=min-sidebar-width)
 * @self: a navigation split view
 * @width: the minimum width
 *
 * Sets the minimum sidebar width for @self.
 *
 * Minimum width is affected by
 * [property@NavigationSplitView:sidebar-width-unit].
 *
 * The sidebar widget can still be allocated with larger width if its own
 * minimum width exceeds it.
 *
 * Since: 1.4
 */
void
adw_navigation_split_view_set_min_sidebar_width (AdwNavigationSplitView *self,
                                                 double                  width)
{
  g_return_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self));

  if (G_APPROX_VALUE (self->min_sidebar_width, width, DBL_EPSILON))
    return;

  self->min_sidebar_width = width;

  if (!self->collapsed)
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MIN_SIDEBAR_WIDTH]);
}

/**
 * adw_navigation_split_view_get_max_sidebar_width: (attributes org.gtk.Method.get_property=max-sidebar-width)
 * @self: a navigation split view
 *
 * Gets the maximum sidebar width for @self.
 *
 * Returns: the maximum width
 *
 * Since: 1.4
 */
double
adw_navigation_split_view_get_max_sidebar_width (AdwNavigationSplitView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self), 0.0);

  return self->max_sidebar_width;
}

/**
 * adw_navigation_split_view_set_max_sidebar_width: (attributes org.gtk.Method.set_property=max-sidebar-width)
 * @self: a navigation split view
 * @width: the maximum width
 *
 * Sets the maximum sidebar width for @self.
 *
 * Maximum width is affected by
 * [property@NavigationSplitView:sidebar-width-unit].
 *
 * The sidebar widget can still be allocated with larger width if its own
 * minimum width exceeds it.
 *
 * Since: 1.4
 */
void
adw_navigation_split_view_set_max_sidebar_width (AdwNavigationSplitView *self,
                                                 double                  width)
{
  g_return_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self));

  if (G_APPROX_VALUE (self->max_sidebar_width, width, DBL_EPSILON))
    return;

  self->max_sidebar_width = width;

  if (!self->collapsed)
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAX_SIDEBAR_WIDTH]);
}

/**
 * adw_navigation_split_view_get_sidebar_width_fraction: (attributes org.gtk.Method.get_property=sidebar-width-fraction)
 * @self: a navigation split view
 *
 * Gets the preferred sidebar width fraction for @self.
 *
 * Returns: the preferred width fraction
 *
 * Since: 1.4
 */
double
adw_navigation_split_view_get_sidebar_width_fraction (AdwNavigationSplitView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self), 0.0);

  return self->sidebar_width_fraction;
}

/**
 * adw_navigation_split_view_set_sidebar_width_fraction: (attributes org.gtk.Method.set_property=sidebar-width-fraction)
 * @self: a navigation split view
 * @fraction: the preferred width fraction
 *
 * Sets the preferred sidebar width as a fraction of the total width of @self.
 *
 * The preferred width is additionally limited by
 * [property@NavigationSplitView:min-sidebar-width] and
 * [property@NavigationSplitView:max-sidebar-width].
 *
 * The sidebar widget can be allocated with larger width if its own minimum
 * width exceeds the preferred width.
 *
 * Since: 1.4
 */
void
adw_navigation_split_view_set_sidebar_width_fraction (AdwNavigationSplitView *self,
                                                      double                  fraction)
{
  g_return_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self));

  if (G_APPROX_VALUE (self->sidebar_width_fraction, fraction, DBL_EPSILON))
    return;

  self->sidebar_width_fraction = fraction;

  if (!self->collapsed)
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR_WIDTH_FRACTION]);
}

/**
 * adw_navigation_split_view_get_sidebar_width_unit: (attributes org.gtk.Method.get_property=sidebar-width-unit)
 * @self: a navigation split view
 *
 * Gets the length unit for minimum and maximum sidebar widths.
 *
 * Returns: the length unit
 *
 * Since: 1.4
 */
AdwLengthUnit
adw_navigation_split_view_get_sidebar_width_unit (AdwNavigationSplitView *self)
{
  g_return_val_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self), ADW_LENGTH_UNIT_PX);

  return self->sidebar_width_unit;
}

/**
 * adw_navigation_split_view_set_sidebar_width_unit: (attributes org.gtk.Method.set_property=sidebar-width-unit)
 * @self: a navigation split view
 * @unit: the length unit
 *
 * Sets the length unit for minimum and maximum sidebar widths.
 *
 * See [property@NavigationSplitView:min-sidebar-width] and
 * [property@NavigationSplitView:max-sidebar-width].
 *
 * Since: 1.4
 */
void
adw_navigation_split_view_set_sidebar_width_unit (AdwNavigationSplitView *self,
                                                  AdwLengthUnit           unit)
{
  g_return_if_fail (ADW_IS_NAVIGATION_SPLIT_VIEW (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  if (unit == self->sidebar_width_unit)
    return;

  self->sidebar_width_unit = unit;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR_WIDTH_UNIT]);
}
