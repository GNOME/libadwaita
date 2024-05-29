#include "adw-style-demo-dialog.h"

struct _AdwStyleDemoDialog
{
  AdwDialog parent_instance;

  gboolean progress;

  AdwDialog *status_page_dialog;
  AdwDialog *sidebar_dialog;
  AdwNavigationSplitView *split_view;
};

G_DEFINE_FINAL_TYPE (AdwStyleDemoDialog, adw_style_demo_dialog, ADW_TYPE_DIALOG)

enum {
  PROP_0,
  PROP_DEVEL,
  PROP_PROGRESS,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
status_page_cb (AdwStyleDemoDialog *self)
{
  adw_dialog_present (self->status_page_dialog, GTK_WIDGET (self));
}

static void
sidebar_cb (AdwStyleDemoDialog *self)
{
  adw_dialog_present (self->sidebar_dialog, GTK_WIDGET (self));
}

static void
dummy_cb (AdwStyleDemoDialog *self)
{
}

static gboolean
get_devel_style (AdwStyleDemoDialog *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  if (!GTK_IS_WIDGET (root))
    return FALSE;

  return gtk_widget_has_css_class (GTK_WIDGET (root), "devel");
}

static void
set_devel_style (AdwStyleDemoDialog *self,
                 gboolean            devel)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  if (!GTK_IS_WIDGET (root))
    return;

  if (devel)
    gtk_widget_add_css_class (GTK_WIDGET (root), "devel");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (root), "devel");
}

static void
sidebar_forward_cb (AdwStyleDemoDialog *self)
{
  adw_navigation_split_view_set_show_content (self->split_view, TRUE);
}

static void
adw_style_demo_dialog_root (GtkWidget *widget)
{
  AdwStyleDemoDialog *self = ADW_STYLE_DEMO_DIALOG (widget);

  GTK_WIDGET_CLASS (adw_style_demo_dialog_parent_class)->root (widget);

  if (get_devel_style (self))
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEVEL]);
}

static void
adw_style_demo_dialog_unroot (GtkWidget *widget)
{
  AdwStyleDemoDialog *self = ADW_STYLE_DEMO_DIALOG (widget);
  gboolean has_devel = get_devel_style (self);

  GTK_WIDGET_CLASS (adw_style_demo_dialog_parent_class)->unroot (widget);

  if (has_devel)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEVEL]);
}

static void
adw_style_demo_dialog_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwStyleDemoDialog *self = ADW_STYLE_DEMO_DIALOG (object);

  switch (prop_id) {
  case PROP_DEVEL:
    g_value_set_boolean (value, get_devel_style (self));
    break;
  case PROP_PROGRESS:
    g_value_set_boolean (value, self->progress);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_demo_dialog_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwStyleDemoDialog *self = ADW_STYLE_DEMO_DIALOG (object);

  switch (prop_id) {
  case PROP_DEVEL:
    set_devel_style (self, g_value_get_boolean (value));
    break;
  case PROP_PROGRESS:
    self->progress = g_value_get_boolean (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_demo_dialog_class_init (AdwStyleDemoDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_style_demo_dialog_get_property;
  object_class->set_property = adw_style_demo_dialog_set_property;
  widget_class->root = adw_style_demo_dialog_root;
  widget_class->unroot = adw_style_demo_dialog_unroot;

  props[PROP_DEVEL] =
    g_param_spec_boolean ("devel", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  props[PROP_PROGRESS] =
    g_param_spec_boolean ("progress", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/styles/adw-style-demo-dialog.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoDialog, status_page_dialog);
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoDialog, sidebar_dialog);
  gtk_widget_class_bind_template_child (widget_class, AdwStyleDemoDialog, split_view);
  gtk_widget_class_bind_template_callback (widget_class, sidebar_forward_cb);

  gtk_widget_class_install_property_action (widget_class, "style.devel", "devel");
  gtk_widget_class_install_property_action (widget_class, "style.progress", "progress");
  gtk_widget_class_install_action (widget_class, "style.status-page", NULL, (GtkWidgetActionActivateFunc) status_page_cb);
  gtk_widget_class_install_action (widget_class, "style.sidebar", NULL, (GtkWidgetActionActivateFunc) sidebar_cb);
  gtk_widget_class_install_action (widget_class, "style.dummy", NULL, (GtkWidgetActionActivateFunc) dummy_cb);
}

static void
adw_style_demo_dialog_init (AdwStyleDemoDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwDialog *
adw_style_demo_dialog_new (void)
{
  return g_object_new (ADW_TYPE_STYLE_DEMO_DIALOG, NULL);
}
