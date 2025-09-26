/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-shortcuts-dialog.h"

#include "adw-preferences-group.h"
#include "adw-preferences-page.h"
#include "adw-shortcut-row-private.h"
#include "adw-view-stack.h"
#include "adw-widget-utils-private.h"
#include "adw-wrap-box.h"

#define N_BUILTIN_WIDGETS 1
#define N_MIN_SECTIONS 4
#define N_MIN_SHORTCUTS 20

/**
 * AdwShortcutsDialog:
 *
 * A dialog that displays application's keyboard shortcuts.
 *
 * <picture>
 *   <source srcset="shortcuts-dialog-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="shortcuts-dialog.png" alt="shortcuts-dialog">
 * </picture>
 *
 * Shortcuts are grouped into sections, represented by [class@ShortcutsSection]
 * objects. Each section has one or more items, represented by
 * [class@ShortcutsItem] objects.
 *
 * To add a section to the dialog, use [method@ShortcutsDialog.add], or add it
 * as a child when using UI files.
 *
 * Sections without titles can be used to further subdivide each section into
 * groups.
 *
 * Example of an `AdwShortcutsDialog` UI definition:
 *
 * ```xml
 * <object class="AdwShortcutsDialog" id="shortcuts_dialog">
 *   <child>
 *     <object class="AdwShortcutsSection">
 *       <property name="title" translatable="yes">General</property>
 *       <child>
 *         <object class="AdwShortcutsItem">
 *           <property name="title" translatable="yes">Open Menu</property>
 *           <property name="accelerator">F10</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="AdwShortcutsItem">
 *           <property name="title" translatable="yes">Quit</property>
 *           <property name="action-name">app.quit</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="AdwShortcutsSection">
 *       <child>
 *         <object class="AdwShortcutsItem">
 *           <property name="title" translatable="yes">Move Tab Left</property>
 *           <property name="accelerator">&lt;Shift&gt;&lt;Ctrl&gt;Page_Up</property>
 *           <property name="direction">ltr</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="AdwShortcutsItem">
 *           <property name="title" translatable="yes">Move Tab Right</property>
 *           <property name="accelerator">&lt;Shift&gt;&lt;Ctrl&gt;Page_Down</property>
 *           <property name="direction">ltr</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="AdwShortcutsItem">
 *           <property name="title" translatable="yes">Move Tab Right</property>
 *           <property name="accelerator">&lt;Shift&gt;&lt;Ctrl&gt;Page_Up</property>
 *           <property name="direction">rtl</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="AdwShortcutsItem">
 *           <property name="title" translatable="yes">Move Tab Left</property>
 *           <property name="accelerator">&lt;Shift&gt;&lt;Ctrl&gt;Page_Down</property>
 *           <property name="direction">rtl</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * If the `app.quit` action has the <kbd>Ctrl</kbd><kbd>Q</kbd> accelerator
 * associated with it, the result will look as follows:
 *
 * <picture>
 *   <source srcset="shortcuts-dialog-example-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="shortcuts-dialog-example.png" alt="shortcuts-dialog-example">
 * </picture>
 *
 * The recommended way to use `AdwShortcutsDialog` is via [class@Application]'s
 * automatic resource loading.
 *
 * See also: [class@ShortcutLabel].
 *
 * Since: 1.8
 */

struct _AdwShortcutsDialog
{
  AdwDialog parent_instance;

  GtkSearchEntry *search_entry;
  AdwViewStack *stack;
  AdwPreferencesGroup *nav_group;
  AdwWrapBox *nav_box;
  AdwPreferencesPage *contents;
  AdwPreferencesPage *search;
  GtkListBox *search_list;
  GtkWidget *empty;

  GListStore *sections;

  GtkCustomFilter *direction_filter;
  GtkMapListModel *filtered_sections;

  GtkFilterListModel *title_sections;

  GListModel *all_rows;
  GtkStringFilter *title_filter;
  GtkStringFilter *subtitle_filter;
  GtkFilterListModel *search_model;

  AdwShortcutsSection *selected_section;
  GtkWidget *selected_button;
};

static void adw_shortcuts_dialog_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwShortcutsDialog, adw_shortcuts_dialog, ADW_TYPE_DIALOG,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_shortcuts_dialog_buildable_init))

