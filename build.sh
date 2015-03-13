#!/usr/bin/env bash
echo -n "Enter prefix path [/usr/local]: "
read ans
if [ "x$ans" = "x" ]; then
	prefix="/usr/local"
else
	prefix="$ans"
fi

echo -n "Enter qmake [qmake-qt4]: "
read qans
if [ "x$qans" = "x" ]; then
	qmake="qmake-qt4"
else
	qmake="$qans"
fi

if ! $qmake PREFIX="$prefix"; then
	echo "E: Can't exec qmake. Please, try qmake, qmake-qt4 or others."
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
