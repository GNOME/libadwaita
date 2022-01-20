#!/bin/bash

DOC_DIR=public/doc
REFS="
main
"

LATEST_STABLE_1=1.0.0

IFS='
'

mkdir -p $DOC_DIR

for REF in $REFS; do
  curl -L --output "$REF.zip" "https://gitlab.gnome.org/GNOME/libadwaita/-/jobs/artifacts/$REF/download?job=doc"
  unzip -d "$REF" "$REF.zip"
  mv "$REF/_doc" $DOC_DIR/$REF

  rm "$REF.zip"
  rm -rf "$REF"
done

ln -s $LATEST_STABLE_1 $DOC_DIR/1-latest
