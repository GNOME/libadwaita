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

Some colors also have standalone versions. They are similar to the background
version, but provide better contrast when used as foreground colors on top of a
neutral background - for example, colorful text in a window.

```css
my-widget {
  color: var(--accent-color);
}
```

Standalone colors are typically darker than the corresponding background color
for the light style, and lighter than the background for the dark style.

Applications can override any of these colors by re-declaring them, for example:

```css
:root {
  --accent-color: #c01c28;
  --accent-bg-color: #e01b24;
}
```

## Accent Colors

The accent color is used across many different widgets, often to indicate that a
widget is important, interactive, or currently active. Try to avoid using it on
large surfaces, or on too many items on the same view.

The [`.accent`](style-classes.html#colors) style class allows to use it for
widgets such as [class@Gtk.Label].

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--accent-color</tt></td>
    <td><div class="color-pill" style="background-color: #1c71d8"/></td>
    <td><tt>#1c71d8</tt></td>
    <td><div class="color-pill" style="background-color: #78aeed"/></td>
    <td><tt>#78aeed</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-bg-color</tt></td>
    <td><div class="color-pill" style="background-color: #3584e4"/></td>
    <td><tt>#3584e4</tt></td>
    <td><div class="color-pill" style="background-color: #3584e4"/></td>
    <td><tt>#3584e4</tt></td>
  </tr>
  <tr>
    <td><tt>--accent-fg-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
</table>

## Destructive Colors

The destructive color indicates a dangerous action, such as deleting a file.
It's used by [class@Gtk.Button] and [class@ButtonRow] with the
[`.destructive-action`](style-classes.html#destructive-action) style class.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--destructive-color</tt></td>
    <td><div class="color-pill" style="background-color: #c01c28"/></td>
    <td><tt>#c01c28</tt></td>
    <td><div class="color-pill" style="background-color: #ff7b63"/></td>
    <td><tt>#ff7b63</tt></td>
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
</table>

## Success Colors

This color is used with the [`.success`](style-classes.html#colors) style class,
or in a [class@Gtk.LevelBar] with the [const@Gtk.LEVEL_BAR_OFFSET_HIGH] offset.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--success-color</tt></td>
    <td><div class="color-pill" style="background-color: #1b8553"/></td>
    <td><tt>#1b8553</tt></td>
    <td><div class="color-pill" style="background-color: #8ff0a4"/></td>
    <td><tt>#8ff0a4</tt></td>
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
</table>

## Warning Colors

This color is used with the [`.warning`](style-classes.html#colors) style class,
or in a [class@Gtk.LevelBar] with the [const@Gtk.LEVEL_BAR_OFFSET_LOW] offset.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--warning-color</tt></td>
    <td><div class="color-pill" style="background-color: #9c6e03"/></td>
    <td><tt>#9c6e03</tt></td>
    <td><div class="color-pill" style="background-color: #f8e45c"/></td>
    <td><tt>#f8e45c</tt></td>
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
</table>

## Error Colors

This color is used with the [`.error`](style-classes.html#colors) style class.

<table>
  <tr>
    <th>Name</th>
    <th/>
    <th>Light</th>
    <th/>
    <th>Dark</th>
  </tr>
  <tr>
    <td><tt>--error-color</tt></td>
    <td><div class="color-pill" style="background-color: #c01c28"/></td>
    <td><tt>#c01c28</tt></td>
    <td><div class="color-pill" style="background-color: #ff7b63"/></td>
    <td><tt>#ff7b63</tt></td>
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
    <td><div class="color-pill light" style="background-color: #fafafa"/></td>
    <td><tt>#fafafa</tt></td>
    <td><div class="color-pill dark" style="background-color: #242424"/></td>
    <td><tt>#242424</tt></td>
  </tr>
  <tr>
    <td><tt>--window-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
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
    <td><div class="color-pill dark" style="background-color: #1e1e1e"/></td>
    <td><tt>#1e1e1e</tt></td>
  </tr>
  <tr>
    <td><tt>--view-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
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
    <td><div class="color-pill dark" style="background-color: #303030"/></td>
    <td><tt>#303030</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-border-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-backdrop-color</tt></td>
    <td><div class="color-pill light" style="background-color: #fafafa"/></td>
    <td><tt>#fafafa</tt></td>
    <td><div class="color-pill dark" style="background-color: #242424"/></td>
    <td><tt>#242424</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 12%)"/></td>
    <td><tt>rgb(0 0 0 / 12%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 36%)"/></td>
    <td><tt>rgb(0 0 0 / 36%)</tt></td>
  </tr>
  <tr>
    <td><tt>--headerbar-darker-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 12%)"/></td>
    <td><tt>rgb(0 0 0 / 12%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 90%)"/></td>
    <td><tt>rgb(0 0 0 / 90%)</tt></td>
  </tr>
</table>

<code>--headerbar-border-color</code> has the same default value as
<code>--headerbar-fg-color</code>, but doesn't change along with it. This can
be useful if a light window has a dark header bar with light text; in this case
it may be desirable to keep the border dark. This variable is only used for
vertical borders - for example, separators between the 2 header bars in a split
header bar layout.

<code>--headerbar-backdrop-color</code> is used instead of
<code>--headerbar-bg-color</code> when the window is not focused. By default
it's an alias of [<code>--window-bg-color</code>](#window-colors) and changes
together with it. When overriding header bar colors, make sure to set it to a
value matching your <code>--headerbar-bg-color</code>.

<code>--headerbar-shade-color</code> is used to provide a dark shadow or a
border for header bars and similar widgets. This color should always be
partially transparent black.

<code>--headerbar-darker-shade-color</code> is used for the
`ADW_TOOLBAR_RAISED_BORDER` border. This color should always be
partially transparent black, and is intended to be darker than both
<code>--headerbar-bg-color</code> and
<code>--headerbar-backdrop-color</code> on top of white color.

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
    <td><div class="color-pill" style="background-color: #ebebeb"/></td>
    <td><tt>#ebebeb</tt></td>
    <td><div class="color-pill dark" style="background-color: #303030"/></td>
    <td><tt>#303030</tt></td>
  </tr>
  <tr>
    <td><tt>--sidebar-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--sidebar-backdrop-color</tt></td>
    <td><div class="color-pill" style="background-color: #f2f2f2"/></td>
    <td><tt>#f2f2f2</tt></td>
    <td><div class="color-pill dark" style="background-color: #2a2a2a"/></td>
    <td><tt>#2a2a2a</tt></td>
  </tr>
  <tr>
    <td><tt>--sidebar-border-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 7%)"/></td>
    <td><tt>rgb(0 0 0 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 36%)"/></td>
    <td><tt>rgb(0 0 0 / 36%)</tt></td>
  </tr>
  <tr>
    <td><tt>--sidebar-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 7%)"/></td>
    <td><tt>rgb(0 0 0 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 25%)"/></td>
    <td><tt>rgb(0 0 0 / 25%)</tt></td>
  </tr>
</table>

<code>--sidebar-backdrop-color</code> is used instead of
<code>--sidebar-bg-color</code> when the window is not focused. When
overriding sidebar colors, make sure to set it to a value matching your
<code>--sidebar-bg-color</code>.

<code>--sidebar-shade-color</code> is used to provide a dark border for
sidebars, scroll undershoots within sidebars, as well as transitions in
[class@NavigationView], [class@OverlaySplitView], [class@Leaflet] and
[class@Flap]. This color should always be partially transparent black, with the
opacity tuned to be well visible on top of <code>--sidebar-bg-color</code>.

<code>--sidebar-border-color</code> is used to provide a dark border for
sidebars. This color should always be partially transparent black, with the
opacity tuned to be well visible on top of <code>--sidebar-bg-color</code>
next to <code>--window-bg-color</code> or <code>--view-bg-color</code>.

<code>--sidebar-shade-color</code> is used to provide scroll undershoots
within sidebars, as well as transitions in [class@NavigationView],
[class@OverlaySplitView], [class@Leaflet] and [class@Flap]. This color should
always be partially transparent black, with the opacity tuned to be well visible
on top of <code>--sidebar-bg-color</code>.

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
    <td><div class="color-pill" style="background-color: #f3f3f3"/></td>
    <td><tt>#f3f3f3</tt></td>
    <td><div class="color-pill dark" style="background-color: #2a2a2a"/></td>
    <td><tt>#2a2a2a</tt></td>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-backdrop-color</tt></td>
    <td><div class="color-pill" style="background-color: #f6f6f6"/></td>
    <td><tt>#f6f6f6</tt></td>
    <td><div class="color-pill dark" style="background-color: #272727"/></td>
    <td><tt>#272727</tt></td>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-border-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 7%)"/></td>
    <td><tt>rgb(0 0 0 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 36%)"/></td>
    <td><tt>rgb(0 0 0 / 36%)</tt></td>
  </tr>
  <tr>
    <td><tt>--secondary-sidebar-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 7%)"/></td>
    <td><tt>rgb(0 0 0 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 25%)"/></td>
    <td><tt>rgb(0 0 0 / 25%)</tt></td>
  </tr>
</table>

<code>--secondary-sidebar-backdrop-color</code> is used instead of
<code>--secondary-sidebar-bg-color</code> when the window is not focused.
When overriding secondary sidebar colors, make sure to set it to a value
matching your <code>--secondary-sidebar-bg-color</code>.

<code>--secondary-sidebar-border-color</code> is used to provide a dark
border for secondary sidebars. This color should always be partially transparent black, with
the opacity tuned to be well visible on top of
<code>--secondary-sidebar-bg-color</code> next to
<code>--sidebar-bg-color</code>.

<code>--secondary-sidebar-shade-color</code> is used to provide scroll
undershoots within secondary sidebars, as well as transitions in
[class@NavigationView], [class@OverlaySplitView], [class@Leaflet] and
[class@Flap]. This color should always be partially transparent black, with the
opacity tuned to be well visible on top of
<code>--secondary-sidebar-bg-color</code>.

<code>--secondary-sidebar-shade-color</code> is used to provide a dark border
for secondary sidebars, scroll undershoots within secondary sidebars, as well as
transitions in [class@NavigationView], [class@OverlaySplitView], [class@Leaflet]
and [class@Flap]. This color should always be partially transparent black, with
the opacity tuned to be well visible on top of
<code>--secondary-sidebar-bg-color</code>.

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
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--card-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 7%)"/></td>
    <td><tt>rgb(0 0 0 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 36%)"/></td>
    <td><tt>rgb(0 0 0 / 36%)</tt></td>
  </tr>
