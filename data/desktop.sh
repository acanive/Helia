#!/bin/sh

echo "[Desktop Entry]" > $1
for info in "Name=Helia" "Comment=Media Player & Digital TV" "Type=Application" "Exec=$2/helia %U" "Icon=display" "Terminal=false" "Categories=GTK;AudioVideo;Audio;Video;Player;TV;";
do echo "$info" >> $1; done
cat data/desktop-mime >> $1