/*
	Wrapper of Execute

	by opa
*/

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdio>

#include <windows.h>
#include <shlobj.h>
#include <lmcons.h>

#define PGM					"wrapexec"
#define PGM_DEBUG			PGM ": "
#define PGM_INFO			PGM ": "
#define PGM_WARN			PGM " warning: "
#define PGM_ERR				PGM " error: "
#define VERSTR				"1.00"

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
BidirectionalIterator rfind(BidirectionalIterator first, BidirectionalIterator last, const T &value)
{
	while(first != last){
		--last;
		if(*last == value)
			break;
	}

	return last;
}

template <class BidirectionalIterator, class Predicate>
BidirectionalIterator rfind_if(BidirectionalIterator first, BidirectionalIterator last, Predicate pred)
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

	string to_ansi() const;
	String to_upper() const;
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
	String &assign_from_ansi(const char *s);
	String &assign_from_ansi(const string &s)	{ return assign_from_ansi(s.c_str()); }
	String &assign_from_utf8(const char *s);
	String &assign_from_utf8(const string &s)	{ return assign_from_utf8(s.c_str()); }
	String &assign_from_env(const String &name);
	String &printf(String format, ...);
};

string String::to_ansi() const
{
	sint
		siz = WideCharToMultiByte(CP_ACP, 0, c_str(), size(), NULL, 0, NULL, NULL);
	char
		*buf = new char[siz+1];

	fill(buf, buf + siz, ' ');
	buf[siz] = L'\0';

	WideCharToMultiByte(CP_ACP, 0, c_str(), size(), buf, siz, NULL, NULL);

	string
		r(buf);

	delete [] buf;

	return r;
}

String String::to_upper() const
{
	Self
		r(*this);

	for(iterator i = r.begin() ; i != r.end() ; ++i)
		*i = towupper(*i);

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
		return String(begin()+1, end()-1);
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
	e = rfind_if(b, e, isnotwspace);

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
		e = ::rfind(begin(), end(), L'\\');

	if(e != end() && *e == L'\\')
		++e;

	return Self(begin(), e);
}

String String::basename() const
{
	const_iterator
		b = begin(),
		e = end();

	b = ::rfind(b, e, L'\\');

	if(b != end() && *b == L'\\')
		++b;

	if((e = ::rfind(b, e, L'.')) == b)
		e = end();

	return Self(b, e);
}

String &String::assign_from_ansi(const char *s)
{
	sint
		size = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
	wchar_t
		*buf = new wchar_t[size+1];

	fill(buf, buf + size, L' ');
	buf[size] = L'\0';

	MultiByteToWideChar(CP_ACP, 0, s, -1, buf, size);

	assign(buf);

	delete [] buf;

	return *this;
}

