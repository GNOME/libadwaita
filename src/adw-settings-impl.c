/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-settings-impl-private.h"

#include "adw-marshalers.h"

typedef struct
{
  gboolean has_color_scheme;
  gboolean has_high_contrast;
  gboolean has_accent_colors;
  gboolean has_document_font_name;
  gboolean has_monospace_font_name;

  AdwSystemColorScheme color_scheme;
  gboolean high_contrast;
  AdwAccentColor accent_color;
  char *document_font_name;
  char *monospace_font_name;
} AdwSettingsImplPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (AdwSettingsImpl, adw_settings_impl, G_TYPE_OBJECT)

enum {
  SIGNAL_PREPARE,
  SIGNAL_COLOR_SCHEME_CHANGED,
  SIGNAL_HIGH_CONTRAST_CHANGED,
  SIGNAL_ACCENT_COLOR_CHANGED,
  SIGNAL_DOCUMENT_FONT_NAME_CHANGED,
  SIGNAL_MONOSPACE_FONT_NAME_CHANGED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
adw_settings_impl_dispose (GObject *object)
{
  AdwSettingsImpl *self = ADW_SETTINGS_IMPL (object);
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_clear_pointer (&priv->document_font_name, g_free);
  g_clear_pointer (&priv->monospace_font_name, g_free);

  G_OBJECT_CLASS (adw_settings_impl_parent_class)->dispose (object);
}

static void
adw_settings_impl_class_init (AdwSettingsImplClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_settings_impl_dispose;

  signals[SIGNAL_COLOR_SCHEME_CHANGED] =
    g_signal_new ("color-scheme-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__ENUM,
                  G_TYPE_NONE,
                  1,
                  ADW_TYPE_SYSTEM_COLOR_SCHEME);
  g_signal_set_va_marshaller (signals[SIGNAL_COLOR_SCHEME_CHANGED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__ENUMv);

  signals[SIGNAL_HIGH_CONTRAST_CHANGED] =
    g_signal_new ("high-contrast-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_BOOLEAN);
  g_signal_set_va_marshaller (signals[SIGNAL_HIGH_CONTRAST_CHANGED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__BOOLEANv);

  signals[SIGNAL_ACCENT_COLOR_CHANGED] =
    g_signal_new ("accent-color-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__ENUM,
                  G_TYPE_NONE,
                  1,
                  ADW_TYPE_ACCENT_COLOR);
  g_signal_set_va_marshaller (signals[SIGNAL_ACCENT_COLOR_CHANGED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__ENUMv);

  signals[SIGNAL_DOCUMENT_FONT_NAME_CHANGED] =
    g_signal_new ("document-font-name-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);
  g_signal_set_va_marshaller (signals[SIGNAL_DOCUMENT_FONT_NAME_CHANGED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__STRINGv);

  signals[SIGNAL_MONOSPACE_FONT_NAME_CHANGED] =
    g_signal_new ("monospace-font-name-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);
  g_signal_set_va_marshaller (signals[SIGNAL_MONOSPACE_FONT_NAME_CHANGED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__STRINGv);
}

static void
adw_settings_impl_init (AdwSettingsImpl *self)
{
}

gboolean
adw_settings_impl_get_has_color_scheme (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), FALSE);

  return priv->has_color_scheme;
}

gboolean
adw_settings_impl_get_has_high_contrast (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), FALSE);

  return priv->has_high_contrast;
}

gboolean
adw_settings_impl_get_has_accent_colors (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), FALSE);

  return priv->has_accent_colors;
}

gboolean
adw_settings_impl_get_has_document_font_name (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), FALSE);

  return priv->has_document_font_name;
}

gboolean
adw_settings_impl_get_has_monospace_font_name (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), FALSE);

  return priv->has_monospace_font_name;
}

