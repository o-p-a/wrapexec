@goto BATCH_BEGIN && -*- mode:BAT; coding:US-ASCII; -*-
@ vi:set ft=BAT fenc=US-ASCII :
[exec]
${MY_ININAME}
[end]
:BATCH_BEGIN
@echo off
::----------------------------------------------------------------
rem Example for wrapexec. Replace your batch file below.
::----------------------------------------------------------------

echo Hello world !! I am '%~dpn0.bat'

