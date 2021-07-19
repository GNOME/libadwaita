Title: Migrating between development versions
Slug: migrating-between-development-versions

# Migrating between development versions

This guide outlines the differences between Libadwaita development releases.
It assumes you've already migrated from Libhandy 1.4 to Libadwaita, or you
created a new project using a development release of Libadwaita.

If you want to migrate from Libhandy 1.4 to the latest Libadwaita release,
[follow this guide](migrating-libhandy-1-4-to-libadwaita.html).

## Migrating from alpha 1 to alpha 2

### Adapt to view switcher API Changes

[class@Adw.ViewSwitcher], [class@Adw.ViewSwitcherBar] and
[class@Adw.ViewSwitcherTitle] now use [class@Adw.ViewStack] instead of
[class@Gtk.Stack].

You should stop using [property@Gtk.Stack:transition-type] and
[property@Gtk.Stack:transition-duration] properties before switching to
[class@Adw.ViewStack].

### Adapt to Stylesheet Changes

The public colors `@theme_selected_bg_color` and `@theme_selected_fg_color` have
been renamed to `@accent_bg_color` and `@accent_fg_color`.

If you were using `@theme_selected_bg_color` as a text color, use
`@accent_color` instead to make sure the text is readable. You can also use the
`.accent` style class to apply the correct color.
