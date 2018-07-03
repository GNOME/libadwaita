# Handy
[![Pipeline status](https://source.puri.sm/Librem5/libhandy/badges/master/build.svg)](https://source.puri.sm/Librem5/libhandy/commits/master)
[![Code coverage](https://source.puri.sm/Librem5/libhandy/badges/master/coverage.svg)](https://source.puri.sm/Librem5/libhandy/commits/master)

The aim of The handy library is to help with developing UI for mobile devices
using GTK+/GNOME.

## License

libhandy is licensed under the LGPL-2.1+.

## Build dependencies

To build libhandy you need the following build-deps:

```sh
sudo apt-get -y install gtk-doc-tools libgirepository1.0-dev libgnome-desktop-3-dev libgtk-3-dev meson pkg-config valac
```

## Building

We use the meson (and thereby Ninja) build system for libhandy. The quickest
way to get going is to do the following:

```sh
meson . _build
ninja -C _build
ninja -C _build install
```

For build options see [meson_options.txt](./meson_options.txt). E.g. to enable documentation:

```sh
meson . _build -Dgtk_doc=true
ninja -C _build/ libhandy-doc
```

## Usage

There's a C example:

```sh
_build/examples/example
```

and one in Python. When running from the built source tree it
needs several environment varibles so use \_build/run to set them:

```sh
_build/run examples/example.py
```

### Glade

To be able to use Handy's widgets in the glade interface designer without
installing the library use:

```sh
_build/run glade
```
