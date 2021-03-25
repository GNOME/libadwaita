# See https://sourceware.org/bugzilla/show_bug.cgi?id=27267
FROM fedora:33

RUN dnf -y update \
 && dnf -y install \
    @development-tools \
    dnf-plugins-core \
    gcc \
    git \
    gobject-introspection \
    gtk4-devel \
    libabigail \
    meson \
    redhat-rpm-config \
    vala \
 && dnf clean all

# See https://sourceware.org/bugzilla/show_bug.cgi?id=27269
RUN rpm -Uvh --oldpackage https://kojipkgs.fedoraproject.org//packages/libabigail/1.7/2.fc33/x86_64/libabigail-1.7-2.fc33.x86_64.rpm