static void
select_section (AdwShortcutsDialog  *self,
                AdwShortcutsSection *section)
{
  guint start, end, i, n = g_list_model_get_n_items (G_LIST_MODEL (self->sections));

  self->selected_section = section;

  start = end = GTK_INVALID_LIST_POSITION;

  if (self->selected_section) {
    for (i = 0; i < n; i++) {
      AdwShortcutsSection *s = g_list_model_get_item (G_LIST_MODEL (self->sections), i);

      if (start == GTK_INVALID_LIST_POSITION) {
        if (s == section) {
          start = i;
          end = i;
          continue;
        }
      } else {
        const char *title = adw_shortcuts_section_get_title (s);
        if (title && *title)
          break;

        end = i;
      }
    }
  }

  for (i = 0; i < n; i++) {
    AdwPreferencesGroup *group = adw_preferences_page_get_group (self->contents, i + N_BUILTIN_WIDGETS);

    if (section)
      gtk_widget_set_visible (GTK_WIDGET (group), i >= start && i <= end);
    else
      gtk_widget_set_visible (GTK_WIDGET (group), true);
  }
}

static void
unselect_section (AdwShortcutsDialog *self)
{
  select_section (self, NULL);

  if (self->selected_button) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->selected_button), FALSE);
    self->selected_button = NULL;
  }
}

static void
nav_button_clicked_cb (AdwShortcutsDialog *self,
                       GtkWidget          *button)
{
  AdwShortcutsSection *section = g_object_get_data (G_OBJECT (button), "-adw-nav-button-section");

  if (self->selected_section != section) {
    select_section (self, section);

    if (self->selected_button) {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->selected_button), FALSE);
      self->selected_button = NULL;
    }
    self->selected_button = button;
  } else {
    unselect_section (self);
  }
}

static void
update_nav_visibility (AdwShortcutsDialog *self)
{
  gboolean has_many_sections = g_list_model_get_n_items (G_LIST_MODEL (self->title_sections)) >= N_MIN_SECTIONS;
  gboolean has_many_shortcuts = g_list_model_get_n_items (self->all_rows) >= N_MIN_SHORTCUTS;
  gboolean is_search = adw_view_stack_get_visible_child (self->stack) != GTK_WIDGET (self->contents);

  gtk_widget_set_visible (GTK_WIDGET (self->nav_group), has_many_sections && has_many_shortcuts);

  if (is_search && self->selected_section)
    unselect_section (self);
}

static void
update_stack (AdwShortcutsDialog *self)
{
  const char *text = gtk_editable_get_text (GTK_EDITABLE (self->search_entry));

  if (text && *text) {
    if (g_list_model_get_n_items (G_LIST_MODEL (self->search_model)) > 0)
      adw_view_stack_set_visible_child (self->stack, GTK_WIDGET (self->search));
    else
      adw_view_stack_set_visible_child (self->stack, GTK_WIDGET (self->empty));
  } else {
    adw_view_stack_set_visible_child (self->stack, GTK_WIDGET (self->contents));
  }

  update_nav_visibility (self);
}

static void
stop_search (AdwShortcutsDialog *self)
{
  gtk_editable_set_text (GTK_EDITABLE (self->search_entry), "");
  gtk_string_filter_set_search (self->title_filter, "");
  gtk_string_filter_set_search (self->subtitle_filter, "");
  update_stack (self);
}

static GtkWidget *
find_row (AdwShortcutsDialog *self,
          AdwShortcutsItem   *the_item)
{
  guint i, j, n, m;
  gboolean found = FALSE;
  AdwPreferencesGroup *group;
  GtkWidget *row;

  n = g_list_model_get_n_items (G_LIST_MODEL (self->filtered_sections));

  for (i = 0; i < n; i++) {
    GListModel *section;

    section = g_list_model_get_item (G_LIST_MODEL (self->filtered_sections), i);
    m = g_list_model_get_n_items (section);

    for (j = 0; j < m; j++) {
      AdwShortcutsItem *item = g_list_model_get_item (section, j);

      if (item == the_item)
        found = TRUE;

      g_object_unref (item);

      if (found)
        break;
    }

    g_object_unref (section);

    if (found)
      break;
  }

  if (!found)
    return NULL;

  group = adw_preferences_page_get_group (self->contents, i + N_BUILTIN_WIDGETS);
  g_assert (group != NULL);

  row = adw_preferences_group_get_row (group, j);
  g_assert (row != NULL);

  return row;
}

static void
search_row_activated_cb (AdwShortcutsDialog *self,
                         GtkListBoxRow      *row)
{
  AdwShortcutsItem *item = adw_shortcut_row_get_item (ADW_SHORTCUT_ROW (row));
  GtkWidget *source_row = find_row (self, item);

  stop_search (self);

  if (source_row) {
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

    gtk_widget_grab_focus (source_row);

    if (GTK_IS_WINDOW (root))
      gtk_window_set_focus_visible (GTK_WINDOW (root), TRUE);
  }
}

