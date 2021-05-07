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
