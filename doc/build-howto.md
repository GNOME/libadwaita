Title: Compiling with Libadwaita
Slug: building

# Compiling with Libadwaita

If you need to build Libadwaita, get the source from
[here](https://gitlab.gnome.org/GNOME/libadwaita/) and see the `README.md` file.

## Using `pkg-config`

Like other GNOME libraries, Libadwaita uses `pkg-config` to provide compiler
options. The package name is `libadwaita-1`.

When using the Meson build system you can declare a dependency like:

```meson
dependency('libadwaita-1')
```

The `1` in the package name is the "API version" (indicating "the version of the
Libadwaita API that first appeared in version 1") and is essentially just part
of the package name.

## Bundling the Library

As Libadwaita uses the Meson build system, bundling it as a subproject when it
is not installed is easy. Add this to your `meson.build`:

```meson
libadwaita_dep = dependency('libadwaita-1', version: '>= 1.0.0', required: false)
if not libadwaita_dep.found()
libadwaita = subproject(
  'libadwaita',
  default_options: [
    'examples=false',
    'package_subdir=my-project-name',
    'tests=false',
  ]
)
libadwaita_dep = libadwaita.get_variable('libadwaita_dep')
endif
```

Then add Libadwaita as a git submodule:

```bash
git submodule add https://gitlab.gnome.org/GNOME/libadwaita.git subprojects/libadwaita
```

To bundle the library with your Flatpak application, add the following module to
your manifest:

```json
{
  "name" : "libadwaita",
  "buildsystem" : "meson",
  "config-opts": [
    "-Dexamples=false",
    "-Dtests=false"
  ],
  "sources" : [
    {
      "type" : "git",
      "url" : "https://gitlab.gnome.org/GNOME/libadwaita.git",
      "branch" : "main"
    }
  ]
}
```

## Building on macOS

To build on macOS you need to install the build-dependencies first. This can
e.g. be done via [`brew`](https://brew.sh):

```bash
brew install pkg-config gtk4 meson gobject-introspection vala
```

After running the command above, one may now build the library:

```bash
git clone https://gitlab.gnome.org/GNOME/libadwaita.git
cd libadwaita
meson _build
ninja -C _build
ninja -C _build install
```

Working with the library on macOS is pretty much the same as on Linux. To link
it, use `pkg-config`:

```bash
gcc $(pkg-config --cflags --libs gtk4) $(pkg-config --cflags --libs libadwaita-1) main.c -o main
```
