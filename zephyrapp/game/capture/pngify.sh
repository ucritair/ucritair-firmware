#!/usr/bin/env bash
mogrify -format png $(dirname "$0")/*.ppm
trash $(dirname "$0")/*.ppm