#include "adw-demo-page-wrap-box.h"

#include <glib/gi18n.h>

#define LOREM_IPSUM \
    "Lorem Ipsum Dolor Sit Amet Consectetur Adipiscing Elit Sed Do Eiusmod " \
    "Tempor Incididunt Ut Labore Et Dolore Magnam Aliquam Quaerat Voluptatem " \
    "Ut Enim Aeque Doleamus Animo Cum Corpore Dolemus Fieri Tamen Permagna " \
    "Accessio Potest Si Aliquod Aeternum Ullus Investigandi Veri Nisi " \
    "Inveneris Et Quaerendi Defatigatio Turpis Est Cum Esset Accusata Et " \
    "Vituperata Ab Hortensio Qui Liber Cum Et Mortem Contemnit Qua Qui Est " \
    "Imbutus Quietus Esse Numquam Potest Praeterea Bona Praeterita Grata " \
    "Recordatione Renovata Delectant Est Autem Situm In"

struct _AdwDemoPageWrapBox
{
  AdwBin parent_instance;

  AdwWrapBox *wrap_box;
  GtkWidget *add_btn;

  char **text;
  int text_length;
  int current_word;
};

G_DEFINE_TYPE (AdwDemoPageWrapBox, adw_demo_page_wrap_box, ADW_TYPE_BIN)

static void
remove_tag_cb (GtkWidget *tag)
{
  AdwDemoPageWrapBox *self =
    ADW_DEMO_PAGE_WRAP_BOX (gtk_widget_get_ancestor (tag, ADW_TYPE_DEMO_PAGE_WRAP_BOX));

  adw_wrap_box_remove (self->wrap_box, tag);
}

static void
add_tag (AdwDemoPageWrapBox *self)
{
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *label = gtk_label_new (self->text[self->current_word]);
  GtkWidget *close_btn = gtk_button_new_from_icon_name ("window-close-symbolic");
  GtkWidget *last_tag;

  gtk_label_set_xalign (GTK_LABEL (label), 0);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_widget_set_hexpand (label, TRUE);

  gtk_widget_add_css_class (close_btn, "flat");
  gtk_widget_add_css_class (close_btn, "circular");

  gtk_widget_set_hexpand (box, FALSE);
  gtk_widget_add_css_class (box, "tag");

  gtk_box_append (GTK_BOX (box), label);
  gtk_box_append (GTK_BOX (box), close_btn);

  g_signal_connect_swapped (close_btn, "clicked", G_CALLBACK (remove_tag_cb), box);

  last_tag = gtk_widget_get_prev_sibling (self->add_btn);

  adw_wrap_box_insert_child_after (self->wrap_box, box, last_tag);

  self->current_word = (self->current_word + 1) % self->text_length;
}

static void
adw_demo_page_wrap_box_finalize (GObject *object)
{
  AdwDemoPageWrapBox *self = ADW_DEMO_PAGE_WRAP_BOX (object);

  g_strfreev (self->text);

  G_OBJECT_CLASS (adw_demo_page_wrap_box_parent_class)->finalize (object);
}

static void
adw_demo_page_wrap_box_class_init (AdwDemoPageWrapBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = adw_demo_page_wrap_box_finalize;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/wrap-box/adw-demo-page-wrap-box.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageWrapBox, wrap_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageWrapBox, add_btn);

  gtk_widget_class_install_action (widget_class, "demo.add-tag", NULL, (GtkWidgetActionActivateFunc) add_tag);
}

static void
adw_demo_page_wrap_box_init (AdwDemoPageWrapBox *self)
{
  int i;

  gtk_widget_init_template (GTK_WIDGET (self));

  self->text = g_strsplit (LOREM_IPSUM, " ", 0);

  for (i = 0; self->text[i]; i++)
    self->text_length++;

  for (i = 0; i < 10; i++)
    add_tag (self);
}
