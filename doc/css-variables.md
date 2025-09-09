Title: CSS Variables
Slug: css-variables

<style type="text/css" rel="stylesheet">
.color-pill {
    width: 20px;
    height: 20px;
    border-radius: 100%;
    background-clip: padding-box;
    border: 1px solid transparent;
}

.color-pill.light {
    border-color: rgb(0 0 0 / 10%);
}

.color-pill.dark {
    border-color: rgb(255 255 255 / 20%);
}

.color-pill.light.dark {
    border-color: rgb(200 200 200 / 35%);
}
</style>

# CSS Variables

The Adwaita stylesheet provides a number of predefined CSS variables for colors
that can be used from applications.

# UI Colors

These colors are used throughout the UI. They can differ between the light and
dark styles.

Many colors are grouped as background/foreground pairs. These colors are always
meant to be used together as the background and foreground color, for example:

```css
my-widget {
  background-color: var(--accent-bg-color);
  color: var(--accent-fg-color);
}
```

Applications can override these colors by re-declaring them, for example:

```css
:root {
  --accent-bg-color: #e01b24;
}
```

## Standalone colors

Some colors also have standalone versions. They are similar to the background
version, but provide better contrast when used as foreground colors on top of a
neutral background - for example, colorful text in a window.

```css
my-widget {
  color: var(--accent-color);
}
```

Standalone colors are typically darker than the corresponding background color
for the light style, and lighter than the background for the dark style. They
are automatically derived from the background color, so it's not necesssary to
override them manually when setting app-wide accent color.

However, when overriding the background colors for specific widgets, the
standalone colors must be overridden too, as follows:

```css
my-widget {
  --accent-bg-color: var(--purple-3);
  --accent-color: oklab(from var(--accent-bg-color) var(--standalone-color-oklab));
}
```

The `--standalone-color-oklab` variable has the following values for light and
dark styles:

Name                              | Light                    | Dark
--------------------------------- | ------------------------ | -------------------------
<tt>--standalone-color-oklab</tt> | <tt>min(l, 0.5) a b</tt> | <tt>max(l, 0.85) a b</tt>

::: note "<a id='out-of-gamut-colors'>Note</a>"
    Adjusting colors in the Oklab color space may produce colors outside of sRGB
    gamut. These colors need to be gamut mapped in order to be displayed on
    screen. This page lists colors after gamut mapping, and out of gamut colors
    will be marked with an asterisk.

    For example, the dark standalone color for blue accent color is
    `oklab(0.850000 -0.081818 -0.053404)`, so
    `color(srgb 0.504313 0.817230 1.218717)` when converted to sRGB. However,
    the blue channel in this color is outside the [0-1] range and is clipped to
    it before being displayed, so the end result is
    `color(srgb 0.504313 0.817230 1)`, or `#81d0ff`.

## Accent Colors

The accent color is used across many different widgets, often to indicate that a
widget is important, interactive, or currently active. Try to avoid using it on
large surfaces, or on too many items on the same view.

