/* adw-tagged-entry.c: Tagged entry widget
 *
 * SPDX-FileCopyrightText: 2022 Emmanuele Bassi
 * SPDX-FileCopyrightText: 2019 Matthias Clasen
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-tagged-entry.h"

#include "adw-tag-match-private.h"
#include "adw-tag-widget-private.h"

/**
 * AdwTaggedEntry:
 *
 * An entry that allows you to have tags near the text.
 *
 * ## AdwTaggedEntry as GtkBuildable
 *
 * You can include tags directly inside the UI definition of a tagged entry
 * by using the `<child>` element to add objects of type [class@Adw.Tag]; for
 * instance, the following definition:
 *
 * ```xml
 * <object class="AdwTaggedEntry">
 *   <child>
 *     <object class="AdwTag">
 *       <property name="gicon">
 *         <object class="GThemedIcon">
 *           <property name="names">go-down-symbolic</property>
 *         </object>
 *       </property>
 *       <property name="label">First Tag</property>
 *       <property name="show-close">False</property>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * while create an `AdwTaggedEntry` with a single tag, whose label is set to
 * "First Tag"; the tag will not have a "close" button.
 *
 * ## CSS nodes
 *
 * `AdwTaggedEntry` has a single CSS node with the name `entry` and the
 * CSS class `tagged`.
 *
 * Each tag has a single CSS node with the name `tag`, and contains a `label`
 * node; if the [property@Adw.Tag:has-icon] property is true, the tag will have
 * an `image` node; similarly, if the [property@Adw.Tag:show-close] is true,
 * the tag will have a `button` node.
 *
 * ```
 * entry.tagged
 * ├── .tags
 * ┊   ├── tag
 * ┊   ┊   ├── [image]
 * ┊   ┊   ├── label
 * ┊   ┊   ╰── [button]
 * ┊   ┊
 * ┊   ╰── tag
 * ╰── text
 * ```
 */
struct _AdwTaggedEntry
{
  GtkWidget parent_instance;

  GtkWidget *tags_box;
  GtkWidget *text;

  GListModel *tags;

  GHashTable *widget_for_tag;

  char *delimiters;
  char *search;

  guint idle_match_id;

  GString *buffer;

  /* Completion popover */
  GtkWidget *popover;
  GtkWidget *list_view;

  GtkListItemFactory *factory;
  GtkFilter *filter;
  GtkMapListModel *map_model;
  GtkSingleSelection *selection;

  GtkExpression *match_expression;
  GListModel *match_model;

  AdwTaggedEntryMatchFunc match_func;
  gpointer match_func_data;
  GDestroyNotify match_func_notify;
};

enum
{
  PROP_PLACEHOLDER_TEXT = 1,
  PROP_DELIMITER_CHARS,
  PROP_MATCH_MODEL,
  PROP_MATCH_EXPRESSION,
  N_PROPS
};

static void buildable_iface_init (GtkBuildableIface *iface);
static void editable_iface_init (GtkEditableInterface *iface);

static AdwTag *default_match_func (AdwTaggedEntry *self,
                                   const char     *text,
                                   gpointer        item,
                                   gpointer        user_data);

static void adw_tagged_entry_update_map (AdwTaggedEntry *self);

static void on_text_notify (GtkText        *text,
                            GParamSpec     *pspec,
                            AdwTaggedEntry *self);

static GtkBuildableIface *parent_buildable_iface;

static GParamSpec *entry_props[N_PROPS];

G_DEFINE_TYPE_WITH_CODE (AdwTaggedEntry, adw_tagged_entry, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, buildable_iface_init)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_EDITABLE, editable_iface_init))

static GtkEditable *
adw_tagged_entry_editable_get_delegate (GtkEditable *editable)
{
  AdwTaggedEntry *self = ADW_TAGGED_ENTRY (editable);

  return GTK_EDITABLE (self->text);
}

