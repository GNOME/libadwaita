/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#define _WIN32_WINNT 0x0602

#include "config.h"

#include "adw-settings-private.h"

#include <gio/gio.h>
#include <gtk/gtk.h>

#ifdef __APPLE__
# include <AppKit/AppKit.h>
#endif

#ifdef G_OS_WIN32
#  define INITGUID
#  include <gdk/win32/gdkwin32.h>
#  ifdef HAS_WINRT
#    define COBJMACROS
#    include <inspectable.h>
#    include <roapi.h>
#    include <winstring.h>
#    include <Windows.UI.ViewManagement.h>
#    include <Windows.Foundation.h>
#  endif
#  ifndef  WM_THEMECHANGED
#    define WM_THEMECHANGED                 0x031A
#  endif
#endif

#define PORTAL_BUS_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_OBJECT_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_SETTINGS_INTERFACE "org.freedesktop.portal.Settings"

#define PORTAL_ERROR_NOT_FOUND "org.freedesktop.portal.Error.NotFound"

struct _AdwSettings
{
  GObject parent_instance;

  GDBusProxy *settings_portal;
  GSettings *interface_settings;
  GSettings *a11y_settings;

  AdwSystemColorScheme color_scheme;
  gboolean high_contrast;

  gboolean has_high_contrast;
  gboolean has_color_scheme;

  enum {
    COLOR_SCHEME_STATE_FDO,
    COLOR_SCHEME_STATE_GNOME,
    COLOR_SCHEME_STATE_NONE
  } color_scheme_portal_state;
  enum {
    HIGH_CONTRAST_STATE_GNOME,
    HIGH_CONTRAST_STATE_NONE
  } high_contrast_portal_state;

  gboolean override;
  gboolean system_supports_color_schemes_override;
  AdwSystemColorScheme color_scheme_override;
  gboolean high_contrast_override;
};

