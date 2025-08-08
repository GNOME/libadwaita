/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-style-manager-private.h"

#include "adw-accent-color-private.h"
#include "adw-main-private.h"
#include "adw-settings-private.h"
#include <gtk/gtk.h>

#define SWITCH_DURATION 250

/**
 * AdwColorScheme:
 * @ADW_COLOR_SCHEME_DEFAULT: Inherit the parent color-scheme. When set on the
 *   `AdwStyleManager` returned by [func@StyleManager.get_default], it's
 *   equivalent to `ADW_COLOR_SCHEME_PREFER_LIGHT`.
 * @ADW_COLOR_SCHEME_FORCE_LIGHT: Always use light appearance.
 * @ADW_COLOR_SCHEME_PREFER_LIGHT: Use light appearance unless the system
 *   prefers dark colors.
 * @ADW_COLOR_SCHEME_PREFER_DARK: Use dark appearance unless the system prefers
 *   prefers light colors.
 * @ADW_COLOR_SCHEME_FORCE_DARK: Always use dark appearance.
 *
 * Application color schemes for [property@StyleManager:color-scheme].
 */

/**
 * AdwStyleManager:
 *
 * A class for managing application-wide styling.
 *
 * `AdwStyleManager` provides a way to query and influence the application
 * styles, such as whether to use dark style, the system accent color or high
 * contrast appearance.
 *
 * It allows to set the color scheme via the
 * [property@StyleManager:color-scheme] property, and to query the current
 * appearance, as well as whether a system-wide color scheme and accent color
 * preferences exists.
 */

#define DEFAULT_DOCUMENT_FONT_FAMILY "Sans"
#define DEFAULT_DOCUMENT_FONT_SIZE 10
#define DEFAULT_DOCUMENT_FONT_SIZE_STR "10"

#define DEFAULT_MONOSPACE_FONT_FAMILY "Monospace"
#define DEFAULT_MONOSPACE_FONT_SIZE 10
#define DEFAULT_MONOSPACE_FONT_SIZE_STR "10"

#define DEFAULT_DOCUMENT_FONT (DEFAULT_DOCUMENT_FONT_FAMILY " " DEFAULT_DOCUMENT_FONT_SIZE_STR)
#define DEFAULT_MONOSPACE_FONT (DEFAULT_MONOSPACE_FONT_FAMILY " " DEFAULT_MONOSPACE_FONT_SIZE_STR)

struct _AdwStyleManager
{
  GObject parent_instance;

  GdkDisplay *display;
  AdwSettings *settings;
  GtkSettings *gtk_settings;
  GtkCssProvider *provider;
  GtkCssProvider *accent_provider;
  GtkCssProvider *fonts_provider;

  AdwColorScheme color_scheme;
  gboolean dark;
  gboolean changing_gtk_settings;
  char *document_font_name;
  char *monospace_font_name;

  GtkCssProvider *animations_provider;
  guint animation_timeout_id;
};

