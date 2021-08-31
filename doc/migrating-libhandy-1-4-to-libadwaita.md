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

### Bundle the Icons You're Using

The preferred way to use icons in applications is to copy them into the
application and to bundle them via `GResource`.
Referencing system icons won't work in Libadwaita other than for icons GTK
itself ships, so make sure to bundle the icons.

### Use `HdyFlap` Properties for Adding Children Instead of `gtk_container_add()`

`HdyFlap` provides the "content", "flap" and "separator" properties that can be
used for managing children instead of `GtkContainer` API. In Libadwaita
[property@Adw.Flap:content], [property@Adw.Flap:flap] and
[property@Adw.Flap:separator] are the only way to manage [class@Adw.Flap]
children.

Adding children in a UI file still works.

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
[class@Adw.ExpanderRow]       | [method@Adw.ExpanderRow.add]       | [method@Adw.ExpanderRow.remove]
[class@Adw.Leaflet]           | [method@Adw.Leaflet.append]        | [method@Adw.Leaflet.remove]
[class@Adw.PreferencesGroup]  | [method@Adw.PreferencesGroup.add]  | [method@Adw.PreferencesGroup.remove]
[class@Adw.PreferencesPage]   | [method@Adw.PreferencesPage.add]   | [method@Adw.PreferencesPage.remove]
[class@Adw.PreferencesWindow] | [method@Adw.PreferencesWindow.add] | [method@Adw.PreferencesWindow.remove]

Adding children in a UI file still works.

### Adapt to `HdySearchBar` Removal

`HdySearchBar` has been removed, use [class@Gtk.SearchBar] instead.

### Adapt to `HdyWindowHandle` Removal

`HdyWindowHandle` has been removed, use [class@Gtk.WindowHandle] instead.

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
expr = gtk_property_expression_new (ADW_TYPE_ENUM_VALUE_OBJECT, NULL, "nick");
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

### Stop Creating `HdyEnumValueObject` Instances

[class@Adw.EnumValueObject] can no longer be manually created and is only
intended to be used with [class@Adw.EnumListModel].

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

### Adapt to view switcher API Changes

[class@Adw.ViewSwitcher], [class@Adw.ViewSwitcherBar] and
[class@Adw.ViewSwitcherTitle] now use [class@Adw.ViewStack] instead of
[class@Gtk.Stack].

You should stop using [property@Gtk.Stack:transition-type] and
[property@Gtk.Stack:transition-duration] properties before switching to
[class@Adw.ViewStack].

### Adapt to [class@Adw.Avatar] API Changes

The `HdyAvatar:loadable-icon` property has been removed along with its getter
and setter. It can be replaced by [property@Adw.Avatar:custom-image].

The `hdy_avatar_draw_to_pixbuf_async()` function has been removed, use the
regular [method@Adw.Avatar.draw_to_pixbuf] instead.

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
