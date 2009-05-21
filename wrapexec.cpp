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
#include <shlobj.h>
#include <lmcons.h>

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

	String &operator=(const wchar_t *s)			{ assign(s); return *this; }
	String &operator=(const String &s)			{ assign(s); return *this; }
	String &assign_from_utf8(const string &s);
	String &assign_from_env(const String &name);
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

String &String::assign_from_env(const String &name)
{
	wchar_t
		*g = _wgetenv(name.c_str());

	if(g)
		assign(g);
	else
		clear();

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

class case_ignore_less : public binary_function<String, String, bool>
{
public:
	typedef binary_function<String, String, bool>::first_argument_type first_argument_type;
	typedef binary_function<String, String, bool>::second_argument_type second_argument_type;
	typedef binary_function<String, String, bool>::result_type result_type;
	bool operator() (const String &x, const String &y) const { return x.to_upper() < y.to_upper(); }
};

////////////////////////////////////////////////////////////////////////

class ExpandValues : public map<String, String, case_ignore_less> {
	typedef ExpandValues Self;
	typedef map<String, String> Super;

public:
	mapped_type add(const key_type &name, const mapped_type &val);
	mapped_type get(const key_type &name) const;
};

ExpandValues::mapped_type ExpandValues::add(const ExpandValues::key_type &name, const ExpandValues::mapped_type &val)
{
	erase(name); // nameも更新するために、一旦消す

	return operator[](name) = val;
}

ExpandValues::mapped_type ExpandValues::get(const ExpandValues::key_type &name) const
{
	const_iterator
		i = find(name);

	if(i == end())
		return String();

	return i->second;
}

////////////////////////////////////////////////////////////////////////

class IniFileStream : private ifstream {
	typedef IniFileStream Self;
	typedef ifstream Super;

private:
	String _inifilename;

	IniFileStream()						{}
	IniFileStream(int fd)				: Super(fd) {}

public:
	Super::is_open;
	Super::eof;
	Super::close;

	static String ininame(const String &exename);
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

String IniFileStream::ininame(const String &exename)
{
	return exename.subext(L".ini");
}

IniFileStream *IniFileStream::new_with_exename(const String &exename)
{
	Self
		*r;
	String
		ini = ininame(exename);
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
	return s.size()==0 || s[0]==L'#' || s[0]==L';';
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
					return true; // BE
			}
		}else if(c == '\xff'){
			if(!eof()){
				get(c);
				if(c == '\xfe')
					return true; // LE
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

class ExString : public String {
	typedef ExString Self;
	typedef String Super;

public:
	ExString()							{}
	ExString(const wstring &s)			: Super(s) {}

	String expand(const ExpandValues &ev) const;
	Self &operator=(const wchar_t *s)	{ assign(s); return *this; }
	Self &operator=(const Self &s)		{ assign(s); return *this; }
};

typedef vector<ExString>
	ExStringList;

String ExString::expand(const ExpandValues &ev) const
{
	String
		r,
		evname;
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

				evname.erase();
				while(1){
					c2 = *i;
					if(c2 == cc){
						if(evname.empty()){
							wcerr << PGM_WARN "variable name not presented" << endl;
						}else if((f=ev.find(evname)) != ev.end()){
							r.append(f->second);
						}else{
							wcerr << PGM_WARN "variable not defined: " << evname.to_wcout() << endl;
							r.append(1, c);
							r.append(1, c1);
							r.append(evname);
							r.append(1, c2);
						}
						break;
					}else{
						evname.append(1, c2);
					}

					++i;
					if(i == end()){
						r.append(1, c);
						r.append(1, c1);
						r.append(evname);
						goto BREAK;
					}
				}
			}else if(c1 == L'$'){
				r.append(1, c);
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
	typedef vector<String> EnvnameList;

private:
	ExpandValues
		_expandValues;
	ExStringList
		_exStringList;
	EnvnameList
		_import_env,
		_export_env;
	ExString
		_arg,
		_chdir;
	bool
		_verbose,
		_internal_cmd,
		_gui,
		_wait,
		_maximize,
		_minimize;

	static sint system(const String &cmd);
	static String getenv(const String &name);
	static sint putenv(const String &name, const String &val);
	static String system_escape(const String &s);
	static bool executable(const String &cmd);

public:
	ExecuteInfo();

	const ExpandValues &ev() const						{ return _expandValues; }
	String ev(const ExpandValues::key_type &name) const	{ return _expandValues.get(name); }
	const ExStringList &exs() const						{ return _exStringList; }
	const EnvnameList &import_env() const				{ return _import_env; }
	const EnvnameList &export_env() const				{ return _export_env; }
	const ExString &arg() const							{ return _arg; }
	const ExString &chdir() const						{ return _chdir; }
	bool verbose() const								{ return _verbose; }
	bool internal_cmd() const							{ return _internal_cmd; }
	bool gui() const									{ return _gui; }
	bool wait() const									{ return _wait; }
	bool maximize() const								{ return _maximize; }
	bool minimize() const								{ return _minimize; }

	void add_ev(const ExpandValues::key_type &name, const ExpandValues::mapped_type &val);
	void add_exs(const String &s);
	void add_import_env(const String &s);
	void add_export_env(const String &s);
	ExString &set_arg(const String &s);
	ExString &set_chdir(const String &s);
	bool verbose(bool v)								{ return _verbose = v; }
	bool internal_cmd(bool v)							{ return _internal_cmd = v; }
	bool gui(bool v)									{ return _gui = v; }
	bool wait(bool v)									{ return _wait = v; }
	bool maximize(bool v)								{ return _maximize = v; }
	bool minimize(bool v)								{ return _minimize = v; }

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
	_expandValues.clear();
	_exStringList.clear();
	_import_env.clear();
	_export_env.clear();
	_arg			= L"${ARG}";
	_chdir			= L"";
	_verbose		= false;
	_internal_cmd	= false;
	_gui			= false;
	_wait			= false;
	_maximize		= false;
	_minimize		= false;
}

sint ExecuteInfo::system(const String &cmd)
{
	return _wsystem(cmd.c_str());
}

String ExecuteInfo::getenv(const String &name)
{
	return String().assign_from_env(name);
}

sint ExecuteInfo::putenv(const String &name, const String &val)
{
	return _wputenv((name + L'=' + val).c_str());
}

String ExecuteInfo::system_escape(const String &s)
{
	String
		r;
	bool
		in_quote = false;

	r.reserve(s.size());

	for(String::const_iterator i = s.begin() ; i != s.end() ; ++i){
		wchar_t
			c = *i;

		if(!in_quote)
			if(c == L'^' || c == L'<' || c == L'>' || c == L'|' || c == L'&' || c == L'(' || c == L')' || c == L'@')
				r.append(1, L'^');

		r.append(1, c);

		if(c == L'"')
			in_quote = in_quote ? false : true;
	}

	return r;
}

bool ExecuteInfo::executable(const String &cmd)
{
	FILE
		*fp = _wfopen(cmd.c_str(), L"rb");

	fclose(fp);

	return fp!=NULL;
}

void ExecuteInfo::add_ev(const ExpandValues::key_type &name, const ExpandValues::mapped_type &val)
{
	_expandValues.add(name, val);
}

void ExecuteInfo::add_exs(const String &s)
{
	_exStringList.push_back(s.doublequote_del());
}

void ExecuteInfo::add_import_env(const String &s)
{
	_import_env.push_back(s.trim());
}

void ExecuteInfo::add_export_env(const String &s)
{
	_export_env.push_back(s.trim());
}

ExString &ExecuteInfo::set_arg(const String &s)
{
	return _arg = s;
}

ExString &ExecuteInfo::set_chdir(const String &s)
{
	return _chdir = s.trim().doublequote_del();
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

	for(EnvnameList::const_iterator i = import_env().begin() ; i != import_env().end() ; ++i)
		add_ev(*i, getenv(*i));

	if(verbose()){
		verbose_out(L"ExecuteInfo.arg: " + arg());
		verbose_out(L"ExecuteInfo.chdir: " + chdir());

		for(EnvnameList::const_iterator i = import_env().begin() ; i != import_env().end() ; ++i)
			verbose_out(L"import_env: " + *i);
		for(EnvnameList::const_iterator i = export_env().begin() ; i != export_env().end() ; ++i)
			verbose_out(L"export_env: " + *i);
		for(ExpandValues::const_iterator i = ev().begin() ; i != ev().end() ; ++i)
			verbose_out(L"expandvalue: ${" + i->first + L"}=" + i->second);
		for(ExStringList::const_iterator i = exs().begin() ; i != exs().end() ; ++i)
			verbose_out(L"exstring: " + *i);
	}

	if(exs().size() <= 0){
		wcerr << PGM_ERR "command not defined" << endl;
		error = true;
		rcode = -2;
		return 1;
	}

	for(EnvnameList::const_iterator i = export_env().begin() ; i != export_env().end() ; ++i){
		ExpandValues::const_iterator f = ev().find(*i);
		if(f != ev().end())
			putenv(L"WRAPEXEC_" + f->first, f->second);
		else
			wcerr << PGM_WARN "variable not defined: " << i->to_wcout() << endl;
	}

	for(ExStringList::const_iterator i = exs().begin() ; i != exs().end() ; ++i){
		cl.erase();
		cmd = i->expand(ev());
		arge = system_escape(arg().expand(ev()));
		if(arge.size() > 0 && !iswspace(arge[0]))
			arge = L' ' + arge; // 空白がないとコマンド名とくっついてしまうのでスペースを補う

		verbose_out(L"try to exec: " + cmd);

		if(chdir().size() > 0)
			cl += L"pushd \"" + chdir().expand(ev()) + L"\" && ";

		if(internal_cmd()){
			verbose_out(L"internal command");
			cl = cmd + arge;
		}else if(executable(cmd)){
			verbose_out(L"executable");
			if(gui()){
				cl += L"start \"" + ev(L"MY_BASENAME") + L"\" ";
				if(wait())
					cl += L"/wait ";
				if(maximize())
					cl += L"/max ";
				if(minimize())
					cl += L"/min ";
			}
			cl += cmd.doublequote() + arge;
		}else{
			verbose_out(L"unexecutable: skip");
			cl.erase(); // for safe
		}

		if(cl.size() > 0){
			verbose_out(L"execute: " + cl);

			rcode = system(L'"' + cl + L'"');
			done = true;

			verbose_out(L"done: " + to_String(rcode));

			break;
		}
	}

	// BUG: 「元に戻す」のではなく「クリアする」ということをしている
	for(EnvnameList::const_iterator i = export_env().begin() ; i != export_env().end() ; ++i){
		ExpandValues::const_iterator f = ev().find(*i);
		if(f != ev().end())
			putenv(L"WRAPEXEC_" + f->first, L"");
	}

	if(!done){
#if 1
		rcode = 1;
		wcerr << L"'" + system_escape(ev(L"MY_BASENAME")) + L"' is not recognized as an internal or external command," << endl
				<< "operable program or batch file." << endl;
#else
		rcode = system(system_escape(ev(L"MY_BASENAME")) + L"\"\b");
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

class WindowsAPI {
public:
	static String GetClipboardText();
	static String GetCommandLine()		{ return ::GetCommandLine(); }
	static String GetComputerName();
	static String GetTempPath();
	static String GetUserName();
	static String SHGetSpecialFolder(sint nFolder);
};

String WindowsAPI::GetClipboardText()
{
	String
		r;
	HANDLE
		h;

	if(::OpenClipboard(NULL) == 0)
		return String();

	if((h = ::GetClipboardData(CF_UNICODETEXT)) != NULL){
		r.assign((wchar_t *)::GlobalLock(h));
		::GlobalUnlock(h);
	}

	if(::CloseClipboard() == 0)
		return String();

	// 改行文字等はスペースに置換する
	for(String::iterator i = r.begin() ; i != r.end() ; ++i)
		if(iswspace(*i))
			*i = L' ';

	return r;
}

String WindowsAPI::GetComputerName()
{
	wchar_t
		buf[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD
		size = sizeof buf;

	if(::GetComputerName(buf, &size) == 0)
		return String();

	return String(buf, buf + size);
}

String WindowsAPI::GetTempPath()
{
	DWORD
		size = ::GetTempPath(0, NULL);
	wchar_t
		*buf = new wchar_t[size];

	::GetTempPath(size, buf);

	String
		r(buf);

	delete [] buf;

	return r;
}

String WindowsAPI::GetUserName()
{
	wchar_t
		buf[UNLEN + 1];
	DWORD
		size = sizeof buf;

	if(::GetUserName(buf, &size) == 0)
		return String();

	return String(buf, buf + size - 1);
}

String WindowsAPI::SHGetSpecialFolder(sint nFolder)
{
	wchar_t
		buf[MAX_PATH];
	LPITEMIDLIST
		pidl;
	IMalloc
		*m;

	SHGetMalloc(&m);
	if(SHGetSpecialFolderLocation(0, nFolder, &pidl) == 0){
		SHGetPathFromIDList(pidl, buf);
		m->Free(pidl);
	}
	m->Release();

	return String(buf);
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
	wcout << " [multi]" << endl;
	wcout << " [eof]" << endl;
	wcout << endl;

	wcout << "available options:" << endl;
	wcout << " help" << endl;
	wcout << " arg" << endl;
	wcout << " chdir" << endl;
	wcout << " import_env" << endl;
	wcout << " export_env" << endl;
	wcout << " verbose" << endl;
	wcout << " internal_cmd" << endl;
	wcout << " gui" << endl;
	wcout << " wait" << endl;
	wcout << " maximize" << endl;
	wcout << " minimize" << endl;
	wcout << endl;

	wcout << "available variables:" << endl;
	for(ExpandValues::const_iterator i = execinfo_default.ev().begin() ; i != execinfo_default.ev().end() ; ++i)
		wcout << " ${" << i->first << '}' << endl;
}

sint setup_expandvalues(ExecuteInfo &ei,String exename)
{
	String
		s;

	ei.add_ev(L"ARG", get_given_option(WindowsAPI::GetCommandLine()));
	ei.add_ev(L"CLIPBOARD", WindowsAPI::GetClipboardText());

	s = exename;
	ei.add_ev(L"MY_EXENAME", s);
	ei.add_ev(L"MY_ININAME", IniFileStream::ininame(s));
	ei.add_ev(L"MY_BASENAME", s.basename());
	ei.add_ev(L"MY_DRIVE", s.drivename());
	ei.add_ev(L"MY_DIR", s.dirname());
	ei.add_ev(L"MY", s.dirname());

	s = WindowsAPI::GetComputerName();										// SOMEONESPC
	ei.add_ev(L"SYS_NAME", s);
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_WINDOWS);						// C:\WINDOWS
	ei.add_ev(L"SYS_ROOT", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_SYSTEM);						// C:\WINDOWS\system32
	ei.add_ev(L"SYS_DRIVE", s.drivename());
	ei.add_ev(L"SYS_DIR", s.backslash());
	ei.add_ev(L"SYS", s.backslash());

	s = WindowsAPI::GetUserName();											// someone
	ei.add_ev(L"USER_NAME", s);
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_PROFILE);						// C:\Documents and Settings\someone
	ei.add_ev(L"USER_DRIVE", s.drivename());
	ei.add_ev(L"USER_DIR", s.backslash());
	ei.add_ev(L"USER", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_PERSONAL);						// C:\Documents and Settings\someone\My Documents
	ei.add_ev(L"USER_DOC", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_DESKTOPDIRECTORY);				// C:\Documents and Settings\someone\デスクトップ
	ei.add_ev(L"USER_DESKTOP", s.backslash());

	s = WindowsAPI::SHGetSpecialFolder(CSIDL_COMMON_DOCUMENTS).dirname();	// C:\Documents and Settings\All Users\Documents
	ei.add_ev(L"ALL_USER_DRIVE", s.drivename());
	ei.add_ev(L"ALL_USER_DIR", s.backslash());
	ei.add_ev(L"ALL_USER", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_COMMON_DOCUMENTS);				// C:\Documents and Settings\All Users\Documents
	ei.add_ev(L"ALL_USER_DOC", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_COMMON_DESKTOPDIRECTORY);		// C:\Documents and Settings\All Users\デスクトップ
	ei.add_ev(L"ALL_USER_DESKTOP", s.backslash());

	s = WindowsAPI::SHGetSpecialFolder(CSIDL_PROGRAM_FILES);				// C:\Program Files
	ei.add_ev(L"PROGRAM_DRIVE", s.drivename());
	ei.add_ev(L"PROGRAM_DIR", s.backslash());
	ei.add_ev(L"PROGRAM", s.backslash());

	s = WindowsAPI::GetTempPath();											// C:\DOCUME~1\SOMEONE\LOCALS~1\Temp
	ei.add_ev(L"TMP_DRIVE", s.drivename());
	ei.add_ev(L"TMP_DIR", s.backslash());
	ei.add_ev(L"TMP", s.backslash());

	return 0;
}

sint load_inifile(String exename)
{
	enum section_id{
		SECTION_NONE,
		SECTION_OPTION,
		SECTION_EXEC,
		SECTION_MULTIEXEC,
		SECTION_EOF,
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
					// TODO: 初期処理で作られたexecinfolist[0]を取り除く
					execinfolist.push_back(execinfo_default);
				}else if(ifs->is_section(aline, L"EOF")){
					execinfo_default.verbose_out(L"ini section: eof");
					break;
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
						execinfo_default.set_arg(ifs->get_value_String(aline));
					}else if(ifs->is_key(aline, L"CHDIR")){
						execinfo_default.verbose_out(L"ini option: chdir");
						execinfo_default.set_chdir(ifs->get_value_String(aline));
					}else if(ifs->is_key(aline, L"IMPORT_ENV")){
						execinfo_default.verbose_out(L"ini option: import_env");
						execinfo_default.add_import_env(ifs->get_value_String(aline));
					}else if(ifs->is_key(aline, L"EXPORT_ENV")){
						execinfo_default.verbose_out(L"ini option: export_env");
						execinfo_default.add_export_env(ifs->get_value_String(aline));
					}else if(ifs->is_key(aline, L"VERBOSE")){
						execinfo_default.verbose_out(L"ini option: verbose");
						execinfo_default.verbose(ifs->get_value_bool(aline));
					}else if(ifs->is_key(aline, L"INTERNAL_CMD")){
						execinfo_default.verbose_out(L"ini option: internal_cmd");
						execinfo_default.internal_cmd(ifs->get_value_bool(aline));
					}else if(ifs->is_key(aline, L"GUI")){
						execinfo_default.verbose_out(L"ini option: gui");
						execinfo_default.gui(ifs->get_value_bool(aline));
					}else if(ifs->is_key(aline, L"WAIT")){
						execinfo_default.verbose_out(L"ini option: wait");
						execinfo_default.wait(ifs->get_value_bool(aline));
					}else if(ifs->is_key(aline, L"MAXIMIZE")){
						execinfo_default.verbose_out(L"ini option: maximize");
						execinfo_default.maximize(ifs->get_value_bool(aline));
						execinfo_default.minimize(false);
					}else if(ifs->is_key(aline, L"MINIMIZE")){
						execinfo_default.verbose_out(L"ini option: minimize");
						execinfo_default.minimize(ifs->get_value_bool(aline));
						execinfo_default.maximize(false);
					}else{
						wcerr << PGM_ERR "invalid option: " << aline.to_wcout() << endl;
						error = true;
						rcode = -5;
					}
					break;
				case SECTION_EXEC:
					execinfo_default.verbose_out(L"ini exec: " + aline);
					execinfolist[0].add_exs(aline);
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
		exename = av[0];

	locale::global(locale(""));
//	wcout.imbue(locale(""));
//	wcerr.imbue(locale(""));

	setup_expandvalues(execinfo_default, exename);

	load_inifile(exename);

	for(ExecuteInfoList::iterator i = execinfolist.begin() ; !error && i != execinfolist.end() ; ++i)
		i->execute();

	return rcode;
}

