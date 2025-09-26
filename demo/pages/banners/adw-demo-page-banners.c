#include "adw-demo-page-banners.h"

#include <glib/gi18n.h>

struct _AdwDemoPageBanners
{
  AdwBin parent_instance;

  AdwBanner *banner;
  AdwEntryRow *button_label_row;
  AdwSwitchRow *button_style_row;
};

G_DEFINE_TYPE (AdwDemoPageBanners, adw_demo_page_banners, ADW_TYPE_BIN)

static void
update_button_cb (AdwDemoPageBanners *self)
{
  GtkEditable *editable = GTK_EDITABLE (self->button_label_row);

  if (gtk_editable_get_editable (editable))
    adw_banner_set_button_label (self->banner, gtk_editable_get_text (editable));
  else
    adw_banner_set_button_label (self->banner, NULL);
}

static void
banner_activate_cb (AdwDemoPageBanners *self)
{
  GtkWidget *toast_overlay = gtk_widget_get_ancestor (GTK_WIDGET (self), ADW_TYPE_TOAST_OVERLAY);
  AdwToast *toast = adw_toast_new (_("Banner action triggered"));

  adw_toast_overlay_add_toast (ADW_TOAST_OVERLAY (toast_overlay), toast);
}

static void
button_style_notify_active_cb (AdwDemoPageBanners *self)
{
  if (adw_switch_row_get_active (self->button_style_row))
    adw_banner_set_button_style (self->banner, ADW_BANNER_BUTTON_SUGGESTED);
  else
    adw_banner_set_button_style (self->banner, ADW_BANNER_BUTTON_DEFAULT);
}

static void
adw_demo_page_banners_class_init (AdwDemoPageBannersClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita1/Demo/ui/pages/banners/adw-demo-page-banners.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageBanners, banner);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageBanners, button_label_row);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageBanners, button_style_row);
  gtk_widget_class_bind_template_callback (widget_class, button_style_notify_active_cb);
  gtk_widget_class_bind_template_callback (widget_class, update_button_cb);

  gtk_widget_class_install_action (widget_class, "demo.activate", NULL, (GtkWidgetActionActivateFunc) banner_activate_cb);
}

static void
adw_demo_page_banners_init (AdwDemoPageBanners *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  update_button_cb (self);
}
