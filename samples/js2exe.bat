@if(0)==(0) echo off & goto BATCH_BEGIN
[option]
arg = //E:JScript //Nologo "${MY_ININAME}"${ARG}
use_path
[exec]
cscript
[end]
:BATCH_BEGIN
if %~d0 == ~d0 goto OLD_CMD
cscript //E:JScript //Nologo "%~f0" %*
goto :eof
:OLD_CMD
echo Cannot determine script path. Run script directly.
goto :eof
@end
//------------------------------------------------------

// Example for wrapexec. Replace your script here.

function main()
{
	WScript.Echo("Hello world !! I am '" + WScript.ScriptFullName + "'");
	return 0;
}

WScript.Quit(main());

//:BATCH_END
