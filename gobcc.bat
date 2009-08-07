call AddPath "%~d0\opt\bcc55\bin"
set INCLUDE=%~d0\opt\bcc55\include
set DIRCMD=/a /ogn
doskey make=make -f MAKEFILE.borland $*