G_DEFINE_FINAL_TYPE (AdwStyleManager, adw_style_manager, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_DISPLAY,
  PROP_COLOR_SCHEME,
  PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES,
  PROP_DARK,
  PROP_HIGH_CONTRAST,
  PROP_SYSTEM_SUPPORTS_ACCENT_COLORS,
  PROP_ACCENT_COLOR,
  PROP_ACCENT_COLOR_RGBA,
  PROP_DOCUMENT_FONT_NAME,
  PROP_MONOSPACE_FONT_NAME,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static GHashTable *display_style_managers = NULL;
static AdwStyleManager *default_instance = NULL;

typedef enum {
  UPDATE_CONTRAST         = 1 << 0,
  UPDATE_COLOR_SCHEME = 1 << 1,
  UPDATE_ACCENT_COLOR = 1 << 2,
  UPDATE_FONTS        = 1 << 3,
  UPDATE_ALL = UPDATE_CONTRAST | UPDATE_COLOR_SCHEME | UPDATE_ACCENT_COLOR | UPDATE_FONTS
} StylesheetUpdateFlags;

static void
warn_prefer_dark_theme (AdwStyleManager *self)
{
  if (self->changing_gtk_settings)
    return;

  g_warning ("Using GtkSettings:gtk-application-prefer-dark-theme with "
             "libadwaita is unsupported. Please use "
             "AdwStyleManager:color-scheme instead.");
}

static void
unregister_display (GdkDisplay *display)
{
  g_assert (g_hash_table_contains (display_style_managers, display));

  g_hash_table_remove (display_style_managers, display);
}

static void
register_display (GdkDisplayManager *display_manager,
                  GdkDisplay        *display)
{
  AdwStyleManager *style_manager;

  style_manager = g_object_new (ADW_TYPE_STYLE_MANAGER,
                                "display", display,
                                NULL);

  g_assert (!g_hash_table_contains (display_style_managers, display));

  g_hash_table_insert (display_style_managers, display, style_manager);

  g_signal_connect (display,
                    "closed",
                    G_CALLBACK (unregister_display),
                    NULL);
}

static void
enable_animations_cb (AdwStyleManager *self)
{
  gtk_style_context_remove_provider_for_display (self->display,
                                                 GTK_STYLE_PROVIDER (self->animations_provider));

  self->animation_timeout_id = 0;
}

static char*
generate_accent_css (AdwStyleManager *self)
{
  AdwAccentColor accent = adw_style_manager_get_accent_color (self);
  GString *str = g_string_new ("");
  GdkRGBA rgba;
  char *rgba_str;

  adw_accent_color_to_rgba (accent, &rgba);
  rgba_str = gdk_rgba_to_string (&rgba);

  g_string_append_printf (str, "@define-color accent_bg_color %s;\n", rgba_str);
  g_string_append (str, "@define-color accent_fg_color white;\n");

  g_free (rgba_str);

  return g_string_free (str, FALSE);
}

static char*
generate_fonts_css (AdwStyleManager *self)
{
  PangoFontDescription *document_desc = pango_font_description_from_string (self->document_font_name);
  PangoFontDescription *monospace_desc = pango_font_description_from_string (self->monospace_font_name);
  GString *str = g_string_new ("");

  g_string_append (str, ":root {\n");

  if (document_desc && (pango_font_description_get_set_fields (document_desc) & (PANGO_FONT_MASK_FAMILY)) != 0) {
    const char *family = pango_font_description_get_family (document_desc);
    g_string_append_printf (str, "  --document-font-family: %s;\n", family);
  } else {
    g_string_append_printf (str, "  --document-font-family: %s;\n", DEFAULT_DOCUMENT_FONT_FAMILY);
  }

  if (document_desc && (pango_font_description_get_set_fields (document_desc) & (PANGO_FONT_MASK_SIZE)) != 0) {
    int size = pango_font_description_get_size (document_desc);
    char buf[G_ASCII_DTOSTR_BUF_SIZE];

    g_ascii_dtostr (buf, sizeof (buf), (double) size / PANGO_SCALE);

    if (pango_font_description_get_size_is_absolute (document_desc))
      g_string_append_printf (str, "  --document-font-size: %spx;\n", buf);
    else
      g_string_append_printf (str, "  --document-font-size: %spt;\n", buf);
  } else {
    g_string_append_printf (str, "  --document-font-size: %dpt;\n", DEFAULT_DOCUMENT_FONT_SIZE);
  }

  if (monospace_desc && (pango_font_description_get_set_fields (monospace_desc) & (PANGO_FONT_MASK_FAMILY)) != 0) {
    const char *family = pango_font_description_get_family (monospace_desc);
    g_string_append_printf (str, "  --monospace-font-family: %s;\n", family);
  } else {
    g_string_append_printf (str, "  --monospace-font-family: %s;\n", DEFAULT_MONOSPACE_FONT_FAMILY);
  }

  if (monospace_desc && (pango_font_description_get_set_fields (monospace_desc) & (PANGO_FONT_MASK_SIZE)) != 0) {
    int size = pango_font_description_get_size (monospace_desc);
    char buf[G_ASCII_DTOSTR_BUF_SIZE];

    g_ascii_dtostr (buf, sizeof (buf), (double) size / PANGO_SCALE);

    if (pango_font_description_get_size_is_absolute (monospace_desc))
      g_string_append_printf (str, "  --monospace-font-size: %spx;\n", buf);
    else
      g_string_append_printf (str, "  --monospace-font-size: %spt;\n", buf);
  } else {
    g_string_append_printf (str, "  --monospace-font-size: %dpt;\n", DEFAULT_MONOSPACE_FONT_SIZE);
  }

  pango_font_description_free (document_desc);
  pango_font_description_free (monospace_desc);

  g_string_append (str, "}");

  return g_string_free (str, FALSE);
}

static void
update_stylesheet (AdwStyleManager       *self,
                   StylesheetUpdateFlags  flags)
{
  if (!self->display)
    return;

  if (self->animation_timeout_id)
    g_clear_handle_id (&self->animation_timeout_id, g_source_remove);

  gtk_style_context_add_provider_for_display (self->display,
                                              GTK_STYLE_PROVIDER (self->animations_provider),
                                              10000);

  if (flags & UPDATE_ACCENT_COLOR && self->accent_provider) {
    char *accent_css = generate_accent_css (self);
    gtk_css_provider_load_from_string (self->accent_provider, accent_css);
    g_free (accent_css);
  }

  if (flags & UPDATE_FONTS && self->fonts_provider) {
    char *fonts_css = generate_fonts_css (self);
    gtk_css_provider_load_from_string (self->fonts_provider, fonts_css);
    g_free (fonts_css);
  }

  if (flags & UPDATE_COLOR_SCHEME) {
    GtkInterfaceColorScheme color_scheme;

    if (self->dark)
      color_scheme = GTK_INTERFACE_COLOR_SCHEME_DARK;
    else
      color_scheme = GTK_INTERFACE_COLOR_SCHEME_LIGHT;

    if (self->provider)
      g_object_set (self->provider, "prefers-color-scheme", color_scheme, NULL);

    self->changing_gtk_settings = TRUE;

    g_object_set (self->gtk_settings,
                  "gtk-application-prefer-dark-theme", self->dark,
                  NULL);

    g_object_set (self->gtk_settings,
                  "gtk-interface-color-scheme", color_scheme,
                  NULL);

    self->changing_gtk_settings = FALSE;
  }

  if (flags & UPDATE_CONTRAST) {
    GtkInterfaceContrast contrast;

    if (adw_settings_get_high_contrast (self->settings))
      contrast = GTK_INTERFACE_CONTRAST_MORE;
    else
      contrast = GTK_INTERFACE_CONTRAST_NO_PREFERENCE;

    if (self->provider)
      g_object_set (self->provider, "prefers-contrast", contrast, NULL);

    self->changing_gtk_settings = TRUE;

    g_object_set (self->gtk_settings,
                  "gtk-interface-contrast", contrast,
                  NULL);

    self->changing_gtk_settings = FALSE;
  }

  self->animation_timeout_id =
    g_timeout_add_once (SWITCH_DURATION,
                        (GSourceOnceFunc) enable_animations_cb,
                        self);
}

static gboolean
get_is_dark (AdwStyleManager *self)
{
  AdwSystemColorScheme system_scheme = adw_settings_get_color_scheme (self->settings);

  switch (self->color_scheme) {
  case ADW_COLOR_SCHEME_DEFAULT:
    if (self->display)
      return get_is_dark (default_instance);
    return (system_scheme == ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK);
  case ADW_COLOR_SCHEME_FORCE_LIGHT:
    return FALSE;
  case ADW_COLOR_SCHEME_PREFER_LIGHT:
    return system_scheme == ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;
  case ADW_COLOR_SCHEME_PREFER_DARK:
    return system_scheme != ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT;
  case ADW_COLOR_SCHEME_FORCE_DARK:
    return TRUE;
  default:
    g_assert_not_reached ();
  }
}

static void
update_dark (AdwStyleManager *self)
{
  gboolean dark = get_is_dark (self);

  if (dark == self->dark)
    return;

  self->dark = dark;

  update_stylesheet (self, UPDATE_COLOR_SCHEME);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DARK]);
}

