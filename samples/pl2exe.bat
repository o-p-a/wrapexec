@rem vi:set ft=Perl ts=4 : -*- coding:US-ASCII mode:Perl -*-
@perl -x -- "%~dpn0.bat" %*
@goto :eof
[option]
arg = -x -- "${MY_ININAME}"${ARG}
use_path
[exec]
perl
[end]
#----------------------------------------------------------------
#! /usr/bin/perl
#line 14
#----------------------------------------------------------------
# Example for wrapexec. Replace your script below.
#----------------------------------------------------------------

sub main
{
	print "Hello world !! I am '" . $0 . "'\n";
	return 0;
}

exit main();

