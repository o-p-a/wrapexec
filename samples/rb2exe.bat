@rem = '--*-Ruby-*--
@goto BATCH_BEGIN
[option]
arg = -x -- "${MY_ININAME}"${ARG}
use_path
[exec]
ruby
[end]
:BATCH_BEGIN
@ruby -x -- "%~f0" %*
@goto :eof
--------------------------------------------------------
#! /usr/bin/ruby
# coding: US-ASCII

# Example for wrapexec. Replace your script here.

def main
	print "Hello world !! I am '#$0'\n"
	return 0
end

exit main()

