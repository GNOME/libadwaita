FROM fedora:latest

RUN dnf -y update \
 && dnf -y install \
    "dnf-command(builddep)" \
    expat-devel \
    git \
    libabigail \
    libjpeg-turbo-devel \
    python3-jinja2 \
    python3-packaging \
    python3-pygments \
    python3-toml \
    python3-typogrify \
    sassc \
    vala \
 && dnf -y build-dep gtk4 \
 && dnf -y remove gi-docgen \
 && dnf clean all

RUN git clone https://gitlab.gnome.org/GNOME/gtk.git --depth=1 \
 && cd gtk \
 && meson build --prefix=/usr \
 && cd build \
 && ninja \
 && sudo ninja install \
 && cd ../.. \
 && rm -rf gtk