</table>

<code>--card-shade-color</code> is used to provide separators between
boxed list rows and similar widgets. This color should always be partially
transparent black, with the opacity tuned to be well visible on top of
<code>--card-bg-color</code>.

## Thumbnail Colors

These colors are used for [class@TabOverview] thumbnails.

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
    <td><div class="color-pill" style="background-color: #383838"/></td>
    <td><tt>#383838</tt></td>
  </tr>
  <tr>
    <td><tt>--thumbnail-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
</table>

Since: 1.3

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
    <td><div class="color-pill light" style="background-color: #fafafa"/></td>
    <td><tt>#fafafa</tt></td>
    <td><div class="color-pill" style="background-color: #383838"/></td>
    <td><tt>#383838</tt></td>
  </tr>
  <tr>
    <td><tt>--dialog-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
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
    <td><div class="color-pill" style="background-color: #383838"/></td>
    <td><tt>#383838</tt></td>
  </tr>
  <tr>
    <td><tt>--popover-fg-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 80%)"/></td>
    <td><tt>rgb(0 0 0 / 80%)</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
  </tr>
  <tr>
    <td><tt>--popover-shade-color</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 7%)"/></td>
    <td><tt>rgb(0 0 0 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 25%)"/></td>
    <td><tt>rgb(0 0 0 / 25%)</tt></td>
  </tr>
