/*
 * Copyright (C) 2026 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-icon-provider-private.h"

#include "adw-spinner-paintable.h"

#define GTK_PATH "/org/gtk/libgtk/icons"
#define INTERNAL_PATH "/org/gnome/Adwaita/icons"

struct _AdwIconProvider
{
  GObject parent_instance;

  GdkDisplay *display;

  GHashTable *icon_data;
  gboolean invalidated;

  GPtrArray *resource_paths;

  GtkIconTheme *fallback;
};

static void adw_icon_provider_buildable_init (GtkIconProviderInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwIconProvider, adw_icon_provider, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ICON_PROVIDER, adw_icon_provider_buildable_init))

enum {
  PROP_0,
  PROP_DISPLAY,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
scan_icons_for_path (AdwIconProvider *self,
                     const char      *path,
                     gboolean         trim_symbolic)
{
  GError *error = NULL;
  char **children;
  int i;

  children = g_resources_enumerate_children (path,
                                             G_RESOURCE_LOOKUP_FLAGS_NONE,
                                             &error);

  // TODO error check
  if (error) {
    g_error ("%s", error->message);
    g_clear_error (&error);
  }

  for (i = 0; children[i]; i++) {
    char *icon_name, *icon_path;

    if (!g_str_has_suffix (children[i], ".svg") &&
        !g_str_has_suffix (children[i], ".gpa")) {
      continue;
    }

    icon_name = g_strdup (children[i]);
    icon_name[strlen (icon_name) - 4] = '\0';

    icon_path = g_strconcat (path, "/", children[i], NULL);

//    g_print ("%s\n", icon_name);
    g_hash_table_insert (self->icon_data, icon_name, icon_path);

    if (trim_symbolic) {
      char *trimmed_icon_name = g_strdup (icon_name);

      if (g_str_has_suffix (icon_name, "-symbolic-ltr") ||
          g_str_has_suffix (icon_name, "-symbolic-rtl")) {
        int len = strlen (icon_name);
        /* Replace -symbolic-rtl with -rtl */
        // TODO I'm sure glib has something less horrible for this
        memcpy (&trimmed_icon_name[len - 13], &trimmed_icon_name[len - 4], 5);
      } else if (g_str_has_suffix (icon_name, "-symbolic")) {
        trimmed_icon_name[strlen (trimmed_icon_name) - 9] = '\0';
      }

      icon_path = g_strconcat (path, "/", children[i], NULL);

//      g_print ("%s\n", trimmed_icon_name);
      g_hash_table_insert (self->icon_data, trimmed_icon_name, icon_path);
    }
  }

  g_strfreev (children);
}

static void
ensure_icons (AdwIconProvider *self)
{
  int i;

  if (!self->invalidated)
    return;

  scan_icons_for_path (self, GTK_PATH, TRUE);
  scan_icons_for_path (self, INTERNAL_PATH, TRUE);

  for (i = 0; i < self->resource_paths->len; i++) {
    const char *path = g_ptr_array_index (self->resource_paths, i);

    scan_icons_for_path (self, path, TRUE);
  }

  self->invalidated = FALSE;
}

static void
invalidate (AdwIconProvider *self)
{
  g_hash_table_remove_all (self->icon_data);
  self->invalidated = TRUE;

  gtk_icon_provider_invalidate (GTK_ICON_PROVIDER (self));
}

static void
adw_icon_provider_dispose (GObject *object)
{
  AdwIconProvider *self = ADW_ICON_PROVIDER (object);

  if (self->fallback) {
    g_signal_handlers_disconnect_by_func (self->fallback,
                                          gtk_icon_provider_invalidate,
                                          self);
  }

  g_clear_object (&self->display);
  g_clear_object (&self->fallback);

  G_OBJECT_CLASS (adw_icon_provider_parent_class)->dispose (object);
}

static void
adw_icon_provider_finalize (GObject *object)
{
  AdwIconProvider *self = ADW_ICON_PROVIDER (object);

  g_hash_table_unref (self->icon_data);
  g_ptr_array_unref (self->resource_paths);

  G_OBJECT_CLASS (adw_icon_provider_parent_class)->finalize (object);
}

