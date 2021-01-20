# Adwaita
[![Pipeline status](https://gitlab.gnome.org/exalm/libadwaita/badges/main/build.svg)](https://gitlab.gnome.org/exalm/libadwaita/commits/main)
[![Code coverage](https://gitlab.gnome.org/exalm/libadwaita/badges/main/coverage.svg)](https://gitlab.gnome.org/exalm/libadwaita/commits/main)

The aim of the Adwaita library is to help with developing UI for mobile devices
using GTK/GNOME.

## License

libadwaita is licensed under the LGPL-2.1+.

## Build dependencies

To build libadwaita you need to first install the build-deps defined by [the debian/control file](https://gitlab.gnome.org/GNOME/libadwaita/blob/master/debian/control#L6).

If you are running a Debian based distribution, you can easily install all those the dependencies making use of the following command

```sh
sudo apt-get build-dep .
```

## Building

We use the Meson (and thereby Ninja) build system for libadwaita. The quickest
way to get going is to do the following:

```sh
meson . _build
ninja -C _build
ninja -C _build install
```

For build options see [meson_options.txt](./meson_options.txt). E.g. to enable documentation:

```sh
meson . _build -Dgtk_doc=true
ninja -C _build libadwaita-doc
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

## Documentation

The documentation can be found online
[here](https://exalm.pages.gitlab.gnome.org/libadwaita/doc/).

## Getting in touch

Matrix room: [#libadwaita:gnome.org](https://gnome.element.io/#/room/#libadwaita:gnome.org)