static void
search_started_cb (AdwShortcutsDialog *self)
{
  gtk_widget_grab_focus (GTK_WIDGET (self->search_entry));
  gtk_editable_set_position (GTK_EDITABLE (self->search_entry), -1);
}

static void
search_changed_cb (AdwShortcutsDialog *self)
{
  const char *text = gtk_editable_get_text (GTK_EDITABLE (self->search_entry));

  gtk_string_filter_set_search (self->title_filter, text);
  gtk_string_filter_set_search (self->subtitle_filter, text);

  update_stack (self);
}

static void
stop_search_cb (AdwShortcutsDialog *self)
{
  if (adw_view_stack_get_visible_child (self->stack) != GTK_WIDGET (self->contents)) {
    stop_search (self);
    return;
  }

  adw_dialog_close (ADW_DIALOG (self));
}

static GtkWidget *
create_row (AdwShortcutsItem   *item,
            AdwShortcutsDialog *self)
{
  GtkWidget *row = adw_shortcut_row_new (item);

  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), FALSE);

  return row;
}

static GtkWidget *
create_search_row (AdwShortcutsItem   *item,
                   AdwShortcutsDialog *self)
{
  return adw_shortcut_row_new (item);
}

static gboolean
escape_markup (GBinding     *binding,
               const GValue *from_value,
               GValue       *to_value,
               gpointer      user_data)
{
  const char *str = g_value_get_string (from_value);

  if (str && *str)
    g_value_take_string (to_value, g_markup_escape_text (str, -1));
  else
    g_value_take_string (to_value, g_strdup (""));

  return TRUE;
}

static AdwPreferencesGroup *
create_section (AdwShortcutsDialog  *self,
                AdwShortcutsSection *section,
                GListModel          *items)
{
  GtkWidget *group = adw_preferences_group_new ();

  g_object_bind_property_full (section, "title", group, "title", G_BINDING_SYNC_CREATE,
                               escape_markup, NULL, NULL, NULL);

  adw_preferences_group_bind_model (ADW_PREFERENCES_GROUP (group), items,
                                    (GtkListBoxCreateWidgetFunc) create_row,
                                    self, NULL);

  return ADW_PREFERENCES_GROUP (group);
}

static GtkWidget *
create_nav_button (AdwShortcutsDialog  *self,
                   AdwShortcutsSection *section)
{
  GtkWidget *button = gtk_toggle_button_new ();

  gtk_button_set_can_shrink (GTK_BUTTON (button), TRUE);

  g_object_bind_property (section, "title", button, "label", G_BINDING_SYNC_CREATE);

  g_object_set_data (G_OBJECT (button), "-adw-nav-button-section", section);

  g_signal_connect_swapped (button, "clicked", G_CALLBACK (nav_button_clicked_cb), self);

  return button;
}

static void
sections_changed_cb (AdwShortcutsDialog *self,
                     guint               index,
                     guint               removed,
                     guint               added,
                     GListModel         *sections)
{
  guint i;

  for (i = 0; i < removed; i++) {
    AdwPreferencesGroup *group = adw_preferences_page_get_group (self->contents, index + N_BUILTIN_WIDGETS);

    adw_preferences_page_remove (self->contents, group);
  }

  for (i = 0; i < added; i++) {
    GListModel *items = g_list_model_get_item (sections, index + i);
    GListModel *model = gtk_filter_list_model_get_model (GTK_FILTER_LIST_MODEL (items));
    AdwShortcutsSection *section = ADW_SHORTCUTS_SECTION (model);
    AdwPreferencesGroup *group = create_section (self, section, items);

    adw_preferences_page_insert (self->contents, group, index + added + N_BUILTIN_WIDGETS);

    g_object_unref (section);
  }
}

static void
titles_changed_cb (AdwShortcutsDialog *self,
                   guint               index,
                   guint               removed,
                   guint               added,
                   GListModel         *sections)
{
  GtkWidget *sibling = NULL;
  guint i;

  update_nav_visibility (self);

  if (index > 0)
    sibling = adw_widget_get_nth_child (GTK_WIDGET (self->nav_box), index - 1);

  for (i = 0; i < removed; i++) {
    if (sibling)
      adw_wrap_box_remove (self->nav_box, gtk_widget_get_next_sibling (sibling));
    else
      adw_wrap_box_remove (self->nav_box, gtk_widget_get_first_child (GTK_WIDGET (self->nav_box)));
  }

  for (i = 0; i < added; i++) {
    AdwShortcutsSection *section = ADW_SHORTCUTS_SECTION (g_list_model_get_item (sections, index + i));
    GtkWidget *button = create_nav_button (self, section);

    adw_wrap_box_insert_child_after (self->nav_box, button, sibling);
    sibling = button;

    g_object_unref (section);
  }
}

