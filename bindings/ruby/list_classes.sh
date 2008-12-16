#!/bin/sh
cd .libs
ruby1.9 -e "require 'Freej'; puts Freej::constants.sort.join(\"\n\")"
cd -