String &String::assign_from_utf8(const char *s)
{
	sint
		size = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
	wchar_t
		*buf = new wchar_t[size+1];

	fill(buf, buf + size, L' ');
	buf[size] = L'\0';

	MultiByteToWideChar(CP_UTF8, 0, s, -1, buf, size);

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

String &String::printf(String format, ...) // vaを使う必要上、Stringを値渡しする
{
	wchar_t
		buf[1024+1];
	va_list
		va;
	sint
		size;

	va_start(va, format);
	size = wvsprintf(buf, format.c_str(), va);
	va_end(va);

	assign(buf, buf + size);

	return *this;
}

sint putcout(const char *s)
{
	return fputs(s, stdout), fputc('\n', stdout);
}

sint putcerr(const char *s)
{
	return fputs(s, stderr), fputc('\n', stderr);
}

sint putcout(const String &s)
{
	return putcout(s.to_ansi().c_str());
}

sint putcerr(const String &s)
{
	return putcerr(s.to_ansi().c_str());
}

sint putcout(const char *s1, const String &s2)
{
	String s = String().assign_from_ansi(s1) + s2;

	return putcout(s);
}

sint putcerr(const char *s1, const String &s2)
{
	String s = String().assign_from_ansi(s1) + s2;

	return putcerr(s);
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

class IniFileStream {
	typedef IniFileStream Self;
	typedef ifstream Super;

private:
	FILE
		*_fp;
	String
		_filename;

	bool skip_bom();
	Self &get(sint &ch)								{ ch = fgetc(_fp); return *this; }
	Self &unget(sint ch)							{ ungetc(ch, _fp); return *this; }

public:
	IniFileStream()									: _fp(NULL) {}

	static String ininame(const String &exename);
	const String &filename() const					{ return _filename; }
	void open(const String &ininame);
	void open_w_exename(const String &exename)		{ open(ininame(exename)); }
	void close()									{ fclose(_fp); _fp = NULL; }
	bool good()										{ return _fp != NULL && !feof(_fp); }
	bool eof()										{ return _fp == NULL || feof(_fp); }
	bool getline(String &s);

	static bool is_comment(const String &s);
	static bool is_section(const String &s);
	static bool is_section(const String &s, const String &section_name);
	static bool is_key(const String &s, const String &key_name);
	static bool get_value_bool(const String &s);
	static String get_value_String(const String &s);
};

bool IniFileStream::skip_bom()
{
	// BUG:
	//  fe fe ... のようなファイルの場合、二つ目のfeしかunget()されない

	sint
		c;

	if(!eof()){
		get(c);
		if(c == 0xef){
			if(!eof()){
				get(c);
				if(c == 0xbb){
					if(!eof()){
						get(c);
						if(c == 0xbf)
							return true; // UTF8
					}
				}
			}
#if 0
		}else if(c == 0xfe){
			if(!eof()){
				get(c);
				if(c == 0xff)
					return true; // UTF16BE
			}
		}else if(c == 0xff){
			if(!eof()){
				get(c);
				if(c == 0xfe)
					return true; // UTF16LE
			}
#endif
		}
		unget(c);
	}

	return false;
}

String IniFileStream::ininame(const String &exename)
{
	return exename.subext(L".ini");
}

void IniFileStream::open(const String &ininame)
{
	if(_fp != NULL)
		close();

	_filename = ininame;
	_fp = _wfopen(_filename.c_str(), L"rt");
	skip_bom();
}

bool IniFileStream::is_comment(const String &s)
{
	return s.size()==0 || s[0]==L'#' || s[0]==L';';
}

bool IniFileStream::is_section(const String &s)
{
	if(s.size() >= 2 && *s.begin() == L'[' && *(s.end()-1) == L']')
		return true;

	return false;
}

bool IniFileStream::is_section(const String &s, const String &section_name)
{
	if(!is_section(s))
		return false;

	return String(s.begin()+1, s.end()-1).trim().to_upper() == section_name;
}

bool IniFileStream::is_key(const String &s, const String &key_name)
{
	String::const_iterator
		f = find(s.begin(), s.end(), L'=');
	String
		key_c(s.begin(), f);

	return key_c.trim().trim().to_upper() == key_name;
}

bool IniFileStream::get_value_bool(const String &s)
{
	String::const_iterator
		f = find(s.begin(), s.end(), L'=');

	if(f == s.end())
		return true; // 「=」がない → キーのみで値の記述なし → trueを返す

	String
		value(f+1, s.end());
	bool
		r = false;

	value = value.trim().to_upper();

	if(value == L"TRUE" || value == L"YES" || value == L"ON"){
		r = true;
	}else if(value == L"FALSE" || value == L"NO" || value == L"OFF"){
		r = false;
	}else{
		putcerr(PGM_ERR "invalid value: ", value);
		error = true;
		rcode = -1;
	}

	return r;
}

String IniFileStream::get_value_String(const String &s)
{
	String::const_iterator
		f = find(s.begin(), s.end(), L'=');

	if(f == s.end())
		return String(); // 「=」がない → 空文字列を返す

	return String(f+1, s.end());
}

bool IniFileStream::getline(String &s)
{
	s.clear();

	if(!good())
		return false;

	string
		tmp;

	tmp.reserve(200);
	for(int c ; (c = fgetc(_fp)) != EOF ; tmp += c)
		if(c == '\n')
			break;

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
							putcerr(PGM_WARN "variable name not presented");
						}else if((f=ev.find(evname)) != ev.end()){
							r.append(f->second);
						}else{
							putcerr(PGM_WARN "variable not defined: ", evname);
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
		_internal,
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
	bool internal() const								{ return _internal; }
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
	bool internal(bool v)								{ return _internal = v; }
	bool gui(bool v)									{ return _gui = v; }
	bool wait(bool v)									{ return _wait = v; }
	bool maximize(bool v)								{ return _maximize = v; }
	bool minimize(bool v)								{ return _minimize = v; }

	sint execute();
	void verbose_out(const String &s);
};

typedef vector<ExecuteInfo>
	ExecuteInfoList;

ExecuteInfo::ExecuteInfo()
{
	_expandValues.clear();
	_exStringList.clear();
	_import_env.clear();
	_export_env.clear();
	_arg			= L"${ARG}";
	_chdir			= L"";
	_verbose		= false;
	_internal		= false;
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
		verbose_out(L"arg: " + arg());
		verbose_out(L"chdir: " + chdir());

		for(EnvnameList::const_iterator i = import_env().begin() ; i != import_env().end() ; ++i)
			verbose_out(L"import_env: " + *i);
		for(EnvnameList::const_iterator i = export_env().begin() ; i != export_env().end() ; ++i)
			verbose_out(L"export_env: " + *i);
		for(ExpandValues::const_iterator i = ev().begin() ; i != ev().end() ; ++i)
			verbose_out(L"v: " + i->first + L"=" + i->second);
		for(ExStringList::const_iterator i = exs().begin() ; i != exs().end() ; ++i)
			verbose_out(L"e: " + *i);
	}

	if(exs().size() <= 0){
		putcerr(PGM_ERR "execute command not defined");
		error = true;
		rcode = -2;
		return 1;
	}

	for(EnvnameList::const_iterator i = export_env().begin() ; i != export_env().end() ; ++i){
		ExpandValues::const_iterator f = ev().find(*i);
		if(f != ev().end())
			putenv(L"WRAPEXEC_" + f->first, f->second);
		else
			putcerr(PGM_WARN "variable not defined: ", *i);
	}

	for(ExStringList::const_iterator i = exs().begin() ; i != exs().end() ; ++i){
		cl.erase();
		cmd = i->expand(ev());
		arge = system_escape(arg().expand(ev()));
		if(arge.size() > 0 && !iswspace(arge[0]))
			arge = L' ' + arge; // 空白がないとコマンド名とくっついてしまうのでスペースを補う

		if(chdir().size() > 0)
			cl += L"pushd \"" + chdir().expand(ev()) + L"\" && ";

		if(internal()){
			verbose_out(L"executable: " + cmd + L" (internal)");
			cl += cmd + arge;
		}else if(executable(cmd)){
			verbose_out(L"executable: " + cmd);
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
			verbose_out(L"unexecutable: " + cmd);
			cl.erase(); // for safe
		}

		if(cl.size() > 0){
			verbose_out(L"cmdline: " + cl);

			rcode = system(L'"' + cl + L'"'); // ここでクォートするとなぜうまくいくのか判らない。cmd.exeの謎
			done = true;

			verbose_out(String().printf(L"return code: %d", rcode));

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
		putcerr(L"'" + system_escape(ev(L"MY_BASENAME")) +
				L"' is not recognized as an internal or external command,\n"
				L"operable program or batch file.");
#else
		// 存在し得ないコマンド名をcmd.exe与えて、エラーメッセージを出させる
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
		putcout(PGM_DEBUG, s);
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
		buf[MAX_COMPUTERNAME_LENGTH+1];
	DWORD
		size = sizeof buf;

	if(::GetComputerName(buf, &size) == 0)
		return String();

	return String(String::iterator(buf), String::iterator(buf + size));
}

String WindowsAPI::GetTempPath()
{
	String
		r;
	DWORD
		size = ::GetTempPath(0, NULL);
	wchar_t
		*buf = new wchar_t[size];

	::GetTempPath(size, buf);
	r.assign(buf);
	delete [] buf;

	return r;
}

String WindowsAPI::GetUserName()
{
	wchar_t
		buf[UNLEN+1];
	DWORD
		size = sizeof buf;

	if(::GetUserName(buf, &size) == 0)
		return String();

	return String(String::iterator(buf), String::iterator(buf + size - 1));
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
	putcout(credit);
	putcout("");

	putcout(
		"available sections:\n"
		" [global]\n"
		" [option]\n"
		" [exec]\n"
		" [eof]\n"
		""
	);

	putcout(
		"available options:\n"
		" help\n"
		" arg\n"
		" chdir\n"
		" import_env\n"
		" export_env\n"
		" verbose\n"
		" internal\n"
		" gui\n"
		" wait\n"
		" maximize\n"
		" minimize\n"
		""
	);

	putcout(
		"available macros:\n"
		" ARG\n"
		" CLIPBOARD\n"
		" MY_EXENAME\n"
		" MY_ININAME\n"
		" MY_BASENAME\n"
		" MY_DIR (MY)\n"
		" MY_DRIVE\n"
		" SYS_NAME\n"
		" SYS_ROOT\n"
		" SYS_DIR (SYS)\n"
		" SYS_DRIVE\n"
		" USER_NAME\n"
		" USER_DIR (USER)\n"
		" USER_DRIVE\n"
		" USER_DOC\n"
		" USER_DESKTOP\n"
		" ALL_USER_DIR (ALL_USER)\n"
		" ALL_USER_DRIVE\n"
		" ALL_USER_DOC\n"
		" ALL_USER_DESKTOP\n"
		" PROGRAM_DIR (PROGRAM)\n"
		" PROGRAM_DRIVE\n"
		" TMP_DIR (TMP)\n"
		" TMP_DRIVE\n"
		""
	);
}

void setup_expandvalues(ExecuteInfo &ei,const String &exename)
{
	String
		s;

	ei.add_ev(L"ARG", get_given_option(WindowsAPI::GetCommandLine()));
	ei.add_ev(L"CLIPBOARD", WindowsAPI::GetClipboardText());

	ei.add_ev(L"MY_EXENAME", exename);
	ei.add_ev(L"MY_ININAME", IniFileStream::ininame(exename));
	ei.add_ev(L"MY_BASENAME", exename.basename());
	ei.add_ev(L"MY_DRIVE", exename.drivename());
	ei.add_ev(L"MY_DIR", exename.dirname());
	ei.add_ev(L"MY", exename.dirname());

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
}

void do_inifile_option(IniFileStream &ifs, ExecuteInfo &e, const String &aline)
{
	if(ifs.is_key(aline, L"HELP")){
		do_help();
		error = true;	// execフェーズを実行しない
		rcode = 0;		// がエラーではないのでrcodeは0を返す
	}else if(ifs.is_key(aline, L"ARG")){
		e.set_arg(ifs.get_value_String(aline));
	}else if(ifs.is_key(aline, L"CHDIR")){
		e.set_chdir(ifs.get_value_String(aline));
	}else if(ifs.is_key(aline, L"IMPORT_ENV")){
		e.add_import_env(ifs.get_value_String(aline));
	}else if(ifs.is_key(aline, L"EXPORT_ENV")){
		e.add_export_env(ifs.get_value_String(aline));
	}else if(ifs.is_key(aline, L"VERBOSE")){
		e.verbose(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, L"INTERNAL")){
		e.internal(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, L"GUI")){
		e.gui(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, L"WAIT")){
		e.wait(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, L"MAXIMIZE")){
		e.maximize(ifs.get_value_bool(aline));
		e.minimize(false);
	}else if(ifs.is_key(aline, L"MINIMIZE")){
		e.minimize(ifs.get_value_bool(aline));
		e.maximize(false);
	}else{
		putcout(PGM_ERR "invalid option: ", aline);
		error = true;
		rcode = -3;
	}
}

void load_inifile(ExecuteInfo &execinfo_default, ExecuteInfoList &execinfolist, const String &exename)
{
	enum section_id{
		SECTION_NONE,
		SECTION_EOF,
		SECTION_GLOBAL,
		SECTION_OPTION,
		SECTION_EXEC,
	};

	execinfo_default.verbose_out(L"--- prepare ---");

	IniFileStream
		ifs;
	String
		aline;
	section_id
		section;

	ifs.open_w_exename(exename);
	execinfo_default.verbose_out(L"ini filename: " + ifs.filename());

	if(!ifs.good()){
		putcerr(PGM_ERR "ini file open failed: ", ifs.filename());
		error = true;
		rcode = -4;
		return;
	}

	section = SECTION_EXEC;
	execinfolist.clear();
	execinfolist.push_back(execinfo_default);

	while(ifs.getline(aline)){
		if(ifs.is_comment(aline)){
			; // コメントor空行なのでスキップ
		}else{
			if(ifs.is_section(aline)){
				if(ifs.is_section(aline, L"EOF")){
					execinfo_default.verbose_out(L"eof section detected");
					break;
				}else if(ifs.is_section(aline, L"GLOBAL")){
					section = SECTION_GLOBAL;
				}else if(ifs.is_section(aline, L"OPTION")){
					section = SECTION_OPTION;
					if(execinfolist.back().exs().size() > 0)
						execinfolist.push_back(execinfo_default);
					else
						execinfolist.back() = execinfo_default;
				}else if(ifs.is_section(aline, L"EXEC")){
					section = SECTION_EXEC;
					if(execinfolist.back().exs().size() > 0)
						execinfolist.push_back(execinfo_default);
				}else{
					putcerr(PGM_ERR "invalid section name: ", aline);
					section = SECTION_NONE;
					error = true;
					rcode = -5;
				}
			}else{
				switch(section){
				case SECTION_GLOBAL:
					do_inifile_option(ifs, execinfo_default, aline);
					break;
				case SECTION_OPTION:
					do_inifile_option(ifs, execinfolist.back(), aline);
					break;
				case SECTION_EXEC:
					execinfolist.back().add_exs(aline);
					break;
				default:
					putcout(PGM_WARN "ignored: ", aline);
					break;
				}
			}
		}
	}

	ifs.close();
}

void wrapexec_main(String exename)
{
	ExecuteInfo
		execinfo_default;
	ExecuteInfoList
		execinfolist;

	setup_expandvalues(execinfo_default, exename);

	load_inifile(execinfo_default, execinfolist, exename);

	for(ExecuteInfoList::iterator i = execinfolist.begin() ; !error && i != execinfolist.end() ; ++i)
		i->execute();
}

#ifdef __MINGW32__

sint main(sint ac, char *[])
{
	wchar_t
		**av = ::CommandLineToArgvW(::GetCommandLine(), &ac);

	wrapexec_main(av[0]);

	GlobalFree(av);

	return rcode;
}

#else // __MINGW32__

sint wmain(sint, wchar_t *av[])
{
	wrapexec_main(av[0]);

	return rcode;
}

#endif // __MINGW32__
