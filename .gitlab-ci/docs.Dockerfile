FROM fedora:latest

RUN dnf -y update \
 && dnf -y install \
    "dnf-command(builddep)" \
    expat-devel \
    git \
    libjpeg-turbo-devel \
    python3-jinja2 \
    python3-pygments \
    python3-toml \
    python3-typogrify \
    sassc \
 && sudo dnf -y build-dep gtk4 \
 && dnf clean all

RUN git clone https://gitlab.gnome.org/GNOME/gtk.git --depth=1 \
 && cd gtk \
 && meson build --prefix=/usr \
 && cd build \
 && ninja \
 && sudo ninja install \
 && cd ../.. \
 && rm -rf gtk

RUN dnf -y remove gi-docgen
