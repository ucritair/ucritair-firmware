#!/bin/bash


# convert a base64 PSK to C hex string
#
# example input: 'AAECAwQFBgcICQoLDA0ODw=='

echo -n "${1}" | base64 -d | xxd -p | sed -E 's/(..)/0x\1/g' | sed -E 's/(.{4})/\1, /g;s/, $//'

