instead-launcher  - launcher and game downloader for INSTEAD interpreter


============== 1. Normal build ==============

1) Get source from SVN and extract it.
2) Make sure that your QTDIR and PATH contains path to your Qt version
Windows:
set QTDIR=c:\qt\4.5.3
set PATH=%QTDIR%\bin;c:\mingw\bin;%PATH%
Linux:
export QTDIR=/usr/local/Trolltech/Qt-4.5.3
export PATH=$QTDIR/bin:$PATH
2) Run qmake to generate makefiles
3) Run make to build executable
4) In windows copy instead-launcher_ru.qm to directory with executable

============ 2. Static windows build ============

1) Get source from SVN and extract it.
2) Build static version of Qt - extract fresh sources to some dir and execute
cd c:\qt\static-4.5.3
set QTDIR=c:\qt\static-4.5.3
set PATH=%QTDIR%\bin;c:\mingw\bin;%PATH%
configure.exe -static -release -opensource
mingw32-make
3) Build launcher with commands
cd c:\path\to\launcher\sources
set QTDIR=c:\qt\static-4.5.3
set PATH=%QTDIR%\bin;c:\mingw\bin;%PATH%
qmake
make -f Makefile.Release
4) If needed compress resulted file with UPX
5) Place mingwm10.dll and instead-launcher_ru.qm near executable

============= 3. Command line options =============
-log <logfile>
  saves debug logs to <logfile>
