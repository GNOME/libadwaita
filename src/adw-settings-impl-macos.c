/*
 * Copyright (C) 2022 Christian Hergert <christian@hergert.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-settings-impl-private.h"

#include <AppKit/AppKit.h>

struct _AdwSettingsImplMacOS
{
  AdwSettingsImpl parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwSettingsImplMacOS, adw_settings_impl_macos, ADW_TYPE_SETTINGS_IMPL)

@interface SettingsChangedObserver : NSObject
{
  AdwSettingsImpl *impl;
}
@end

@implementation SettingsChangedObserver

-(instancetype)initWithSettings:(AdwSettingsImpl *)_impl
{
  [self init];
  g_set_weak_pointer (&self->impl, _impl);
  return self;
}

-(void)dealloc
{
  g_clear_weak_pointer (&self->impl);
  [super dealloc];
}

static AdwAccentColor
get_accent_color (void)
{
  GdkRGBA rgba;
  NSColor *accentColor = [NSColor.controlAccentColor colorUsingColorSpace:[
                          NSColorSpace sRGBColorSpace]];

  CGFloat red, green, blue, alpha;
  [accentColor getRed:&red green:&green blue:&blue alpha:&alpha];

  rgba.red = red;
  rgba.green = green;
  rgba.blue = blue;
  rgba.alpha = alpha;

  return adw_accent_color_nearest_from_rgba (&rgba);
}

-(void)appDidChangeAccentColor:(NSNotification *)notification
{
  if (self->impl != NULL)
    adw_settings_impl_set_accent_color (self->impl, get_accent_color ());
}

static AdwSystemColorScheme
get_ns_color_scheme (void)
{
  NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
  NSString *style = [userDefaults stringForKey:@"AppleInterfaceStyle"];
  BOOL isDark = [style isEqualToString:@"Dark"];

  return isDark ?
    ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK :
    ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

-(void)appDidChangeTheme:(NSNotification *)notification
{
  if (self->impl != NULL)
    adw_settings_impl_set_color_scheme (self->impl, get_ns_color_scheme ());
}

-(void)appDidChangeHighContrast:(NSNotification *)notification
{
  if (self->impl != NULL) {
    gboolean high_contrast = [[NSWorkspace sharedWorkspace] accessibilityDisplayShouldIncreaseContrast];
    adw_settings_impl_set_high_contrast (self->impl, high_contrast);
  }
}
@end

static void
adw_settings_impl_macos_class_init (AdwSettingsImplMacOSClass *klass)
{
}

static void
adw_settings_impl_macos_init (AdwSettingsImplMacOS *self)
{
}

AdwSettingsImpl *
adw_settings_impl_macos_new (gboolean enable_color_scheme,
                             gboolean enable_high_contrast,
                             gboolean enable_accent_colors,
                             gboolean enable_document_font_name,
                             gboolean enable_monospace_font_name)
{
  static SettingsChangedObserver *observer;

  AdwSettingsImplMacOS *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_MACOS, NULL);

  observer = [[SettingsChangedObserver alloc] initWithSettings:(AdwSettingsImpl *)self];

  if (enable_accent_colors) {
    [[NSDistributedNotificationCenter defaultCenter]
      addObserver:observer
        selector:@selector(appDidChangeAccentColor:)
            name:@"AppleColorPreferencesChangedNotification"
          object:nil];

    [observer appDidChangeAccentColor:nil];
  }

  if (enable_high_contrast) {
    [[[NSWorkspace sharedWorkspace] notificationCenter]
    addObserver:observer
      selector:@selector(appDidChangeHighContrast:)
          name:@"NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification"
        object:nil];

    [observer appDidChangeHighContrast:nil];
  }

  if (enable_color_scheme) {
    [[NSDistributedNotificationCenter defaultCenter]
    addObserver:observer
      selector:@selector(appDidChangeTheme:)
          name:@"AppleInterfaceThemeChangedNotification"
        object:nil];

    [observer appDidChangeTheme:nil];
  }

  adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                  enable_color_scheme,
                                  enable_high_contrast,
                                  enable_accent_colors,
                                  FALSE,
                                  FALSE);

  return ADW_SETTINGS_IMPL (self);
}