static void
on_text_insert_text (GtkEditable *editable,
                     const char *text,
                     int length,
                     int *position,
                     AdwTaggedEntry *self)
{
  if (self->match_model != NULL)
    return;

  if (self->delimiters == NULL)
    return;

  if (self->buffer == NULL)
    self->buffer = g_string_new (NULL);

  g_string_append (self->buffer, text);

  char last = self->buffer->str[self->buffer->len - 1];

  gsize delimiters_len = strlen (self->delimiters);
  for (gsize i = 0; i < delimiters_len; i++) {
    if (last == self->delimiters[i]) {
      AdwTag *tag = adw_tag_new ();

      g_autofree char *label = g_strndup (self->buffer->str, self->buffer->len - 1);

      adw_tag_set_show_close (tag, TRUE);
      adw_tag_set_label (tag, label);

      adw_tagged_entry_add_tag (self, tag);

      gtk_editable_delete_text (editable, 0, -1);

      g_signal_stop_emission_by_name (editable, "insert-text");
      break;
    }
  }
}

static void
on_text_delete_text (GtkEditable *editable,
                     int start,
                     int end,
                     AdwTaggedEntry *self)
{
  if (self->buffer == NULL)
    return;

  if (self->match_model != NULL)
    return;

  if (self->delimiters == NULL)
    return;

  g_string_erase (self->buffer, start, end - start);
}

static void
editable_iface_init (GtkEditableInterface *iface)
{
  iface->get_delegate = adw_tagged_entry_editable_get_delegate;
}

static void
on_tag_closed (AdwTaggedEntry *self,
               AdwTag         *tag)
{
  adw_tagged_entry_remove_tag (self, tag);
}

static void
adw_tagged_entry_add_tag_internal (AdwTaggedEntry *self,
                                   AdwTag         *tag,
                                   gboolean        remove_ref)
{
  g_list_store_append (G_LIST_STORE (self->tags), tag);

  GtkWidget *tag_widget = g_object_new (ADW_TYPE_TAG_WIDGET,
                                        "tag", tag,
                                        NULL);

  g_signal_connect_swapped (tag_widget, "closed", G_CALLBACK (on_tag_closed), self);

  gtk_flow_box_append (GTK_FLOW_BOX (self->tags_box), tag_widget);

  g_hash_table_insert (self->widget_for_tag, tag, tag_widget);

  if (remove_ref)
    g_object_unref (tag);
}

static void
adw_tagged_entry_list_item__setup (AdwTaggedEntry           *self,
                                   GtkListItem              *item,
                                   GtkSignalListItemFactory *factory)
{
  GtkWidget *label = gtk_label_new (NULL);

  gtk_label_set_xalign (GTK_LABEL (label), 0.0);

  gtk_list_item_set_child (item, label);
}

static void
adw_tagged_entry_list_item__bind (AdwTaggedEntry           *self,
                                  GtkListItem              *item,
                                  GtkSignalListItemFactory *factory)
{
  AdwTagMatch *match = gtk_list_item_get_item (item);
  GtkWidget *label = gtk_list_item_get_child (item);

  gtk_label_set_text (GTK_LABEL (label), adw_tag_match_get_string (match));
}

static void
adw_tagged_entry_setup_list_factory (AdwTaggedEntry *self)
{
  GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();

  g_signal_connect_swapped (factory, "setup", G_CALLBACK (adw_tagged_entry_list_item__setup), self);
  g_signal_connect_swapped (factory, "bind", G_CALLBACK (adw_tagged_entry_list_item__bind), self);

  gtk_list_view_set_factory (GTK_LIST_VIEW (self->list_view), factory);
  g_object_unref (factory);

  self->factory = factory;
}

static void
adw_tagged_entry_set_popover_visible (AdwTaggedEntry *self,
                                      gboolean        visible)
{
  visible = !!visible;

  if (gtk_widget_get_visible (self->popover) == visible) {
    return;
  }

  if (g_list_model_get_n_items (G_LIST_MODEL (self->selection)) == 0) {
    visible = FALSE;
  }

  if (visible) {
    if (!gtk_widget_has_focus (self->text))
      gtk_text_grab_focus_without_selecting (GTK_TEXT (self->text));
    gtk_single_selection_set_selected (self->selection, GTK_INVALID_LIST_POSITION);
    gtk_popover_popup (GTK_POPOVER (self->popover));
  } else {
    gtk_popover_popdown (GTK_POPOVER (self->popover));
  }
}

static void
adw_tagged_entry_apply_selection (AdwTaggedEntry *self)
{
  AdwTagMatch *match = gtk_single_selection_get_selected_item (self->selection);
  if (match == NULL) {
    return;
  }

  AdwTag *tag = adw_tag_match_get_tag (match);

  adw_tagged_entry_add_tag_internal (self, tag, FALSE);

  g_signal_handlers_block_by_func (self->text, on_text_notify, self);
  gtk_editable_delete_text (GTK_EDITABLE (self->text), 0, -1);
  g_signal_handlers_unblock_by_func (self->text, on_text_notify, self);

  adw_tagged_entry_set_popover_visible (self, FALSE);
}

