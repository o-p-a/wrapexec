call AddPath C:\borland\bcc55\bin
set INCLUDE=C:\Borland\Bcc55\include
set dircmd=/a /ogn
doskey make=make -f MAKEFILE.borland $*
