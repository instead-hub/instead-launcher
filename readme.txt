INSTEAD LAUNCHER 0.6.3
======================

WARNING! For successfull building you must install qt4.

0) Prepare for building
=======================

Unpack source package with this command:
	$ tar xzvf instead-launcher_<version>.tar.gz

Change current dir to project's build dir:
	$ cd instead-launcher<version>

There are several ways to build package.

1) Native qt build
====================================================
	$ qmake PRREFIX=/usr/local/ or qmake PRREFIX=/usr/
	$ make
	$ make install

2) Easy build
==========================
Try run: 
	$ ./build.sh

Enjoy.