static void
on_list_row_activate (GtkListView *self,
                      guint        position,
                      gpointer     user_data)
{
  adw_tagged_entry_set_popover_visible (user_data, FALSE);
  adw_tagged_entry_apply_selection (user_data);
}

static inline gboolean
keyval_is_cursor_move (guint keyval)
{
  if (keyval == GDK_KEY_Up || keyval == GDK_KEY_KP_Up)
    return TRUE;

  if (keyval == GDK_KEY_Down || keyval == GDK_KEY_KP_Down)
    return TRUE;

  if (keyval == GDK_KEY_Page_Up || keyval == GDK_KEY_Page_Down)
    return TRUE;

  return FALSE;
}

#define PAGE_STEP 10

static gboolean
adw_tagged_entry__key_pressed (AdwTaggedEntry        *self,
                               guint                  keyval,
                               guint                  keycode,
                               GdkModifierType        state,
                               GtkEventControllerKey *controller)
{
  if (self->selection == NULL)
    return FALSE;

  if (state & (GDK_SHIFT_MASK | GDK_ALT_MASK | GDK_CONTROL_MASK))
    return FALSE;

  guint matches = g_list_model_get_n_items (G_LIST_MODEL (self->selection));

  if (keyval == GDK_KEY_Return ||
      keyval == GDK_KEY_KP_Enter ||
      keyval == GDK_KEY_ISO_Enter) {
    /* Shortcut: complete if there's only one match */
    if (matches == 1) {
      adw_tagged_entry_set_popover_visible (self, FALSE);
      gtk_single_selection_set_selected (self->selection, 0);
      adw_tagged_entry_apply_selection (self);
      return TRUE;
    }
    return FALSE;
  }

  if (keyval == GDK_KEY_Escape) {
    adw_tagged_entry_set_popover_visible (self, FALSE);
    return TRUE;
  }

  guint selected = gtk_single_selection_get_selected (self->selection);

  if (keyval_is_cursor_move (keyval)) {
    if (keyval == GDK_KEY_Up || keyval == GDK_KEY_KP_Up) {
      if (selected == 0)
        selected = GTK_INVALID_LIST_POSITION;
      else if (selected == GTK_INVALID_LIST_POSITION)
        selected = matches - 1;
      else
        selected -= 1;
    } else if (keyval == GDK_KEY_Down || keyval == GDK_KEY_KP_Down) {
      if (selected == matches - 1)
        selected = GTK_INVALID_LIST_POSITION;
      else if (selected == GTK_INVALID_LIST_POSITION)
        selected = 0;
      else
        selected += 1;
    } else if (keyval == GDK_KEY_Page_Up) {
      if (selected == 0)
        selected = GTK_INVALID_LIST_POSITION;
      else if (selected == GTK_INVALID_LIST_POSITION)
        selected = matches - 1;
      else if (selected >= PAGE_STEP)
        selected -= PAGE_STEP;
      else
        selected = 0;
    } else if (keyval == GDK_KEY_Page_Down) {
      if (selected == matches - 1)
        selected = GTK_INVALID_LIST_POSITION;
      else if (selected == GTK_INVALID_LIST_POSITION)
        selected = 0;
      else if (selected + PAGE_STEP < matches)
        selected += PAGE_STEP;
      else
        selected = matches - 1;
    }

    gtk_single_selection_set_selected (self->selection, selected);
    return TRUE;
  }

  return FALSE;
}

#undef PAGE_STEP

static gboolean
on_text_notify_idle (gpointer data)
{
  AdwTaggedEntry *self = data;

  if (self->map_model == NULL)
    goto out;

  const char *text = gtk_editable_get_text (GTK_EDITABLE (self->text));

  g_free (self->search);
  self->search = g_strdup (text);

  adw_tagged_entry_update_map (self);

  guint matches = g_list_model_get_n_items (G_LIST_MODEL (self->selection));

  adw_tagged_entry_set_popover_visible (self, matches > 0);

out:
  self->idle_match_id = 0;

  return G_SOURCE_REMOVE;
}

