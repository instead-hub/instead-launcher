#!/bin/bash
echo -n "Enter prefix path [/usr/local]: "
read ans
if [ "x$ans" = "x" ]; then
	prefix="/usr/local"
else
	prefix="$ans"
fi

if ! qmake PREFIX="$ans"; then
	echo "E: Can't exec qmake"
	exit 1
fi

make clean

if ! make; then
	echo "E: Error while it was building"
	exit 1
fi

echo ":: Build is completed."
echo ":: Try: "
echo "::    \$ sudo make install"
echo ":: or just run launcher from build dir:"
echo "::    \$ ./instead-launcher"
echo ":: Have fun!"
