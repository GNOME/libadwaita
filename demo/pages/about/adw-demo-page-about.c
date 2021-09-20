#include "adw-demo-page-about.h"

#include <glib/gi18n.h>

struct _AdwDemoPageAbout
{
  AdwBin parent_instance;
};

G_DEFINE_TYPE (AdwDemoPageAbout, adw_demo_page_about, ADW_TYPE_BIN)

static void
demo_run_cb (AdwDemoPageAbout *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  GtkWidget *about;

  const char *developers[] = {
    "Angela Avery <angela@example.org>",
    NULL
  };

  const char *artists[] = {
    "GNOME Design Team",
    NULL
 };

  const char *special_thanks[] = {
    "My cat",
    NULL
 };

  const char *release_notes = "\
<p>\
  This release adds the following features:\
</p>\
<ul>\
  <li>Added a way to export fonts.</li>\
  <li>Better support for <code>monospace</code> fonts.</li>\
  <li>Added a way to preview <em>italic</em> text.</li>\
  <li>Bug fixes and performance improvements.</li>\
  <li>Translation updates.</li>\
</ul>\
  ";

  about =
    g_object_new (ADW_TYPE_ABOUT_WINDOW,
                  "transient-for", root,
                  "application-icon", "org.example.Typeset",
                  "application-name", _("Typeset"),
                  "developer-name", _("Angela Avery"),
                  "version", "1.2.3",
                  "release-notes-version", "1.2.0",
                  "release-notes", release_notes,
                  "comments", _("Typeset is an app that doesn’t exist and is used as an example content for this about window."),
                  "website", "https://example.org",
                  "issue-url", "https://example.org",
                  "support-url", "https://example.org",
                  "copyright", "© 2022 Angela Avery",
                  "license-type", GTK_LICENSE_LGPL_2_1,
                  "developers", developers,
                  "artists", artists,
                  "translator-credits", _("translator-credits"),
                  NULL);

  adw_about_window_add_link (ADW_ABOUT_WINDOW (about),
                             _("_Documentation"),
                             "https://gnome.pages.gitlab.gnome.org/libadwaita/doc/main/class.AboutWindow.html");

  adw_about_window_add_legal_section (ADW_ABOUT_WINDOW (about),
                                      _("Fonts"),
                                      NULL,
                                      GTK_LICENSE_CUSTOM,
                                      "This application uses font data from <a href='https://example.org'>somewhere</a>.");

  adw_about_window_add_acknowledgement_section (ADW_ABOUT_WINDOW (about),
                                                _("Special thanks to"),
                                                special_thanks);

  gtk_window_present (GTK_WINDOW (about));

  gtk_window_present (GTK_WINDOW (about));
}

static void
adw_demo_page_about_class_init (AdwDemoPageAboutClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/about/adw-demo-page-about.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adw_demo_page_about_init (AdwDemoPageAbout *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
