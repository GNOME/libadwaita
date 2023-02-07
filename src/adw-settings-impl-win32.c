/*
 * Copyright (C) // FIXME
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-settings-impl-private.h"

#include <gtk/gtk.h>

#define _WIN32_WINNT 0x0602

#define INITGUID
#include <gdk/win32/gdkwin32.h>

#ifdef HAS_WINRT
#define COBJMACROS
#include <inspectable.h>
#include <roapi.h>
#include <winstring.h>
#include <Windows.UI.ViewManagement.h>
#include <Windows.Foundation.h>
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x031A
#endif

struct _AdwSettingsImplWin32
{
  AdwSettingsImpl parent_instance;

  gboolean initialized;

  HMODULE combase;
  HRESULT (WINAPI *RoInitialize_func)(RO_INIT_TYPE initType);
  HRESULT (WINAPI *RoActivateInstance_func)(HSTRING        activatableClassId,
                                            IInspectable **instance);
  HRESULT (WINAPI *WindowsCreateStringReference_func)(PCWSTR          sourceString,
                                                      UINT32          length,
                                                      HSTRING_HEADER *hstringHeader,
                                                      HSTRING        *string);

  IInspectable *ii_ui;
  __x_ABI_CWindows_CUI_CViewManagement_CIUISettings3 *ui;

  EventRegistrationToken color_changed_token;
};

G_DEFINE_FINAL_TYPE (AdwSettingsImplWin32, adw_settings_impl_win32, ADW_TYPE_SETTINGS_IMPL)

/* Set dark mode if the foreground color is brighter than a threshold.
 * Algorithm is suggested by IsColorLight() in this example:
 * https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes
 */
static inline gboolean
scheme_for_fg_color (DWORD c)
{
  if (5 * GetGValue (c) + 2 * GetRValue (c) + GetBValue (c) > 8 * 128)
    return ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;
  else
    return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

#ifdef HAS_WINRT

DEFINE_GUID (IID_IUISettings3, 0x03021be4, 0x5254, 0x4781, 0x81, 0x94, 0x51, 0x68, 0xf7, 0xd0, 0x6d, 0x7b);
DEFINE_GUID (IID_UISettingsEventHandler, 0x2dbdba9d, 0x20da, 0x519d, 0x90, 0x78, 0x09, 0xf8, 0x35, 0xbc, 0x5b, 0xc7);

#define _RoInitialize (self->RoInitialize_func)
#define _RoActivateInstance (self->RoActivateInstance_func)
#define _WindowsCreateStringReference (self->WindowsCreateStringReference_func)

static inline IInspectable *
activate (AdwSettingsImplWin32 *self,
          const WCHAR          *name)
{
  HRESULT res;
  HSTRING_HEADER header;
  HSTRING str;
  IInspectable *iface;

  if (!self->initialized)
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
  gpointer lpVtbl;
  int ref_count;
  AdwSettingsImplWin32 *self;
} TypedEventHandler;

static inline TypedEventHandler *
TypedEventHandler_New (gpointer              vtbl,
                       AdwSettingsImplWin32 *self)
{
  TypedEventHandler *handler = g_new0 (TypedEventHandler, 1);

  handler->lpVtbl = vtbl;
  handler->ref_count = 1;
  handler->self = self;

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
  int rc;

  rc = g_atomic_int_add (&self->ref_count, -1) - 1;
  if (rc < 0)
    g_error ("Event handler over-released");
  if (rc == 0)
    g_free (self);
  return rc;
}

static HRESULT STDMETHODCALLTYPE
UISettingsEvent_QueryInterface (__FITypedEventHandler_2_Windows__CUI__CViewManagement__CUISettings_IInspectable *self,
                                REFIID                                                                           iid,
                                gpointer                                                                        *iface)
{
   if (IsEqualIID (iid, &IID_UISettingsEventHandler) ||
       IsEqualIID (iid, &IID_IUnknown) ||
       IsEqualIID (iid, &IID_IAgileObject)) {
    *iface = &self->lpVtbl;
    self->lpVtbl->AddRef (self);
    return S_OK;
  }

  *iface = NULL;
  return E_NOINTERFACE;
}

static inline HRESULT
color_values_changed (AdwSettingsImplWin32 *self)
{
  struct __x_ABI_CWindows_CUI_CColor color;
  HRESULT res;

  res = self->ui->lpVtbl->GetColorValue (self->ui, UIColorType_Foreground, &color);
  if (FAILED (res))
    return res;

  adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                      scheme_for_fg_color (RGB (color.R, color.G, color.B)));

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
  TypedEventHandler *handler = (gpointer) self;

  g_idle_add (color_values_changed_idle, handler->self);
  return S_OK;
}

struct __FITypedEventHandler_2_Windows__CUI__CViewManagement__CUISettings_IInspectableVtbl ColorValuesChangedVtbl = {
  .QueryInterface = UISettingsEvent_QueryInterface,
  .AddRef = (gpointer) TypedEventHandler_AddRef,
  .Release = (gpointer) TypedEventHandler_Release,
  .Invoke = ColorValuesChanged_Invoke,
};

