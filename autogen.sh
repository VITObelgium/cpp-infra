#!/bin/sh

echo "Bootstrapping OPAQ"
rm -rf autom4te.cache

autoreconf -fvi

echo "All done :)"

