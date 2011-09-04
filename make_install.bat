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

call :exec1 C "%USRLOCAL%\bat\7z"
call :exec1 C "%USRLOCAL%\bat\awk"
call :exec1 C "%USRLOCAL%\bat\bzr"
call :exec1 C "%USRLOCAL%\bat\cal"
call :exec1 C "%USRLOCAL%\bat\hd"
call :exec1 C "%USRLOCAL%\bat\irb"
call :exec1 C "%USRLOCAL%\bat\more"
call :exec1 C "%USRLOCAL%\bat\perl"
call :exec1 C "%USRLOCAL%\bat\plink"
call :exec1 C "%USRLOCAL%\bat\ps"
call :exec1 C "%USRLOCAL%\bat\python"
call :exec1 C "%USRLOCAL%\bat\ruby"
call :exec1 C "%USRLOCAL%\bat\sh"
call :exec1 C "%USRLOCAL%\bat\vi"

call :exec1 G "%USRLOCAL%\bat\chrome"
call :exec1 G "%USRLOCAL%\bat\ckw"
call :exec1 G "%USRLOCAL%\bat\CmdPrompt"
call :exec1 G "%USRLOCAL%\bat\emacs"
call :exec1 G "%USRLOCAL%\bat\eval"
call :exec1 G "%USRLOCAL%\bat\firefoxExecutor"
call :exec1 G "%USRLOCAL%\bat\gv"
call :exec1 G "%USRLOCAL%\bat\gvim"
call :exec1 G "%USRLOCAL%\bat\MicrosoftAccess"
call :exec1 G "%USRLOCAL%\bat\MicrosoftExcel"
call :exec1 G "%USRLOCAL%\bat\MicrosoftOutlook"
call :exec1 G "%USRLOCAL%\bat\MicrosoftPowerPoint"
call :exec1 G "%USRLOCAL%\bat\MicrosoftWord"
call :exec1 G "%USRLOCAL%\bat\mplayer"
call :exec1 G "%USRLOCAL%\bat\mplayerc"
call :exec1 G "%USRLOCAL%\bat\opera"
call :exec1 G "%USRLOCAL%\bat\PdfViewer"
call :exec1 G "%USRLOCAL%\bat\ped"
call :exec1 G "%USRLOCAL%\bat\pedc"
call :exec1 G "%USRLOCAL%\bat\PuTTY"
call :exec1 G "%USRLOCAL%\bat\pythonw"
call :exec1 G "%USRLOCAL%\bat\rubyw"
call :exec1 G "%USRLOCAL%\bat\ViX"
call :exec1 G "%USRLOCAL%\bat\WinDVD"
call :exec1 G "%USRLOCAL%\bat\WinMerge"
call :exec1 G "%USRLOCAL%\bat\wperl"

popd
endlocal
echo [END]
pause>nul
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
