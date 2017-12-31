# Handy

The aim of The handy library is to help with developing UI for mobile devices
using GTK+/GNOME.

## License

libhandy is licensed under the LGPLv3+.

## Building

We use the meson (and thereby Ninja) build system for libhandy.  The quickest
way to get going is to do the following:

	meson . _build
	ninja -C _build
	ninja -C _build install

For build options see [meson_options.txt](./meson_otions.txt). E.g. to enable documentation:

     meson . _build -Dgtk_doc=true
     ninja -C _build/ libhandy-doc

## Usage

There's a C example:

     _build/examples/example

and one in Python. When running from the built source tree it
needs several environment varibles so use \_build/run to set them:

     _build/run examples/example.py
