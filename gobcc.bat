@echo off

if exist "C:\Borland\bcc55\" (
	set BCCDIR=C:\Borland\bcc55\
) else if exist "D:\opt\bcc55\" (
	set BCCDIR=D:\opt\bcc55\
) else (
	set BCCDIR=%~d0\opt\bcc55\
)

call AddPath "%BCCDIR%bin"
set INCLUDE=%BCCDIR%include
set DIRCMD=/a /ogn
doskey make=make -f MAKEFILE.borland $*
