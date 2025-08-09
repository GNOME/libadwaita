Title: Styles & Appearance
Slug: styles-and-appearance

# Styles & Appearance

## Dark Style

Libadwaita applications can use a light or a dark appearance. This can be used
to request a darker UI, or to support a system-wide dark style preference if one
exists.

By default, applications use light appearance unless the system prefers dark
appearance, matching the `ADW_COLOR_SCHEME_PREFER_LIGHT` color scheme. The
[property@StyleManager:color-scheme] property allows to change this behavior
when set to:

* `ADW_COLOR_SCHEME_PREFER_DARK`: Use dark appearance unless the system prefers
  light appearance.
* `ADW_COLOR_SCHEME_FORCE_DARK`: Always use dark appearance.
* `ADW_COLOR_SCHEME_FORCE_LIGHT`: Always use light appearance.

<table>
  <tr>
    <th>App \ System</th>
    <th>Prefer Light</th>
    <th>No Preference</th>
    <th>Prefer Dark</th>
  </tr>
  <tr>
    <th><tt>FORCE_LIGHT</tt></th>
    <td>Light</td>
    <td>Light</td>
    <td>Light</td>
  </tr>
  <tr>
    <th><tt>PREFER_LIGHT</tt></th>
    <td>Light</td>
    <td>Light</td>
    <td>Dark</td>
  </tr>
  <tr>
    <th><tt>PREFER_DARK</tt></th>
    <td>Light</td>
    <td>Dark</td>
    <td>Dark</td>
  </tr>
  <tr>
    <th><tt>FORCE_DARK</tt></th>
    <td>Dark</td>
    <td>Dark</td>
    <td>Dark</td>
  </tr>
</table>

Common use cases:

1. An application wants to use dark UI. Use the `ADW_COLOR_SCHEME_PREFER_DARK`
   color scheme.

2. An application has a style switcher with the system, light and
dark states. Use the following color scheme values:

    * System: `ADW_COLOR_SCHEME_PREFER_LIGHT`
    * Light: `ADW_COLOR_SCHEME_FORCE_LIGHT`
    * Dark: `ADW_COLOR_SCHEME_FORCE_DARK`

If the system does not provide a style preference, the
[property@StyleManager:system-supports-color-schemes] property can be used to
provide a fallback. For example, applications with a system/light/dark switcher
may want to hide or disable the system value.

All standard GTK and Libadwaita widgets automatically support both styles.
Applications that use custom drawing or styles may need to ensure the UI
remains legible in both appearances:

* When possible, use [CSS variables](css-variables.html) instead of hardcoded
  colors. For custom drawing, use [method@Gtk.StyleContext.get_color] to get the
  current text color for your widget, or [method@Gtk.StyleContext.lookup_color]
  to look up other colors.

* If that's not possible, use the [property@StyleManager:dark] property to check
  the current appearance and vary the drawing accordingly.

* The `prefers-color-scheme: dark` CSS media query can be used to add custom
  styles specific to dark appearance.

* Alternatively, [class@Application] allows loading additional styles for dark
  appearance via the `style-dark.css` resource.

## Accent Color

Libadwaita applications follow the system accent color by default. Applications
with custom drawing or styles may need to ensure that they use the actual accent
color instead of hardcoding blue:

* Use [accent color variables](css-variables.html#accent-colors) and the
  [`.accent`](style-classes.html#colors) style class in CSS.

* Use [property@StyleManager:accent-color-rgba] to get the background accent
  color programmatically.

* Use [property@StyleManager:accent-color] and 
  [func@AccentColor.to_standalone_rgba] to get the standalone accent color
  programmatically.

Applications can override the accent color using CSS, as follows:

```css
:root {
  --accent-bg-color: var(--accent-green); /* Always use the green color */
}
```

Accent can also be overridden for an individual widget. In this case
`--accent-color` must be manually overridden as well:

```css
my-widget {
  --accent-bg-color: var(--accent-purple); /* Always use the purple color */
  --accent-color: oklab(from var(--accent-bg-color) var(--standalone-color-oklab));
}
```

All of the default accent colors use white text as the foreground color. If the
custom accent color is too bright, use a dark foreground color instead:

```css
:root {
  --accent-bg-color: var(--yellow-3);
  --accent-fg-color: rgb(0 0 0 / 80%);
}
```

::: note
    When accent color is overridden, [class@StyleManager] API will still return
    the system color.

## High Contrast

The system can provide a high contrast preference. Libadwaita applications
automatically support it; applications cannot disable it.

High contrast appearance can be combined with the [dark style](#dark-style) and
is independent from it.

All standard GTK and Libadwaita widgets automatically support the high contrast
appearance. Applications that use custom drawing or styles may need to support
it manually.

* Use style classes such as [`.dimmed`](style-classes.html#dimmed) instead
  of changing widget opacity manually.

* Use the [helper variables](css-variables.html#helpers) when possible, instead
  of hardcoding colors and opacities.

* The [property@StyleManager:high-contrast] property can be used to check the
  current appearance.

* The `prefers-contrast: more` CSS media query can be used to add custom styles
  specific to high contrast.

* Alternatively, [class@Application] allows loading additional styles for high
  contrast appearance via the `style-hc.css` and `style-hc-dark.css` resources.

## Custom Styles

[class@Application] provides a simple way to load additional styles from
[struct@Gio.Resource], relative to the application's base path (see
[method@Gio.Application.set_resource_base_path]).

The following resources will automatically be loaded if present:

- `style.css` contains styles that are always used.
- `style-dark.css` contains styles only used with the dark appearance.
- `style-hc.css` contains styles used with the high contrast appearance.
- `style-hc-dark.css` contains styles used when both dark and high contrast

Styles are stacked on top of each other: when using dark appearance, both
`style.css` and `style-dark.css` are loaded, and so on.

:::note
    `style.css` can contain styles for dark and high contrast appearance as
    well, using media queries:

    - `prefers-color-scheme: dark` for styles used only with dark appearance.
    - `prefers-contrast: more` for styles used only with high contrast appearance.

## See Also

- [Style Classes](style-classes.html)
- [CSS Variables](css-variables.html)
