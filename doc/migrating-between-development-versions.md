Title: Migrating between development versions
Slug: migrating-between-development-versions

# Migrating Between Development Versions

This guide outlines the differences between Libadwaita development releases.
It assumes you've already migrated from Libhandy 1.4 to Libadwaita, or you
created a new project using a development release of Libadwaita.

If you want to migrate from Libhandy 1.4 to the latest Libadwaita release,
[follow this guide](migrating-libhandy-1-4-to-libadwaita.html).

## Migrating From alpha 1 to alpha 2

### Adapt to view switcher API Changes

[class@ViewSwitcher], [class@ViewSwitcherBar] and [class@ViewSwitcherTitle] now
use [class@ViewStack] instead of [class@Gtk.Stack].

You should stop using [property@Gtk.Stack:transition-type] and
[property@Gtk.Stack:transition-duration] properties before switching to
[class@ViewStack].

### Adapt to Stylesheet Changes

If you were using
[<code>&#64;theme_selected_bg_color</code>](named-colors.html#compatibility-colors)
as a text color, use
code>&#64;accent_color</code>](named-colors.html#accent-colors) instead to make
sure the text is readable. You can also use the
[`.accent`](style-classes.html#colors) style class to apply the correct color.

## Migrating From alpha 2 to alpha 3

### Stop Using `AdwValueObject` With Non-string Values

`AdwValueObject` has been removed. The typical use for storing strings in
combination with [class@Gio.ListStore] can be replaced by using
[class@Gtk.StringList], others cases can be replaced by creating your own
objects to store those values .

### Adapt to `AdwEnumValueObject` API Changes

`AdwEnumValueObject` has been renamed to [class@EnumListItem].

### Adapt to Window API Changes

The `child` property in [class@Window] and [class@ApplicationWindow] has
been renamed to `content`.

### Adapt to [class@Leaflet] API Changes

The `hhomogeneous-folded`, `vhomogeneous-folded`, `hhomogeneous-unfolded`, and
`vhomogeneous-unfolded` properties have been replaced by a single
[property@Leaflet:homogeneous] property, set to `TRUE` by default, applied when
the leaflet is folded for the opposite orientation.

When unfolded, children are never homogeneous. Use [class@Gtk.SizeGroup] to make
them homogeneous if needed.

The `interpolate-size` property has been removed with no replacement, it's
always enabled when [property@Leaflet:homogeneous] is set to `FALSE`.

### Adapt to View Switcher API Changes

The `auto` view switcher policy has been removed. [class@ViewSwitcher] only has
narrow and wide policies; if you had used the `auto` policy, use an
[class@Squeezer] with two view switchers inside.

#### Adapt to [class@ViewSwitcher] API Changes

The "narrow-ellipsize" property has been removed. Narrow view switchers always
ellipsize their labels, wide switchers never do.

#### Adapt to [class@ViewSwitcherBar] API Changes

The "policy" property has been removed. If you had used it, use a plain
[class@ViewSwitcher] in a [class@Gtk.ActionBar] instead.

#### Adapt to [class@ViewSwitcherTitle] API Changes

The "policy" property has been removed, the behavior is similar to the removed
`auto` policy. If you had used `wide` or `narrow` policies, use an
[class@Squeezer] with an [class@ViewSwitcher] and an [class@WindowTitle] inside,
with the switcher having the desired policy.

### Adapt to [class@Avatar] API Changes

The `adw_avatar_draw_to_pixbuf()` function have been removed, use the newly
added [method@Avatar.draw_to_texture] instead. [class@Gdk.Texture] implements
[iface@Gio.Icon], so it should just work for that case.

[method@Avatar.draw_to_texture] does not have the `size` parameter. Instead, it
uses the avatar's current size, with no replacement.

### Use [class@StyleManager] Instead of [property@Gtk.Settings:gtk-application-prefer-dark-theme]

Using [property@Gtk.Settings:gtk-application-prefer-dark-theme] to control dark
appearance is not supported anymore, set [property@StyleManager:color-scheme] to
`ADW_COLOR_SCHEME_PREFER_DARK` and make sure the application can work with light
appearance as well. If that's not possible, set it to or
`ADW_COLOR_SCHEME_FORCE_DARK` instead.

If your application is using light appearance, make sure it works wit dark
appearance as well, or set the color scheme to `ADW_COLOR_SCHEME_FORCE_LIGHT`
otherwise.

### Adapt to Stylesheet Changes

#### Adapt to Header Bar, Action Bar and Toolbar Style Changes

When possible, buttons in [class@Gtk.HeaderBar] and [class@Gtk.ActionBar] will
use flat appearance by default.

The following rules are used when deciding when to make buttons flat or not:

The following buttons get flat appearance:

* Icon-only buttons;
* Buttons with an icon and a label (using [class@ButtonContent]);
* Menu buttons containing an arrow;
* [class@SplitButton];
* Any other button with the [`.flat`](style-classes.html#flat) style class.

The following buttons keep default appearance:

* Text-only buttons;
* Buttons with other content;
* Buttons within widgets containing the
  [`.linked`](style-classes.html#linked-controls) class;
* Buttons with the [`.suggested-action`](style-classes.html#suggested-action),
  [`.destructive-action`](style-classes.html#destructive-action) or
  [`.opaque`](style-classes.html#opaque) style classes.
* Buttons with the [`.raised`](style-classes.html#raised) style class.

It's important to avoid ambiguous layouts, for example text-only buttons with
no icon, since such a button would be indistinguishable from the window title
without hovering it.

In rare cases, the existing layout may need a redesign to work with the new
style.

The same rules are also used for the [`.toolbar`](style-classes.html#toolbars)
style class now, instead of making every button appear flat.

#### Adjusting Icon+Arrow Menu Buttons

If you had menu buttons containing an icon and a dropdown arrow, switch to
[property@Gtk.MenuButton:icon-name] and set the
[property@Gtk.MenuButton:always-show-arrow] property to `TRUE`.

#### Adjusting Text-only Buttons

If you had text-only buttons, consider using [class@ButtonContent]. For example,
the following button:

```xml
<object class="GtkButton">
  <property name="label" translatable="yes">_Open</property>
  <property name="use-underline">True</property>
</object>
```

can be changed into:

```xml
<object class="GtkButton">
  <property name="child">
    <object class="AdwButtonContent">
      <property name="icon-name">document-open-symbolic</property>
      <property name="label" translatable="yes">_Open</property>
      <property name="use-underline">True</property>
    </object>
  </property>
</object>
```

One exception are the two primary buttons in a dialog, for example, "Cancel" and
"Open". Those buttons should retain their default appearance.

#### Adjusting Split Buttons

If you had split buttons implemented via a `GtkBox` with the
[`.linked`](style-classes.html#linked-controls) style class and two buttons
packed inside, use [class@SplitButton] as follows:

```xml
<object class="AdwSplitButton">
  <property name="menu-model">some_menu</property>
  <property name="icon-name">view-list-symbolic</property>
</object>
```

#### Adjusting Linked Buttons

For other linked together buttons, simply stop linking them.

If multiple linked groups were used to separate different groups of actions,
insert extra spacing as follows:

```xml
<object class="GtkSeparator">
  <style>
    <class name="spacer"/>
  </style>
</object>
```

#### Custom Adjustments

The [`.flat`](style-classes.html#flat) and
[`.raised`](style-classes.html#raised) style classes can always be used to
override the default appearance.

Important: the [property@Gtk.Button:has-frame] property will **not** be set to
`FALSE` when a button gets the flat appearance automatically. It also cannot be
set to `TRUE` to make a button raised, the style class should be used directly
instead.

## Migrating From alpha 3 to alpha 4

### Adapt to [class@ActionRow] and [class@ExpanderRow] API Changes

The "use-underline" property and its accessors have been removed. Use
[property@PreferencesRow:use-underline] and its accessors instead.

The title and subtitle have markup enabled, make sure to escape it with
[func@GLib.markup_escape_text] if this is unwanted.

### Adapt to [class@ExpanderRow] API Changes

The `adw_expander_row_add()` function has been renamed to
[method@ExpanderRow.add_row].

### Adjusting Header Bar Icons

If you're using the `object-select-symbolic` icon in a header bar button
(typically for selection mode), use `selection-mode-symbolic` instead.

### Adapt to Stylesheet Changes

### Stop Using the `.sidebar` Style Class

The [`.sidebar`](style-classes.html#sidebar) style class is now deprecated,
although still works for compatibility reasons. The main use case - adjusting
the background color of [class@Gtk.ListBox] and [class@Gtk.ListView] - can now
be done with the [`.navigation-sidebar`](style-classes.html#sidebars) style
class on those widgets instead, along with adjusting the item selection style.
The border can be replicated by manually adding a [class@Gtk.Separator].

#### Adapt to the `popover.combo` Style Removal

The `.combo` popover style class has been removed. Use
[`.menu`](style-classes.html#menu-popovers) instead. You may need to remove
manually added margins, padding or minimum height from the list items inside
while doing it.

#### Adapt to List Style Changes

For boxed lists we now have the
[`.boxed-list`](style-classes.html#boxed-lists-cards) style class that matches
the name of the design pattern. If you were using the
[`.content`](style-classes.html#content) style class, you should use
`.boxed-list` instead.

The `.content` style class currently remains for compatibility purposes.

Neither the `.content` style class nor the `.boxed-list` style class work
for [class@Gtk.ListView], as the widget cannot currently be used for the
boxed list pattern.

## Migrating From alpha 4 to beta 1

#### Adapt to [class@SwipeTracker] API Changes

The [signal@SwipeTracker::begin-swipe] signal is now emitted immediately before
the swipe starts, after the drag threshold has been reached, and it has lost its
`direction` parameter. The new [signal@SwipeTracker::prepare] signal behaves
exactly like `begin-swipe` did, and can be used instead of it.

The type of the `duration` parameter in [signal@SwipeTracker::end-swipe] has
changed from `gint64` to `guint`.

### Adapt to [class@TabView] API Changes

The `HdyTabVoew:shortcut-widget` property has been removed with no replacement;
[class@TabView] automatically installs shortcuts with the
`GTK_SHORTCUT_SCOPE_MANAGED` scope, so they are automatically available
throughout the window without the need to set shortcut widget.

If some of these shortcuts conflict with another widget, the latter has
priority, and it should work automatically if the widget correctly stops event
propagation.

### Adapt to [class@Leaflet] API Changes

The `can-swipe-back` and `can-swipe-forward` properties have been renamed to
[property@Leaflet:can-navigate-back] and
[property@Leaflet:can-navigate-forward], along with their accessors. The new
properties also handle keyboard and mouse shortcuts in addition to swipes.

`AdwLeaflet` now uses spring animations instead of timed animations for child
transitions. As such, the `child-transition-duration` property has been replaced
with [property@Leaflet:child-transition-params], allowing to customize the
animation. Unlike the duration, spring parameters are also used for animation
triggered by swipe gestures.

### Adapt to [class@Flap] API Changes

`AdwFlap` now uses spring animations instead of timed animations for reveal
animations. As such, the `reveal-duration` property has been replaced with
[property@Flap:reveal-params], allowing to customize the animation. Unlike the
duration, spring parameters are also used for transitions triggered by swipe
gestures.

### Adapt to [class@Carousel] API Changes

`AdwCarousel` now uses spring animations instead of timed animations for
scrolling. As such, the `animation-duration` property has been replaced with
[property@Carousel:scroll-params], allowing to customize the animation. Unlike
the duration, spring parameters are also used for animation triggered by swipe
gestures.

The `adw_carousel_scroll_to_full()` method has been removed. Instead,
[method@Carousel.scroll_to] has got an additional parameter `animate`.

### Adapt to [class@PreferencesWindow] API Changes

The `can-swipe-back` property have been renamed to
[property@PreferencesWindow:can-navigate-back], along with its accessors. The
new properties also handle keyboard and mouse shortcuts in addition to swipes.

### Adapt to [class@ViewStack] API Changes

[class@ViewStack] has stopped supporting transitions. As such, the
`interpolate-size` and `transition-running` properties have been removed with
no replacement.

### Adapt to Miscellaneous Changes

The `adw_ease_out_cubic()` function has been removed. Instead,
[func@Easing.ease] can be used with the `ADW_EASE_OUT_CUBIC` parameter.

### Adapt to Stylesheet Changes

### Adapt to the `button.outline` Style Removal

The `.outline` style class has been removed with no replacement. The regular
button style should be used instead.

### Adapt to the `content-view` Check Button Style Removal

The selection mode [class@Gtk.CheckButton] style, used inside views with the
`.content-view` has been changed into a separate style class `.selection-mode`
that can be applied directly onto check buttons.
