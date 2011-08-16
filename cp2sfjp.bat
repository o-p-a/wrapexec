@pushd "%~dp0"
@setlocal
::
@set "TGT=sfjp"
@set "BASE=wrapexec"
::
copy "MAKEFILE.borland"		"%TGT%"
copy "%BASE%.cpp"		"%TGT%"
copy "%BASE%.ini"		"%TGT%"
copy "%BASE%.rc"		"%TGT%"
copy "%BASE%.txt"		"%TGT%"
copy "%BASE%1.ico"		"%TGT%"
copy "%BASE%2.ico"		"%TGT%"
copy "%BASE%3.ico"		"%TGT%"
copy "%BASE%w.ini"		"%TGT%"
::
@endlocal
@popd
pause
