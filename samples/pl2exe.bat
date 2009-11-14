@rem = '--*-Perl-*--
@goto BATCH_BEGIN
[option]
arg = -x -- "${MY_ININAME}"${ARG}
use_path
[exec]
perl
[end]
:BATCH_BEGIN
@perl -x -- "%~f0" %*
@goto :eof
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

