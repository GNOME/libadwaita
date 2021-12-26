Title: Boxed Lists
Slug: boxed-lists

# Boxed Lists

Libadwaita provides API to implement the [boxed lists](https://developer.gnome.org/hig/patterns/containers/boxed-lists.html)
pattern.

Boxed lists are composed of a [class@Gtk.ListBox] with the
[property@Gtk.ListBox:selection-mode] set to `GTK_SELECTION_NONE` and with the
[`.boxed-list`](style-classes.html#boxed-lists-cards) style class.

[class@Gtk.ListView] cannot be used as a boxed list at the moment.

An example boxed list:

```xml
<object class="GtkListBox">
  <property name="selection-mode">none</property>
  <style>
    <class name="boxed-list"/>
  </style>
  <child>
    <object class="AdwActionRow">
      <property name="title">Item 1</property>
    </object>
  </child>
  <child>
    <object class="AdwActionRow">
      <property name="title">Item 2</property>
    </object>
  </child>
  <child>
    <object class="AdwActionRow">
      <property name="title">Item 3</property>
    </object>
  </child>
</object>
```

<picture>
  <source srcset="boxed-lists-dark.png" media="(prefers-color-scheme: dark)">
  <img src="boxed-lists.png" alt="boxed-lists">
</picture>

# Rows

A number of predefined list row classes are available and intended to be used
inside boxed lists:

## Action Rows

[class@ActionRow] is a basic row. It has a title, a subtitle, an icon, and can
have prefix and suffix children.

<picture>
  <source srcset="action-row-dark.png" media="(prefers-color-scheme: dark)">
  <img src="action-row.png" alt="action-row">
</picture>

## Expander Rows

[class@ExpanderRow] is similar to [class@ActionRow], but can expand to show
other rows.

<picture>
  <source srcset="expander-row-dark.png" media="(prefers-color-scheme: dark)">
  <img src="expander-row.png" alt="expander-row">
</picture>

## Combo Rows

[class@ComboRow] is a row with an embedded drop down menu, similar to
[class@Gtk.DropDown].

<picture>
  <source srcset="combo-row-dark.png" media="(prefers-color-scheme: dark)">
  <img src="combo-row.png" alt="combo-row">
</picture>

## Preferences Group

[class@PreferencesGroup] provides a boxed list along with a title and a
description. It's mainly meant to be used as a child of [class@PreferencesPage],
but can also be used separately.

<picture>
  <source srcset="preferences-group-dark.png" media="(prefers-color-scheme: dark)">
  <img src="preferences-group.png" alt="preferences-group">
</picture>