static gboolean
unselect_section_cb (GtkWidget *widget,
                     GVariant  *args,
                     gpointer   user_data)
{
  AdwShortcutsDialog *self = ADW_SHORTCUTS_DIALOG (widget);

  if (!self->selected_section)
    return GDK_EVENT_PROPAGATE;

  unselect_section (self);
  return GDK_EVENT_STOP;
}

static gboolean
search_shortcut_cb (GtkWidget *widget,
                    GVariant  *args,
                    gpointer   user_data)
{
  AdwShortcutsDialog *self = ADW_SHORTCUTS_DIALOG (widget);

  gtk_widget_grab_focus (GTK_WIDGET (self->search_entry));

  return GDK_EVENT_STOP;
}

static void
title_changed_cb (AdwShortcutsDialog *self)
{
  GtkFilter *filter = gtk_filter_list_model_get_filter (self->title_sections);

  gtk_filter_changed (filter, GTK_FILTER_CHANGE_DIFFERENT);
}

static void
adw_shortcuts_dialog_root (GtkWidget *widget)
{
  GTK_WIDGET_CLASS (adw_shortcuts_dialog_parent_class)->root (widget);

  /* The previous size was calculated with empty content, so recalculate it */
  adw_dialog_set_content_height (ADW_DIALOG (widget), -1);
}

static void
adw_shortcuts_dialog_direction_changed (GtkWidget        *widget,
                                        GtkTextDirection  previous_direction)
{
  AdwShortcutsDialog *self = ADW_SHORTCUTS_DIALOG (widget);

  gtk_filter_changed (GTK_FILTER (self->direction_filter), GTK_FILTER_CHANGE_DIFFERENT);
}

static void
adw_shortcuts_dialog_dispose (GObject *object)
{
  AdwShortcutsDialog *self = ADW_SHORTCUTS_DIALOG (object);

  g_clear_object (&self->sections);
  g_clear_object (&self->search_model);
  g_clear_object (&self->title_sections);
  g_clear_object (&self->direction_filter);
  self->all_rows = NULL;
  self->title_filter = NULL;
  self->subtitle_filter = NULL;
  self->filtered_sections = NULL;

  G_OBJECT_CLASS (adw_shortcuts_dialog_parent_class)->dispose (object);
}

static void
adw_shortcuts_dialog_class_init (AdwShortcutsDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_shortcuts_dialog_dispose;

  widget_class->root = adw_shortcuts_dialog_root;
  widget_class->direction_changed = adw_shortcuts_dialog_direction_changed;

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-shortcuts-dialog.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, search_entry);
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, stack);
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, nav_group);
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, nav_box);
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, contents);
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, search);
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, search_list);
  gtk_widget_class_bind_template_child (widget_class, AdwShortcutsDialog, empty);
  gtk_widget_class_bind_template_callback (widget_class, search_started_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, stop_search_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_row_activated_cb);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_f, GDK_CONTROL_MASK, search_shortcut_cb, NULL);
}

static GObject *
section_to_filter_model (AdwShortcutsSection *section,
                         AdwShortcutsDialog  *self)
{
  GListModel *model = G_LIST_MODEL (g_object_ref (section));
  GtkFilter *filter = GTK_FILTER (g_object_ref (self->direction_filter));

  return G_OBJECT (gtk_filter_list_model_new (model, filter));
}

static gboolean
section_title_filter_func (AdwShortcutsSection *item)
{
  const char *title = adw_shortcuts_section_get_title (item);

  return title && *title;
}

static gboolean
direction_filter_func (AdwShortcutsItem   *item,
                       AdwShortcutsDialog *self)
{
  GtkTextDirection dir = adw_shortcuts_item_get_direction (item);

  if (dir == GTK_TEXT_DIR_NONE)
    return TRUE;

  return dir == gtk_widget_get_direction (GTK_WIDGET (self));
}

