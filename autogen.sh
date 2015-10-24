#!/bin/sh

echo "Bootstrapping OPAQ"
rm -rf autom4te.cache

# libtoolize is called glibtoolize on osx.. check this !
echo " -- library has libtool support, libtoolizing..."
syst=$(uname)
if [ ${syst} = "Darwin" ]; then
    glibtoolize --copy --force > /dev/null
else
    libtoolize --copy --force > /dev/null
fi

echo " -- running aclocal..."
aclocal  

echo " -- running autoheader..."
autoheader

echo " -- running automake, creating Makefile template..."
automake --copy --add-missing --force-missing --foreign

echo " -- running autoconf, creating configure scripts..."
autoconf --force

echo "All done :)"

