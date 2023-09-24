#!/bin/sh

FIN="$0"
PAK=../zipzop

zip -1 "$FIN.zip" "$FIN"
$PAK 15 "$FIN.zip" "$FIN-zipzop15.zip"
if [ $? -eq 0 ] && [ -f "$FIN-zipzop15.zip" ]; then
    echo self-test successful
fi
rm "$FIN.zip" "$FIN-zipzop15.zip"