static void
adw_shortcuts_dialog_init (AdwShortcutsDialog *self)
{
  GtkExpression *title_expr, *subtitle_expr;
  GtkFilter *filter, *section_title_filter;

  gtk_widget_init_template (GTK_WIDGET (self));

  self->sections = g_list_store_new (ADW_TYPE_SHORTCUTS_SECTION);

  gtk_search_entry_set_key_capture_widget (self->search_entry, GTK_WIDGET (self));

  title_expr = gtk_property_expression_new (ADW_TYPE_SHORTCUTS_ITEM,
                                            NULL, "title");
  subtitle_expr = gtk_property_expression_new (ADW_TYPE_SHORTCUTS_ITEM,
                                               NULL, "subtitle");

  self->title_filter = gtk_string_filter_new (title_expr);
  self->subtitle_filter = gtk_string_filter_new (subtitle_expr);
  self->direction_filter = gtk_custom_filter_new ((GtkCustomFilterFunc) direction_filter_func,
                                                  self, NULL);

  filter = GTK_FILTER (gtk_any_filter_new ());
  gtk_multi_filter_append (GTK_MULTI_FILTER (filter), GTK_FILTER (self->title_filter));
  gtk_multi_filter_append (GTK_MULTI_FILTER (filter), GTK_FILTER (self->subtitle_filter));

  section_title_filter = GTK_FILTER (gtk_custom_filter_new ((GtkCustomFilterFunc) section_title_filter_func,
                                                            NULL, NULL));
  self->title_sections = gtk_filter_list_model_new (G_LIST_MODEL (g_object_ref (self->sections)),
                                                    section_title_filter);

  g_signal_connect_object (self->title_sections, "items-changed",
                           G_CALLBACK (titles_changed_cb), self,
                           G_CONNECT_SWAPPED);

  self->filtered_sections = gtk_map_list_model_new (G_LIST_MODEL (g_object_ref (self->sections)),
                                                    (GtkMapListModelMapFunc) section_to_filter_model,
                                                    self, NULL);
  self->all_rows = G_LIST_MODEL (gtk_flatten_list_model_new (G_LIST_MODEL (self->filtered_sections)));
  self->search_model = gtk_filter_list_model_new (self->all_rows, filter);

  g_signal_connect_object (self->filtered_sections, "items-changed",
                           G_CALLBACK (sections_changed_cb), self,
                           G_CONNECT_SWAPPED);

  gtk_list_box_bind_model (self->search_list,
                           G_LIST_MODEL (self->search_model),
                           (GtkListBoxCreateWidgetFunc) create_search_row,
                           self,
                           NULL);

  g_signal_connect_swapped (self->search_model, "notify::n-items",
                            G_CALLBACK (update_stack), self);

  GtkShortcut *shortcut;
  GtkEventController *controller;

  /* Esc to close */
  shortcut = gtk_shortcut_new (gtk_keyval_trigger_new (GDK_KEY_Escape, 0),
                               gtk_callback_action_new ((GtkShortcutFunc) unselect_section_cb, self, NULL));

  controller = gtk_shortcut_controller_new ();
  gtk_shortcut_controller_set_scope (GTK_SHORTCUT_CONTROLLER (controller), GTK_SHORTCUT_SCOPE_MANAGED);
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (controller), shortcut);
  gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_CAPTURE);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);
}

static void
adw_shortcuts_dialog_add_child (GtkBuildable *buildable,
                                GtkBuilder   *builder,
                                GObject      *child,
                                const char   *type)
{
  AdwShortcutsDialog *self = ADW_SHORTCUTS_DIALOG (buildable);

  if (ADW_IS_SHORTCUTS_SECTION (child)) {
    adw_shortcuts_dialog_add (self, g_object_ref (ADW_SHORTCUTS_SECTION (child)));
  } else {
    g_warning ("Cannot add an object of type %s to AdwShortcutsDialog",
               g_type_name (G_OBJECT_TYPE (child)));
  }
}

static void
adw_shortcuts_dialog_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = adw_shortcuts_dialog_add_child;
}

/**
 * adw_shortcuts_dialog_new:
 *
 * Creates a new `AdwShortcutsDialog`.
 *
 * Returns: the newly created `AdwShortcutsDialog`
 *
 * Since: 1.8
 */
AdwDialog *
adw_shortcuts_dialog_new (void)
{
  return g_object_new (ADW_TYPE_SHORTCUTS_DIALOG, NULL);
}

/**
 * adw_shortcuts_dialog_add:
 * @self: a shortcuts dialog
 * @section: (transfer full): the section to add
 *
 * Adds @section to @self.
 *
 * Since: 1.8
 */
void
adw_shortcuts_dialog_add (AdwShortcutsDialog  *self,
                          AdwShortcutsSection *section)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_DIALOG (self));
  g_return_if_fail (ADW_IS_SHORTCUTS_SECTION (section));

  g_list_store_append (self->sections, section);

  g_signal_connect_swapped (section, "notify::title", G_CALLBACK (title_changed_cb), self);
}
