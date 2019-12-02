#!/bin/sh

echo "<gresources>" > $1
echo "  <gresource prefix=\"/helia\">" >> $1
for grfile in data/icons/*.png; do echo "    <file preprocess=\"to-pixdata\">$grfile</file>" >> $1; done
echo "  </gresource>" >> $1
echo "</gresources>" >> $1