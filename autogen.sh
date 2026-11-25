#!/bin/sh

CDPATH='' cd -- "$(dirname -- "$0")" || exit
# Automake's forced install would replace the project's INSTALL document.
exec autoreconf -iv
