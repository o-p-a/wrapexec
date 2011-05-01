@goto BATCH_BEGIN & rem vi:set ft=BAT : -*- coding:US-ASCII mode:BAT -*-
[exec]
${MY_ININAME}
[end]
:BATCH_BEGIN
@echo off
::----------------------------------------------------------------
rem Example for wrapexec. Replace your batch file below.
::----------------------------------------------------------------

echo Hello world !! I am '%~dpn0.bat'