static void
adw_icon_provider_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwIconProvider *self = ADW_ICON_PROVIDER (object);

  switch (prop_id) {
  case PROP_DISPLAY:
    g_value_set_object (value, self->display);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_icon_provider_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwIconProvider *self = ADW_ICON_PROVIDER (object);

  switch (prop_id) {
  case PROP_DISPLAY:
    g_set_object (&self->display, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_icon_provider_class_init (AdwIconProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_icon_provider_dispose;
  object_class->finalize = adw_icon_provider_finalize;
  object_class->get_property = adw_icon_provider_get_property;
  object_class->set_property = adw_icon_provider_set_property;

  props[PROP_DISPLAY] =
    g_param_spec_object ("display", NULL, NULL,
                         GDK_TYPE_DISPLAY,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_icon_provider_init (AdwIconProvider *self)
{
  self->icon_data = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  self->resource_paths = g_ptr_array_new_full (0, g_free);
  self->invalidated = TRUE;
}

static void
ensure_fallback (AdwIconProvider *self)
{
  if (self->fallback)
    return;

  self->fallback = gtk_icon_theme_get_for_display (self->display);

  g_signal_connect_swapped (self->fallback, "changed",
                            G_CALLBACK (gtk_icon_provider_invalidate), self);
}

static GdkPaintable *
create_missing (int size)
{
  GtkSnapshot *snapshot = gtk_snapshot_new ();
  GdkRGBA rgba;

  rgba.red = rgba.blue = rgba.alpha = 1;
  rgba.green = 0;

  gtk_snapshot_append_color (snapshot, &rgba, &GRAPHENE_RECT_INIT (0, 0, size, size));

  return gtk_snapshot_free_to_paintable (snapshot, &GRAPHENE_SIZE_INIT (size, size));
}

static gboolean
is_legacy_symbolic (const char *path)
{
  if (g_str_has_prefix (path, GTK_PATH))
    return FALSE;

  return g_str_has_suffix (path, "-symbolic.svg") ||
         g_str_has_suffix (path, ".symbolic.png") ||
         g_str_has_suffix (path, "-symbolic-ltr.svg") ||
         g_str_has_suffix (path, "-symbolic-rtl.svg");
}

static GdkPaintable *
adw_icon_provider_lookup_icon (GtkIconProvider    *provider,
                               const char         *icon_name,
                               int                 size,
                               float               scale,
                               GtkTextDirection    direction,
                               GtkIconLookupFlags  flags)
{
  AdwIconProvider *self = ADW_ICON_PROVIDER (provider);
  const char *path;
  char **parts;
  int state = -2;

  ensure_icons (self);

  parts = g_strsplit (icon_name, ":", 2);

  if (g_strv_length (parts) > 1) {
    long int parsed;

    icon_name = parts[0];

    parsed = strtol (parts[1], NULL, 10);

    if (parsed >= 0 && parsed < 32)
      state = parsed;
  }

  path = g_hash_table_lookup (self->icon_data, icon_name);
  if (path) {
    GtkSvg *svg = gtk_svg_new ();
    gboolean legacy = is_legacy_symbolic (path);

    if (legacy)
      gtk_svg_set_features (svg, GTK_SVG_EXTENSIONS | GTK_SVG_TRADITIONAL_SYMBOLIC);
    else
      gtk_svg_set_features (svg, GTK_SVG_EXTENSIONS);

    gtk_svg_load_from_resource (svg, path);

    if (!legacy && state >= -1 && state < 64)
      gtk_svg_set_state (svg, state);

#if 0
      char *uri = g_strconcat ("resource://", path, NULL);
      GFile *file = g_file_new_for_uri (uri);

      /* We want to be able to use symbolic icons without -symbolic suffix */
      // TODO ask if gtk people want a constructor to force symbolic status, e.g.
      // gtk_icon_paintable_new_full (file, size, scale, style) where
      // style is an enum with -gtk-icon-style values
      ret = g_object_new (GTK_TYPE_ICON_PAINTABLE,
                          "file", file,
                          "size", size,
                          "scale", (int) scale,
                          "is-symbolic", TRUE,
                          NULL);

      g_object_unref (file);
      g_free (uri);
#endif

//    g_print ("Found %s at %s\n", icon_name, path);

    g_strfreev (parts);

    return GDK_PAINTABLE (svg);
  }

  ensure_fallback (self);

  if (state < -1 && gtk_icon_theme_has_icon (self->fallback, icon_name)) {
    GtkIconPaintable *ret;

    ret = gtk_icon_theme_lookup_icon (self->fallback, icon_name, NULL,
                                      size, (int) ceilf (scale), direction, flags);

//    g_print ("Found %s in icon theme\n", icon_name);

    g_strfreev (parts);

    if (ret)
      return GDK_PAINTABLE (ret);
  }

  g_strfreev (parts);

  return create_missing (size);
}

static gboolean
adw_icon_provider_has_icon (GtkIconProvider *provider,
                            const char      *icon_name)
{
  AdwIconProvider *self = ADW_ICON_PROVIDER (provider);

  ensure_icons (self);

//  g_print ("Has icon? %s\n", icon_name);

  if (g_hash_table_contains (self->icon_data, icon_name))
    return TRUE;

  ensure_fallback (self);

  if (gtk_icon_theme_has_icon (self->fallback, icon_name))
    return TRUE;

  return FALSE;
}

static void
adw_icon_provider_buildable_init (GtkIconProviderInterface *iface)
{
  iface->lookup_icon = adw_icon_provider_lookup_icon;
  iface->has_icon = adw_icon_provider_has_icon;
}

AdwIconProvider *
adw_icon_provider_new (GdkDisplay *display)
{
  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);

  return g_object_new (ADW_TYPE_ICON_PROVIDER, "display", display, NULL);
}

void
adw_icon_provider_add_resource_path (AdwIconProvider *self,
                                     const char      *path)
{
  g_return_if_fail (ADW_IS_ICON_PROVIDER (self));
  g_return_if_fail (path != NULL);

  g_ptr_array_add (self->resource_paths, g_strdup (path));

  invalidate (self);
}

char *
adw_icon_provider_lookup_path (AdwIconProvider *self,
                               const char      *icon_name)
{
  const char *path;

  g_return_val_if_fail (ADW_IS_ICON_PROVIDER (self), NULL);
  g_return_val_if_fail (icon_name != NULL, NULL);

  path = g_hash_table_lookup (self->icon_data, icon_name);

  if (path)
    return g_strdup (path);

  return NULL;
}
