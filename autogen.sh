#!/bin/bash -e
mkdir -p "$(dirname "$0")"/build-aux/am "$(dirname "$0")"/build-aux/h

autoreconf --force --install "$(dirname "$0")"
