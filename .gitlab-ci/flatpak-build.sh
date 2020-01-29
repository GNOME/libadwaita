#!/bin/bash

set -e

builddir=app
repodir=repo
appid="$1"
manifest="$2"

flatpak-builder \
       --stop-at=libhandy \
       ${builddir} \
       ${manifest}

flatpak-builder -v \
        --run ${builddir} ${manifest} \
        meson \
                --prefix /app \
                --libdir /app/lib \
                --buildtype debug \
                -Dintrospection=disabled \
                -Dvapi=false \
                _build .

flatpak-builder \
        --run ${builddir} ${manifest} \
        ninja -C _build install

flatpak-builder \
        --finish-only \
        --repo=${repodir} \
        ${builddir} \
        ${manifest}

flatpak build-bundle \
        ${repodir} \
        ${appid}-dev.flatpak \
        --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo \
        ${appid}
