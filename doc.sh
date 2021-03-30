#!/bin/bash

DOC_DIR=public/doc
REFS="
main
"

# LATEST_STABLE_1=1.0

IFS='
'

mkdir -p $DOC_DIR

for REF in $REFS; do
  API_VERSION=`echo $REF | sed 's/libadwaita-\([0-9][0-9]*\)-\([0-9][0-9]*\)/\1.\2/'`

  curl -L --output "$REF.zip" "https://gitlab.gnome.org/GNOME/libadwaita/-/jobs/artifacts/$REF/download?job=doc"
  unzip -d "$REF" "$REF.zip"
  mv "$REF/_doc" $DOC_DIR/$API_VERSION

  rm "$REF.zip"
  rm -rf "$REF"
done

# cp -r $DOC_DIR/$LATEST_STABLE_1 $DOC_DIR/1-latest

find $DOC_DIR -type f -print0 | xargs -0 sed -i 's|\.\./gdk4/|https://developer.gnome.org/gdk4/stable/|g'
find $DOC_DIR -type f -print0 | xargs -0 sed -i 's|\.\./gsk4/|https://developer.gnome.org/gsk4/stable/|g'
find $DOC_DIR -type f -print0 | xargs -0 sed -i 's|\.\./gdk-pixbuf/|https://developer.gnome.org/gdk-pixbuf/stable/|g'
find $DOC_DIR -type f -print0 | xargs -0 sed -i 's|\.\./gio/|https://developer.gnome.org/gio/stable/|g'
find $DOC_DIR -type f -print0 | xargs -0 sed -i 's|\.\./glib/|https://developer.gnome.org/glib/stable/|g'
find $DOC_DIR -type f -print0 | xargs -0 sed -i 's|\.\./gobject/|https://developer.gnome.org/gobject/stable/|g'
find $DOC_DIR -type f -print0 | xargs -0 sed -i 's|\.\./gtk4/|https://developer.gnome.org/gtk4/stable/|g'
find $DOC_DIR -type f -print0 | xargs -0 sed -i 's|\.\./pango/|https://developer.gnome.org/pango/stable/|g'
