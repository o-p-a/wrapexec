@echo off
:
if not exist "wrapexec.exe" (
	echo %~n0: wrapexec.exe not found
	goto :eof
)
if not exist "wrapexecw.exe" (
	echo %~n0: wrapexecw.exe not found
	goto :eof
)
:
call :exec1 C "samples\bat2exe.exe"
call :exec1 C "samples\js2exe.exe"
call :exec1 C "samples\pl2exe.exe"
call :exec1 C "samples\rb2exe.exe"
call :exec1 C "samples\runbatch.exe"
call :exec1 C "samples\runjscript.exe"
call :exec1 C "samples\runperl.exe"
call :exec1 C "samples\runruby.exe"
:
call :exec1 G "\usrlocal\bat\CmdPrompt.exe"
call :exec1 G "\usrlocal\bat\emacs.exe"
call :exec1 G "\usrlocal\bat\eval.exe"
call :exec1 G "\usrlocal\bat\ExtractArchive.exe"
call :exec1 G "\usrlocal\bat\gv.exe"
call :exec1 G "\usrlocal\bat\ped.exe"
call :exec1 G "\usrlocal\bat\pedc.exe"
call :exec1 C "\usrlocal\bat\irb.exe"
call :exec1 C "\usrlocal\bat\perl.exe"
call :exec1 C "\usrlocal\bat\ruby.exe"
call :exec1 G "\usrlocal\bat\WinMerge.exe"
:
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
echo "%SRC%" -^> "%~2"
copy /y "%SRC%" "%~2"
