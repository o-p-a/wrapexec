@if (0)==(0) goto BATCH_BEGIN && -*- mode:JavaScript; tab-width:4; coding:US-ASCII; -*-
@ vi:set ft=JavaScript ts=4 fenc=US-ASCII :
[option]
arg = //E:JScript //Nologo "${MY_ININAME}"${ARG}
use_path
[exec]
cscript
[end]
:BATCH_BEGIN
@ cscript //E:JScript //Nologo "%~dpn0.bat" %*
@ goto :eof
@end
//----------------------------------------------------------------
// Example for wrapexec. Replace your script below.
//----------------------------------------------------------------

function main()
{
	WScript.Echo("Hello world !! I am '" + WScript.ScriptFullName + "'");
	return 0;
}

WScript.Quit(main());

