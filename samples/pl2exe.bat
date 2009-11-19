@rem vim:set ft=Perl : -*- coding:US-ASCII mode:Perl -*-
@perl -x -- "%~f0" %*
@goto :eof
[option]
arg = -x -- "${MY_ININAME}"${ARG}
use_path
[exec]
perl
[end]
--------------------------------------------------------
#! /usr/bin/perl
#line 14

# Example for wrapexec. Replace your script here.

sub main
{
	print "Hello world !! I am '" . $0 . "'\n";
	return 0;
}

exit main();