G_DEFINE_FINAL_TYPE (AdwSettings, adw_settings, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES,
  PROP_COLOR_SCHEME,
  PROP_HIGH_CONTRAST,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static AdwSettings *default_instance;

static void
set_color_scheme (AdwSettings          *self,
                  AdwSystemColorScheme  color_scheme)
{
  if (color_scheme == self->color_scheme)
    return;

  self->color_scheme = color_scheme;

  if (!self->override)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
}

static void
set_high_contrast (AdwSettings *self,
                   gboolean     high_contrast)
{
  if (high_contrast == self->high_contrast)
    return;
  
  self->high_contrast = high_contrast;

  if (!self->override)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}

static void
init_debug (AdwSettings *self)
{
  const char *env = g_getenv ("ADW_DEBUG_HIGH_CONTRAST");
  if (env && *env) {
    if (!g_strcmp0 (env, "1")) {
      self->has_high_contrast = TRUE;
      self->high_contrast = TRUE;
    } else if (!g_strcmp0 (env, "0")) {
      self->has_high_contrast = TRUE;
      self->high_contrast = FALSE;
    } else {
      g_warning ("Invalid value for ADW_DEBUG_HIGH_CONTRAST: %s (Expected 0 or 1)", env);
    }
  }

  env = g_getenv ("ADW_DEBUG_COLOR_SCHEME");
  if (env) {
    if (!g_strcmp0 (env, "default")) {
      self->has_color_scheme = TRUE;
      self->color_scheme = ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
    } else if (!g_strcmp0 (env, "prefer-dark")) {
      self->has_color_scheme = TRUE;
      self->color_scheme = ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;
    } else if (!g_strcmp0 (env, "prefer-light")) {
      self->has_color_scheme = TRUE;
      self->color_scheme = ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT;
    } else {
      g_warning ("Invalid color scheme %s (Expected one of: default, prefer-dark, prefer-light)", env);
    }
  }
}

/* Settings portal */

#if defined(G_OS_UNIX) && !defined(__APPLE__)
static gboolean
get_disable_portal (void)
{
  const char *disable_portal = g_getenv ("ADW_DISABLE_PORTAL");

  return disable_portal && disable_portal[0] == '1';
}

static gboolean
read_portal_setting (AdwSettings  *self,
                     const char   *schema,
                     const char   *name,
                     const char   *type,
                     GVariant    **out)
{
  GError *error = NULL;
  GVariant *ret;
  GVariant *child, *child2;
  GVariantType *out_type;
  gboolean result = FALSE;

  ret = g_dbus_proxy_call_sync (self->settings_portal,
                                "Read",
                                g_variant_new ("(ss)", schema, name),
                                G_DBUS_CALL_FLAGS_NONE,
                                G_MAXINT,
                                NULL,
                                &error);
  if (error) {
    if (error->domain == G_DBUS_ERROR &&
        error->code == G_DBUS_ERROR_SERVICE_UNKNOWN) {
      g_debug ("Portal not found: %s", error->message);
    } else if (error->domain == G_DBUS_ERROR &&
               error->code == G_DBUS_ERROR_UNKNOWN_METHOD) {
      g_debug ("Portal doesn't provide settings: %s", error->message);
    } else if (g_dbus_error_is_remote_error (error)) {
      char *remote_error = g_dbus_error_get_remote_error (error);

      if (!g_strcmp0 (remote_error, PORTAL_ERROR_NOT_FOUND)) {
        g_debug ("Setting %s.%s of type %s not found", schema, name, type);
      }
      g_free (remote_error);
    } else {
      g_critical ("Couldn't read the %s setting: %s", name, error->message);
    }

    g_clear_error (&error);

    return FALSE;
  }

  g_variant_get (ret, "(v)", &child);
  g_variant_get (child, "v", &child2);

  out_type = g_variant_type_new (type);
  if (g_variant_type_equal (g_variant_get_type (child2), out_type)) {
    *out = child2;

    result = TRUE;
  } else {
    g_critical ("Invalid type for %s.%s: expected %s, got %s",
                schema, name, type, g_variant_get_type_string (child2));

    g_variant_unref (child2);
  }

  g_variant_type_free (out_type);
  g_variant_unref (child);
  g_variant_unref (ret);
  g_clear_error (&error);

  return result;
}

static AdwSystemColorScheme
get_fdo_color_scheme (GVariant *variant)
{
  guint32 color_scheme = g_variant_get_uint32 (variant);

  if (color_scheme > ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT) {
    g_warning ("Invalid color scheme: %u", color_scheme);

    color_scheme = ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
  }

  return color_scheme;
}

static AdwSystemColorScheme
get_gnome_color_scheme (GVariant *variant)
{
  const char *str = g_variant_get_string (variant, NULL);

  if (!g_strcmp0 (str, "default"))
    return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;

  if (!g_strcmp0 (str, "prefer-dark"))
    return ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;

  if (!g_strcmp0 (str, "prefer-light"))
    return ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT;

  g_warning ("Invalid color scheme: %s", str);

  return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

static void
settings_portal_changed_cb (GDBusProxy  *proxy,
                            const char  *sender_name,
                            const char  *signal_name,
                            GVariant    *parameters,
                            AdwSettings *self)
{
  const char *namespace;
  const char *name;
  GVariant *value = NULL;

  if (g_strcmp0 (signal_name, "SettingChanged"))
    return;

  g_variant_get (parameters, "(&s&sv)", &namespace, &name, &value);

  if (!g_strcmp0 (namespace, "org.freedesktop.appearance") &&
      !g_strcmp0 (name, "color-scheme") &&
      self->color_scheme_portal_state == COLOR_SCHEME_STATE_FDO) {
    set_color_scheme (self, get_fdo_color_scheme (value));

    g_variant_unref (value);

    return;
  }

  if (!g_strcmp0 (namespace, "org.gnome.desktop.interface") &&
      !g_strcmp0 (name, "color-scheme") &&
      self->color_scheme_portal_state == COLOR_SCHEME_STATE_GNOME) {
    set_color_scheme (self, get_gnome_color_scheme (value));

    g_variant_unref (value);

    return;
  }

  if (!g_strcmp0 (namespace, "org.gnome.desktop.a11y.interface") &&
      !g_strcmp0 (name, "high-contrast") &&
      self->high_contrast_portal_state == HIGH_CONTRAST_STATE_GNOME) {
    set_high_contrast (self, g_variant_get_boolean (value));

    g_variant_unref (value);

    return;
  }
}

static void
init_portal (AdwSettings *self)
{
  GError *error = NULL;
  GVariant *variant;

  if (get_disable_portal ())
    return;

  self->settings_portal = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                         G_DBUS_PROXY_FLAGS_NONE,
                                                         NULL,
                                                         PORTAL_BUS_NAME,
                                                         PORTAL_OBJECT_PATH,
                                                         PORTAL_SETTINGS_INTERFACE,
                                                         NULL,
                                                         &error);
  if (error) {
    g_debug ("Settings portal not found: %s", error->message);

    g_error_free (error);

    return;
  }

  if (!self->has_color_scheme &&
      read_portal_setting (self, "org.freedesktop.appearance",
                           "color-scheme", "u", &variant)) {
    self->has_color_scheme = TRUE;
    self->color_scheme_portal_state = COLOR_SCHEME_STATE_FDO;
    self->color_scheme = get_fdo_color_scheme (variant);

    g_variant_unref (variant);
  }

  if (!self->has_color_scheme &&
      read_portal_setting (self, "org.gnome.desktop.interface",
                           "color-scheme", "s", &variant)) {
    self->has_color_scheme = TRUE;
    self->color_scheme_portal_state = COLOR_SCHEME_STATE_GNOME;
    self->color_scheme = get_gnome_color_scheme (variant);

    g_variant_unref (variant);
  }

  if (!self->has_high_contrast &&
      read_portal_setting (self, "org.gnome.desktop.a11y.interface",
                           "high-contrast", "b", &variant)) {
    self->has_high_contrast = TRUE;
    self->high_contrast_portal_state = HIGH_CONTRAST_STATE_GNOME;
    self->high_contrast = g_variant_get_boolean (variant);

    g_variant_unref (variant);
  }

  if (!self->has_color_scheme && !self->has_high_contrast)
    return;

  g_signal_connect (self->settings_portal, "g-signal",
                    G_CALLBACK (settings_portal_changed_cb), self);
}
#endif

#ifdef __APPLE__
@interface ThemeChangedObserver : NSObject
{
  AdwSettings *settings;
}
@end

@implementation ThemeChangedObserver

-(instancetype)initWithSettings:(AdwSettings *)_settings
{
  [self init];
  g_set_weak_pointer (&self->settings, _settings);
  return self;
}

-(void)dealloc
{
  g_clear_weak_pointer (&self->settings);
  [super dealloc];
}

static AdwSystemColorScheme
get_ns_color_scheme (void)
{
  if (@available(*, macOS 10.14)) {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    NSString *style = [userDefaults stringForKey:@"AppleInterfaceStyle"];
    BOOL isDark = [style isEqualToString:@"Dark"];
#if 0
    BOOL isAuto = [userDefaults boolForKey:@"AppleInterfaceStyleSwitchesAutomatically"];
    BOOL isHighContrast = NO;

    /* We can get HighContrast using [NSAppearance currentAppearance] and
     * checking for the variants with HighContrast in their name, however
     * those do not update when the notifications come in (or ever it
     * seems unless a NSView changes them while drawing. If we can monitor
     * a NSView, we could watch for effectiveAppearance changes.
     */
#endif

    [style release];

    return isDark ?
      ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK :
      ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
  }

  return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

-(void)appDidChangeTheme:(NSNotification *)notification
{
  if (self->settings != NULL)
    set_color_scheme (self->settings, get_ns_color_scheme ());
}
@end

static void
init_nsapp_observer (AdwSettings *settings)
{
  static ThemeChangedObserver *observer;

  if (settings->has_color_scheme)
    return;

  if (@available(*, macOS 10.14)) {
    settings->has_color_scheme = TRUE;
  }

  observer = [[ThemeChangedObserver alloc] initWithSettings:settings];

  [[NSDistributedNotificationCenter defaultCenter]
    addObserver:observer
       selector:@selector(appDidChangeTheme:)
           name:@"AppleInterfaceThemeChangedNotification"
         object:nil];

  [observer appDidChangeTheme:nil];
}
#endif

#ifdef G_OS_WIN32

// Set dark mode if the foreground color is brighter than a threshold.
// Algorithm is suggested by IsColorLight() in this example:
// https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes
static inline gboolean
scheme_for_fg_color (DWORD c)
{
  if (((5 * GetGValue (c)) + (2 * GetRValue (c)) + GetBValue (c)) > (8 * 128))
    return ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;
  else
    return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

#  ifdef HAS_WINRT

G_DEFINE_QUARK (winrt-settings, winrt_settings);

DEFINE_GUID (IID_IUISettings3, 0x03021be4, 0x5254, 0x4781, 0x81, 0x94, 0x51, 0x68, 0xf7, 0xd0, 0x6d, 0x7b);
DEFINE_GUID (IID_UISettingsEventHandler, 0x2dbdba9d, 0x20da, 0x519d, 0x90, 0x78, 0x09, 0xf8, 0x35, 0xbc, 0x5b, 0xc7);

#define _RoInitialize (data->RoInitialize_func)
#define _RoActivateInstance (data->RoActivateInstance_func)
#define _WindowsCreateStringReference (data->WindowsCreateStringReference_func)

typedef struct {
  AdwSettings *settings;
  gboolean initialized;

  HMODULE combase;
  HRESULT (WINAPI *RoInitialize_func)(RO_INIT_TYPE initType);
  HRESULT (WINAPI *RoActivateInstance_func)(HSTRING activatableClassId, IInspectable **instance);
  HRESULT (WINAPI *WindowsCreateStringReference_func)(PCWSTR sourceString, UINT32 length, HSTRING_HEADER *hstringHeader, HSTRING *string);

  IInspectable *ii_ui;
  __x_ABI_CWindows_CUI_CViewManagement_CIUISettings3 *ui;

  EventRegistrationToken color_changed_token;
} SettingsData;

static void
cleanup_winrt_settings (gpointer user_data)
{
  SettingsData *data = user_data;

  if (data->color_changed_token.value)
    data->ui->lpVtbl->remove_ColorValuesChanged (data->ui, data->color_changed_token);
  if (data->ui)
    data->ui->lpVtbl->Release (data->ui);
  if (data->ii_ui)
    IInspectable_Release (data->ii_ui);

  if (data->combase)
    FreeLibrary (data->combase);

  g_free (data);
}

static inline IInspectable *
activate (SettingsData *data, const WCHAR *name)
{
  HRESULT res;
  HSTRING_HEADER header;
  HSTRING str;
  IInspectable *iface;

  if (!data->initialized)
    return NULL;

  res = _WindowsCreateStringReference (name, wcslen (name), &header, &str);
  if (FAILED (res))
    return NULL;
  res = _RoActivateInstance (str, &iface);
  if (FAILED (res))
    return NULL;

  return iface;
}

typedef struct
{
  void *lpVtbl;
  gint ref_count;
  SettingsData *data;
} TypedEventHandler;

static inline TypedEventHandler *
TypedEventHandler_New (void *vtbl, SettingsData *data)
{
  TypedEventHandler *handler = g_new0 (TypedEventHandler, 1);

  handler->lpVtbl = vtbl;
  handler->ref_count = 1;
  handler->data = data;

  return handler;
}

static ULONG STDMETHODCALLTYPE
TypedEventHandler_AddRef (TypedEventHandler *self)
{
  return g_atomic_int_add (&self->ref_count, 1) + 1;
}

static ULONG STDMETHODCALLTYPE
TypedEventHandler_Release (TypedEventHandler *self)
{
  gint rc;

  rc = g_atomic_int_add (&self->ref_count, -1) - 1;
  if (rc < 0)
    g_error ("Event handler over-released");
  if (rc == 0)
    g_free (self);
  return rc;
}

static HRESULT STDMETHODCALLTYPE
UISettingsEvent_QueryInterface (__FITypedEventHandler_2_Windows__CUI__CViewManagement__CUISettings_IInspectable *self,
                                REFIID iid,
                                void **iface)
{
   if (IsEqualIID (iid, &IID_UISettingsEventHandler)
      || IsEqualIID (iid, &IID_IUnknown) || IsEqualIID (iid, &IID_IAgileObject))
    {
      *iface = &self->lpVtbl;
      self->lpVtbl->AddRef (self);
      return S_OK;
    }
  *iface = NULL;
  return E_NOINTERFACE;
}

static inline HRESULT
color_values_changed (AdwSettings *settings)
{
  SettingsData *data;
  struct __x_ABI_CWindows_CUI_CColor color;
  HRESULT res;

  data = g_object_get_qdata (G_OBJECT (settings), winrt_settings_quark ());
  if (!data)
    return S_FALSE;

  res = data->ui->lpVtbl->GetColorValue (data->ui, UIColorType_Foreground, &color);
  if (FAILED (res))
    return res;

  set_color_scheme (settings, scheme_for_fg_color (RGB (color.R, color.G, color.B)));

  return S_OK;
}

static int
color_values_changed_idle (gpointer settings)
{
  color_values_changed (settings);
  return G_SOURCE_REMOVE;
}

static HRESULT STDMETHODCALLTYPE
ColorValuesChanged_Invoke (__FITypedEventHandler_2_Windows__CUI__CViewManagement__CUISettings_IInspectable *self,
                           __x_ABI_CWindows_CUI_CViewManagement_CIUISettings *sender,
                           IInspectable *args)
{
  TypedEventHandler *handler = (void *) self;

  g_idle_add (color_values_changed_idle, handler->data->settings);
  return S_OK;
}

struct __FITypedEventHandler_2_Windows__CUI__CViewManagement__CUISettings_IInspectableVtbl ColorValuesChangedVtbl = {
  .QueryInterface = UISettingsEvent_QueryInterface,
  .AddRef = (void *) TypedEventHandler_AddRef,
  .Release = (void *) TypedEventHandler_Release,
  .Invoke = ColorValuesChanged_Invoke,
};

static void
init_winrt_ui_settings (SettingsData *data)
{
  HRESULT res;
  TypedEventHandler *handler;

  data->ii_ui = activate (data, RuntimeClass_Windows_UI_ViewManagement_UISettings);
  if (!data->ii_ui)
    return;
  res = IInspectable_QueryInterface (data->ii_ui,
                                     &IID_IUISettings3,
                                     (void **) &data->ui);
  if (FAILED (res))
    return;
  res = color_values_changed (data->settings);
  if (FAILED (res))
    return;
  handler = TypedEventHandler_New (&ColorValuesChangedVtbl, data);
  data->ui->lpVtbl->add_ColorValuesChanged (data->ui,
                                            (void *) handler,
                                            &data->color_changed_token);
  TypedEventHandler_Release (handler);

  data->settings->has_color_scheme = TRUE;
}

static void
init_winrt_module (SettingsData *data)
{
  HRESULT res;

  data->RoInitialize_func = (void *) GetProcAddress (data->combase, "RoInitialize");
  if (data->RoInitialize_func == NULL)
    return;
  data->RoActivateInstance_func = (void *) GetProcAddress (data->combase, "RoActivateInstance");
  if (data->RoActivateInstance_func == NULL)
    return;
  data->WindowsCreateStringReference_func = (void *) GetProcAddress (data->combase, "WindowsCreateStringReference");
  if (data->WindowsCreateStringReference_func == NULL)
    return;

  res = _RoInitialize (RO_INIT_MULTITHREADED);
  if (res != RPC_E_CHANGED_MODE && FAILED (res))
    return;

  data->initialized = TRUE;
}

static void
init_winrt_settings (AdwSettings *settings)
{
  SettingsData *data;
  HMODULE combase;

  combase = LoadLibraryW (L"combase.dll");
  if (combase == NULL)
    return;

  data = g_new0 (SettingsData, 1);
  data->settings = settings;
  data->combase = combase;

  init_winrt_module (data);
  init_winrt_ui_settings (data);

  if (settings->has_color_scheme)
    g_object_set_qdata_full (G_OBJECT (settings),
                             winrt_settings_quark (),
                             data,
                             cleanup_winrt_settings);
  else
    cleanup_winrt_settings (data);
}

static gboolean
has_winrt_settings (AdwSettings *settings)
{
  return g_object_get_qdata (G_OBJECT (settings), winrt_settings_quark ()) != NULL;
}
#  else
static void
init_winrt_settings (AdwSettings *settings)
{
  // noop
}
static gboolean
has_winrt_settings (AdwSettings *settings)
{
  return FALSE;
}

static inline HRESULT
color_values_changed (AdwSettings *settings)
{
  DWORD c = GetSysColor(COLOR_WINDOWTEXT);

  set_color_scheme (settings, scheme_for_fg_color (c));

  return S_OK;
}
#  endif

static void
system_colors_changed (AdwSettings *settings)
{
  HIGHCONTRASTA hc;

  hc.cbSize = sizeof hc;
  hc.dwFlags = 0;
  hc.lpszDefaultScheme = NULL;
  if (SystemParametersInfoA (SPI_GETHIGHCONTRAST, sizeof hc, &hc, 0))
    set_high_contrast (settings, !!(hc.dwFlags & HCF_HIGHCONTRASTON));
  color_values_changed (settings);
}

static GdkWin32MessageFilterReturn
system_colors_filter (GdkWin32Display *display,
                      MSG             *message,
                      int             *return_value,
                      gpointer         data)
{
  if (message->message == WM_SYSCOLORCHANGE || message->message == WM_THEMECHANGED)
    system_colors_changed (data);
  return GDK_WIN32_MESSAGE_FILTER_CONTINUE;
}

static void
cleanup_high_contrast_filter (gpointer display, GObject *settings)
{
  gdk_win32_display_remove_filter (GDK_WIN32_DISPLAY (display), system_colors_filter, settings);
  g_object_unref (display);
}

static void
init_win32_settings (AdwSettings *settings)
{
  GdkDisplay *display;

  init_winrt_settings (settings);

  display = gdk_display_get_default ();
  if (GDK_IS_WIN32_DISPLAY (display))
    {
      settings->has_high_contrast = TRUE;
      settings->has_color_scheme = TRUE;

      gdk_win32_display_add_filter (GDK_WIN32_DISPLAY (display), system_colors_filter, settings);
      g_object_weak_ref (G_OBJECT (settings), cleanup_high_contrast_filter, g_object_ref (display));
      system_colors_changed (settings);
    }
}
#endif

/* GSettings */

#ifndef G_OS_WIN32
static gboolean
is_running_in_flatpak (void)
{
  return g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS);
}
#endif

static void
gsettings_color_scheme_changed_cb (AdwSettings *self)
{
  set_color_scheme (self, g_settings_get_enum (self->interface_settings, "color-scheme"));
}

static void
gsettings_high_contrast_changed_cb (AdwSettings *self)
{
  set_high_contrast (self, g_settings_get_boolean (self->a11y_settings, "high-contrast"));
}

static void
init_gsettings (AdwSettings *self)
{
  GSettingsSchemaSource *source;
  GSettingsSchema *schema;

#ifndef G_OS_WIN32
  /* While we can access gsettings in flatpak, we can't do anything useful with
   * them as they aren't propagated from the system. */
  if (is_running_in_flatpak ())
    return;
#endif

  source = g_settings_schema_source_get_default ();

  schema = g_settings_schema_source_lookup (source, "org.gnome.desktop.interface", TRUE);
  if (schema &&
      !self->has_color_scheme &&
      g_settings_schema_has_key (schema, "color-scheme")) {
    self->has_color_scheme = TRUE;
    self->interface_settings = g_settings_new ("org.gnome.desktop.interface");
    self->color_scheme = g_settings_get_enum (self->interface_settings, "color-scheme");

    g_signal_connect_swapped (self->interface_settings,
                              "changed::color-scheme",
                              G_CALLBACK (gsettings_color_scheme_changed_cb),
                              self);
  }

  g_clear_pointer (&schema, g_settings_schema_unref);

  schema = g_settings_schema_source_lookup (source, "org.gnome.desktop.a11y.interface", TRUE);
  if (schema &&
      !self->has_high_contrast &&
      g_settings_schema_has_key (schema, "high-contrast")) {
    self->has_high_contrast = TRUE;
    self->a11y_settings = g_settings_new ("org.gnome.desktop.a11y.interface");
    self->high_contrast = g_settings_get_boolean (self->a11y_settings, "high-contrast");

    g_signal_connect_swapped (self->a11y_settings,
                              "changed::high-contrast",
                              G_CALLBACK (gsettings_high_contrast_changed_cb),
                              self);
  }

  g_clear_pointer (&schema, g_settings_schema_unref);
}

/* Legacy */

static gboolean
is_theme_high_contrast (GdkDisplay *display)
{
  GValue value = G_VALUE_INIT;
  const char *theme_name;
  gboolean ret;

  g_value_init (&value, G_TYPE_STRING);
  if (!gdk_display_get_setting (display, "gtk-theme-name", &value))
    return FALSE;

  theme_name = g_value_get_string (&value);

  ret = !g_strcmp0 (theme_name, "HighContrast") ||
        !g_strcmp0 (theme_name, "HighContrastInverse");

  g_value_unset (&value);

  return ret;
}

static void
display_setting_changed_cb (AdwSettings *self,
                            const char  *setting,
                            GdkDisplay  *display)
{
  if (!g_strcmp0 (setting, "gtk-theme-name"))
    set_high_contrast (self, is_theme_high_contrast (display));
}

static void
init_legacy (AdwSettings *self)
{
  GdkDisplay *display = gdk_display_get_default ();

  if (!display)
    return;

  self->has_high_contrast = TRUE;
  self->high_contrast = is_theme_high_contrast (display);

  g_signal_connect_swapped (display,
                            "setting-changed",
                            G_CALLBACK (display_setting_changed_cb),
                            self);
}

static void
adw_settings_constructed (GObject *object)
{
  AdwSettings *self = ADW_SETTINGS (object);

  G_OBJECT_CLASS (adw_settings_parent_class)->constructed (object);

  init_debug (self);
#ifdef __APPLE__
  if (!self->has_color_scheme || !self->has_high_contrast)
    init_nsapp_observer (self);
#elif defined(G_OS_WIN32)
  if (!self->has_color_scheme || !self->has_high_contrast)
    init_win32_settings (self);
#elif defined(G_OS_UNIX)
  if (!self->has_color_scheme || !self->has_high_contrast)
    init_portal (self);
#endif

  if (!self->has_color_scheme || !self->has_high_contrast)
    init_gsettings (self);

  if (!self->has_high_contrast)
    init_legacy (self);
}

static void
adw_settings_dispose (GObject *object)
{
  AdwSettings *self = ADW_SETTINGS (object);

  g_clear_object (&self->settings_portal);
  g_clear_object (&self->interface_settings);
  g_clear_object (&self->a11y_settings);

  G_OBJECT_CLASS (adw_settings_parent_class)->dispose (object);
}

static void
adw_settings_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwSettings *self = ADW_SETTINGS (object);

  switch (prop_id) {
  case PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES:
    g_value_set_boolean (value, adw_settings_get_system_supports_color_schemes (self));
    break;

  case PROP_COLOR_SCHEME:
    g_value_set_enum (value, adw_settings_get_color_scheme (self));
    break;

  case PROP_HIGH_CONTRAST:
    g_value_set_boolean (value, adw_settings_get_high_contrast (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_settings_class_init (AdwSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_settings_constructed;
  object_class->dispose = adw_settings_dispose;
  object_class->get_property = adw_settings_get_property;

  props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES] =
    g_param_spec_boolean ("system-supports-color-schemes", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_COLOR_SCHEME] =
    g_param_spec_enum ("color-scheme", NULL, NULL,
                       ADW_TYPE_SYSTEM_COLOR_SCHEME,
                       ADW_SYSTEM_COLOR_SCHEME_DEFAULT,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_HIGH_CONTRAST] =
    g_param_spec_boolean ("high-contrast", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_settings_init (AdwSettings *self)
{
  self->high_contrast_portal_state = HIGH_CONTRAST_STATE_NONE;
  self->color_scheme_portal_state = COLOR_SCHEME_STATE_NONE;
}

AdwSettings *
adw_settings_get_default (void)
{
  if (!default_instance)
    default_instance = g_object_new (ADW_TYPE_SETTINGS, NULL);

  return default_instance;
}

gboolean
adw_settings_get_system_supports_color_schemes (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), FALSE);

  if (self->override)
    return self->system_supports_color_schemes_override;

  return self->has_color_scheme;
}

AdwSystemColorScheme
adw_settings_get_color_scheme (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), ADW_SYSTEM_COLOR_SCHEME_DEFAULT);

  if (self->override)
    return self->color_scheme_override;

  return self->color_scheme;
}

gboolean
adw_settings_get_high_contrast (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), FALSE);

  if (self->override)
    return self->high_contrast_override;

  return self->high_contrast;
}

