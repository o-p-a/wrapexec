@if (0)==(0) echo off & goto BATCH_BEGIN
[option]
arg = //E:JScript //Nologo "${MY_ININAME}"${ARG}
use_path
[exec]
cscript
[end]
:BATCH_BEGIN
cscript //E:JScript //Nologo %0 %*
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