static void
update_fonts (AdwStyleManager *self)
{
  gboolean document_changed = FALSE;
  gboolean monospace_changed = FALSE;

  const char *new_document_font_name = adw_settings_get_document_font_name (self->settings);
  const char *new_monospace_font_name = adw_settings_get_monospace_font_name (self->settings);

  if (new_document_font_name) {
    if (g_set_str (&self->document_font_name, new_document_font_name))
      document_changed = TRUE;
  } else {
    char *font_name = NULL;

    g_object_get (self->gtk_settings, "gtk-font-name", &font_name, NULL);

    if (!font_name)
      font_name = g_strdup (DEFAULT_DOCUMENT_FONT);

    if (g_set_str (&self->document_font_name, font_name))
      document_changed = TRUE;

    g_free (font_name);
  }

  if (new_monospace_font_name) {
    if (g_set_str (&self->monospace_font_name, new_monospace_font_name))
      monospace_changed = TRUE;
  } else {
    char *font_name = NULL;
    PangoFontDescription *desc = NULL;

    g_object_get (self->gtk_settings, "gtk-font-name", &font_name, NULL);

    if (font_name)
      desc = pango_font_description_from_string (font_name);

    if (desc) {
      char *new_str;

      pango_font_description_set_family (desc, DEFAULT_MONOSPACE_FONT_FAMILY);

      new_str = pango_font_description_to_string (desc);

      if (g_set_str (&self->monospace_font_name, new_str))
        monospace_changed = TRUE;

      g_free (new_str);
    } else {
      if (g_set_str (&self->monospace_font_name, DEFAULT_MONOSPACE_FONT))
        monospace_changed = TRUE;
    }

    pango_font_description_free (desc);
    g_free (font_name);
  }

  if (document_changed || monospace_changed)
    update_stylesheet (self, UPDATE_FONTS);

  if (document_changed)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DOCUMENT_FONT_NAME]);

  if (monospace_changed)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MONOSPACE_FONT_NAME]);
}

