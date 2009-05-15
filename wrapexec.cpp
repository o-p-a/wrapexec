/*
	Wrapper of Execute

	by opa
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <process.h>
#include <fcntl.h>

#include <windows.h>
#include <winnls.h>

#define PGM					"wrapexec"
#define PGM_DEBUG			PGM ": "
#define PGM_INFO			PGM ": "
#define PGM_WARN			PGM " warning: "
#define PGM_ERR				PGM " error: "
#define VERSTR				"0.01"

#define CREDIT2009			"Copyright (c) 2009 by opa"

typedef signed char schar;
typedef unsigned char uchar;
typedef signed int sint;
typedef unsigned int uint;
typedef signed long slong;
typedef unsigned long ulong;

using namespace std;

char
	credit[] = PGM " version " VERSTR " " CREDIT2009;
bool
	error = false;
sint
	rcode = 0;

////////////////////////////////////////////////////////////////////////

template <class BidirectionalIterator, class T>
BidirectionalIterator find_last(BidirectionalIterator first, BidirectionalIterator last, const T &value)
{
	while(first != last){
		--last;
		if(*last == value)
			break;
	}

	return last;
}

template <class BidirectionalIterator, class Predicate>
BidirectionalIterator find_last_if(BidirectionalIterator first, BidirectionalIterator last, Predicate pred)
{
	while(first != last){
		--last;
		if(pred(*last))
			break;
	}

	return last;
}

inline bool isnotwspace(wchar_t c)
{
	return !iswspace(c);
}

////////////////////////////////////////////////////////////////////////

class String : public wstring {
	typedef String Self;
	typedef wstring Super;
public:
	String()									{}
	String(const wstring &s)					: Super(s) {}
	String(const wchar_t *s)					: Super(s) {}
	String(const_iterator b, const_iterator e)	: Super(b, e) {}

	String to_upper() const;
	String to_wcout() const;
	String singlequote() const;
	bool isdoublequote() const;
	String doublequote() const;
	String doublequote_del() const;
	bool isbackslash() const;
	String backslash() const;
	String trim() const;
	String subext(const String &ext) const;
	String drivename() const;
	String dirname() const;
	String basename() const;

	String &assign_from_utf8(const string &s);
	String &assign_from_env(const String &name, const String &default_value = L"");

};

String String::to_upper() const
{
	Self
		r(*this);

	for(iterator i = r.begin() ; i != r.end() ; ++i)
		*i = towupper(*i);

	return r;
}

String String::to_wcout() const
{
	sint
		siz = WideCharToMultiByte(CP_ACP, 0, c_str(), size(), NULL, 0, NULL, NULL);
	char
		*buf = new char[siz];

	fill(buf, buf + siz, ' ');

	WideCharToMultiByte(CP_ACP, 0, c_str(), size(), buf, siz, NULL, NULL);

	sint
		wsiz = MultiByteToWideChar(CP_ACP, 0, buf, siz, NULL, 0);
	wchar_t
		*wbuf = new wchar_t[wsiz + 1];

	fill(wbuf, wbuf + wsiz, L' ');
	wbuf[wsiz] = L'\0';

	MultiByteToWideChar(CP_ACP, 0, buf, siz, wbuf, wsiz);

	String
		r(wbuf);

	delete [] wbuf;
	delete [] buf;

	return r;
}

//String String::singlequote() const
//{
//	Self
//		r;
//
//	r.reserve(size() + 2);
//	r.append(1, L'\'');
//	r.append(*this);
//	r.append(1, L'\'');
//
//	return r;
//}

bool String::isdoublequote() const
{
	// BUG:
	//  単に先頭と最後の文字が " かどうかを判定しているだけなので、文字列の途中に " があった場合などを考慮していない。

	if(size() >= 2 && *begin() == L'"' && *(end()-1) == L'"')
		return true;

	return false;
}

String String::doublequote() const
{
	Self
		r;

	if(isdoublequote()){
		r.append(*this);
	}else{
		r.reserve(size() + 2);
		r.append(1, L'"');
		r.append(*this);
		r.append(1, L'"');
	}

	return r;
}

String String::doublequote_del() const
{
	// BUG:
	//  単に先頭と最後の文字が " かどうかを判定しているだけなので、文字列の途中に " があった場合などを考慮していない。

	if(isdoublequote())
		return String(begin() + 1, end() - 1);
	else
		return String(*this);
}

bool String::isbackslash() const
{
	return size() > 0 && *(end()-1) == L'\\';
}

String String::backslash() const
{
	String
		r(*this);

	if(!isbackslash())
		r.append(1, L'\\');

	return r;
}

String String::trim() const
{
	const_iterator
		b = begin(),
		e = end();

	b = find_if(b, e, isnotwspace);
	e = find_last_if(b, e, isnotwspace);

	if(e != end() && isnotwspace(*e))
		++e;

	return Self(b, e);
}

String String::subext(const String &ext) const
{
	size_type
		period = rfind(L'.');

	if(period == npos)
		period = size();

	return Self(begin(), begin() + period).append(ext);
}

String String::drivename() const
{
	if(size() >= 2 && iswalpha((*this)[0]) && (*this)[1] == L':')
		return Self(begin(), begin() + 2);

	return Self();
}

String String::dirname() const
{
	const_iterator
		e = find_last(begin(), end(), L'\\');

	if(e != end() && *e == L'\\')
		++e;

	return Self(begin(), e);
}

String String::basename() const
{
	const_iterator
		b = begin(),
		e = end();

	b = find_last(b, e, L'\\');

	if(b != end() && *b == L'\\')
		++b;

	if((e = find_last(b, e, L'.')) == b)
		e = end();

	return Self(b, e);
}

String &String::assign_from_utf8(const string &s)
{
	sint
		size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
	wchar_t
		*buf = new wchar_t[size + 1];

	fill(buf, buf + size, L' ');
	buf[size] = L'\0';

	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buf, size);

	assign(buf);

	delete [] buf;

	return *this;
}

String &String::assign_from_env(const String &name, const String &default_value)
{
	wchar_t
		*g = _wgetenv(name.c_str());

	if(g)
		assign(g);
	else
		assign(default_value);

	return *this;
}

template<class T>
String to_String(T v)
{
	wostringstream
		os;

	os << v;

	return os.str();
}

////////////////////////////////////////////////////////////////////////

class ExpandValues : public map<String, String> {
	typedef ExpandValues Self;
	typedef map<String, String> Super;

private:
	static sint putenv(const key_type &name, const mapped_type &val);

public:
//	iterator find_i(const key_type &name);
	const_iterator find_i(const key_type &name) const;

	bool add(const key_type &name, const mapped_type &val);
//	bool contain(const key_type &name) const;

	void export_env();
	void export_env_clear();

};

ExpandValues
	expandvalues;

ExpandValues::const_iterator ExpandValues::find_i(const ExpandValues::key_type &name) const
{
	const key_type
		name_upper = name.to_upper();

	for(const_iterator i = begin() ; i != end() ; ++i)
		if(i->first.to_upper() == name_upper)
			return i;

	return end();
}

bool ExpandValues::add(const ExpandValues::key_type &name, const ExpandValues::mapped_type &val)
{
	return insert(pair<String, String>(name, val)).second;
}

sint ExpandValues::putenv(const ExpandValues::key_type &name, const ExpandValues::mapped_type &val)
{
	String
		envstr;

	envstr = name;
	envstr += L'=';
	envstr += val;

	return _wputenv(envstr.c_str());
}

void ExpandValues::export_env()
{
	for(iterator i = begin() ; i != end() ; ++i)
		putenv(L"WRAPEXEC_" + i->first, i->second);
}

void ExpandValues::export_env_clear()
{
	for(iterator i = begin() ; i != end() ; ++i)
		putenv(L"WRAPEXEC_" + i->first, L"");
}

////////////////////////////////////////////////////////////////////////

class IniFileStream : private ifstream {
	typedef IniFileStream Self;
	typedef ifstream Super;

private:
	String _inifilename;

	static String filename_to_ini(const String &exename);

	IniFileStream()						{}
	IniFileStream(int fd)				: Super(fd) {}

public:
	Super::is_open;
	Super::eof;
	Super::close;

	static IniFileStream *new_with_exename(const String &exename);

	const String &inifilename() const	{ return _inifilename; }
	bool is_comment(const String &s) const;
	bool is_section(const String &s) const;
	bool is_section(const String &s, const String &section_name) const;
	bool is_key(const String &s, const String &key_name) const;
	bool get_value_bool(const String &s) const;
	String get_value_String(const String &s) const;
	bool skip_bom();
	bool getline(String &s);

};

String IniFileStream::filename_to_ini(const String &exename)
{
	return exename.subext(L".ini");
}

IniFileStream *IniFileStream::new_with_exename(const String &exename)
{
	Self
		*r;
	String
		ini = filename_to_ini(exename);
	sint
		fd = _wopen(ini.c_str(), O_RDONLY + O_TEXT);

	if(fd < 0){
		r = new Self;
		r->_inifilename = ini;
	}else{
		r = new Self(fd);
		r->_inifilename = ini;
		r->skip_bom();
	}

	return r;
}

bool IniFileStream::is_comment(const String &s) const
{
	return s.size()==0 || s[0]==L'#';
}

bool IniFileStream::is_section(const String &s) const
{
	if(s.size() >= 2 && s[0] == L'[' && s[s.size()-1] == L']')
		return true;

	return false;
}

bool IniFileStream::is_section(const String &s, const String &section_name) const
{
	if(!is_section(s))
		return false;

	return String(s.begin() + 1, s.end() - 1).trim().to_upper() == section_name;
}

bool IniFileStream::is_key(const String &s, const String &key_name) const
{
	String::const_iterator
		f = find(s.begin(), s.end(), L'=');
	String
		key_c(s.begin(), f);

	return key_c.trim().trim().to_upper() == key_name;
}

bool IniFileStream::get_value_bool(const String &s) const
{
	String::const_iterator
		f = find(s.begin(), s.end(), L'=');

	if(f == s.end())
		return true; // 「=」がない → キーのみで値の記述なし → trueを返す

	String
		value(f + 1, s.end());
	bool
		r = false;

	value = value.trim().to_upper();

	if(value == L"TRUE" || value == L"YES" || value == L"ON"){
		r = true;
	}else if(value == L"FALSE" || value == L"NO" || value == L"OFF"){
		r = false;
	}else{
		wcerr << PGM_ERR "invalid value: " << value.to_wcout() << endl;
		error = true;
		rcode = -1;
	}

	return r;
}

String IniFileStream::get_value_String(const String &s) const
{
	String::const_iterator
		f = find(s.begin(), s.end(), L'=');

	if(f == s.end())
		return String(); // 「=」がない → 空文字列を返す

	return String(f + 1, s.end());
}

bool IniFileStream::skip_bom()
{
	// BUG:
	//  fe fe ... のようなファイルの場合、二つ目のfeしかunget()されない

	char
		c;

#if 1 // for UTF8
	if(!eof()){
		get(c);
		if(c == '\xef'){
			if(!eof()){
				get(c);
				if(c == '\xbb'){
					if(!eof()){
						get(c);
						if(c == '\xbf')
							return true;
					}
				}
			}
		}
		unget();
	}
#else // for UTF-16
	if(!eof()){
		get(c);
		if(c == '\xfe'){
			if(!eof()){
				get(c);
				if(c == '\xff')
					return true;
			}
		}else if(c == '\xff'){
			if(!eof()){
				get(c);
				if(c == '\xfe')
					return true;
			}
		}
		unget();
	}
#endif

	return false;
}

bool IniFileStream::getline(String &s)
{
	string
		tmp;

	s.clear();

	if(!good())
		return false;

	::getline(*this, tmp);
	s.assign_from_utf8(tmp);
	s = s.trim();

	return true;
}

////////////////////////////////////////////////////////////////////////

class exstring : public String {
	typedef exstring Self;
	typedef String Super;
private:
	const ExpandValues &ev() const		{ return expandvalues; }
public:
	exstring()							{}
	exstring(const wstring &s)			: Super(s) {}

	String expand() const;
};

typedef vector<exstring>
	exstringList;

String exstring::expand() const
{
	String
		r, expand_var_name;
	const_iterator
		i, j;
	ExpandValues::const_iterator
		f;
	wchar_t
		c, c1, c2, cc;

	for(i = begin() ; i != end() ; ++i){
		c = *i;
		if(c == L'$'){
			++i;
			if(i == end()){
				r.append(1, c);
				goto BREAK;
			}

			c1 = *i;
			if(c1 == L'(' || c1 == L'{' || c1 == L'['){
				if(c1 == L'(')
					cc = L')';
				else if(c1 == L'{')
					cc = L'}';
				else if(c1 == L'[')
					cc = L']';
				else
					cc = L'\0';

				++i;
				if(i == end()){
					r.append(1, c);
					r.append(1, c1);
					goto BREAK;
				}

				expand_var_name.erase();
				while(1){
					c2 = *i;
					if(c2 == cc){
						if(expand_var_name.empty()){
							wcerr << PGM_WARN "variable name not presented" << endl;
						}else if((f=ev().find_i(expand_var_name)) != ev().end()){
							r.append(f->second);
						}else{
							wcerr << PGM_WARN "variable not defined: " << expand_var_name.to_wcout() << endl;
							r.append(1, c);
							r.append(1, c1);
							r.append(expand_var_name);
							r.append(1, c2);
						}
						break;
					}

					expand_var_name.append(1, c2);
					++i;
					if(i == end()){
						r.append(1, c);
						r.append(1, c1);
						r.append(expand_var_name);
						goto BREAK;
					}
				}
			}else if(c1 == L'$'){
				r.append(1, c1);
			}else{
				r.append(1, c);
				r.append(1, c1);
			}
		}else{
			r.append(1, c);
		}
	}
BREAK:;

	return r;
}

////////////////////////////////////////////////////////////////////////

class ExecuteInfo {
	typedef ExecuteInfo Self;

private:
	exstringList
		_exstringList;
	exstring
		_arg,
		_chdir;
	bool
		_verbose,
		_internal_cmd,
		_gui,
		_maximize,
		_minimize,
		_import_env_all,
		_export_env_all;

	static sint system(const String &s);
	static bool executable(const String &cmd);

public:
	ExecuteInfo();

	exstringList &eslist()						{ return _exstringList; }
	const exstringList &eslist() const			{ return _exstringList; }
	const exstring &arg() const					{ return _arg; }
	const exstring &arg(const exstring &v)		{ return _arg = v; }
	const exstring &chdir() const				{ return _chdir; }
	const exstring &chdir(const exstring &v)	{ return _chdir = v; }
	bool verbose() const						{ return _verbose; }
	bool verbose(bool v)						{ return _verbose = v; }
	bool internal_cmd() const					{ return _internal_cmd; }
	bool internal_cmd(bool v)					{ return _internal_cmd = v; }
	bool gui() const							{ return _gui; }
	bool gui(bool v)							{ return _gui = v; }
	bool maximize() const						{ return _maximize; }
	bool maximize(bool v)						{ return _maximize = v; }
	bool minimize() const						{ return _minimize; }
	bool minimize(bool v)						{ return _minimize = v; }
	bool import_env_all() const					{ return _import_env_all; }
	bool import_env_all(bool v)					{ return _import_env_all = v; }
	bool export_env_all() const					{ return _export_env_all; }
	bool export_env_all(bool v)					{ return _export_env_all = v; }

	sint execute();
	void verbose_out(const String &s);

};

typedef vector<ExecuteInfo>
	ExecuteInfoList;

ExecuteInfo
	execinfo_default;
ExecuteInfoList
	execinfolist;

ExecuteInfo::ExecuteInfo()
{
	_exstringList.clear();
	_arg			= exstring(L"${ARGV_ALL}");
	_chdir			= exstring(L"");
	_verbose		= false;
	_internal_cmd	= false;
	_gui			= false;
	_maximize		= false;
	_minimize		= false;
	_import_env_all	= false;
	_export_env_all	= false;
}

sint ExecuteInfo::system(const String &cmd)
{
	String
		r;
	bool
		in_quote = false;

	r.reserve(cmd.size());

	for(String::const_iterator i = cmd.begin() ; i != cmd.end() ; ++i ){
		wchar_t
			c = *i;

		if(!in_quote){
			if(c == L'^' || c == L'<' || c == L'>' || c == L'|' || c == L'&' || c == L'(' || c == L')' || c == L'@'){
				r.append(1, L'^');
				r.append(1, c);
			}else{
				r.append(1, c);
			}
		}else{
			r.append(1, c);
		}

		if(c == L'"')
			in_quote = in_quote ? false : true;
	}

	return _wsystem(r.c_str());
}

bool ExecuteInfo::executable(const String &cmd)
{
	FILE
		*fp = _wfopen(cmd.c_str(), L"rb");

	fclose(fp);

	return fp!=NULL;
}

sint ExecuteInfo::execute()
{
	String
		cmd,
		arge,
		cl;
	bool
		done = false;

	verbose_out(L"--- execute ---");

	if(verbose()){
		verbose_out(L"ExecuteInfo.arg: " + arg());
		verbose_out(L"ExecuteInfo.chdir: " + chdir());

		for(ExpandValues::const_iterator i = expandvalues.begin() ; i != expandvalues.end() ; ++i)
			verbose_out(L"expandvalue: ${" + i->first + L"}=" + i->second);
		for(exstringList::const_iterator i = eslist().begin() ; i != eslist().end() ; ++i)
			verbose_out(L"exstring: " + *i);
	}

	if(eslist().size() <= 0){
		wcerr << PGM_ERR "command not defined" << endl;
		error = true;
		rcode = -2;
		return 1;
	}

	if(export_env_all()){
		verbose_out(L"export all variable to environment");
		expandvalues.export_env();
	}

	for(exstringList::const_iterator i = eslist().begin() ; i != eslist().end() ; ++i){
		cl.erase();
		cmd = i->expand();
		arge = arg().expand();
		if(arge.size() > 0 && !iswspace(arge[0]))
			arge = L' ' + arge; // 空白がないとコマンド名とくっついてしまうのでスペースを補う

		verbose_out(L"try to exec: " + cmd);

		if(chdir().size() > 0)
			cl = L"pushd \"" + chdir().expand() + L"\" && ";

		if(internal_cmd()){
			verbose_out(L"internal command");
			cl = cmd + arge;
		}else if(executable(cmd)){
			verbose_out(L"executable");
			if(gui()){
				cl += exstring(L"start \"${MY_BASENAME}\" ").expand();
				if(maximize())
					cl += L"/max ";
				if(minimize())
					cl += L"/min ";
			}
			cl += cmd.doublequote() + arge;
		}else{
			verbose_out(L"unexecutable: skip");
			cl.erase();
		}

		if(cl.size() > 0){
			verbose_out(L"execute: " + cl);

			rcode = system(cl);
			done = true;

			verbose_out(L"done: " + to_String(rcode));

			break;
		}
	}

	if(export_env_all()){
		verbose_out(L"clear exported environment variable");
		expandvalues.export_env_clear();
	}

	if(!done){
#if 0
		wcerr << "command not found" << endl;
		rcode = 1;
#else
		rcode = system(exstring(L"${MY_BASENAME}\"\b").expand());
#endif
		error = true;
		return 1;
	}

	return 0;
}

void ExecuteInfo::verbose_out(const String &s)
{
	if(verbose())
		wcout << PGM_DEBUG << s.to_wcout() << endl;
}

////////////////////////////////////////////////////////////////////////

String get_given_option(const String &cmd)
{
	bool
		in_quote = false;

	for(String::const_iterator i = cmd.begin() ; i != cmd.end() ; ++i){
		wchar_t
			c = *i;

		if(!in_quote)
			if(iswspace(c))
				return String(i, cmd.end());

		if(c == L'"')
			in_quote = in_quote ? false : true;
	}

	return String();
}

void do_help()
{
	wcout << credit << endl;
	wcout << endl;

	wcout << "available sections:" << endl;
	wcout << " [option]" << endl;
	wcout << " [exec]" << endl;
//	wcout << " [multi]" << endl;
	wcout << endl;

	wcout << "available options:" << endl;
	wcout << " help" << endl;
	wcout << " arg" << endl;
	wcout << " chdir" << endl;
	wcout << " verbose" << endl;
	wcout << " internal_cmd" << endl;
	wcout << " gui" << endl;
	wcout << " maximize" << endl;
	wcout << " minimize" << endl;
//	wcout << " import_env_all" << endl;
	wcout << " export_env_all" << endl;
	wcout << endl;

	wcout << "available variables:" << endl;
	for(ExpandValues::const_iterator i = expandvalues.begin() ; i != expandvalues.end() ; ++i)
		wcout << " ${" << i->first << '}' << endl;
}

String get_clipboard()
{
	String
		r;
	HANDLE
		h;

	if(OpenClipboard(NULL) == 0)
		return String();

	if((h = GetClipboardData(CF_UNICODETEXT)) != NULL){
		r.assign((wchar_t *)GlobalLock(h));
		GlobalUnlock(h);
	}

	if(CloseClipboard() == 0)
		return String();

	// 改行文字等はスペースに置換する
	for(String::iterator i = r.begin() ; i != r.end() ; ++i)
		if(iswspace(*i))
			*i = L' ';

	return r;
}

sint setup_expandvalues(String exename, String given_option)
{
	ExpandValues
		&ev = expandvalues;
	String
		s;

	ev.add(L"ARGV_ALL", given_option);
	ev.add(L"CLIPBOARD", get_clipboard());
	ev.add(L"MY_FULLNAME", exename);
	ev.add(L"MY_DRIVE", exename.drivename());
	ev.add(L"MY_DIR", exename.dirname());
	ev.add(L"MY_BASENAME", exename.basename());

//	s.assign_from_env(L"OS");					// Windows_NT
//	ev.add(L"OS", s);
	s.assign_from_env(L"COMPUTERNAME");			// LYSINE
	ev.add(L"COMPUTERNAME", s);
	s.assign_from_env(L"SystemDrive");			// C:
	ev.add(L"SystemDrive", s);
	s.assign_from_env(L"SystemRoot");			// C:\WINDOWS
	ev.add(L"SystemRoot", s.backslash());
	s.assign_from_env(L"ProgramFiles");			// C:\Program Files
	ev.add(L"ProgramFiles", s.backslash());
	s.assign_from_env(L"ALLUSERSPROFILE");		// C:\Documents and Settings\All Users
	ev.add(L"ALLUSERSPROFILE", s.backslash());

	s.assign_from_env(L"USERNAME");				// someone
	ev.add(L"USERNAME", s);
	s.assign_from_env(L"HOMEDRIVE");			// C:
	ev.add(L"HOMEDRIVE", s);
//	s.assign_from_env(L"HOMEPATH");				// \Documents and Settings\someone
//	ev.add(L"HOMEPATH", s.backslash());
	s.assign_from_env(L"USERPROFILE");			// C:\Documents and Settings\someone
	ev.add(L"USERPROFILE", s.backslash());
	s.assign_from_env(L"APPDATA");				// C:\Documents and Settings\someone\Application Data
	ev.add(L"APPDATA", s.backslash());
	s.assign_from_env(L"TMP");					// C:\DOCUME~1\SOMEONE\LOCALS~1\Temp
	ev.add(L"TMP", s.backslash());

	return 0;
}

sint load_inifile(String exename)
{
	enum section_id{
		SECTION_NONE,
		SECTION_OPTION,
		SECTION_EXEC,
		SECTION_MULTIEXEC,
	};

	execinfo_default.verbose_out(L"--- ini file read ---");

	IniFileStream
		*ifs = IniFileStream::new_with_exename(exename);
	String
		aline;
	section_id
		section;

	execinfo_default.verbose_out(L"ini filename: " + ifs->inifilename());

	if(!ifs->is_open()){
		wcerr << PGM_ERR "ini file open filed: " << ifs->inifilename().to_wcout() << endl;
		delete ifs;
		error = true;
		rcode = -3;
		return 1;
	}

	section = SECTION_EXEC;
	execinfolist.clear();
	execinfolist.push_back(execinfo_default);

	while(ifs->getline(aline)){
		if(ifs->is_comment(aline)){
			; // コメントor空行なのでスキップ
		}else{
			if(ifs->is_section(aline)){
				if(ifs->is_section(aline, L"OPTION")){
					execinfo_default.verbose_out(L"ini section: option");
					section = SECTION_OPTION;
				}else if(ifs->is_section(aline, L"EXEC")){
					execinfo_default.verbose_out(L"ini section: exec");
					section = SECTION_EXEC;
					execinfolist.clear();
					execinfolist.push_back(execinfo_default);
				}else if(ifs->is_section(aline, L"MULTI")){
					execinfo_default.verbose_out(L"ini section: multi");
					section = SECTION_MULTIEXEC;
					execinfolist.push_back(execinfo_default);
				}else{
					wcerr << PGM_ERR "invalid section name: " << aline.to_wcout() << endl;
					section = SECTION_NONE;
					error = true;
					rcode = -4;
				}
			}else{
				switch(section){
				case SECTION_OPTION:	// execinfo_default に対するオプション設定
					if(ifs->is_key(aline, L"HELP")){
						execinfo_default.verbose_out(L"ini option: help");
						do_help();
						error = true;	// execフェーズを実行しない
						rcode = 0;		// がエラーではないのでrcodeは0を返す
					}else if(ifs->is_key(aline, L"ARG")){
						execinfo_default.verbose_out(L"ini option: arg");
						execinfo_default.arg(ifs->get_value_String(aline));
					}else if(ifs->is_key(aline, L"CHDIR")){
						execinfo_default.verbose_out(L"ini option: chdir");
						execinfo_default.chdir(ifs->get_value_String(aline).trim().doublequote_del());
					}else if(ifs->is_key(aline, L"VERBOSE")){
						execinfo_default.verbose_out(L"ini option: verbose");
						execinfo_default.verbose(ifs->get_value_bool(aline));
					}else if(ifs->is_key(aline, L"INTERNAL_CMD")){
						execinfo_default.verbose_out(L"ini option: internal_cmd");
						execinfo_default.internal_cmd(ifs->get_value_bool(aline));
					}else if(ifs->is_key(aline, L"GUI")){
						execinfo_default.verbose_out(L"ini option: gui");
						execinfo_default.gui(ifs->get_value_bool(aline));
					}else if(ifs->is_key(aline, L"MAXIMIZE")){
						execinfo_default.verbose_out(L"ini option: maximize");
						execinfo_default.maximize(ifs->get_value_bool(aline));
						execinfo_default.minimize(false);
					}else if(ifs->is_key(aline, L"MINIMIZE")){
						execinfo_default.verbose_out(L"ini option: minimize");
						execinfo_default.minimize(ifs->get_value_bool(aline));
						execinfo_default.maximize(false);
					}else if(ifs->is_key(aline, L"IMPORT_ENV_ALL")){
						execinfo_default.verbose_out(L"ini option: import_env_all");
						execinfo_default.import_env_all(ifs->get_value_bool(aline));
					}else if(ifs->is_key(aline, L"EXPORT_ENV_ALL")){
						execinfo_default.verbose_out(L"ini option: export_env_all");
						execinfo_default.export_env_all(ifs->get_value_bool(aline));
					}else{
						wcerr << PGM_ERR "invalid option: " << aline.to_wcout() << endl;
						error = true;
						rcode = -5;
					}
					break;
				case SECTION_EXEC:
					execinfo_default.verbose_out(L"ini exec: " + aline);
					execinfolist[0].eslist().push_back(aline.doublequote_del());
					break;
				default:
					wcerr << PGM_WARN "ignored: " << aline.to_wcout() << endl;
					break;
				}
			}
		}
	}

	ifs->close();
	delete ifs;

	return 0;
}

sint wmain(sint /*ac*/, wchar_t *av[])
{
	String
		exename = av[0],
		given_option = get_given_option(GetCommandLine());

	locale::global(locale(""));
//	wcout.imbue(locale(""));
//	wcerr.imbue(locale(""));

	setup_expandvalues(exename, given_option);

	load_inifile(exename);

	for(ExecuteInfoList::iterator i = execinfolist.begin() ; !error && i != execinfolist.end() ; ++i)
		i->execute();

	return rcode;
}

