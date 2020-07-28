# Handy
[![Pipeline status](https://gitlab.gnome.org/GNOME/libhandy/badges/master/build.svg)](https://gitlab.gnome.org/GNOME/libhandy/commits/master)
[![Code coverage](https://gitlab.gnome.org/GNOME/libhandy/badges/master/coverage.svg)](https://gitlab.gnome.org/GNOME/libhandy/commits/master)

The aim of the Handy library is to help with developing UI for mobile devices
using GTK/GNOME.

## License

libhandy is licensed under the LGPL-2.1+.

## Build dependencies

To build libhandy you need to first install the build-deps defined by [the debian/control file](https://gitlab.gnome.org/GNOME/libhandy/blob/master/debian/control#L6).

If you are running a Debian based distribution, you can easily install all those the dependencies making use of the following command

```sh
sudo apt-get build-dep .
```

## Building

We use the Meson (and thereby Ninja) build system for libhandy. The quickest
way to get going is to do the following:

```sh
meson . _build
ninja -C _build
ninja -C _build install
```

For build options see [meson_options.txt](./meson_options.txt). E.g. to enable documentation:

```sh
meson . _build -Dgtk_doc=true
ninja -C _build libhandy-doc
```

## Usage

There's a C example:

```sh
_build/examples/example
```

and one in Python. When running from the built source tree it
needs several environment variables so use \_build/run to set them:

```sh
_build/run examples/example.py
```

### Glade

To be able to use Handy's widgets in the glade interface designer without
installing the library use:

```sh
_build/run glade
```

## Documentation

The documentation can be found online
[here](https://gnome.pages.gitlab.gnome.org/libhandy).

## Getting in touch

Matrix room: [#libhandy:talk.puri.sm](https://gnome.element.io/#/room/#libhandy:talk.puri.sm)