static void
notify_system_supports_color_schemes_cb (AdwStyleManager *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
}

static void
notify_accent_color_cb (AdwStyleManager *self)
{
  update_stylesheet (self, UPDATE_ACCENT_COLOR);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACCENT_COLOR]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACCENT_COLOR_RGBA]);
}

static void
notify_system_supports_accent_colors_cb (AdwStyleManager *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_ACCENT_COLORS]);
}

static void
notify_high_contrast_cb (AdwStyleManager *self)
{
  update_stylesheet (self, UPDATE_CONTRAST);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}

static void
adw_style_manager_constructed (GObject *object)
{
  AdwStyleManager *self = ADW_STYLE_MANAGER (object);

  G_OBJECT_CLASS (adw_style_manager_parent_class)->constructed (object);

  if (self->display) {
    gboolean prefer_dark_theme;

    self->gtk_settings = gtk_settings_get_for_display (self->display);

    g_object_get (self->gtk_settings,
                  "gtk-application-prefer-dark-theme", &prefer_dark_theme,
                  NULL);

    if (prefer_dark_theme)
      warn_prefer_dark_theme (self);

    g_signal_connect_object (self->gtk_settings,
                             "notify::gtk-application-prefer-dark-theme",
                             G_CALLBACK (warn_prefer_dark_theme),
                             self,
                             G_CONNECT_SWAPPED);

    if (!adw_is_granite_present () && !g_getenv ("GTK_THEME")) {
      g_object_set (self->gtk_settings,
                    "gtk-theme-name", "Adwaita-empty",
                    NULL);

      self->provider = gtk_css_provider_new ();
      gtk_css_provider_load_from_resource (self->provider,
                                           "/org/gnome/Adwaita/styles/main.css");
      gtk_style_context_add_provider_for_display (self->display,
                                                  GTK_STYLE_PROVIDER (self->provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_THEME);

      self->accent_provider = gtk_css_provider_new ();
      gtk_style_context_add_provider_for_display (self->display,
                                                  GTK_STYLE_PROVIDER (self->accent_provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_THEME);

      self->fonts_provider = gtk_css_provider_new ();
      gtk_style_context_add_provider_for_display (self->display,
                                                  GTK_STYLE_PROVIDER (self->fonts_provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_THEME);
    }

    self->animations_provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_string (self->animations_provider,
                                       "* { transition: none; }");
  } else {
    self->gtk_settings = gtk_settings_get_default ();
  }

  g_signal_connect_object (self->gtk_settings, "notify::gtk-font-name",
                           G_CALLBACK (update_fonts),
                           self,
                           G_CONNECT_SWAPPED);

  self->settings = adw_settings_get_default ();

  g_signal_connect_object (self->settings,
                           "notify::system-supports-color-schemes",
                           G_CALLBACK (notify_system_supports_color_schemes_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->settings,
                           "notify::color-scheme",
                           G_CALLBACK (update_dark),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->settings,
                           "notify::system-supports-accent-colors",
                           G_CALLBACK (notify_system_supports_accent_colors_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->settings,
                           "notify::accent-color",
                           G_CALLBACK (notify_accent_color_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->settings,
                           "notify::high-contrast",
                           G_CALLBACK (notify_high_contrast_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->settings,
                           "notify::document-font-name",
                           G_CALLBACK (update_fonts),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->settings,
                           "notify::monospace-font-name",
                           G_CALLBACK (update_fonts),
                           self,
                           G_CONNECT_SWAPPED);

  update_dark (self);
  update_fonts (self);
  update_stylesheet (self, UPDATE_ALL);
}

static void
adw_style_manager_dispose (GObject *object)
{
  AdwStyleManager *self = ADW_STYLE_MANAGER (object);

  g_clear_handle_id (&self->animation_timeout_id, g_source_remove);
  g_clear_object (&self->provider);
  g_clear_object (&self->animations_provider);
  g_clear_object (&self->accent_provider);
  g_clear_object (&self->fonts_provider);
  g_clear_pointer (&self->document_font_name, g_free);
  g_clear_pointer (&self->monospace_font_name, g_free);

  G_OBJECT_CLASS (adw_style_manager_parent_class)->dispose (object);
}

static void
adw_style_manager_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwStyleManager *self = ADW_STYLE_MANAGER (object);

  switch (prop_id) {
  case PROP_DISPLAY:
    g_value_set_object (value, adw_style_manager_get_display (self));
    break;

  case PROP_COLOR_SCHEME:
    g_value_set_enum (value, adw_style_manager_get_color_scheme (self));
    break;

  case PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES:
    g_value_set_boolean (value, adw_style_manager_get_system_supports_color_schemes (self));
    break;

  case PROP_DARK:
    g_value_set_boolean (value, adw_style_manager_get_dark (self));
    break;

  case PROP_HIGH_CONTRAST:
    g_value_set_boolean (value, adw_style_manager_get_high_contrast (self));
    break;

  case PROP_SYSTEM_SUPPORTS_ACCENT_COLORS:
    g_value_set_boolean (value, adw_style_manager_get_system_supports_accent_colors (self));
    break;

  case PROP_ACCENT_COLOR:
    g_value_set_enum (value, adw_style_manager_get_accent_color (self));
    break;

  case PROP_ACCENT_COLOR_RGBA:
    g_value_take_boxed (value, adw_style_manager_get_accent_color_rgba (self));
    break;

  case PROP_DOCUMENT_FONT_NAME:
    g_value_set_string (value, adw_style_manager_get_document_font_name (self));
    break;

  case PROP_MONOSPACE_FONT_NAME:
    g_value_set_string (value, adw_style_manager_get_monospace_font_name (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_manager_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwStyleManager *self = ADW_STYLE_MANAGER (object);

  switch (prop_id) {
  case PROP_DISPLAY:
    self->display = g_value_get_object (value);
    break;

  case PROP_COLOR_SCHEME:
    adw_style_manager_set_color_scheme (self, g_value_get_enum (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_style_manager_class_init (AdwStyleManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_style_manager_constructed;
  object_class->dispose = adw_style_manager_dispose;
  object_class->get_property = adw_style_manager_get_property;
  object_class->set_property = adw_style_manager_set_property;

  /**
   * AdwStyleManager:display:
   *
   * The display the style manager is associated with.
   *
   * The display will be `NULL` for the style manager returned by
   * [func@StyleManager.get_default].
   */
  props[PROP_DISPLAY] =
    g_param_spec_object ("display", NULL, NULL,
                         GDK_TYPE_DISPLAY,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:color-scheme:
   *
   * The requested application color scheme.
   *
   * The effective appearance will be decided based on the application color
   * scheme and the system preferred color scheme. The
   * [property@StyleManager:dark] property can be used to query the current
   * effective appearance.
   *
   * The `ADW_COLOR_SCHEME_PREFER_LIGHT` color scheme results in the application
   * using light appearance unless the system prefers dark colors. This is the
   * default value.
   *
   * The `ADW_COLOR_SCHEME_PREFER_DARK` color scheme results in the application
   * using dark appearance, but can still switch to the light appearance if the
   * system can prefers it, for example, when the high contrast preference is
   * enabled.
   *
   * The `ADW_COLOR_SCHEME_FORCE_LIGHT` and `ADW_COLOR_SCHEME_FORCE_DARK` values
   * ignore the system preference entirely. They are useful if the application
   * wants to match its UI to its content or to provide a separate color scheme
   * switcher.
   *
   * If a per-[class@Gdk.Display] style manager has its color scheme set to
   * `ADW_COLOR_SCHEME_DEFAULT`, it will inherit the color scheme from the
   * default style manager.
   *
   * For the default style manager, `ADW_COLOR_SCHEME_DEFAULT` is equivalent to
   * `ADW_COLOR_SCHEME_PREFER_LIGHT`.
   *
   * The [property@StyleManager:system-supports-color-schemes] property can be
   * used to check if the current environment provides a color scheme
   * preference.
   */
  props[PROP_COLOR_SCHEME] =
    g_param_spec_enum ("color-scheme", NULL, NULL,
                       ADW_TYPE_COLOR_SCHEME,
                       ADW_COLOR_SCHEME_DEFAULT,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwStyleManager:system-supports-color-schemes:
   *
   * Whether the system supports color schemes.
   *
   * This property can be used to check if the current environment provides a
   * color scheme preference. For example, applications might want to show a
   * separate appearance switcher if it's set to `FALSE`.
   *
   * See [property@StyleManager:color-scheme].
   */
  props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES] =
    g_param_spec_boolean ("system-supports-color-schemes", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:dark:
   *
   * Whether the application is using dark appearance.
   *
   * This property can be used to query the current appearance, as requested via
   * [property@StyleManager:color-scheme].
   */
  props[PROP_DARK] =
    g_param_spec_boolean ("dark", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:high-contrast:
   *
   * Whether the application is using high contrast appearance.
   *
   * This cannot be overridden by applications.
   */
  props[PROP_HIGH_CONTRAST] =
    g_param_spec_boolean ("high-contrast", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:system-supports-accent-colors:
   *
   * Whether the system supports accent colors.
   *
   * This property can be used to check if the current environment provides an
   * accent color preference. For example, applications might want to show a
   * preference for choosing accent color if it's set to `FALSE`.
   *
   * See [property@StyleManager:accent-color].
   *
   * Since: 1.6
   */
  props[PROP_SYSTEM_SUPPORTS_ACCENT_COLORS] =
    g_param_spec_boolean ("system-supports-accent-colors", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:accent-color:
   *
   * The current system accent color.
   *
   * See also [property@StyleManager:accent-color-rgba].
   *
   * Since: 1.6
   */
  props[PROP_ACCENT_COLOR] =
    g_param_spec_enum ("accent-color", NULL, NULL,
                       ADW_TYPE_ACCENT_COLOR,
                       ADW_ACCENT_COLOR_BLUE,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:accent-color-rgba:
   *
   * The current system accent color as a `GdkRGBA`.
   *
   * Equivalent to calling [func@AccentColor.to_rgba] on the value of
   * [property@StyleManager:accent-color].
   *
   * This is a background color. The matching foreground color is white.
   *
   * Since: 1.6
   */
  props[PROP_ACCENT_COLOR_RGBA] =
    g_param_spec_boxed ("accent-color-rgba", NULL, NULL,
                        GDK_TYPE_RGBA,
                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:document-font-name:
   *
   * The system document font.
   *
   * The font is in the same format as [property@Gtk.Settings:gtk-font-name],
   * e.g. "Adwaita Sans 12".
   *
   * Use [func@Pango.FontDescription.from_string] to parse it.
   *
   * Since: 1.7
   */
  props[PROP_DOCUMENT_FONT_NAME] =
    g_param_spec_string ("document-font-name", NULL, NULL,
                         DEFAULT_DOCUMENT_FONT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwStyleManager:monospace-font-name:
   *
   * The system monospace font.
   *
   * The font is in the same format as [property@Gtk.Settings:gtk-font-name],
   * e.g. "Adwaita Mono 11".
   *
   * Use [func@Pango.FontDescription.from_string] to parse it.
   *
   * Since: 1.7
   */
  props[PROP_MONOSPACE_FONT_NAME] =
    g_param_spec_string ("monospace-font-name", NULL, NULL,
                         DEFAULT_MONOSPACE_FONT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_style_manager_init (AdwStyleManager *self)
{
  self->color_scheme = ADW_COLOR_SCHEME_DEFAULT;
}

void
adw_style_manager_ensure (void)
{
  GdkDisplayManager *display_manager = gdk_display_manager_get ();
  GSList *displays;
  GSList *l;

  if (display_style_managers)
    return;

  default_instance = g_object_new (ADW_TYPE_STYLE_MANAGER, NULL);
  display_style_managers = g_hash_table_new_full (g_direct_hash,
                                                  g_direct_equal,
                                                  NULL,
                                                  g_object_unref);

  displays = gdk_display_manager_list_displays (display_manager);

  for (l = displays; l; l = l->next)
    register_display (display_manager, l->data);

  g_signal_connect (display_manager,
                    "display-opened",
                    G_CALLBACK (register_display),
                    NULL);

  g_slist_free (displays);
}

/**
 * adw_style_manager_get_default:
 *
 * Gets the default `AdwStyleManager` instance.
 *
 * It manages all [class@Gdk.Display] instances unless the style manager for
 * that display has an override.
 *
 * See [func@StyleManager.get_for_display].
 *
 * Returns: (transfer none): the default style manager
 */
AdwStyleManager *
adw_style_manager_get_default (void)
{
  if (!default_instance)
    adw_style_manager_ensure ();

  return default_instance;
}

/**
 * adw_style_manager_get_for_display:
 * @display: a `GdkDisplay`
 *
 * Gets the `AdwStyleManager` instance managing @display.
 *
 * It can be used to override styles for that specific display instead of the
 * whole application.
 *
 * Most applications should use [func@StyleManager.get_default] instead.
 *
 * Returns: (transfer none): the style manager for @display
 */
AdwStyleManager *
adw_style_manager_get_for_display (GdkDisplay *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);

  if (!display_style_managers)
    adw_style_manager_ensure ();

  g_return_val_if_fail (g_hash_table_contains (display_style_managers, display), NULL);

  return g_hash_table_lookup (display_style_managers, display);
}

/**
 * adw_style_manager_get_display:
 * @self: a style manager
 *
 * Gets the display the style manager is associated with.
 *
 * The display will be `NULL` for the style manager returned by
 * [func@StyleManager.get_default].
 *
 * Returns: (transfer none) (nullable): the display
 */
GdkDisplay *
adw_style_manager_get_display (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), NULL);

  return self->display;
}

/**
 * adw_style_manager_get_color_scheme:
 * @self: a style manager
 *
 * Gets the requested application color scheme.
 *
 * Returns: the color scheme
 */
AdwColorScheme
adw_style_manager_get_color_scheme (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), ADW_COLOR_SCHEME_DEFAULT);

  return self->color_scheme;
}

/**
 * adw_style_manager_set_color_scheme:
 * @self: a style manager
 * @color_scheme: the color scheme
 *
 * Sets the requested application color scheme.
 *
 * The effective appearance will be decided based on the application color
 * scheme and the system preferred color scheme. The
 * [property@StyleManager:dark] property can be used to query the current
 * effective appearance.
 *
 * The `ADW_COLOR_SCHEME_PREFER_LIGHT` color scheme results in the application
 * using light appearance unless the system prefers dark colors. This is the
 * default value.
 *
 * The `ADW_COLOR_SCHEME_PREFER_DARK` color scheme results in the application
 * using dark appearance, but can still switch to the light appearance if the
 * system can prefers it, for example, when the high contrast preference is
 * enabled.
 *
 * The `ADW_COLOR_SCHEME_FORCE_LIGHT` and `ADW_COLOR_SCHEME_FORCE_DARK` values
 * ignore the system preference entirely. They are useful if the application
 * wants to match its UI to its content or to provide a separate color scheme
 * switcher.
 *
 * If a per-[class@Gdk.Display] style manager has its color scheme set to
 * `ADW_COLOR_SCHEME_DEFAULT`, it will inherit the color scheme from the
 * default style manager.
 *
 * For the default style manager, `ADW_COLOR_SCHEME_DEFAULT` is equivalent to
 * `ADW_COLOR_SCHEME_PREFER_LIGHT`.
 *
 * The [property@StyleManager:system-supports-color-schemes] property can be
 * used to check if the current environment provides a color scheme
 * preference.
 */
void
adw_style_manager_set_color_scheme (AdwStyleManager *self,
                                    AdwColorScheme   color_scheme)
{
  g_return_if_fail (ADW_IS_STYLE_MANAGER (self));

  if (color_scheme == self->color_scheme)
    return;

  self->color_scheme = color_scheme;

  g_object_freeze_notify (G_OBJECT (self));

  update_dark (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);

  g_object_thaw_notify (G_OBJECT (self));

  if (!self->display) {
    GHashTableIter iter;
    AdwStyleManager *manager;

    g_hash_table_iter_init (&iter, display_style_managers);

    while (g_hash_table_iter_next (&iter, NULL, (gpointer) &manager))
      if (manager->color_scheme == ADW_COLOR_SCHEME_DEFAULT)
        update_dark (manager);
  }
}

/**
 * adw_style_manager_get_system_supports_color_schemes:
 * @self: a style manager
 *
 * Gets whether the system supports color schemes.
 *
 * This can be used to check if the current environment provides a color scheme
 * preference. For example, applications might want to show a separate
 * appearance switcher if it's set to `FALSE`.
 *
 * Returns: whether the system supports color schemes
 */
gboolean
adw_style_manager_get_system_supports_color_schemes (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), FALSE);

  return adw_settings_get_system_supports_color_schemes (self->settings);
}

/**
 * adw_style_manager_get_dark:
 * @self: a style manager
 *
 * Gets whether the application is using dark appearance.
 *
 * This can be used to query the current appearance, as requested via
 * [property@StyleManager:color-scheme].
 *
 * Returns: whether the application is using dark appearance
 */
gboolean
adw_style_manager_get_dark (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), FALSE);

  return self->dark;
}

/**
 * adw_style_manager_get_high_contrast:
 * @self: a style manager
 *
 * Gets whether the application is using high contrast appearance.
 *
 * This cannot be overridden by applications.
 *
 * Returns: whether the application is using high contrast appearance
 */
gboolean
adw_style_manager_get_high_contrast (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), FALSE);

  return adw_settings_get_high_contrast (self->settings);
}

/**
 * adw_style_manager_get_system_supports_accent_colors:
 * @self: a style manager
 *
 * Gets whether the system supports accent colors.
 *
 * This can be used to check if the current environment provides an accent color
 * preference. For example, applications might want to show a preference for
 * choosing accent color if it's set to `FALSE`.
 *
 * See [property@StyleManager:accent-color].
 *
 * Returns: whether the system supports accent colors
 *
 * Since: 1.6
 */
gboolean
adw_style_manager_get_system_supports_accent_colors (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), FALSE);

  return adw_settings_get_system_supports_accent_colors (self->settings);
}

/**
 * adw_style_manager_get_accent_color:
 * @self: a style manager
 *
 * Gets the current system accent color.
 *
 * See also [property@StyleManager:accent-color-rgba].
 *
 * Returns: the current system accent color
 *
 * Since: 1.6
 */
AdwAccentColor
adw_style_manager_get_accent_color (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), ADW_ACCENT_COLOR_BLUE);

  return adw_settings_get_accent_color (self->settings);
}

/**
 * adw_style_manager_get_accent_color_rgba:
 * @self: a style manager
 *
 * Gets the current system accent color as a `GdkRGBA`.
 *
 * Equivalent to calling [func@AccentColor.to_rgba] on the value of
 * [property@StyleManager:accent-color].
 *
 * This is a background color. The matching foreground color is white.
 *
 * Returns: the current system accent color
 *
 * Since: 1.6
 */
GdkRGBA *
adw_style_manager_get_accent_color_rgba (AdwStyleManager *self)
{
  AdwAccentColor color;
  GdkRGBA rgba;

  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), NULL);

  color = adw_style_manager_get_accent_color (self);

  adw_accent_color_to_rgba (color, &rgba);

  return gdk_rgba_copy (&rgba);
}

/**
 * adw_style_manager_get_document_font_name:
 * @self: a style manager
 *
 * Gets the system document font.
 *
 * The font is in the same format as [property@Gtk.Settings:gtk-font-name],
 * e.g. "Adwaita Sans 12".
 *
 * Use [func@Pango.FontDescription.from_string] to parse it.
 *
 * Returns: the system document font
 *
 * Since: 1.7
 */
const char *
adw_style_manager_get_document_font_name (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), NULL);

  if (!self->document_font_name)
    return DEFAULT_DOCUMENT_FONT;

  return self->document_font_name;
}

/**
 * adw_style_manager_get_monospace_font_name:
 * @self: a style manager
 *
 * Gets the system monospace font.
 *
 * The font is in the same format as [property@Gtk.Settings:gtk-font-name],
 * e.g. "Adwaita Mono 11".
 *
 * Use [func@Pango.FontDescription.from_string] to parse it.
 *
 * Returns: the system monospace font
 *
 * Since: 1.7
 */
const char *
adw_style_manager_get_monospace_font_name (AdwStyleManager *self)
{
  g_return_val_if_fail (ADW_IS_STYLE_MANAGER (self), NULL);

  if (!self->document_font_name)
    return DEFAULT_MONOSPACE_FONT;

  return self->monospace_font_name;
}
