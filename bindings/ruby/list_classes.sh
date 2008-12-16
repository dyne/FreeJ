#!/bin/sh
ruby1.9 -I.libs -e "require 'Freej'; puts Freej::constants.sort.join(\"\n\")"
