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

@interface AccentColorChangedObserver : NSObject
{
  AdwSettingsImpl *impl;
}
@end

@implementation AccentColorChangedObserver

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
  if (@available(*, macOS 10.14)) {
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

  return ADW_ACCENT_COLOR_BLUE;
}

-(void)appDidChangeAccentColor:(NSNotification *)notification
{
  if (self->impl != NULL)
    adw_settings_impl_set_accent_color (self->impl, get_accent_color ());
}

@end

@interface ThemeChangedObserver : NSObject
{
  AdwSettingsImpl *impl;
}
@end

@implementation ThemeChangedObserver

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

    return isDark ?
      ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK :
      ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
  }

  return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

-(void)appDidChangeTheme:(NSNotification *)notification
{
  if (self->impl != NULL)
    adw_settings_impl_set_color_scheme (self->impl, get_ns_color_scheme ());
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
                             gboolean enable_accent_colors)
{
  AdwSettingsImplMacOS *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_MACOS, NULL);

  if (enable_accent_colors) {
    if (@available(*, macOS 10.14)) {
      static AccentColorChangedObserver *observer;

      observer = [[AccentColorChangedObserver alloc] initWithSettings:(AdwSettingsImpl *)self];

      [[NSDistributedNotificationCenter defaultCenter]
        addObserver:observer
          selector:@selector(appDidChangeAccentColor:)
              name:@"AppleColorPreferencesChangedNotification"
            object:nil];

      [observer appDidChangeAccentColor:nil];
    } else {
      enable_accent_colors = false;
    }
  }

  if (enable_color_scheme) {
    if (@available(*, macOS 10.14)) {
      static ThemeChangedObserver *observer;

      observer = [[ThemeChangedObserver alloc] initWithSettings:(AdwSettingsImpl *)self];

      [[NSDistributedNotificationCenter defaultCenter]
      addObserver:observer
        selector:@selector(appDidChangeTheme:)
            name:@"AppleInterfaceThemeChangedNotification"
          object:nil];

      [observer appDidChangeTheme:nil];
    } else {
      enable_color_scheme = false;
    }
  }

  adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                  enable_color_scheme,
                                  enable_high_contrast,
                                  enable_accent_colors);

  return ADW_SETTINGS_IMPL (self);
}
