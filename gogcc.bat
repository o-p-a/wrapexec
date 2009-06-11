call AddPath C:\MinGW\bin
set dircmd=/a /ogn
doskey make=make --file=MAKEFILE.mingw $*
