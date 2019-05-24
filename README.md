# Handy
[![Pipeline status](https://source.puri.sm/Librem5/libhandy/badges/master/build.svg)](https://source.puri.sm/Librem5/libhandy/commits/master)
[![Code coverage](https://source.puri.sm/Librem5/libhandy/badges/master/coverage.svg)](https://source.puri.sm/Librem5/libhandy/commits/master)

The aim of The handy library is to help with developing UI for mobile devices
using GTK/GNOME.

## License

libhandy is licensed under the LGPL-2.1+.

## Build dependencies

To build libhandy you need to first install the build-deps defined by [the debian/control file](https://source.puri.sm/Librem5/libhandy/blob/master/debian/control#L6)

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

## Documentation

The documentation can be found online
[here](http://honk.sigxcpu.org/projects/libhandy/doc/). Further examples on how
to use the widgets in your favorite programming language can be found here:

### C
- [calls](https://source.puri.sm/Librem5/calls) (HdyDialer)
- [gnome-bluetooth](https://gitlab.gnome.org/GNOME/gnome-bluetooth) (HdyColumn)
- [GNOME Settings](https://gitlab.gnome.org/GNOME/gnome-control-center) (HdyActionRow, HdyColumn, HdyLeaflet, HdyTitleBar)
- [GNOME Web](https://gitlab.gnome.org/GNOME/epiphany) (HdyActionRow, HdyColumn, HdyComboRow, HdySearchBar)
- [phosh](https://source.puri.sm/Librem5/phosh) (HdyDialer)

### Python 3
- [Daty](https://gitlab.gnome.org/World/Daty) (HdyColumn, HdyLeaflet, HdySearchBar, HdyTitleBar)
- [PasswordSafe](https://gitlab.gnome.org/World/PasswordSafe) (HdyColumn)

### Rust
- [Fractal](https://gitlab.gnome.org/World/fractal) (HdyColumn, HdyLeaflet)
- [Podcasts](https://gitlab.gnome.org/World/podcasts) (HdyColumn)

### Vala
- [GNOME Contacts](https://gitlab.gnome.org/GNOME/gnome-contacts) (HdyLeaflet, HdyHeaderGroup, HdyTitleBar)
- [GNOME Games](https://gitlab.gnome.org/GNOME/gnome-games) (HdyColumn, HdyTitleBar)