void
adw_settings_start_override (AdwSettings *self)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));

  if (self->override)
    return;

  self->override = TRUE;

  self->system_supports_color_schemes_override = self->has_color_scheme;
  self->color_scheme_override = self->color_scheme;
  self->high_contrast_override = self->high_contrast;
}

void
adw_settings_end_override (AdwSettings *self)
{
  gboolean notify_system_supports_color_scheme, notify_color_scheme, notify_hc;

  g_return_if_fail (ADW_IS_SETTINGS (self));

  if (!self->override)
    return;

  notify_system_supports_color_scheme = self->system_supports_color_schemes_override != self->has_color_scheme;
  notify_color_scheme = self->color_scheme_override != self->color_scheme;
  notify_hc = self->high_contrast_override != self->high_contrast;

  self->override = FALSE;
  self->system_supports_color_schemes_override = FALSE;
  self->color_scheme_override = ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
  self->high_contrast_override = FALSE;

  if (notify_system_supports_color_scheme)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
  if (notify_color_scheme)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
  if (notify_hc)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}

void
adw_settings_override_system_supports_color_schemes (AdwSettings *self,
                                                     gboolean     system_supports_color_schemes)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  system_supports_color_schemes = !!system_supports_color_schemes;

  if (system_supports_color_schemes == self->system_supports_color_schemes_override)
    return;

  if (!system_supports_color_schemes)
    adw_settings_override_color_scheme (self, ADW_SYSTEM_COLOR_SCHEME_DEFAULT);

  self->system_supports_color_schemes_override = system_supports_color_schemes;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
}

void
adw_settings_override_color_scheme (AdwSettings          *self,
                                    AdwSystemColorScheme  color_scheme)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  if (color_scheme == self->color_scheme_override ||
      !self->system_supports_color_schemes_override)
    return;

  self->color_scheme_override = color_scheme;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
}

void
adw_settings_override_high_contrast (AdwSettings *self,
                                     gboolean     high_contrast)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  high_contrast = !!high_contrast;

  if (high_contrast == self->high_contrast_override)
    return;

  self->high_contrast_override = high_contrast;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}
