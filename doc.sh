#!/bin/bash

DOC_DIR=public/doc/
REFS="
master
libhandy-1-0
"

LATEST_STABLE_1=1.0

IFS='
'

mkdir -p $DOC_DIR

for REF in $REFS; do
  API_VERSION=`echo $REF | sed 's/libhandy-\([0-9][0-9]*\)-\([0-9][0-9]*\)/\1.\2/'`
  API_VERSION=`echo $API_VERSION | sed 's/v0.0.\([0-9][0-9]*\)/0.0.\1/'`

  curl -L --output "$REF.zip" "https://gitlab.gnome.org/GNOME/libhandy/-/jobs/artifacts/$REF/download?job=build-gtkdoc"
  unzip -d "$REF" "$REF.zip"
  mv "$REF/_reference" $DOC_DIR/$API_VERSION

  rm "$REF.zip"
  rm -rf "$REF"
done

cp -r $DOC_DIR/$LATEST_STABLE_1 $DOC_DIR/1-latest
