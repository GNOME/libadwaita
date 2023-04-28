/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-back-button-private.h"
#include "adw-navigation-view.h"
#include "adw-widget-utils-private.h"

typedef struct {
  AdwBackButton *self;
  AdwNavigationView *view;
  AdwNavigationPage *page;
} NavigationViewData;

struct _AdwBackButton {
  GtkButton parent_instance;

  GSList *navigation_views;

  AdwNavigationPage *page;
  GBinding *title_binding;
};

G_DEFINE_FINAL_TYPE (AdwBackButton, adw_back_button, GTK_TYPE_BUTTON)

static gboolean
title_to_tooltip (GBinding     *binding,
                  const GValue *from,
                  GValue       *to,
                  gpointer      user_data)
{
  const char *title = g_value_get_string (from);

  g_value_set_string (to, (title && *title) ? title : _("Back"));

  return TRUE;
}

static void
update_page (AdwBackButton *self)
{
  AdwNavigationPage *prev_page = NULL;
  GSList *l;

  for (l = self->navigation_views; l; l = l->next) {
    NavigationViewData *data = l->data;

    prev_page = adw_navigation_view_get_previous_page (data->view, data->page);

    if (!adw_navigation_page_get_can_pop (data->page)) {
      prev_page = NULL;
      break;
    }

    if (prev_page)
      break;
  }

  if (prev_page == self->page)
    return;

  self->page = prev_page;

  gtk_widget_set_visible (GTK_WIDGET (self), !!prev_page);

  g_clear_pointer (&self->title_binding, g_binding_unbind);

  if (prev_page) {
    self->title_binding =
      g_object_bind_property_full (prev_page, "title",
                                   self, "tooltip-text",
                                   G_BINDING_SYNC_CREATE,
                                   title_to_tooltip, NULL, NULL, NULL);
  }
}

static void
pushed_cb (NavigationViewData *data)
{
  AdwNavigationPage *visible_page;

  g_assert (data->self);
  g_assert (data->view);
  g_assert (data->page);

  visible_page = adw_navigation_view_get_visible_page (data->view);

  if (visible_page != data->page)
    return;

  update_page (data->self);
}

static void
adw_back_button_root (GtkWidget *widget)
{
  AdwBackButton *self = ADW_BACK_BUTTON (widget);
  GtkWidget *page;

  GTK_WIDGET_CLASS (adw_back_button_parent_class)->root (widget);


  page = adw_widget_get_ancestor_same_native (widget, ADW_TYPE_NAVIGATION_PAGE);

  while (page) {
    GtkWidget *view = gtk_widget_get_parent (page);

    if (ADW_IS_NAVIGATION_VIEW (view)) {
      NavigationViewData *data = g_new0 (NavigationViewData, 1);

      data->self = self;
      data->view = ADW_NAVIGATION_VIEW (view);
      data->page = ADW_NAVIGATION_PAGE (page);

      g_signal_connect_swapped (data->view, "pushed", G_CALLBACK (pushed_cb), data);
      g_signal_connect_swapped (data->view, "replaced",
                                G_CALLBACK (update_page), self);
      g_signal_connect_swapped (data->page, "notify::can-pop",
                                G_CALLBACK (update_page), self);

      self->navigation_views = g_slist_prepend (self->navigation_views, data);
    }

    page = adw_widget_get_ancestor_same_native (view, ADW_TYPE_NAVIGATION_PAGE);
  }

  self->navigation_views = g_slist_reverse (self->navigation_views);

  update_page (self);
}

static void
adw_back_button_unroot (GtkWidget *widget)
{
  AdwBackButton *self = ADW_BACK_BUTTON (widget);
  GSList *l;

  for (l = self->navigation_views; l; l = l->next) {
    NavigationViewData *data = l->data;

    g_signal_handlers_disconnect_by_func (data->view, pushed_cb, data);
    g_signal_handlers_disconnect_by_func (data->view, update_page, self);
    g_signal_handlers_disconnect_by_func (data->page, update_page, self);
  }

  g_clear_pointer (&self->navigation_views, g_slist_free);

  update_page (self);

  GTK_WIDGET_CLASS (adw_back_button_parent_class)->unroot (widget);
}

static void
adw_back_button_class_init (AdwBackButtonClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->root = adw_back_button_root;
  widget_class->unroot = adw_back_button_unroot;
}

static void
adw_back_button_init (AdwBackButton *self)
{
  gtk_actionable_set_action_name (GTK_ACTIONABLE (self), "navigation.pop");
  gtk_button_set_icon_name (GTK_BUTTON (self), "go-previous-symbolic");
  gtk_widget_add_css_class (GTK_WIDGET (self), "back");
  gtk_widget_set_visible (GTK_WIDGET (self), FALSE);
}

GtkWidget *
adw_back_button_new (void)
{
  return g_object_new (ADW_TYPE_BACK_BUTTON, NULL);
}
