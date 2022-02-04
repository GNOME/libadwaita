#include "adw-demo-page-tagged-entry.h"

#include <glib/gi18n.h>

struct _AdwDemoPageTaggedEntry
{
  AdwBin parent_instance;

  AdwTaggedEntry *manual_tagged_entry;
  GtkWidget *add_tag_button;
  GtkWidget *remove_all_button;

  AdwTaggedEntry *automatic_tagged_entry;

  AdwTaggedEntry *completion_tagged_entry;
};

G_DEFINE_TYPE (AdwDemoPageTaggedEntry, adw_demo_page_tagged_entry, ADW_TYPE_BIN)

static guint last_tag = 0;

static void
on_add_tag_clicked (GtkButton              *button,
                    AdwDemoPageTaggedEntry *self)
{
  char *tag_label = g_strdup_printf ("Tag: %d", last_tag);

  AdwTag *tag = adw_tag_new ();
  adw_tag_set_label (tag, tag_label);
  adw_tag_set_action_name (tag, "tags.removable");
  adw_tag_set_action_target (tag, "i", last_tag);

  adw_tagged_entry_add_tag (self->manual_tagged_entry, tag);

  last_tag += 1;
}

static void
on_remove_all_clicked (GtkButton              *button,
                       AdwDemoPageTaggedEntry *self)
{
  adw_tagged_entry_remove_all_tags (self->manual_tagged_entry);
}

static void
on_tag_clicked_activate (GAction                *action,
                         GVariant               *parameter G_GNUC_UNUSED,
                         AdwDemoPageTaggedEntry *self)
{
  g_print ("Tag activated\n");
}

static void
on_tag_removable_activate (GAction                *action,
                           GVariant               *parameter,
                           AdwDemoPageTaggedEntry *self)
{
  g_print ("Removable tag %d activated\n",
           g_variant_get_int32 (parameter));
}

static void
adw_demo_page_tagged_entry_class_init (AdwDemoPageTaggedEntryClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/tagged-entry/adw-demo-page-tagged-entry.ui");

  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageTaggedEntry, manual_tagged_entry);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageTaggedEntry, add_tag_button);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageTaggedEntry, remove_all_button);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageTaggedEntry, automatic_tagged_entry);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageTaggedEntry, completion_tagged_entry);

  gtk_widget_class_bind_template_callback (widget_class, on_add_tag_clicked);
  gtk_widget_class_bind_template_callback (widget_class, on_remove_all_clicked);
}

static void
adw_demo_page_tagged_entry_init (AdwDemoPageTaggedEntry *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  GSimpleActionGroup *group = g_simple_action_group_new ();
  gtk_widget_insert_action_group (GTK_WIDGET (self), "tags", G_ACTION_GROUP (group));
  g_object_unref (group);

  GSimpleAction *tag_action = g_simple_action_new ("clicked", NULL);
  g_signal_connect (tag_action, "activate", G_CALLBACK (on_tag_clicked_activate), self);
  g_action_map_add_action (G_ACTION_MAP (group), G_ACTION (tag_action));
  g_object_unref (tag_action);

  GSimpleAction *removable_action = g_simple_action_new ("removable", G_VARIANT_TYPE ("i"));
  g_signal_connect (removable_action, "activate", G_CALLBACK (on_tag_removable_activate), self);
  g_action_map_add_action (G_ACTION_MAP (group), G_ACTION (removable_action));
  g_object_unref (removable_action);
}
