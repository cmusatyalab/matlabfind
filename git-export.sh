#!/bin/sh

set -e

if [ -z "$1" ] ; then
	echo "Usage: $0 <version>" >&2
	exit 1
fi

b="matlabfind"

git archive --format tar "v$1" "--prefix=$b-$1/" -o "$b-$1.tar"
gzip -9f "$b-$1.tar"