static void
init_winrt_ui_settings (AdwSettingsImplWin32 *self)
{
  HRESULT res;
  TypedEventHandler *handler;

  self->ii_ui = activate (self, RuntimeClass_Windows_UI_ViewManagement_UISettings);
  if (!self->ii_ui)
    return;

  res = IInspectable_QueryInterface (self->ii_ui,
                                     &IID_IUISettings3,
                                     (gpointer*) &self->ui);
  if (FAILED (res))
    return;

  res = color_values_changed (self);
  if (FAILED (res))
    return;

  handler = TypedEventHandler_New (&ColorValuesChangedVtbl, self);
  self->ui->lpVtbl->add_ColorValuesChanged (self->ui,
                                            (gpointer) handler,
                                            &self->color_changed_token);
  TypedEventHandler_Release (handler);
}

static void
init_winrt_module (AdwSettingsImplWin32 *self)
{
  HRESULT res;

  self->RoInitialize_func = (gpointer) GetProcAddress (self->combase, "RoInitialize");
  if (self->RoInitialize_func == NULL)
    return;

  self->RoActivateInstance_func = (gpointer) GetProcAddress (self->combase, "RoActivateInstance");
  if (self->RoActivateInstance_func == NULL)
    return;

  self->WindowsCreateStringReference_func = (gpointer) GetProcAddress (self->combase, "WindowsCreateStringReference");
  if (self->WindowsCreateStringReference_func == NULL)
    return;

  res = _RoInitialize (RO_INIT_MULTITHREADED);
  if (res != RPC_E_CHANGED_MODE && FAILED (res))
    return;

  self->initialized = TRUE;
}

static void
init_winrt_settings (AdwSettingsImplWin32 *self)
{
  HMODULE combase;

  combase = LoadLibraryW (L"combase.dll");
  if (combase == NULL)
    return;

  init_winrt_module (self);
  init_winrt_ui_settings (self);
}

#else

static inline HRESULT
color_values_changed (AdwSettingsImplWin32 *self)
{
  DWORD c = GetSysColor(COLOR_WINDOWTEXT);

  adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                      scheme_for_fg_color (c));

  return S_OK;
}
#endif

static void
system_colors_changed (AdwSettingsImplWin32 *self)
{
  HIGHCONTRASTA hc;

  hc.cbSize = sizeof hc;
  hc.dwFlags = 0;
  hc.lpszDefaultScheme = NULL;

  if (SystemParametersInfoA (SPI_GETHIGHCONTRAST, sizeof hc, &hc, 0))
    adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                         !!(hc.dwFlags & HCF_HIGHCONTRASTON));

  color_values_changed (settings);
}

static GdkWin32MessageFilterReturn
system_colors_filter (GdkWin32Display *display,
                      MSG             *message,
                      int             *return_value,
                      gpointer         data)
{
  AdwSettingsImplWin32Class *self = data;

  if (message->message == WM_SYSCOLORCHANGE ||
      message->message == WM_THEMECHANGED)
    system_colors_changed (self);

  return GDK_WIN32_MESSAGE_FILTER_CONTINUE;
}

static void
cleanup_high_contrast_filter (gpointer  display,
                              GObject  *settings)
{
  gdk_win32_display_remove_filter (GDK_WIN32_DISPLAY (display),
                                   system_colors_filter,
                                   settings);
  g_object_unref (display);
}

static void
adw_settings_impl_win32_dispose (GObject *object)
{
  if (self->ui) {
    if (self->color_changed_token.value)
      self->ui->lpVtbl->remove_ColorValuesChanged (self->ui, self->color_changed_token);

    self->ui->lpVtbl->Release (self->ui);
    self->ui = NULL;
  }

  if (self->ii_ui) {
    IInspectable_Release (self->ii_ui);
    self->ii_ui = NULL;
  }

  if (self->combase) {
    FreeLibrary (self->combase);
    self->combase = NULL;
  }

  G_OBJECT_CLASS (adw_settings_impl_win32_parent_class)->dispose (object);
}

static void
adw_settings_impl_win32_class_init (AdwSettingsImplWin32Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_settings_impl_win32_dispose;
}

static void
adw_settings_impl_win32_init (AdwSettingsImplWin32 *self)
{
}

AdwSettingsImpl *
adw_settings_impl_win32_new (gboolean has_color_scheme,
                             gboolean has_high_contrast)
{
  AdwSettingsImplWin32 *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_WIN32, NULL);
  GdkDisplay *display = gdk_display_get_default ();

  if (!GDK_IS_WIN32_DISPLAY (display))
    return ADW_SETTINGS_IMPL (self);

#ifdef HAS_WINRT
  init_winrt_settings (self);
#endif

  gdk_win32_display_add_filter (GDK_WIN32_DISPLAY (display),
                                system_colors_filter,
                                self);
  g_object_weak_ref (G_OBJECT (self),
                     cleanup_high_contrast_filter,
                     g_object_ref (display));
  system_colors_changed (self);

  // FIXME need to actually check if it's available
  adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                  /* has_color_scheme */ TRUE,
                                  /* has_high_contrast */ TRUE);

  return ADW_SETTINGS_IMPL (self);
}
