@rem vim:set ft=Ruby : -*- coding:US-ASCII mode:Ruby -*-
@ruby -x -- "%~dpn0.bat" %*
@goto :eof
[option]
arg = -x -- "${MY_ININAME}"${ARG}
use_path
[exec]
ruby
[end]
--------------------------------------------------------
#! /usr/bin/ruby
# coding: US-ASCII

# Example for wrapexec. Replace your script here.

def main
	print "Hello world !! I am '#$0'\n"
	return 0
end

exit main()

