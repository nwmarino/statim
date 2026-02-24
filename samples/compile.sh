#!/bin/bash

cd /home/lovelace/stl
as rt.s -o rt.o
cd /home/lovelace/samples

for file in *.lace; do
    if [[ -f "$file" ]]; then
        /home/lovelace/lace/lace -stl "$file" -o "${file%.lace}"
    fi
done
