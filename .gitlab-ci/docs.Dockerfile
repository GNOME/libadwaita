FROM fedora:34

RUN dnf -y update \
 && dnf -y install \
    @development-tools \
    dnf-plugins-core \
    gcc \
    git \
    gobject-introspection-devel \
    gtk4-devel \
    meson \
    pcre-static \
    python3 \
    python3-jinja2 \
    python3-markdown \
    python3-pip \
    python3-pygments \
    python3-toml \
    python3-typogrify \
    python3-wheel \
    redhat-rpm-config \
    sassc \
 && dnf clean all