void
adw_settings_impl_set_features (AdwSettingsImpl *self,
                                gboolean         has_color_scheme,
                                gboolean         has_high_contrast,
                                gboolean         has_accent_colors,
                                gboolean         has_document_font_name,
                                gboolean         has_monospace_font_name)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_if_fail (ADW_IS_SETTINGS_IMPL (self));

  priv->has_color_scheme = !!has_color_scheme;
  priv->has_high_contrast = !!has_high_contrast;
  priv->has_accent_colors = !!has_accent_colors;
  priv->has_document_font_name = !!has_document_font_name;
  priv->has_monospace_font_name = !!has_monospace_font_name;
}

AdwSystemColorScheme
adw_settings_impl_get_color_scheme (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), ADW_SYSTEM_COLOR_SCHEME_DEFAULT);

  return priv->color_scheme;
}

void
adw_settings_impl_set_color_scheme (AdwSettingsImpl      *self,
                                    AdwSystemColorScheme  color_scheme)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_if_fail (ADW_IS_SETTINGS_IMPL (self));

  if (priv->color_scheme == color_scheme)
    return;

  priv->color_scheme = color_scheme;

  if (priv->has_color_scheme)
    g_signal_emit (G_OBJECT (self), signals[SIGNAL_COLOR_SCHEME_CHANGED], 0, color_scheme);
}

gboolean
adw_settings_impl_get_high_contrast (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), FALSE);

  return priv->high_contrast;
}

void
adw_settings_impl_set_high_contrast (AdwSettingsImpl *self,
                                     gboolean         high_contrast)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_if_fail (ADW_IS_SETTINGS_IMPL (self));

  high_contrast = !!high_contrast;

  if (priv->high_contrast == high_contrast)
    return;

  priv->high_contrast = high_contrast;

  if (priv->has_high_contrast)
    g_signal_emit (G_OBJECT (self), signals[SIGNAL_HIGH_CONTRAST_CHANGED], 0, high_contrast);
}

AdwAccentColor
adw_settings_impl_get_accent_color (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv  = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), ADW_ACCENT_COLOR_BLUE);

  return priv->accent_color;
}

void
adw_settings_impl_set_accent_color (AdwSettingsImpl *self,
                                    AdwAccentColor   accent_color)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_if_fail (ADW_IS_SETTINGS_IMPL (self));

  if (priv->accent_color == accent_color)
    return;

  priv->accent_color = accent_color;

  if (priv->has_accent_colors)
    g_signal_emit (G_OBJECT (self), signals[SIGNAL_ACCENT_COLOR_CHANGED], 0, accent_color);
}

const char *
adw_settings_impl_get_document_font_name (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv  = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), NULL);

  return priv->document_font_name;
}

void
adw_settings_impl_set_document_font_name (AdwSettingsImpl *self,
                                          const char      *font_name)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_if_fail (ADW_IS_SETTINGS_IMPL (self));

  if (!g_set_str (&priv->document_font_name, font_name))
    return;

  if (priv->has_document_font_name)
    g_signal_emit (G_OBJECT (self), signals[SIGNAL_DOCUMENT_FONT_NAME_CHANGED], 0, font_name);
}

const char *
adw_settings_impl_get_monospace_font_name (AdwSettingsImpl *self)
{
  AdwSettingsImplPrivate *priv  = adw_settings_impl_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SETTINGS_IMPL (self), NULL);

  return priv->monospace_font_name;
}

void
adw_settings_impl_set_monospace_font_name (AdwSettingsImpl *self,
                                           const char      *font_name)
{
  AdwSettingsImplPrivate *priv = adw_settings_impl_get_instance_private (self);

  g_return_if_fail (ADW_IS_SETTINGS_IMPL (self));

  if (!g_set_str (&priv->monospace_font_name, font_name))
    return;

  if (priv->has_monospace_font_name)
    g_signal_emit (G_OBJECT (self), signals[SIGNAL_MONOSPACE_FONT_NAME_CHANGED], 0, font_name);
}

gboolean
adw_get_disable_portal (void)
{
  const char *disable_portal = g_getenv ("ADW_DISABLE_PORTAL");

  return disable_portal && disable_portal[0] == '1';
}
