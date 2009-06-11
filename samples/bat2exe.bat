@goto BATCH_BEGIN
[exec]
${MY_ININAME}
[end]
:BATCH_BEGIN
@echo off

rem Example for wrapexec. Replace your batch file here.

echo Hello world !! I am '%~f0'

