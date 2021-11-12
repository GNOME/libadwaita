Title: Migrating from Libhandy 1.4 to Libadwaita
Slug: migrating-libhandy-1-4-to-libadwaita

# Migrating from Libhandy 1.4 to Libadwaita

Libadwaita is being developed as a successor to Libhandy 1.4. As such, it
offers to GTK 4 many features Libhandy was offering to GTK 3.

Migrating from Libhandy 1.4 to Libadwaita implies migrating from GTK 3 to 4.
This guide only focuses on on Libhandy and Libadwaita, and is designed to be
used together with the [GTK 3 to 4 migration guide](https://docs.gtk.org/gtk4/migrating-3to4.html).

If you notice that some differences between Libhandy and Libadwaita are missing
in this guide, [please report them](https://gitlab.gnome.org/GNOME/libadwaita/-/issues/new).

## Preparation in Libhandy 1.4

The steps outlined in the following sections assume that your software is
working with Libhandy 1.4, which is the latest stable release of Libhandy 1.x.
It includes all the necessary APIs and tools to help you port your software to
Libadwaita. If you are using an older version of Libhandy, you should first get
your software to build and work with Libhandy 1.4.

### Do not Use Deprecated Symbols

Over the years, a number of functions, and in some cases, entire widgets have
been deprecated. These deprecations are clearly spelled out in the API
reference, with hints about the recommended replacements. The API reference for
Libhandy 1.4 also includes an [index](https://gnome.pages.gitlab.gnome.org/libhandy/doc/1-latest/deprecated-api-index.html)
of all deprecated symbols.

### Subclassing

Following GTK4's emphasis on composition and delegation over subclassing,
[class@Adw.Leaflet] and [class@Adw.HeaderBar] are no longer derivable. As a
replacement, you can subclass `GtkBin` or `GtkBox` and include a leaflet or a
header bar as a child widget.

### Stop Using `HdyKeypad`

`HdyKeypad` has been removed from Libadwaita. Applications that had used it can
copy it in tree instead.

### Stop Using Named WM Colors

The following named colors have been removed from the stylesheet in
Libadwaita:

* `@wm_title`
* `@wm_unfocused_title`
* `@wm_highlight`
* `@wm_borders_edge`
* `@wm_bg_a`
* `@wm_bg_b`
* `@wm_shadow`
* `@wm_border`
* `@wm_button_hover_color_a`
* `@wm_button_hover_color_b`
* `@wm_button_active_color_a`
* `@wm_button_active_color_b`
* `@wm_button_active_color_c`

Applications should not use them.

### Use `HdyFlap` Properties for Adding Children Instead of `gtk_container_add()`

`HdyFlap` provides the "content", "flap" and "separator" properties that can be
used for managing children instead of `GtkContainer` API. In Libadwaita
[property@Adw.Flap:content], [property@Adw.Flap:flap] and
[property@Adw.Flap:separator] are the only way to manage [class@Adw.Flap]
children.

### Stop Using `HdyValueObject` with non-string values

`HdyValueObject` has been removed. While it's not practical to replace the cases
where it's storing strings in GTK3, as the preferred replacement only exists in
4, it can also be used with any other [struct@GObject.Value]. That use has no
replacement and you can instead create your own objects to store those values.

### Use `HdyStyleManager` instead of [property@Gtk.Settings:gtk-application-prefer-dark-theme]

If your application is setting [property@Gtk.Settings:gtk-application-prefer-dark-theme]
to `TRUE` to request dark appearance, consider setting `HdyStyleManager:color-scheme`
to `HDY_COLOR_SCHEME_PREFER_DARK` and making sure the application can work with
light appearance as well. If that's not possible, set it to 
`HDY_COLOR_SCHEME_FORCE_DARK` instead.

If your application is using light appearance, consider setting the color scheme
to `HDY_COLOR_SCHEME_PREFER_LIGHT` and support dark appearance.

In libadwaita color schemes will be the only way to request dark appearance.

## Changes that Need to Be Done at the Time of the Switch

This section outlines porting tasks that you need to tackle when you get to the
point that you actually build your application against Libadwaita 1. Making it
possible to prepare for these in GTK 3 would have been either impossible or
impractical.

### Adapt to `GtkContainer` Removal

Same as GTK itself, all widgets that have children have a new API to replace
`gtk_container_add()` and `gtk_container_remove()`.

The following widgets that formerly subclassed `GtkBin` have a "child" property
now:

* [class@Adw.Clamp]
* [class@Adw.StatusPage]

[class@Adw.Window] and [class@Adw.ApplicationWindow] have a "content" property
instead.

For other widgets use the following replacements:

Widget                        | `gtk_container_add()` replacement  | `gtk_container_remove()` replacement
----------------------------- | ---------------------------------- | ------------------------------------
[class@Adw.ActionRow]         | [method@Adw.ActionRow.add_suffix]  | [method@Adw.ActionRow.remove]
[class@Adw.Carousel]          | [method@Adw.Carousel.append]       | [method@Adw.Carousel.remove]
[class@Adw.ExpanderRow]       | [method@Adw.ExpanderRow.add_row]   | [method@Adw.ExpanderRow.remove]
[class@Adw.Leaflet]           | [method@Adw.Leaflet.append]        | [method@Adw.Leaflet.remove]
[class@Adw.PreferencesGroup]  | [method@Adw.PreferencesGroup.add]  | [method@Adw.PreferencesGroup.remove]
[class@Adw.PreferencesPage]   | [method@Adw.PreferencesPage.add]   | [method@Adw.PreferencesPage.remove]
[class@Adw.PreferencesWindow] | [method@Adw.PreferencesWindow.add] | [method@Adw.PreferencesWindow.remove]

Adding children in a UI file still works.

### Adapt to `HdySearchBar` Removal

`HdySearchBar` has been removed, use [class@Gtk.SearchBar] instead.

### Adapt to `HdyWindowHandle` Removal

`HdyWindowHandle` has been removed, use [class@Gtk.WindowHandle] instead.

### Adapt to [class@Adw.ActionRow] and [class@Adw.ExpanderRow] API Changes

The "use-underline" property and its accessors have been removed. Use
[property@Adw.PreferencesRow:use-underline] and its accessors instead.

The title and subtitle have markup enabled, make sure to escape it with
[func@GLib.markup_escape_text] if this is unwanted.

### Adapt to [class@Adw.Clamp] API Changes

`HdyClamp` previously had `.small`, `.medium` or `.large` style classes
depending on the current size of its child. These style classes are now
added to the child instead of the clamp itself.

### Adapt to [class@Adw.ComboRow] API Changes

[class@Adw.ComboRow] API has been completely overhauled compared to
`HdyComboRow` and closely mirrors [class@Gtk.DropDown].
Refer to [class@Gtk.DropDown]'s documentation for details.

`hdy_combo_row_bind_name_model()` can be replaced with using the
[property@Adw.ComboRow:model] property in conjunction with
[property@Adw.ComboRow:expression].

`hdy_combo_row_bind_model()` can be replaced with using the
[property@Adw.ComboRow:model] property in conjunction with
[property@Adw.ComboRow:factory] and/or [property@Adw.ComboRow:list-factory].

`hdy_combo_row_set_for_enum()` can be replaced with an [class@Adw.EnumListModel]
in conjunction with the [property@Adw.ComboRow:expression] property, for
example:

```c
expr = gtk_property_expression_new (ADW_TYPE_ENUM_LIST_ITEM, NULL, "nick");
model = G_LIST_MODEL (adw_enum_list_model_new (GTK_TYPE_ORIENTATION));

adw_combo_row_set_expression (row, expr);
adw_combo_row_set_model (row, model);
```

As with [class@Gtk.DropDown], if the model is a [class@Gtk.StringList], the
model items can be converted into human-readable strings automatically without
requiring an expression.

The `HdyComboRow:selected-index` property has been renamed to
[property@Adw.ComboRow:selected], matching [class@Gtk.DropDown].

### Adapt to [class@Adw.PreferencesGroup] API Changes

`HdyPreferencesGroup:use-markup` has been removed, the labels always use markup
now.

### Adapt to `HdyEnumValueObject` API Changes

`HdyEnumValueObject` has been renamed to [class@Adw.EnumListItem] and can no
longer be manually created. It's only intended to be used with
[class@Adw.EnumListModel].

### Stop Using `HdyValueObject`

`HdyValueObject` has been removed. The typical use for storing strings in
combination with [class@Gio.ListStore] can be replaced by using
[class@Gtk.StringList].

### Adapt to [class@Adw.HeaderBar] API Changes

[class@Adw.HeaderBar] API mostly mirrors [class@Gtk.HeaderBar], refer to the
[GTK 3 to 4 migration guide](https://docs.gtk.org/gtk4/migrating-3to4.html#adapt-to-gtkheaderbar-and-gtkactionbar-api-changes)
for details

The [property@Gtk.HeaderBar:show-title-buttons] property has been split into
[property@Adw.HeaderBar:show-start-title-buttons] and
[property@Adw.HeaderBar:show-end-title-buttons] to simplify creating multi-pane
layouts. The corresponding getter and the setter have been split as well.

The [class@Adw.WindowTitle] widget may be useful for replacing the title and
subtitle.

### Adapt to `HdyHeaderGroup` Removal

`HdyHeaderGroup` has been removed. Its behavior can be replicated by changing
the [property@Adw.HeaderBar:show-start-title-buttons] and
[property@Adw.HeaderBar:show-end-title-buttons] properties depending on the
layout, for example binding them to the [property@Adw.Leaflet:folded] property
as follows:

```xml
<object class="AdwLeaflet" id="leaflet">
  <child>
    <object class="GtkBox">
      <property name="orientation">vertical</property>
      <object class="AdwHeaderBar">
        <binding name="show-end-title-buttons">
          <lookup name="folded">leaflet</lookup>
        </binding>
      </object>
      ...
    </object>
  </child>
  ...
  <child>
    <object class="GtkBox">
       <property name="orientation">vertical</property>
      <object class="AdwHeaderBar">
        <binding name="show-start-title-buttons">
          <lookup name="folded">leaflet</lookup>
        </binding>
      </object>
      ...
    </object>
  </child>
</object>
```

### Adapt to `HdyDeck` Removal

`HdyDeck` has been removed. Instead, an [class@Adw.Leaflet] can be used the same
way by setting the [property@Adw.Leaflet:can-unfold] property to `FALSE`.

### Adapt to [class@Adw.Leaflet] and [class@Adw.Squeezer] API Changes

The child properties of `HdyLeaflet` and `HdySqueezer` have been converted into
page objects, similarly to [class@Gtk.Stack]. For example,
[method@Adw.SqueezerPage.set_enabled] should be used to replace
`hdy_squeezer_set_child_enabled()`.

The `can-swipe-back` and `can-swipe-forward` properties have been renamed to
[property@AdwLeaflet:can-navigate-back] and
[property@AdwLeaflet:can-navigate-forward], along with their accessors.

The `hhomogeneous-folded`, `vhomogeneous-folded`, `hhomogeneous-unfolded`,
and `vhomogeneous-unfolded` properties have been replaced by a single
[property@Adw.Leaflet:homogeneous] property, set to `TRUE` by default,
applied when the leaflet is folded for the opposite orientation.

When unfolded, children are never homogeneous. Use [class@Gtk.SizeGroup]
to make them homogeneous if needed.

The `interpolate-size` property has been removed with no replacement, it's
always enabled when [property@Adw.Leaflet:homogeneous] is set to `FALSE`.

### Adapt to view switcher API Changes

[class@Adw.ViewSwitcher], [class@Adw.ViewSwitcherBar] and
[class@Adw.ViewSwitcherTitle] now use [class@Adw.ViewStack] instead of
[class@Gtk.Stack].

You should stop using [property@Gtk.Stack:transition-type] and
[property@Gtk.Stack:transition-duration] properties before switching to
[class@Adw.ViewStack].

The `auto` view switcher policy has been removed. [class@Adw.ViewSwitcher] only
has narrow and wide policies; if you had used the `auto` policy, use an
[class@Adw.Squeezer] with two view switchers inside.

#### Adapt to [class@Adw.ViewSwitcher] API Changes

The "narrow-ellipsize" property has been removed. Narrow view switchers always
ellipsize their labels, wide switchers never do.

#### Adapt to [class@Adw.ViewSwitcherBar] API Changes

The "policy" property has been removed. If you had used it, use a plain
[class@Adw.ViewSwitcher] in a [class@Gtk.ActionBar] instead.

#### Adapt to [class@Adw.ViewSwitcherTitle] API Changes

The "policy" property has been removed, the behavior is similar to the removed
`auto` policy. If you had used `wide` or `narrow` policies, use an
[class@Adw.Squeezer] with an [class@Adw.ViewSwitcher] and an
[class@Adw.WindowTitle] inside, with the switcher having the desired policy.

### Adapt to [class@Adw.Avatar] API Changes

The `HdyAvatar:loadable-icon` property has been removed along with its getter
and setter. It can be replaced by [property@Adw.Avatar:custom-image].

The `hdy_avatar_draw_to_pixbuf()` and `hdy_avatar_draw_to_pixbuf_async()`
functions have been removed, use the newly added
[method@Adw.Avatar.draw_to_texture] instead. [class@Gdk.Texture] implements
[iface@Gio.Icon], so it should just work for that case.

[method@Adw.Avatar.draw_to_texture] does not have the `size` parameter. Instead,
it uses the avatar's current size, with no replacement.

### Adapt to [class@Adw.StyleManager] API Changes

When used with the default style manager, `ADW_COLOR_SCHEME_DEFAULT` is now
equivalent to `ADW_COLOR_SCHEME_PREFER_LIGHT` instead of
`HDY_COLOR_SCHEME_FORCE_LIGHT`, following the system dark style preference by
default. Make sure your application works with it, or otherwise set the
`ADW_COLOR_SCHEME_FORCE_LIGHT` color scheme manually.

### Adapt to [class@Adw.SwipeTracker] API changes

The [signal@Adw.SwipeTracker::begin-swipe] signal is now emitted immediately
before the swipe starts, after the drag threshold has been reached, and it has
lost its `direction` parameter. The new [signal@Adw.SwipeTracker::prepare]
signal behaves exactly like `begin-swipe` was, and can be used instead of it.

### Adapt to [class@Adw.TabView] API changes

The `HdyTabVoew:shortcut-widget` property has been removed with no replacement;
[class@AdwTabView] automatically installs shortcuts with the
`GTK_SHORTCUT_SCOPE_MANAGED` scope, so they are automatically available
throughout the window without the need to set shortcut widget.

If some of these shortcuts conflict with another widget, the latter has
priority, and it should work automatically if the widget correctly stops event
propagation.

### Adapt to Stylesheet Changes

Most widgets don't have a backdrop state anymore, and the following public
colors have been removed:

* `@theme_unfocused_fg_color`
* `@theme_unfocused_text_color`
* `@theme_unfocused_bg_color`
* `@theme_unfocused_base_color`
* `@theme_unfocused_selected_bg_color`
* `@theme_unfocused_selected_fg_color`
* `@unfocused_insensitive_color`
* `@unfocused_borders`

The `.list-button` style class has been renamed to the more accurate name
`.outline`.

The public colors `@theme_selected_bg_color` and `@theme_selected_fg_color` have
been renamed to `@accent_bg_color` and `@accent_fg_color`.

If you were using `@theme_selected_bg_color` as a text color, use
`@accent_color` instead to make sure the text is readable. You can also use the
`.accent` style class to apply the correct color.

### Stop using the `.sidebar` style class

The `.sidebar` style class is now deprecated, although still works for
compatibility reasons. The main use case - adjusting the background color of
[class@Gtk.ListBox] and [class@Gtk.ListView] - can now be done with the
`.navigation-sidebar` style class on those widgets instead, along with adjusting
the item selection style. The border can be replicated by manually adding a
[class@Gtk.Separator].

### Adapt to the `popover.combo` style removal

The `.combo` popover style class has been removed. Use `.menu` instead. You may
need to remove manually added margins, padding or minimum height from the list
items inside while doing it.

#### Adapt to Header Bar, Action Bar, Search Bar and Toolbar Style Changes

When possible, buttons in [class@Gtk.HeaderBar], [class@Gtk.ActionBar] and
[class@Gtk.SearchBar] will use flat appearance by default.

The following rules are used when deciding when to make buttons flat or not:

The following buttons get flat appearance:

* Icon-only buttons;
* Buttons with an icon and a label (using [class@Adw.ButtonContent]);
* Menu buttons containing an arrow;
* [class@Adw.SplitButton];
* Any other button with the `.flat` style class.

The following buttons keep default appearance:

* Text-only buttons;
* Buttons with other content;
* Buttons within widgets containing the `.linked` style class;
* Buttons with the `.suggested-action` or `.destructive-action` style classes.
* Buttons with the `.raised` style class.

It's important to avoid ambiguous layouts, for example text-only buttons with
no icon, since such a button would be indistinguishable from the window title
without hovering it.

In rare cases, the existing layout may need a redesign to work with the new
style.

The same rules are also used for the `.toolbar` style class now, instead of
making every button appear flat.

#### Adapt to List Style Changes

For boxed lists we now have the `.boxed-list` style class that matches
the name of the design pattern. If you were using the `.content` style
class, you should use `.boxed-list` instead.

The `.content` style class currently remains for compatibility purposes.

#### Adjusting Icons

If you're bundling icons from the icon library with your application, make sure
to update them. Many icons have been redrawn to be larger to work better without
button frames.

If you're using the `object-select-symbolic` icon in a header bar button
(typically for selection mode), use `selection-mode-symbolic` instead.

#### Adjusting Icon+Arrow Menu Buttons

If you had menu buttons containing an icon and a dropdown arrow, switch to
[property@Gtk.MenuButton:icon-name] and set the
[property@Gtk.MenuButton:always-show-arrow] property to `TRUE`.

#### Adjusting Text-only Buttons

If you had text-only buttons, consider using [class@Adw.ButtonContent]. For
example, the following button:

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

If you had split buttons implemented via a `GtkBox` with the `.linked` style
class and two buttons packed inside, use `AdwSplitButton` as follows:

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

#### Custom adjustments

The `.flat` and `.raised` style classes can always be used to override the
default appearance.

Important: the [property@Gtk.Button:has-frame] property will **not** be set to
`FALSE` when a button gets the flat appearance automatically. It also cannot be
set to `TRUE` to make a button raised, the style class should be used directly
instead.
