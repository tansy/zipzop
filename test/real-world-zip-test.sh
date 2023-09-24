#!/bin/sh

FIN="test.odt"
PAK=../zipzop

$PAK 15 "$FIN" "$FIN-zipzop15.zip"
if [ $? -eq 0 ] && [ -f "$FIN-zipzop15.zip" ]; then
    echo real world zip test successful
fi
#rm "$FIN-zipzop15.zip"
