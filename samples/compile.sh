#!/bin/bash

cd ./std/
as rt.s -o rt.o
cd ../samples/

for file in *.lace; do
    if [[ -f "$file" ]]; then
        ./../lace/lace "$file"
        ld "$file".o ../std/rt.o -o "${file%.lace}"
    fi
done
