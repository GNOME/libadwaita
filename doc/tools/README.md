# Adding a screenshot to the docs

1. Create an `image.ui` file in the `data/` directory.
2. Put the widget to screenshot inside with the `widget` id. For example:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <requires lib="libadwaita" version="1.0"/>
  <object class="GtkButton" id="widget">
    <property name="label">Example</property>
  </object>
</interface>
```

If a widget needs to be hovered - for example, a list item - put the `hover` id
onto it.

If the widget needs special treatment - for example, it's a `GtkPopover` - it
should be special-cased in `screenshot.c` based on its type.

3. Add it to `screenshot.gresources.xml`
4. From the build directory, run:

```
ninja doc/tools/screenshot
./doc/tools/screenshot ../doc/images/
```

5. The generator will create `image.png` and `image-dark.png` images. Add them
to `libadwaita.toml.in`.
6. Use them in the docs as follows:

```html
<picture>
  <source srcset="image-dark.png" media="(prefers-color-scheme: dark)">
  <img src="image.png" alt="image">
</picture>
```

# Regenerating screenshots

To regenerate all screenshots, run:

```c
./doc/tools/screenshot ../doc/images/
```

from the build directory.