</table>

<code>--popover-shade-color</code> is used for scroll undershoot styles
within popovers, as well as transitions in [class@NavigationView],
[class@OverlaySplitView], [class@Leaflet] and [class@Flap]. This color should
always be partially transparent black, with the opacity tuned to be well visible
on top of <code>--popover-bg-color</code>.
This color is only available since 1.4.

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
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 7%)"/></td>
    <td><tt>rgb(0 0 0 / 7%)</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 25%)"/></td>
    <td><tt>rgb(0 0 0 / 25%)</tt></td>
  </tr>
  <tr>
    <td><tt>--scrollbar-outline-color</tt></td>
    <td><div class="color-pill light" style="background-color: #ffffff"/></td>
    <td><tt>#ffffff</tt></td>
    <td><div class="color-pill dark" style="background-color: rgb(0 0 0 / 50%)"/></td>
    <td><tt>rgb(0 0 0 / 50%)</tt></td>
  </tr>
</table>

<code>--shade-color</code> is used for scroll undershoots, as well as
transitions in [class@NavigationView], [class@OverlaySplitView], [class@Leaflet]
and [class@Flap]. This color should always be partially transparent black, with
the opacity tuned to be well visible on top of <code>--window-bg-color</code>.

<code>--scrollbar-outline-color</code> is used by [class@Gtk.Scrollbar] to
ensure that overlay scrollbars are visible regardless of the content color. It
should always be the opposite of the scrollbar color - light with a dark
scrollbar and dark with a light scrollbar.