static void
on_text_notify (GtkText        *text,
                GParamSpec     *pspec,
                AdwTaggedEntry *self)
{
  if (self->match_model == NULL)
    return;

  if (self->idle_match_id == 0) {
    self->idle_match_id = g_idle_add (on_text_notify_idle, self);
    g_source_set_name_by_id (self->idle_match_id, "[adw] tagged entry text notify");
  }
}

static void
adw_tagged_entry_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  if (ADW_IS_TAG (child)) {
    adw_tagged_entry_add_tag_internal (ADW_TAGGED_ENTRY (buildable),
                                       ADW_TAG (child),
                                       FALSE);
  } else {
    parent_buildable_iface->add_child (buildable, builder, child, type);
  }
}

static void
buildable_iface_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_tagged_entry_buildable_add_child;
}

static void
adw_tagged_entry_set_property (GObject      *gobject,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwTaggedEntry *self = ADW_TAGGED_ENTRY (gobject);

  if (gtk_editable_delegate_set_property (gobject, prop_id, value, pspec))
    return;

  switch (prop_id) {
  case PROP_PLACEHOLDER_TEXT:
    adw_tagged_entry_set_placeholder_text (self, g_value_get_string (value));
    break;

  case PROP_DELIMITER_CHARS:
    adw_tagged_entry_set_delimiter_chars (self, g_value_get_string (value));
    break;

  case PROP_MATCH_MODEL:
    adw_tagged_entry_set_match_model (self, g_value_get_object (value));
    break;

  case PROP_MATCH_EXPRESSION:
    adw_tagged_entry_set_match_expression (self, gtk_value_get_expression (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

static void
adw_tagged_entry_get_property (GObject    *gobject,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwTaggedEntry *self = ADW_TAGGED_ENTRY (gobject);

  if (gtk_editable_delegate_get_property (gobject, prop_id, value, pspec))
    return;

  switch (prop_id) {
  case PROP_PLACEHOLDER_TEXT:
    g_value_set_string (value, adw_tagged_entry_get_placeholder_text (self));
    break;

  case PROP_DELIMITER_CHARS:
    g_value_set_string (value, self->delimiters);
    break;

  case PROP_MATCH_MODEL:
    g_value_set_object (value, self->match_model);
    break;

  case PROP_MATCH_EXPRESSION:
    gtk_value_set_expression (value, self->match_expression);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

static void
adw_tagged_entry_dispose (GObject *gobject)
{
  AdwTaggedEntry *self = ADW_TAGGED_ENTRY (gobject);

  if (self->text != NULL)
    gtk_editable_finish_delegate (GTK_EDITABLE (self));

  adw_tagged_entry_set_match_func (self, NULL, NULL, NULL);
  adw_tagged_entry_set_match_model (self, NULL);
  adw_tagged_entry_set_match_expression (self, NULL);

  g_clear_pointer (&self->text, gtk_widget_unparent);
  g_clear_pointer (&self->tags_box, gtk_widget_unparent);
  g_clear_pointer (&self->popover, gtk_widget_unparent);

  g_clear_object (&self->tags);
  g_clear_pointer (&self->widget_for_tag, g_hash_table_unref);

  g_clear_pointer (&self->delimiters, g_free);
  g_clear_pointer (&self->search, g_free);

  if (self->buffer != NULL) {
    g_string_free (self->buffer, TRUE);
    self->buffer = NULL;
  }

  G_OBJECT_CLASS (adw_tagged_entry_parent_class)->dispose (gobject);
}

static void
adw_tagged_entry_class_init (AdwTaggedEntryClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gobject_class->set_property = adw_tagged_entry_set_property;
  gobject_class->get_property = adw_tagged_entry_get_property;
  gobject_class->dispose = adw_tagged_entry_dispose;

  /**
   * AdwTaggedEntry:placeholder-text:
   *
   * The text that will be displayed in the tagged entry when it is empty
   * and unfocused.
   *
   * Since: 1.2
   */
  entry_props[PROP_PLACEHOLDER_TEXT] =
    g_param_spec_string ("placeholder-text", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);
  /**
   * AdwTaggedEntry:delimiter-chars:
   *
   * A set of characters used to denote a tag.
   *
   * If set to `NULL`, the tagged entry will not try to turn its contents
   * into tags.
   *
   * Since: 1.2
   */
  entry_props[PROP_DELIMITER_CHARS] =
    g_param_spec_string ("delimiter-chars", NULL, NULL,
                         " ,",
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);
  /**
   * AdwTaggedEntry:match-model:
   *
   * A list model containing all possible objects that can be matched
   * to the contents of the tagged entry.
   *
   * Since: 1.2
   */
  entry_props[PROP_MATCH_MODEL] =
    g_param_spec_object ("match-model", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);
  /**
   * AdwTaggedEntry:match-expression:
   *
   * An expression that can be used to match the contents of the
   * [property@TaggedEntry:match-model] with the contents of the
   * entry.
   *
   * Since: 1.2
   */
  entry_props[PROP_MATCH_EXPRESSION] =
    gtk_param_spec_expression ("match-expression", NULL, NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_STATIC_STRINGS |
                               G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (gobject_class, N_PROPS, entry_props);
  gtk_editable_install_properties (gobject_class, N_PROPS);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/ui/adw-tagged-entry.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwTaggedEntry, tags_box);
  gtk_widget_class_bind_template_child (widget_class, AdwTaggedEntry, text);
  gtk_widget_class_bind_template_child (widget_class, AdwTaggedEntry, popover);
  gtk_widget_class_bind_template_child (widget_class, AdwTaggedEntry, list_view);
  gtk_widget_class_bind_template_callback (widget_class, on_text_insert_text);
  gtk_widget_class_bind_template_callback (widget_class, on_text_delete_text);
  gtk_widget_class_bind_template_callback (widget_class, adw_tagged_entry__key_pressed);
  gtk_widget_class_bind_template_callback (widget_class, on_list_row_activate);

  gtk_widget_class_set_css_name (widget_class, "entry");
  gtk_widget_class_set_layout_manager_type (GTK_WIDGET_CLASS (klass), GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_TEXT_BOX);
}

static void
adw_tagged_entry_init (AdwTaggedEntry *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_widget_add_css_class (GTK_WIDGET (self), "tagged");

  gtk_editable_init_delegate (GTK_EDITABLE (self));

  g_signal_connect (self->text, "notify::text", G_CALLBACK (on_text_notify), self);

  self->tags = G_LIST_MODEL (g_list_store_new (ADW_TYPE_TAG));

  self->widget_for_tag = g_hash_table_new (NULL, NULL);

  self->delimiters = g_strdup (" ,");

  adw_tagged_entry_setup_list_factory (self);
}

/**
 * adw_tagged_entry_new:
 *
 * Creates a new tagged entry widget.
 *
 * Returns: (transfer floating): the new tagged entry widget
 *
 * Since: 1.2
 */
GtkWidget *
adw_tagged_entry_new (void)
{
  return g_object_new (ADW_TYPE_TAGGED_ENTRY, NULL);
}

/**
 * adw_tagged_entry_add_tag:
 * @self: the tagged entry we want to update
 * @tag: (transfer full): the tag object
 *
 * Adds a new @tag into the tagged entry.
 *
 * Since: 1.2
 */
void
adw_tagged_entry_add_tag (AdwTaggedEntry *self,
                          AdwTag         *tag)
{
  g_return_if_fail (ADW_IS_TAGGED_ENTRY (self));
  g_return_if_fail (ADW_IS_TAG (tag));

  guint n_tags = g_list_model_get_n_items (self->tags);
  for (guint i = 0; i < n_tags; i++) {
    g_autoptr(AdwTag) iter = g_list_model_get_item (self->tags, i);

    if (iter == tag) {
      g_critical ("Tag %p already set", tag);
      return;
    }
  }

  adw_tagged_entry_add_tag_internal (self, tag, TRUE);
}

/**
 * adw_tagged_entry_remove_tag:
 * @self: the tagged entry we want to update
 * @tag: the tag to remove
 *
 * Removes the given tag from the tagged entry.
 *
 * Since: 1.2
 */
void
adw_tagged_entry_remove_tag (AdwTaggedEntry *self,
                             AdwTag         *tag)
{
  g_return_if_fail (ADW_IS_TAGGED_ENTRY (self));
  g_return_if_fail (ADW_IS_TAG (tag));

  GtkWidget *tag_widget = g_hash_table_lookup (self->widget_for_tag, tag);
  if (tag_widget == NULL) {
    g_critical ("No widget found for tag %p", tag);
    return;
  }

  gtk_flow_box_remove (GTK_FLOW_BOX (self->tags_box), gtk_widget_get_parent (tag_widget));

  guint n_tags = g_list_model_get_n_items (self->tags);
  for (guint i = 0; i < n_tags; i++) {
    AdwTag *iter = g_list_model_get_item (self->tags, i);

    if (iter == tag) {
      g_list_store_remove (G_LIST_STORE (self->tags), i);
      g_object_unref (iter);
      break;
    }

    g_object_unref (iter);
  }
}

/**
 * adw_tagged_entry_get_tags:
 * @self: the tagged entry we want to query
 *
 * Retrieves a list model of all tags inside the tagged entry widget.
 *
 * Returns: (transfer full): a list model of all the tags
 *
 * Since: 1.2
 */
GListModel *
adw_tagged_entry_get_tags (AdwTaggedEntry *self)
{
  g_return_val_if_fail (ADW_IS_TAGGED_ENTRY (self), NULL);

  return self->tags;
}

/**
 * adw_tagged_entry_remove_all_tags:
 * @self: the tagged entry we want to change
 *
 * Removes all tags from the tagged entry widget.
 *
 * Since: 1.2
 */
void
adw_tagged_entry_remove_all_tags (AdwTaggedEntry *self)
{
  g_return_if_fail (ADW_IS_TAGGED_ENTRY (self));

  GtkWidget *child = gtk_widget_get_first_child (self->tags_box);
  while (child != NULL) {
    GtkWidget *next = gtk_widget_get_next_sibling (child);

    gtk_flow_box_remove (GTK_FLOW_BOX (self->tags_box), child);

    child = next;
  }

  g_list_store_remove_all (G_LIST_STORE (self->tags));
}

/**
 * adw_tagged_entry_set_placeholder_text:
 * @self: the tagged entry to update
 * @text: (nullable): the placeholder text
 *
 * Sets text to be displayed in the tagged entry when it is empty.
 *
 * Since: 1.2
 */
void
adw_tagged_entry_set_placeholder_text (AdwTaggedEntry *self,
                                       const char     *text)
{
  g_return_if_fail (ADW_IS_TAGGED_ENTRY (self));

  gtk_text_set_placeholder_text (GTK_TEXT (self->text), text);
  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_PLACEHOLDER, text,
                                  -1);

  g_object_notify_by_pspec (G_OBJECT (self), entry_props[PROP_PLACEHOLDER_TEXT]);
}

/**
 * adw_tagged_entry_get_placeholder_text:
 * @self: the tagged entry to query
 *
 * Retrieves the placeholder text of the tagged entry.
 *
 * Returns: (transfer none) (nullable): the placeholder text
 *
 * Since: 1.2
 */
const char *
adw_tagged_entry_get_placeholder_text (AdwTaggedEntry *self)
{
  g_return_val_if_fail (ADW_IS_TAGGED_ENTRY (self), NULL);

  return gtk_text_get_placeholder_text (GTK_TEXT (self->text));
}

/**
 * adw_tagged_entry_get_delimiter_chars:
 * @self: a tagged entry
 *
 * Retrieves the characters that act as delimiters for automatic tag
 * insertion.
 *
 * Returns: (transfer none) (nullable): the delimiter characters
 *
 * Since: 1.2
 */
const char *
adw_tagged_entry_get_delimiter_chars (AdwTaggedEntry *self)
{
  g_return_val_if_fail (ADW_IS_TAGGED_ENTRY (self), NULL);

  return self->delimiters;
}

/**
 * adw_tagged_entry_set_delimiter_chars:
 * @self: a tagged entry
 * @delimiters: (nullable): all the delimiter characters
 *
 * Sets the characters that act as a delimiter for automatic tag
 * insertion.
 *
 * Whenever a matching character is inserted in the tagged entry, the
 * current contents of the entry are replaced by a tag.
 *
 * If @delimiters is `NULL` then automatic tag insertion is disabled.
 *
 * Since: 1.2
 */
void
adw_tagged_entry_set_delimiter_chars (AdwTaggedEntry *self,
                                      const char     *delimiters)
{
  g_return_if_fail (ADW_IS_TAGGED_ENTRY (self));

  if (g_strcmp0 (self->delimiters, delimiters) == 0)
    return;

  g_free (self->delimiters);
  self->delimiters = g_strdup (delimiters);

  g_object_notify_by_pspec (G_OBJECT (self), entry_props[PROP_DELIMITER_CHARS]);
}

static AdwTag *
default_match_func (AdwTaggedEntry *self,
                    const char     *text,
                    gpointer        item,
                    gpointer        user_data G_GNUC_UNUSED)
{
  AdwTagMatch *match = item;
  char *tmp1, *tmp2, *tmp3, *tmp4;
  AdwTag *res = NULL;

  tmp1 = g_utf8_normalize (adw_tag_match_get_string (match), -1, G_NORMALIZE_ALL);
  tmp2 = g_utf8_casefold (tmp1, -1);

  tmp3 = g_utf8_normalize (text, -1, G_NORMALIZE_ALL);
  tmp4 = g_utf8_casefold (tmp3, -1);

  if (g_str_has_prefix (tmp2, tmp4)) {
    res = adw_tag_new ();

    adw_tag_set_label (res, adw_tag_match_get_string (match));
    adw_tag_set_show_close (res, TRUE);
  }

  g_free (tmp1);
  g_free (tmp2);
  g_free (tmp3);
  g_free (tmp4);

  return res;
}

static gboolean
filter_func (gpointer item,
             gpointer user_data)
{
  return adw_tag_match_get_tag (item) != NULL;
}

static gpointer
map_func (gpointer item,
          gpointer user_data)
{
  AdwTaggedEntry *self = user_data;
  GValue value = G_VALUE_INIT;

  if (self->match_expression != NULL)
    gtk_expression_evaluate (self->match_expression, item, &value);
  else if (GTK_IS_STRING_OBJECT (item))
    g_object_get_property (item, "string", &value);
  else {
    g_critical ("Missing match expression for tagged entry %p, and the "
                "match model is not a GtkStringList",
                self);
    g_value_set_string (&value, "No value");
  }

  AdwTagMatch *obj = adw_tag_match_new (item, g_value_get_string (&value));

  g_value_unset (&value);

  if (self->search != NULL) {
    AdwTag *tag = NULL;

    if (self->match_func == NULL)
      tag = default_match_func (self, self->search, obj, NULL);
    else
      tag = self->match_func (self,
                              self->search,
                              adw_tag_match_get_item (obj),
                              self->match_func_data);

    adw_tag_match_set_tag (obj, tag);
    if (tag != NULL)
      g_object_unref (tag);
  }

  return obj;
}

static void
adw_tagged_entry_update_map (AdwTaggedEntry *self)
{
  gtk_map_list_model_set_map_func (self->map_model, map_func, self, NULL);
}

/**
 * adw_tagged_entry_set_match_model:
 * @self: a tagged entry
 * @model: (nullable): a list model of potential matches
 *
 * Sets the matching model for the tagged entry.
 *
 * Every time a new tag is entered in the tagged entry, it is
 * compared to the contents of the model.
 *
 * The comparison is automatic if @model is a [class@Gtk.StringModel],
 * otherwise you need to call [method@TaggedEntry.set_match_func] and
 * provide a match function.
 *
 * Since: 1.2
 */
void
adw_tagged_entry_set_match_model (AdwTaggedEntry *self,
                                  GListModel     *model)
{
  g_return_if_fail (ADW_IS_TAGGED_ENTRY (self));
  g_return_if_fail (model == NULL || G_IS_LIST_MODEL (model));

  if (!g_set_object (&self->match_model, model))
    return;

  if (model == NULL) {
    gtk_list_view_set_model (GTK_LIST_VIEW (self->list_view), NULL);
    g_clear_object (&self->selection);
    g_clear_object (&self->map_model);
    g_clear_object (&self->filter);
  } else {
    /* 1. We map the contents of the given model to a model of AdwTagMatch objects */
    GtkMapListModel *map_model =
      gtk_map_list_model_new (g_object_ref (model), map_func, self, NULL);
    g_set_object (&self->map_model, map_model);

    /* 2. We set up a custom filter function to eliminate non-matching elements */
    GtkCustomFilter *filter =
      gtk_custom_filter_new (filter_func, self, NULL);
    GtkFilterListModel *filter_model =
      gtk_filter_list_model_new (G_LIST_MODEL (self->map_model), GTK_FILTER (filter));
    g_set_object (&self->filter, (GtkFilter *) filter);

    adw_tagged_entry_update_map (self);

    /* 3. Sort alphabetically on the string property of the tag match object */
    GtkStringSorter *sorter =
      gtk_string_sorter_new (gtk_property_expression_new (ADW_TYPE_TAG_MATCH, NULL, "string"));
    gtk_string_sorter_set_ignore_case (sorter, TRUE);
    GtkSortListModel *sort_model =
      gtk_sort_list_model_new (G_LIST_MODEL (filter_model), GTK_SORTER (sorter));

    /* 4. Create a selection model for the list view */
    GtkSingleSelection *selection =
      gtk_single_selection_new (G_LIST_MODEL (sort_model));
    gtk_single_selection_set_autoselect (selection, FALSE);
    gtk_single_selection_set_can_unselect (selection, TRUE);
    gtk_single_selection_set_selected (selection, GTK_INVALID_LIST_POSITION);
    g_set_object (&self->selection, selection);

    /* 5. Assign the selection model to the list view */
    gtk_list_view_set_model (GTK_LIST_VIEW (self->list_view), GTK_SELECTION_MODEL (selection));
    g_object_unref (selection);
  }

  g_object_notify_by_pspec (G_OBJECT (self), entry_props[PROP_MATCH_MODEL]);
}

/**
 * adw_tagged_entry_get_match_model:
 * @self: a tagged entry
 *
 * Retrieves the model set using [method@TaggedEntry.set_match_model].
 *
 * Returns: (transfer none) (nullable): the model with all the possible tags
 *
 * Since: 1.2
 */
GListModel *
adw_tagged_entry_get_match_model (AdwTaggedEntry *self)
{
  g_return_val_if_fail (ADW_IS_TAGGED_ENTRY (self), NULL);

  return self->match_model;
}

/**
 * adw_tagged_entry_set_match_expression:
 * @self: a tagged entry
 * @expression: (nullable): the expression used for matching
 *
 * Sets the expression used for matching tags.
 *
 * Since: 1.2
 */
void
adw_tagged_entry_set_match_expression (AdwTaggedEntry *self,
                                       GtkExpression  *expression)
{
  g_return_if_fail (ADW_IS_TAGGED_ENTRY (self));
  g_return_if_fail (expression == NULL || GTK_IS_EXPRESSION (expression));

  if (self->match_expression == expression)
    return;

  g_clear_pointer (&self->match_expression, gtk_expression_unref);
  self->match_expression = expression;
  if (self->match_expression != NULL) {
    gtk_expression_ref (self->match_expression);
  }

  adw_tagged_entry_update_map (self);

  g_object_notify_by_pspec (G_OBJECT (self), entry_props[PROP_MATCH_EXPRESSION]);
}

/**
 * adw_tagged_entry_get_match_expression:
 * @self: a tagged entry
 *
 * Retrieves the matching expression set using
 * [method@TaggedEntry.set_match_expression].
 *
 * Returns: (transfer none) (nullable): the matching expression
 *
 * Since: 1.2
 */
GtkExpression *
adw_tagged_entry_get_match_expression (AdwTaggedEntry *self)
{
  g_return_val_if_fail (ADW_IS_TAGGED_ENTRY (self), NULL);

  return self->match_expression;
}

/**
 * adw_tagged_entry_set_match_func:
 * @self: a tagged entry
 * @match_func: (nullable) (scope notified) (closure user_data): the matching function for the entry
 * @user_data: (nullable): data to be passed to the matching function
 * @notify: (nullable): function to be called when the matching function is
 *   removed from the tagged entry
 *
 * Sets the matching function for the tagged entry.
 *
 * The default matching function will try to compare the contents of the
 * entry with each item in the model set using [method@TaggedEntry.set_match_model],
 * and will create a [class@Tag] instance in case of a match.
 *
 * You can use this function to control the matching between the entry's text
 * and the data inside your match model, as well as the creation of the tag
 * object.
 *
 * Since: 1.2
 */
void
adw_tagged_entry_set_match_func (AdwTaggedEntry          *self,
                                 AdwTaggedEntryMatchFunc  match_func,
                                 gpointer                 user_data,
                                 GDestroyNotify           notify)
{
  g_return_if_fail (ADW_IS_TAGGED_ENTRY (self));

  if (self->match_func_notify != NULL)
    self->match_func_notify (self->match_func_data);

  self->match_func = match_func;
  self->match_func_data = user_data;
  self->match_func_notify = notify;
}

/* }}} */
