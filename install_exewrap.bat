@echo off
title %~n0
setlocal
pushd "%~dp0"

if not exist "wrapexec.exe" (
	echo %~n0: wrapexec.exe not found
	goto :eof
)
if not exist "wrapexecw.exe" (
	echo %~n0: wrapexecw.exe not found
	goto :eof
)

call :exec1 C "samples\bat2exe"
call :exec1 C "samples\js2exe"
call :exec1 C "samples\pl2exe"
call :exec1 C "samples\rb2exe"
call :exec1 C "samples\runbatch"
call :exec1 C "samples\runjscript"
call :exec1 C "samples\runperl"
call :exec1 C "samples\runruby"

call :exec1 C "\usrlocal\bat\DeleteIfExist"
call :exec1 C "\usrlocal\bat\irb"
call :exec1 C "\usrlocal\bat\MakeShortcut"
call :exec1 C "\usrlocal\bat\more"
call :exec1 C "\usrlocal\bat\MoveWithDatetime"
call :exec1 C "\usrlocal\bat\NetUse"
call :exec1 C "\usrlocal\bat\NetUseDeleteAll"
call :exec1 C "\usrlocal\bat\perl"
call :exec1 C "\usrlocal\bat\ruby"
call :exec1 C "\usrlocal\bat\TimeSync"
call :exec1 C "\usrlocal\bat\vi"

call :exec1 G "\usrlocal\bat\CmdPrompt"
call :exec1 G "\usrlocal\bat\emacs"
call :exec1 G "\usrlocal\bat\eval"
call :exec1 G "\usrlocal\bat\ExtractArchive"
call :exec1 G "\usrlocal\bat\gv"
call :exec1 G "\usrlocal\bat\gvim"
call :exec1 G "\usrlocal\bat\MicrosoftExcel"
call :exec1 G "\usrlocal\bat\MicrosoftWord"
call :exec1 G "\usrlocal\bat\mplayer"
call :exec1 G "\usrlocal\bat\mplayerc"
call :exec1 G "\usrlocal\bat\PdfViewer"
call :exec1 G "\usrlocal\bat\ped"
call :exec1 G "\usrlocal\bat\pedc"
call :exec1 G "\usrlocal\bat\pedTaskTray"
call :exec1 G "\usrlocal\bat\PuTTY"
call :exec1 G "\usrlocal\bat\TeraTerm"
call :exec1 G "\usrlocal\bat\UnplugDrive2"
call :exec1 G "\usrlocal\bat\ViX"
call :exec1 G "\usrlocal\bat\vncviewer"
call :exec1 G "\usrlocal\bat\WinMerge"

popd
endlocal
goto :eof

:exec1
if /i "%~1" == "C" (
	set SRC=wrapexec.exe
) else if /i "%~1" == "G" (
	set SRC=wrapexecw.exe
) else (
	echo %~n0: unknown action: "%~1"
	goto :eof
)
if exist "%~dpn2.ini" (
	goto found1
) else if exist "%~dpn2.bat" (
	goto found1
) else (
	echo %~n0: not found .ini or .bat: "%~2"
	goto :eof
)
:found1
set DST=%~dpn2.exe
echo "%SRC%" -^> "%DST%"
copy /y "%SRC%" "%DST%"