# Helpers

These variables are provided for convenience and should not be overridden.

## Border Color

Name                    | Regular                            | High contrast
----------------------- | ---------------------------------- | ---------------------------------
<tt>--border-color</tt> | <tt>alpha(currentColor, 0.15)</tt> | <tt>alpha(currentColor, 0.5)</tt>

Border color is derived from the current foreground color (`currentColor`) and
changes between regular and high contrast modes. It should be used to support
the high contrast mode automatically.

## Window Radius

Name                     | Value
------------------------ | -----
<tt>--window-radius</tt> | 12px

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

A number of colors has been available in Adwaita in GTK3. They are aliases of
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
<tt>&#64;insensitive_bg_color</tt>              | <tt>mix(&#64;window_bg_color, &#64;view_bg_color, 0.4)</tt>
<tt>&#64;insensitive_fg_color</tt>              | <tt>alpha(&#64;window_fg_color, 0.5)</tt>
<tt>&#64;insensitive_base_color</tt>            | [<tt>&#64;view_bg_color</tt>](#view-colors)
<tt>&#64;theme_unfocused_bg_color</tt>          | [<tt>&#64;window_bg_color</tt>](#window-colors)
<tt>&#64;theme_unfocused_fg_color</tt>          | [<tt>&#64;window_fg_color</tt>](#window-colors)
<tt>&#64;theme_unfocused_base_color</tt>        | [<tt>&#64;view_bg_color</tt>](#view-colors)
<tt>&#64;theme_unfocused_text_color</tt>        | [<tt>&#64;view_fg_color</tt>](#view-colors)
<tt>&#64;theme_unfocused_selected_bg_color</tt> | [<tt>&#64;accent_bg_color</tt>](#accent-colors)
<tt>&#64;theme_unfocused_selected_fg_color</tt> | [<tt>&#64;accent_fg_color</tt>](#accent-colors)
<tt>&#64;unfocused_insensitive_color</tt>       | <tt>&#64;insensitive_bg_color</tt>
<tt>&#64;borders</tt>                           | <tt>alpha(currentColor, 0.15)</tt>
<tt>&#64;unfocused_borders</tt>                 | <tt>&#64;borders</tt>