The [`.accent`](style-classes.html#colors) style class allows to use it for
widgets such as [class@Gtk.Label].

The background color is available as `--accent-bg-color`, the foreground as
`--accent-fg-color` and the standalone color as `--accent-color`.

The `--accent-color` color is derived from `--accent-bg-color` as detailed
above.

The default values of these colors depend on the system preferences, and will
always be one of the following:

<table>
  <tr>
    <th>Color</th>
    <th/>
    <th>Background</th>
    <th/>
    <th>Foreground</th>
    <th/>
    <th>Standalone (Light)</th>
    <th/>
    <th>Standalone (Dark)</th>
  </tr>
  <tr>
    <td>Blue</td>
    <td><div class="color-pill" style="background-color: #3584e4"/></td>
    <td><tt>#3584e4</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #0461be"/></td>
    <td><tt>#0461be</tt></td>
    <td><div class="color-pill" style="background-color: #81d0ff"/></td>
    <td><tt>#81d0ff</tt> <a href="#out-of-gamut-colors">*</a></td>
  </tr>
  <tr>
    <td>Teal</td>
    <td><div class="color-pill" style="background-color: #2190a4"/></td>
    <td><tt>#2190a4</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #007184"/></td>
    <td><tt>#007184</tt> <a href="#out-of-gamut-colors">*</a></td>
    <td><div class="color-pill" style="background-color: #7bdff4"/></td>
    <td><tt>#7bdff4</tt></td>
  </tr>
  <tr>
    <td>Green</td>
    <td><div class="color-pill" style="background-color: #3a944a"/></td>
    <td><tt>#3a944a</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #15772e"/></td>
    <td><tt>#15772e</tt></td>
    <td><div class="color-pill" style="background-color: #8de698"/></td>
    <td><tt>#8de698</tt></td>
  </tr>
  <tr>
    <td>Yellow</td>
    <td><div class="color-pill" style="background-color: #c88800"/></td>
    <td><tt>#c88800</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #905300"/></td>
    <td><tt>#905300</tt> <a href="#out-of-gamut-colors">*</a></td>
    <td><div class="color-pill" style="background-color: #ffc057"/></td>
    <td><tt>#ffc057</tt> <a href="#out-of-gamut-colors">*</a></td>
  </tr>
  <tr>
    <td>Orange</td>
    <td><div class="color-pill" style="background-color: #ed5b00"/></td>
    <td><tt>#ed5b00</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #b62200"/></td>
    <td><tt>#b62200</tt> <a href="#out-of-gamut-colors">*</a></td>
    <td><div class="color-pill" style="background-color: #ff9c5b"/></td>
    <td><tt>#ff9c5b</tt> <a href="#out-of-gamut-colors">*</a></td>
  </tr>
  <tr>
    <td>Red</td>
    <td><div class="color-pill" style="background-color: #e62d42"/></td>
    <td><tt>#e62d42</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #c00023"/></td>
    <td><tt>#c00023</tt> <a href="#out-of-gamut-colors">*</a></td>
    <td><div class="color-pill" style="background-color: #ff888c"/></td>
    <td><tt>#ff888c</tt> <a href="#out-of-gamut-colors">*</a></td>
  </tr>
  <tr>
    <td>Pink</td>
    <td><div class="color-pill" style="background-color: #d56199"/></td>
    <td><tt>#d56199</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #a2326c"/></td>
    <td><tt>#a2326c</tt></td>
    <td><div class="color-pill" style="background-color: #ffa0d8"/></td>
    <td><tt>#ffa0d8</tt> <a href="#out-of-gamut-colors">*</a></td>
  </tr>
  <tr>
    <td>Purple</td>
    <td><div class="color-pill" style="background-color: #9141ac"/></td>
    <td><tt>#9141ac</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #8939a4"/></td>
    <td><tt>#8939a4</tt></td>
    <td><div class="color-pill" style="background-color: #fba7ff"/></td>
    <td><tt>#fba7ff</tt> <a href="#out-of-gamut-colors">*</a></td>
  </tr>
  <tr>
    <td>Slate</td>
    <td><div class="color-pill" style="background-color: #6f8396"/></td>
    <td><tt>#6f8396</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #526678"/></td>
    <td><tt>#526678</tt></td>
    <td><div class="color-pill" style="background-color: #bbd1e5"/></td>
    <td><tt>#bbd1e5</tt></td>
  </tr>
</table>

Each background color is also available as a variable, as follows:

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Value</th>
  </tr>
  <tr>
    <td><tt>--accent-blue</tt></td>
    <td><div class="color-pill" style="background-color: #3584e4"/></td>
    <td><tt>#3584e4</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-teal</tt></td>
    <td><div class="color-pill" style="background-color: #2190a4"/></td>
    <td><tt>#2190a4</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-green</tt></td>
    <td><div class="color-pill" style="background-color: #3a944a"/></td>
    <td><tt>#3a944a</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-yellow</tt></td>
    <td><div class="color-pill" style="background-color: #c88800"/></td>
    <td><tt>#c88800</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-orange</tt></td>
    <td><div class="color-pill" style="background-color: #ed5b00"/></td>
    <td><tt>#ed5b00</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-red</tt></td>
    <td><div class="color-pill" style="background-color: #e62d42"/></td>
    <td><tt>#e62d42</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-pink</tt></td>
    <td><div class="color-pill" style="background-color: #d56199"/></td>
    <td><tt>#d56199</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-purple</tt></td>
    <td><div class="color-pill" style="background-color: #9141ac"/></td>
    <td><tt>#9141ac</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-slate</tt></td>
    <td><div class="color-pill" style="background-color: #6f8396"/></td>
    <td><tt>#6f8396</tt></td>
  </tr>
</table>

## Destructive Colors

The destructive color indicates a dangerous action, such as deleting a file.
It's used by [class@Gtk.Button] and [class@ButtonRow] with the
[`.destructive-action`](style-classes.html#destructive-action) style class.

The `--destructive-color` color is derived from `--destructive-bg-color` as
detailed above.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--destructive-bg-color</tt></td>
    <td><div class="color-pill" style="background-color: #e01b24"/></td>
    <td><tt>#e01b24</tt></td>
    <td><div class="color-pill" style="background-color: #c01c28"/></td>
    <td><tt>#c01c28</tt></td>
  </tr>
  <tr>
    <td><tt>--destructive-fg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--destructive-color</tt></td>
    <td><div class="color-pill" style="background-color: #c30000"/></td>
    <td><tt>#c30000</tt> <a href="#out-of-gamut-colors">*</a></td>
    <td><div class="color-pill" style="background-color: #ff938c"/></td>
    <td><tt>#ff938c</tt> <a href="#out-of-gamut-colors">*</a></td>
  </tr>
</table>

## Success Colors

This color is used with the [`.success`](style-classes.html#colors) style class,
or in a [class@Gtk.LevelBar] with the [const@Gtk.LEVEL_BAR_OFFSET_HIGH] offset.

The `--success-color` color is derived from `--success-bg-color` as detailed
above.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--success-bg-color</tt></td>
    <td><div class="color-pill" style="background-color: #2ec27e"/></td>
    <td><tt>#2ec27e</tt></td>
    <td><div class="color-pill" style="background-color: #26a269"/></td>
    <td><tt>#26a269</tt></td>
  </tr>
  <tr>
    <td><tt>--success-fg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--success-color</tt></td>
    <td><div class="color-pill" style="background-color: #007c3d"/></td>
    <td><tt>#007c3d</tt> <a href="#out-of-gamut-colors">*</a></td>
    <td><div class="color-pill" style="background-color: #78e9ab"/></td>
    <td><tt>#78e9ab</tt></td>
  </tr>
</table>

## Warning Colors

This color is used with the [`.warning`](style-classes.html#colors) style class,
or in a [class@Gtk.LevelBar] with the [const@Gtk.LEVEL_BAR_OFFSET_LOW] offset.

The `--warning-color` color is derived from `--warning-bg-color` as detailed
above.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--warning-bg-color</tt></td>
    <td><div class="color-pill" style="background-color: #e5a50a"/></td>
    <td><tt>#e5a50a</tt></td>
    <td><div class="color-pill" style="background-color: #cd9309"/></td>
    <td><tt>#cd9309</tt></td>
  </tr>
  <tr>
    <td><tt>--warning-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
  </tr>
  <tr>
    <td><tt>--warning-color</tt></td>
    <td><div class="color-pill" style="background-color: #905400"/></td>
    <td><tt>#905400</tt> <a href="#out-of-gamut-colors">*</a></td>
    <td><div class="color-pill" style="background-color: #ffc252"/></td>
    <td><tt>#ffc252</tt></td>
  </tr>
</table>

## Error Colors

This color is used with the [`.error`](style-classes.html#colors) style class.

The `--error-color` color is derived from `--error-bg-color` as detailed above.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--error-bg-color</tt></td>
    <td><div class="color-pill" style="background-color: #e01b24"/></td>
    <td><tt>#e01b24</tt></td>
    <td><div class="color-pill" style="background-color: #c01c28"/></td>
    <td><tt>#c01c28</tt></td>
  </tr>
  <tr>
    <td><tt>--error-fg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--error-color</tt></td>
    <td><div class="color-pill" style="background-color: #c30000"/></td>
    <td><tt>#c30000</tt> <a href="#out-of-gamut-colors">*</a></td>
    <td><div class="color-pill" style="background-color: #ff938c"/></td>
    <td><tt>#ff938c</tt> <a href="#out-of-gamut-colors">*</a></td>
  </tr>
</table>

## Window Colors

These colors are used on [class@Gtk.Window], as well as with the
[`.background`](style-classes.html#background) style class.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--window-bg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #fafafb"/></td>
    <td><tt>#fafafb</tt></td>
    <td><div class="color-pill dark" style="background-color: #222226"/></td>
    <td><tt>#222226</tt></td>
  </tr>
  <tr>
    <td><tt>--window-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
</table>

## View Colors

These colors are used in a variety of widgets such as [class@Gtk.TextView], as
well as with the [`.view`](style-classes.html#view) style class.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--view-bg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill dark" style="background-color: #1d1d20"/></td>
    <td><tt>#1d1d20</tt></td>
  </tr>
  <tr>
    <td><tt>--view-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
</table>

## Header Bar Colors

These colors are used for header bars and similar widgets, generally attached to
the top or bottom sides of a window. The full list of widgets using them:

- [class@ToolbarView] uses it for the top and bottom bars
- [class@HeaderBar] except with the
  [`.flat`](style-classes.html#flat-header-bar) style class
- [class@TabBar] except with the
  [`.inline`](style-classes.html#inline-tab-bars-search-bars) style class
- [class@Gtk.HeaderBar] except with the
  [`.flat`](style-classes.html#flat-header-bar) style class
- [class@Gtk.SearchBar] except with the
  [`.inline`](style-classes.html#inline-tab-bars-search-bars) style class
- [class@Gtk.ActionBar]

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--headerbar-bg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill dark" style="background-color: #2e2e32"/></td>
    <td><tt>#2e2e32</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-border-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-backdrop-color</tt></td>
    <td><div class="color-pill light" style="background-color: #fafafb"/></td>
    <td><tt>#fafafb</tt></td>
    <td><div class="color-pill dark" style="background-color: #222226"/></td>
    <td><tt>#222226</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 12%)"/></td>
    <td><tt>rgb(0 0 6 / 12%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 36%)"/></td>
    <td><tt>rgb(0 0 6 / 36%)</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-darker-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 12%)"/></td>
    <td><tt>rgb(0 0 6 / 12%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 12 / 90%)"/></td>
    <td><tt>rgb(0 0 6 / 90%)</tt></td>
  </tr>
</table>

`--headerbar-border-color` has the same default value as `--headerbar-fg-color`,
but doesn't change along with it. This can be useful if a light window has a
dark header bar with light text; in this case it may be desirable to keep the
border dark. This variable is only used for vertical borders - for example,
separators between the 2 header bars in a split header bar layout.

`--headerbar-backdrop-color` is used instead of `--headerbar-bg-color` when the
window is not focused. By default it's an alias of
[`--window-bg-color`](#window-colors) and changes together with it. When
overriding header bar colors, make sure to set it to a value matching your
`--headerbar-bg-color`.

`--headerbar-shade-color` is used to provide a dark shadow or a border for
header bars and similar widgets. This color should always be partially
transparent black.

`--headerbar-darker-shade-color` is used for the `ADW_TOOLBAR_RAISED_BORDER`
border. This color should always be partially transparent black, and is intended
to be darker than both `--headerbar-bg-color` and `--headerbar-backdrop-color`
on top of white color.

## Sidebar Colors

These colors are used for sidebars, generally attached to the left or right
sides of a window. They are used by [class@NavigationSplitView] and
[class@OverlaySplitView] when they are not collapsed.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--sidebar-bg-color</tt></td>
    <td><div class="color-pill" style="background-color: #ebebed"/></td>
    <td><tt>#ebebed</tt></td>
    <td><div class="color-pill dark" style="background-color: #2e2e32"/></td>
    <td><tt>#2e2e32</tt></td>
  </tr>
  <tr>
    <td><tt>--sidebar-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--sidebar-backdrop-color</tt></td>
    <td><div class="color-pill" style="background-color: #f2f2f4"/></td>
    <td><tt>#f2f2f4</tt></td>
    <td><div class="color-pill dark" style="background-color: #28282c"/></td>
    <td><tt>#28282c</tt></td>
  </tr>
  <tr>
    <td><tt>--sidebar-border-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 7%)"/></td>
    <td><tt>rgb(0 0 6 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 36%)"/></td>
    <td><tt>rgb(0 0 6 / 36%)</tt></td>
  </tr>
  <tr>
    <td><tt>--sidebar-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 7%)"/></td>
    <td><tt>rgb(0 0 6 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 25%)"/></td>
    <td><tt>rgb(0 0 6 / 25%)</tt></td>
  </tr>
</table>

`--sidebar-backdrop-color` is used instead of `--sidebar-bg-color` when the
window is not focused. When overriding sidebar colors, make sure to set it to a
value matching your `--sidebar-bg-color`.

`--sidebar-shade-color` is used to provide a dark border for sidebars, scroll
undershoots within sidebars, as well as transitions in [class@NavigationView],
[class@OverlaySplitView], [class@Leaflet] and [class@Flap]. This color should
always be partially transparent black, with the opacity tuned to be well visible
on top of `--sidebar-bg-color`.

`--sidebar-border-color` is used to provide a dark border for sidebars. This
color should always be partially transparent black, with the opacity tuned to be
well visible on top of `--sidebar-bg-color` next to `--window-bg-color` or
`--view-bg-color`.

`--sidebar-shade-color`is used to provide scroll undershoots within sidebars, as
well as transitions in [class@NavigationView], [class@OverlaySplitView],
[class@Leaflet] and [class@Flap]. This color should always be partially
transparent black, with the opacity tuned to be well visible on top of
`--sidebar-bg-color`.

Since: 1.4

## Secondary Sidebar Colors

These colors are used for middle panes in triple-pane layouts, created via
nesting two split views within one another.

<picture>
  <source srcset="secondary-sidebar-dark.png" media="(prefers-color-scheme: dark)">
  <img src="secondary-sidebar.png" alt="secondary-sidebar">
</picture>

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-bg-color</tt></td>
    <td><div class="color-pill" style="background-color: #f3f3f5"/></td>
    <td><tt>#f3f3f5</tt></td>
    <td><div class="color-pill dark" style="background-color: #28282c"/></td>
    <td><tt>#28282c</tt></td>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-backdrop-color</tt></td>
    <td><div class="color-pill" style="background-color: #f6f6fa"/></td>
    <td><tt>#f6f6fa</tt></td>
    <td><div class="color-pill dark" style="background-color: #252529"/></td>
    <td><tt>#252529</tt></td>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-border-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 7%)"/></td>
    <td><tt>rgb(0 0 6 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 36%)"/></td>
    <td><tt>rgb(0 0 6 / 36%)</tt></td>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 7%)"/></td>
    <td><tt>rgb(0 0 6 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 25%)"/></td>
    <td><tt>rgb(0 0 6 / 25%)</tt></td>
  </tr>
</table>

`--secondary-sidebar-backdrop-color` is used instead of
`--secondary-sidebar-bg-color` when the window is not focused. When overriding
secondary sidebar colors, make sure to set it to a value matching your
`--secondary-sidebar-bg-color`.

`--secondary-sidebar-border-color` is used to provide a dark border for
secondary sidebars. This color should always be partially transparent black,
with the opacity tuned to be well visible on top of
`--secondary-sidebar-bg-color` next to `--sidebar-bg-color`.

`--secondary-sidebar-shade-color` is used to provide scroll undershoots within
secondary sidebars, as well as transitions in [class@NavigationView],
[class@OverlaySplitView], [class@Leaflet] and [class@Flap]. This color should
always be partially transparent black, with the opacity tuned to be well visible
on top of `--secondary-sidebar-bg-color`.

`--secondary-sidebar-shade-color` is used to provide a dark border for secondary
sidebars, scroll undershoots within secondary sidebars, as well as transitions
in [class@NavigationView], [class@OverlaySplitView], [class@Leaflet] and
[class@Flap]. This color should always be partially transparent black, with the
opacity tuned to be well visible on top of `--secondary-sidebar-bg-color`.

Since: 1.4

## Card Colors

These colors are used for
[cards and boxed lists](style-classes.html#boxed-lists-cards).

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--card-bg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill light dark" style="background-color: rgb(255 255 255 / 8%)"/></td>
    <td><tt>rgb(255 255 255 / 8%)</tt></td>
  </tr>
  <tr>
    <td><tt>--card-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--card-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 7%)"/></td>
    <td><tt>rgb(0 0 6 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 36%)"/></td>
    <td><tt>rgb(0 0 6 / 36%)</tt></td>
  </tr>
</table>

`--card-shade-color` is used to provide separators between boxed list rows and
similar widgets. This color should always be partially transparent black, with
the opacity tuned to be well visible on top of `--card-bg-color`.

## Tab Overview Colors

These colors are used for [class@TabOverview].

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--overview-bg-color</tt></td>
    <td><div class="color-pill" style="background-color: #f3f3f5"/></td>
    <td><tt>#f3f3f5</tt></td>
    <td><div class="color-pill dark" style="background-color: #28282c"/></td>
    <td><tt>#28282c</tt></td>
  </tr>
  <tr>
    <td><tt>--overview-fg-color</tt></td>
    <td><div class="color-pill" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill dark" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
</table>

`--overview-bg-color` and `--overview-bg-color` are used for the overview itself.

Since: 1.7

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--thumbnail-bg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #39393d"/></td>
    <td><tt>#39393d</tt></td>
  </tr>
  <tr>
    <td><tt>--thumbnail-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
</table>

`--thumbnail-bg-color` and `--thumbnail-fg-color` are used for the tab thumbnails.

Since: 1.3

## Active Toggle Colors

These colors are used for the active toggle in [class@ToggleGroup].

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--active-toggle-bg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: rgb(255 255 255 / 20%)"/></td>
    <td><tt>rgb(255 255 255 / 20%)</tt></td>
  </tr>
  <tr>
    <td><tt>--active-toggle-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
</table>

Since: 1.7

## Dialog Colors

These colors are used for [class@AlertDialog].

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--dialog-bg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #fafafb"/></td>
    <td><tt>#fafafb</tt></td>
    <td><div class="color-pill" style="background-color: #36363a"/></td>
    <td><tt>#36363a</tt></td>
  </tr>
  <tr>
    <td><tt>--dialog-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
</table>

Since: 1.2

## Popover Colors

These colors are used for [class@Gtk.Popover].

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--popover-bg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill" style="background-color: #36363a"/></td>
    <td><tt>#36363a</tt></td>
  </tr>
  <tr>
    <td><tt>--popover-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 80%)"/></td>
    <td><tt>rgb(0 0 6 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--popover-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 7%)"/></td>
    <td><tt>rgb(0 0 6 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 25%)"/></td>
    <td><tt>rgb(0 0 6 / 25%)</tt></td>
  </tr>
</table>

`--popover-shade-color` is used for scroll undershoot styles within popovers, as
well as transitions in [class@NavigationView], [class@OverlaySplitView],
[class@Leaflet] and [class@Flap]. This color should always be partially
transparent black, with the opacity tuned to be well visible on top of
`--popover-bg-color`. This color is only available since 1.4.

## Miscellaneous Colors

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 7%)"/></td>
    <td><tt>rgb(0 0 6 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 6 / 25%)"/></td>
    <td><tt>rgb(0 0 6 / 25%)</tt></td>
  </tr>
  <tr>
    <td><tt>--scrollbar-outline-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 12 / 50%)"/></td>
    <td><tt>rgb(0 0 6 / 50%)</tt></td>
  </tr>
</table>

`--shade-color` is used for scroll undershoots, as well as transitions in
[class@NavigationView], [class@OverlaySplitView], [class@Leaflet] and
[class@Flap]. This color should always be partially transparent black, with the
opacity tuned to be well visible on top of `--window-bg-color`.

`--scrollbar-outline-color` is used by [class@Gtk.Scrollbar] to ensure that
overlay scrollbars are visible regardless of the content color. It should always
be the opposite of the scrollbar color - light with a dark scrollbar and dark
otherwise.

# Fonts

These variables allow to access system fonts. For each font, two variables are
defined, corresponding to the font family and size. Most of the time, they
should be used together, as follows:

```css
.my-content {
  font-family: var(--monospace-font-family);
  font-size: var(--monospace-font-size);
}
```

## Document Font

Document font should be used in articles, messages and other content.

It's used for the [`.document`](style-classes.html#document)
style class.

Name                             | Example Value
-------------------------------- | -------------
<tt>--document-font-family</tt>  | Adwaita Sans
<tt>--document-font-size</tt>    | 12pt

## Monospace Font

Monospace font should be used for displaying code, logs or shell commands.

It's used for the [`.monospace`](style-classes.html#monospace)
style class.

Name                             | Example Value
-------------------------------- | -------------
<tt>--monospace-font-family</tt> | Adwaita Mono
<tt>--monospace-font-size</tt>   | 11pt

# Helpers

These variables are provided for convenience (particularly, automatic high
contrast mode support) and should not be overridden.

## Opacity

Name                        | Regular      | High contrast
--------------------------- | ------------ | -------------
<tt>--border-opacity</tt>   | <tt>15%</tt> | <tt>50%</tt>
<tt>--dim-opacity</tt>      | <tt>55%</tt> | <tt>90%</tt>
<tt>--disabled-opacity</tt> | <tt>50%</tt> | <tt>40%</tt>

These variables represent the commonly used opacity values.

`--border-opacity` is used for borders. (see [`--border-color`](#border-color))

`--dim-opacity` is used for the [`.dim`](style-classes.html#dimmed)
style class and other similar contexts, like window and row subtitles.

`--disabled-opacity` is used for disabled widgets.
(see [property@Gtk.Widget:sensitive])

These variables can be used to automatically support high contrast mode.

## Border Color

Name                    | Value
----------------------- | ----------------------------------------------------------------------------
<tt>--border-color</tt> | <tt>color-mix(in srgb, currentColor var(--border-opacity), transparent)</tt>

Border color is derived from the current foreground color (`currentColor`) and
changes between regular and high contrast modes. It should be used to support
the high contrast mode automatically.

## Window Radius

Name                     | Value
------------------------ | -----
<tt>--window-radius</tt> | 15px

Matches the current window radius, whether it's floating or maximized. Can be
used for e.g. rounding focus rings next to the edge of the window while
automatically accounting for maximized, fullscreen etc modes.

# Palette Colors

The stylesheet provides the full
[GNOME color palette](https://developer.gnome.org/hig/reference/palette.html)
as the following set of variables:

<!-- Generated with gen-palette.py -->
<table>
  <tr>
    <th></th>
    <th>Name</th>
    <th>Value</th>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #99c1f1"/></td>
    <td><tt>--blue-1</tt></td>
    <td><tt>#99c1f1</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #62a0ea"/></td>
    <td><tt>--blue-2</tt></td>
    <td><tt>#62a0ea</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #3584e4"/></td>
    <td><tt>--blue-3</tt></td>
    <td><tt>#3584e4</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #1c71d8"/></td>
    <td><tt>--blue-4</tt></td>
    <td><tt>#1c71d8</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #1a5fb4"/></td>
    <td><tt>--blue-5</tt></td>
    <td><tt>#1a5fb4</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #8ff0a4"/></td>
    <td><tt>--green-1</tt></td>
    <td><tt>#8ff0a4</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #57e389"/></td>
    <td><tt>--green-2</tt></td>
    <td><tt>#57e389</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #33d17a"/></td>
    <td><tt>--green-3</tt></td>
    <td><tt>#33d17a</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #2ec27e"/></td>
    <td><tt>--green-4</tt></td>
    <td><tt>#2ec27e</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #26a269"/></td>
    <td><tt>--green-5</tt></td>
    <td><tt>#26a269</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #f9f06b"/></td>
    <td><tt>--yellow-1</tt></td>
    <td><tt>#f9f06b</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #f8e45c"/></td>
    <td><tt>--yellow-2</tt></td>
    <td><tt>#f8e45c</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #f6d32d"/></td>
    <td><tt>--yellow-3</tt></td>
    <td><tt>#f6d32d</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #f5c211"/></td>
    <td><tt>--yellow-4</tt></td>
    <td><tt>#f5c211</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #e5a50a"/></td>
    <td><tt>--yellow-5</tt></td>
    <td><tt>#e5a50a</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #ffbe6f"/></td>
    <td><tt>--orange-1</tt></td>
    <td><tt>#ffbe6f</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #ffa348"/></td>
    <td><tt>--orange-2</tt></td>
    <td><tt>#ffa348</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #ff7800"/></td>
    <td><tt>--orange-3</tt></td>
    <td><tt>#ff7800</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #e66100"/></td>
    <td><tt>--orange-4</tt></td>
    <td><tt>#e66100</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #c64600"/></td>
    <td><tt>--orange-5</tt></td>
    <td><tt>#c64600</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #f66151"/></td>
    <td><tt>--red-1</tt></td>
    <td><tt>#f66151</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #ed333b"/></td>
    <td><tt>--red-2</tt></td>
    <td><tt>#ed333b</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #e01b24"/></td>
    <td><tt>--red-3</tt></td>
    <td><tt>#e01b24</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #c01c28"/></td>
    <td><tt>--red-4</tt></td>
    <td><tt>#c01c28</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #a51d2d"/></td>
    <td><tt>--red-5</tt></td>
    <td><tt>#a51d2d</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #dc8add"/></td>
    <td><tt>--purple-1</tt></td>
    <td><tt>#dc8add</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #c061cb"/></td>
    <td><tt>--purple-2</tt></td>
    <td><tt>#c061cb</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #9141ac"/></td>
    <td><tt>--purple-3</tt></td>
    <td><tt>#9141ac</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #813d9c"/></td>
    <td><tt>--purple-4</tt></td>
    <td><tt>#813d9c</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #613583"/></td>
    <td><tt>--purple-5</tt></td>
    <td><tt>#613583</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #cdab8f"/></td>
    <td><tt>--brown-1</tt></td>
    <td><tt>#cdab8f</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #b5835a"/></td>
    <td><tt>--brown-2</tt></td>
    <td><tt>#b5835a</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #986a44"/></td>
    <td><tt>--brown-3</tt></td>
    <td><tt>#986a44</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #865e3c"/></td>
    <td><tt>--brown-4</tt></td>
    <td><tt>#865e3c</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #63452c"/></td>
    <td><tt>--brown-5</tt></td>
    <td><tt>#63452c</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>--light-1</tt></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill light" style="background-color: #f6f5f4"/></td>
    <td><tt>--light-2</tt></td>
    <td><tt>#f6f5f4</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #deddda"/></td>
    <td><tt>--light-3</tt></td>
    <td><tt>#deddda</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #c0bfbc"/></td>
    <td><tt>--light-4</tt></td>
    <td><tt>#c0bfbc</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #9a9996"/></td>
    <td><tt>--light-5</tt></td>
    <td><tt>#9a9996</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #77767b"/></td>
    <td><tt>--dark-1</tt></td>
    <td><tt>#77767b</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #5e5c64"/></td>
    <td><tt>--dark-2</tt></td>
    <td><tt>#5e5c64</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill" style="background-color: #3d3846"/></td>
    <td><tt>--dark-3</tt></td>
    <td><tt>#3d3846</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill dark" style="background-color: #241f31"/></td>
    <td><tt>--dark-4</tt></td>
    <td><tt>#241f31</tt></td>
  </tr>
  <tr>
    <td><div class="color-pill dark" style="background-color: #000000"/></td>
    <td><tt>--dark-5</tt></td>
    <td><tt>#000000</tt></td>
  </tr>
</table>

# Compatibility Colors

A number of colors have been available in Adwaita in GTK3. They are aliases of
UI colors or otherwise derived from them. These colors use the older
GTK-specific syntax for named colors rather than CSS variables, and don't pick
up overridden colors. As such, it's recommended to avoid using these colors
entirely.

Name                                            | Value
----------------------------------------------- | -----------------------------
<tt>&#64;theme_bg_color</tt>                    | [<tt>&#64;window_bg_color</tt>](#window-colors)
<tt>&#64;theme_fg_color</tt>                    | [<tt>&#64;window_fg_color</tt>](#window-colors)
<tt>&#64;theme_base_color</tt>                  | [<tt>&#64;view_bg_color</tt>](#view-colors)
<tt>&#64;theme_text_color</tt>                  | [<tt>&#64;view_fg_color</tt>](#view-colors)
<tt>&#64;theme_selected_bg_color</tt>           | [<tt>&#64;accent_bg_color</tt>](#accent-colors)
<tt>&#64;theme_selected_fg_color</tt>           | [<tt>&#64;accent_fg_color</tt>](#accent-colors)
<tt>&#64;insensitive_bg_color</tt>              | <tt>color-mix(&#64;window_bg_color 60%, &#64;view_bg_color)</tt>
<tt>&#64;insensitive_fg_color</tt>              | <tt>color-mix(in srgb, &#64;window_fg_color 50%, transparent)</tt>
<tt>&#64;insensitive_base_color</tt>            | [<tt>&#64;view_bg_color</tt>](#view-colors)
<tt>&#64;borders</tt>                           | <tt>color-mix(in srgb, currentColor 15%, transparent)</tt>
<tt>&#64;theme_unfocused_bg_color</tt>          | [<tt>&#64;window_bg_color</tt>](#window-colors)
<tt>&#64;theme_unfocused_fg_color</tt>          | [<tt>&#64;window_fg_color</tt>](#window-colors)
<tt>&#64;theme_unfocused_base_color</tt>        | [<tt>&#64;view_bg_color</tt>](#view-colors)
<tt>&#64;theme_unfocused_text_color</tt>        | [<tt>&#64;view_fg_color</tt>](#view-colors)
<tt>&#64;theme_unfocused_selected_bg_color</tt> | [<tt>&#64;accent_bg_color</tt>](#accent-colors)
<tt>&#64;theme_unfocused_selected_fg_color</tt> | [<tt>&#64;accent_fg_color</tt>](#accent-colors)
<tt>&#64;unfocused_insensitive_color</tt>       | <tt>&#64;insensitive_bg_color</tt>
<tt>&#64;unfocused_borders</tt>                 | <tt>&#64;borders</tt>
