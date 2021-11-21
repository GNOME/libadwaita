#include <glib/gi18n.h>
#include "adw-demo-color-row.h"

struct _AdwDemoColorRow
{
  AdwActionRow parent_instance;

  AdwColor color_key;
  GtkWidget *color_button;
  AdwStyleManager *style_manager;

  gboolean color_scheme_changing;
};

G_DEFINE_TYPE (AdwDemoColorRow, adw_demo_color_row, ADW_TYPE_ACTION_ROW)

enum {
  PROP_0,
  PROP_COLOR_KEY,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
rgba_changed (AdwDemoColorRow *self)
{
  GdkRGBA *new_color;

  if (self->color_scheme_changing)
    return;

  new_color = (GdkRGBA *) gtk_color_dialog_button_get_rgba (GTK_COLOR_DIALOG_BUTTON (self->color_button));

  adw_style_manager_set_color_from_rgba (self->style_manager,
                                         self->color_key,
                                         new_color);
}

static void
init_color (AdwDemoColorRow *self)
{
  AdwColorTheme *theme;
  const char *title;

  switch (self->color_key) {
  case ADW_COLOR_ACCENT_BG_COLOR:
    title = _("Accent Color");
    break;
  case ADW_COLOR_DESTRUCTIVE_BG_COLOR:
    title = _("Destructive Color");
    break;
  case ADW_COLOR_SUCCESS_COLOR:
    title = _("Success Color");
    break;
  case ADW_COLOR_WARNING_COLOR:
    title = _("Warning Color");
    break;
  case ADW_COLOR_ERROR_COLOR:
    title = _("Error Color");
    break;
  case ADW_COLOR_WINDOW_BG_COLOR:
    title = _("Window Background Color");
    break;
  case ADW_COLOR_WINDOW_FG_COLOR:
    title = _("Window Foreground Color");
    break;
  case ADW_COLOR_HEADERBAR_BG_COLOR:
    title = _("Headerbar Background Color");
    break;
  case ADW_COLOR_HEADERBAR_FG_COLOR:
    title = _("Headerbar Foreground Color");
    break;
  case ADW_COLOR_HEADERBAR_BORDER_COLOR:
    title = _("Headerbar Border Color");
    break;
  case ADW_COLOR_CARD_BG_COLOR:
    title = _("Card Background Color");
    break;
  case ADW_COLOR_CARD_FG_COLOR:
    title = _("Card Foreground Color");
    break;
  case ADW_COLOR_VIEW_BG_COLOR:
    title = _("View Background Color");
    break;
  case ADW_COLOR_VIEW_FG_COLOR:
    title = _("View Foreground Color");
    break;
  case ADW_COLOR_POPOVER_BG_COLOR:
    title = _("Popover Background Color");
    break;
  case ADW_COLOR_POPOVER_FG_COLOR:
    title = _("Popover Foreground Color");
    break;
  default:
    g_assert_not_reached ();
    break;
  }

  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (self), title);

  if (adw_style_manager_get_dark (self->style_manager))
    theme = adw_style_manager_get_dark_colors (self->style_manager);
  else
    theme = adw_style_manager_get_light_colors (self->style_manager);

  self->color_button = gtk_color_dialog_button_new (gtk_color_dialog_new ());
  gtk_color_dialog_button_set_rgba (GTK_COLOR_DIALOG_BUTTON (self->color_button),
                                    adw_color_theme_get_color (theme, self->color_key));
  gtk_widget_set_valign (self->color_button, GTK_ALIGN_CENTER);

  adw_action_row_add_suffix (ADW_ACTION_ROW (self), self->color_button);
  adw_action_row_set_activatable_widget (ADW_ACTION_ROW (self), self->color_button);
}

static void
dark_changed_cb (AdwDemoColorRow *self)
{
  AdwColorTheme *theme;

  if (adw_style_manager_get_dark (self->style_manager))
    theme = adw_style_manager_get_dark_colors (self->style_manager);
  else
    theme = adw_style_manager_get_light_colors (self->style_manager);

  /* Workaround for `set_rgba` triggering the notify function */
  self->color_scheme_changing = TRUE;

  gtk_color_dialog_button_set_rgba (GTK_COLOR_DIALOG_BUTTON (self->color_button),
                                    adw_color_theme_get_color (theme, self->color_key));

  self->color_scheme_changing = FALSE;
}

static void
adw_demo_color_row_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwDemoColorRow *self = ADW_DEMO_COLOR_ROW (object);

  switch (prop_id) {
  case PROP_COLOR_KEY:
    g_value_set_enum (value, self->color_key);
    break;
  default:
    g_assert_not_reached ();
    break;
  }
}

static void
adw_demo_color_row_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwDemoColorRow *self = ADW_DEMO_COLOR_ROW (object);

  switch (prop_id) {
  case PROP_COLOR_KEY:
    self->color_key = g_value_get_enum (value);
    break;
  default:
    g_assert_not_reached ();
    break;
  }
}

static void
adw_demo_color_row_constructed (GObject *object)
{
  AdwDemoColorRow *self = ADW_DEMO_COLOR_ROW (object);

  init_color (self);

  g_signal_connect_object (self->style_manager,
                           "notify::dark",
                           G_CALLBACK (dark_changed_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->color_button,
                           "notify::rgba",
                           G_CALLBACK (rgba_changed),
                           self,
                           G_CONNECT_SWAPPED);
}

static void
adw_demo_color_row_class_init (AdwDemoColorRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = adw_demo_color_row_get_property;
  object_class->set_property = adw_demo_color_row_set_property;
  object_class->constructed = adw_demo_color_row_constructed;

  props[PROP_COLOR_KEY] =
    g_param_spec_enum ("color-key",
                       "Color Key",
                       "The enum value representing the color this row sets",
                       ADW_TYPE_COLOR,
                       ADW_COLOR_ACCENT_BG_COLOR,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_demo_color_row_init (AdwDemoColorRow *self)
{
  self->style_manager = adw_style_manager_get_for_display (gtk_widget_get_display (GTK_WIDGET (self)));
  self->color_scheme_changing = FALSE;
}

AdwDemoColorRow *
adw_demo_color_row_new (AdwColor  color_key)
{
  return g_object_new (ADW_TYPE_DEMO_COLOR_ROW,
                       "color-key", color_key,
                       NULL);
}
