/*
	Wrapper of Execute

	by opa
*/

#include <string>
#include <list>
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
#define VERSTR				"1.01"

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

inline bool isbackslash(wchar_t c)
{
	return c == L'\\' || c == L'/';
}

bool readable(const wstring &filename)
{
	FILE
		*fp = _wfopen(filename.c_str(), L"rb");

	if(fp != NULL){
		fclose(fp);
		return true;
	}

	return false;
}

inline bool executable(const wstring &filename)
{
	return readable(filename);
}

////////////////////////////////////////////////////////////////////////

class String : public wstring {
	typedef String Self;
	typedef wstring Super;

public:
	String();
	String(const Super &s);
	String(const wchar_t *s);
	String(const_iterator b, const_iterator e);
	String(const char *s);
	~String();

	string to_ansi() const;
	Self to_upper() const;
	Self trim() const;
	bool isdoublequote() const;
	Self doublequote() const;
	Self doublequote_del() const;

	Self &operator=(const Self &s);
	Self &operator=(const wchar_t *s);
	Self &operator=(const char *s);

	Self &operator+=(const Self &s)				{ append(s); return *this; }
	Self &operator+=(const wchar_t *s)			{ append(s); return *this; }
	Self &operator+=(const char *s);

	Self &assign_from_ansi(const char *s);
	Self &assign_from_ansi(const string &s)		{ return assign_from_ansi(s.c_str()); }
	Self &assign_from_utf8(const char *s);
	Self &assign_from_utf8(const string &s)		{ return assign_from_utf8(s.c_str()); }
	Self &assign_from_env(const Self &name);
	Self &printf(Self format, ...);

	// filename operator
	bool isbackslash() const;
	Self backslash() const;
	Self subext(const Self &ext) const;
	Self drivename() const;
	Self dirname() const;
	Self basename() const;
};

String::String()									{}
String::String(const String::Super &s)				: Super(s) {}
String::String(const wchar_t *s)					: Super(s) {}
String::String(const_iterator b, const_iterator e)	: Super(b, e) {}
String::String(const char *s)						{ assign_from_ansi(s); }
String::~String() {}
String &String::operator=(const String &s)			{ assign(s); return *this; }
String &String::operator=(const wchar_t *s)			{ assign(s); return *this; }
String &String::operator=(const char *s)			{ assign_from_ansi(s); return *this; }
String &String::operator+=(const char *s)			{ append(String(s)); return *this; }
String operator+(const String &s1, const char *s2)	{ return s1 + String(s2); }
String operator+(const char *s1, const String &s2)	{ return String(s1) + s2; }
bool operator==(const String &s1, const char *s2)	{ return s1 == String(s2); }
bool operator==(const char *s1, const String &s2)	{ return String(s1) == s2; }

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

bool String::isdoublequote() const
{
	// BUG:
	//  �P�ɐ擪�ƍŌ�̕����� " ���ǂ����𔻒肵�Ă��邾���Ȃ̂ŁA������̓r���� " ���������ꍇ�Ȃǂ��l�����Ă��Ȃ��B

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

String &String::printf(String format, ...) // va���g���K�v��AString��l�n������
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

bool String::isbackslash() const
{
	return size() > 0 && ::isbackslash(*(end()-1));
}

String String::backslash() const
{
	String
		r(*this);

	if(!isbackslash())
		r.append(1, L'\\');

	return r;
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
		e = ::rfind_if(begin(), end(), ::isbackslash);

	if(e != end() && ::isbackslash(*e))
		++e;

	return Self(begin(), e);
}

String String::basename() const
{
	const_iterator
		b = begin(),
		e = end();

	b = ::rfind_if(b, e, ::isbackslash);

	if(b != end() && ::isbackslash(*b))
		++b;

	if((e = ::rfind(b, e, L'.')) == b)
		e = end();

	return Self(b, e);
}

////////////////////////////////////////////////////////////////////////

void _putcxxx(const char *s, FILE *fp)
{
	fputs(s, fp);
	fputc('\n', fp);
}

inline void putcout(const char *s)
{
	_putcxxx(s, stdout);
}

inline void putcerr(const char *s)
{
	_putcxxx(s, stderr);
}

void _putcxxx(const String &s, FILE *fp)
{
	_putcxxx(s.to_ansi().c_str(), fp);
}

inline void putcout(const String &s)
{
	_putcxxx(s, stdout);
}

inline void putcerr(const String &s)
{
	_putcxxx(s, stderr);
}

void _putcxxx(const char *s1, const String &s2, FILE *fp)
{
	_putcxxx(String().assign_from_ansi(s1) + s2, fp);
}

inline void putcout(const char *s1, const String &s2)
{
	_putcxxx(s1, s2, stdout);
}

inline void putcerr(const char *s1, const String &s2)
{
	_putcxxx(s1, s2, stderr);
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

class ExpandValues : public map<String, String, case_ignore_less> {
	typedef ExpandValues Self;
	typedef map<String, String> Super;

public:
	mapped_type add(const key_type &name, const mapped_type &val);
	mapped_type get(const key_type &name) const;
};

ExpandValues::mapped_type ExpandValues::add(const ExpandValues::key_type &name, const ExpandValues::mapped_type &val)
{
	erase(name); // name���X�V���邽�߂ɁA��U����

	return operator[](name) = val;
}

ExpandValues::mapped_type ExpandValues::get(const ExpandValues::key_type &name) const
{
	const_iterator
		i = find(name);

	if(i != end())
		return i->second;

	return String();
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

	static String determine_filename(const String &exename);
	const String &filename() const					{ return _filename; }
	void open(const String &filename);
	void close();
	bool is_open() const							{ return _fp != NULL; }
	bool good() const								{ return _fp != NULL && !feof(_fp); }
	bool eof() const								{ return _fp == NULL || feof(_fp); }
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
	//  fe fe ... �̂悤�ȃt�@�C���̏ꍇ�A��ڂ�fe����unget()����Ȃ�

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

String IniFileStream::determine_filename(const String &exename)
{
	String
		r;

	if(readable(r = exename.subext(".ini")))
		return r;

	if(readable(r = exename.subext(".bat")))
		return r;

	return exename.subext(".ini"); /* default (but not found) */
}

void IniFileStream::open(const String &fn)
{
	close();

	_fp = _wfopen(fn.c_str(), L"rt");

	if(_fp != NULL){
		_filename = fn;
		skip_bom();
	}
}

void IniFileStream::close()
{
	if(_fp != NULL){
		fclose(_fp);
		_fp = NULL;
	}
}

bool IniFileStream::is_comment(const String &s)
{
	return s.size()==0 || s[0]==L'#' || s[0]==L';' || s[0]==L'@';
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
		return true; // �u=�v���Ȃ� �� �L�[�݂̂Œl�̋L�q�Ȃ� �� true��Ԃ�

	String
		value(f+1, s.end());
	bool
		r = false;

	value = value.trim().to_upper();

	if(value == "TRUE" || value == "YES" || value == "ON"){
		r = true;
	}else if(value == "FALSE" || value == "NO" || value == "OFF"){
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
		return String(); // �u=�v���Ȃ� �� �󕶎����Ԃ�

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
	Self &operator=(const Self &s)		{ Super::operator=(s); return *this; }
	Self &operator=(const wchar_t *s)	{ Super::operator=(s); return *this; }
	Self &operator=(const char *s)		{ Super::operator=(s); return *this; }
};

typedef list<ExString>
	ExStrings;

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
	typedef list<String> Envnames;

private:
	ExpandValues
		_expandValues;
	ExStrings
		_exStrings;
	Envnames
		_import_env,
		_export_env;
	ExString
		_arg,
		_chdir;
	bool
		_verbose,
		_internal,
		_use_path,
		_gui,
		_wait,
		_hide,
		_maximize,
		_minimize;

	static sint system(const String &cmd);
	static String getenv(const String &name);
	static sint putenv(const String &name, const String &val);
	static String system_escape(const String &s);

public:
	ExecuteInfo();

	const ExpandValues &ev() const						{ return _expandValues; }
	String ev(const ExpandValues::key_type &name) const	{ return _expandValues.get(name); }
	const ExStrings &exs() const						{ return _exStrings; }
	const Envnames &import_env() const					{ return _import_env; }
	const Envnames &export_env() const					{ return _export_env; }
	const ExString &arg() const							{ return _arg; }
	const ExString &chdir() const						{ return _chdir; }
	bool verbose() const								{ return _verbose; }
	bool internal() const								{ return _internal; }
	bool use_path() const								{ return _use_path; }
	bool gui() const									{ return _gui; }
	bool wait() const									{ return _wait; }
	bool hide() const									{ return _hide; }
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
	bool use_path(bool v)								{ return _use_path = v; }
	bool gui(bool v)									{ return _gui = v; }
	bool wait(bool v)									{ return _wait = v; }
	bool hide(bool v)									{ return _hide = v; }
	bool maximize(bool v)								{ return _maximize = v; }
	bool minimize(bool v)								{ return _minimize = v; }

	sint execute();
	void verbose_out(const String &s);
};

typedef list<ExecuteInfo>
	ExecuteInfos;

ExecuteInfo::ExecuteInfo()
{
	_expandValues.clear();
	_exStrings.clear();
	_import_env.clear();
	_export_env.clear();
	_arg			= "${ARG}";
	_chdir			= "";
	_verbose		= false;
	_internal		= false;
	_use_path		= false;
	_gui			= false;
	_wait			= false;
#ifdef __CONSOLE__
	_hide			= false;
#else
	_hide			= true;
#endif
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

void ExecuteInfo::add_ev(const ExpandValues::key_type &name, const ExpandValues::mapped_type &val)
{
	if(name.size() > 0)
		_expandValues.add(name, val);
}

void ExecuteInfo::add_exs(const String &s)
{
	String
		x = s.trim().doublequote_del();

	if(x.size() > 0)
		_exStrings.push_back(x);
}

void ExecuteInfo::add_import_env(const String &s)
{
	String
		x = s.trim();

	if(x.size() > 0)
		_import_env.push_back(x);
}

void ExecuteInfo::add_export_env(const String &s)
{
	String
		x = s.trim();

	if(x.size() > 0)
		_export_env.push_back(x);
}

ExString &ExecuteInfo::set_arg(const String &s)
{
	// �󕶎�����L�蓾��
	return _arg = s.trim();
}

ExString &ExecuteInfo::set_chdir(const String &s)
{
	// �󕶎�����L�蓾��
	return _chdir = s.trim();
}

bool is_exe_or_com()
{
	return true;
}

sint ExecuteInfo::execute()
{
	String
		cmd,
		arge,
		cl;
	bool
		done = false;

	verbose_out("--- execute ---");

	for(Envnames::const_iterator i = import_env().begin() ; i != import_env().end() ; ++i)
		add_ev(*i, getenv(*i));

	if(verbose()){
		verbose_out("arg: " + arg());
		verbose_out("chdir: " + chdir());

		for(Envnames::const_iterator i = import_env().begin() ; i != import_env().end() ; ++i)
			verbose_out("import_env: " + *i);
		for(Envnames::const_iterator i = export_env().begin() ; i != export_env().end() ; ++i)
			verbose_out("export_env: " + *i);
		for(ExpandValues::const_iterator i = ev().begin() ; i != ev().end() ; ++i)
			verbose_out("v: " + i->first + "=" + i->second);
		for(ExStrings::const_iterator i = exs().begin() ; i != exs().end() ; ++i)
			verbose_out("e: " + *i);
	}

	if(exs().size() <= 0){
		putcerr(PGM_ERR "execute command not defined");
		error = true;
		rcode = -2;
		return 1;
	}

	for(Envnames::const_iterator i = export_env().begin() ; i != export_env().end() ; ++i){
		ExpandValues::const_iterator f = ev().find(*i);
		if(f != ev().end())
			putenv("WRAPEXEC_" + f->first, f->second);
		else
			putcerr(PGM_WARN "variable not defined: ", *i);
	}

	for(ExStrings::const_iterator i = exs().begin() ; i != exs().end() ; ++i){
		cl.erase();
		cmd = i->expand(ev());
		arge = system_escape(arg().expand(ev()));
		if(arge.size() > 0 && !iswspace(arge[0]))
			arge = L' ' + arge; // �󔒂��Ȃ��ƃR�}���h���Ƃ������Ă��܂��̂ŃX�y�[�X��₤

		if(internal()){
			verbose_out("executable: " + cmd + " (internal)");
			cl += cmd + arge;
		}else{
			if(use_path()){





			}else if(executable(cmd)){
				verbose_out("executable: " + cmd);
				if(gui()){
					cl += "start \"" + ev("MY_BASENAME") + "\" ";
					if(wait())
						cl += "/wait ";
					if(hide())
						cl += "/b ";
					if(maximize())
						cl += "/max ";
					if(minimize())
						cl += "/min ";
				}
				cl += cmd.doublequote() + arge;
			}else{
				verbose_out("unexecutable: " + cmd);
				cl.erase(); // for safe
			}
		}

		if(cl.size() > 0){
			if(chdir().size() > 0)
				cl = "pushd \"" + chdir().expand(ev()) + "\" && " + cl;

			if(is_exe_or_com()){
				// CreateProcess() �ŋN��


			}else{
				// cmd.exe�o�R�ŋN��


			}










			verbose_out("cmdline: " + cl);

			rcode = system(L'"' + cl + L'"'); // �����ŃN�H�[�g����ƂȂ����܂������̂�����Ȃ��Bcmd.exe�̓�
			done = true;

			verbose_out(String().printf("return code: %d", rcode));

			break;
		}
	}

	// BUG: �u���ɖ߂��v�̂ł͂Ȃ��u�N���A����v�Ƃ������Ƃ����Ă���
	for(Envnames::const_iterator i = export_env().begin() ; i != export_env().end() ; ++i){
		ExpandValues::const_iterator f = ev().find(*i);
		if(f != ev().end())
			putenv("WRAPEXEC_" + f->first, "");
	}

	if(!done){
#if 1
		rcode = 1;
		putcerr("'" + system_escape(ev("MY_BASENAME")) +
				"' is not recognized as an internal or external command,\n"
				"operable program or batch file.");
#else
		// ���݂����Ȃ��R�}���h����cmd.exe�^���āA�G���[���b�Z�[�W���o������
		rcode = system(system_escape(ev("MY_BASENAME")) + "\"\b");
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

	// ���s�������̓X�y�[�X�ɒu������
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
		" [end]\n"
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
		" use_path\n"
		" gui\n"
		" wait\n"
		" hide\n"
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

void setup_expandvalues(ExecuteInfo &ei, const String &exename, const String &ininame)
{
	String
		s;

	ei.add_ev("ARG", get_given_option(WindowsAPI::GetCommandLine()));
	ei.add_ev("CLIPBOARD", WindowsAPI::GetClipboardText());

	ei.add_ev("MY_EXENAME", exename);
	ei.add_ev("MY_ININAME", ininame);
	ei.add_ev("MY_BASENAME", exename.basename());
	ei.add_ev("MY_DRIVE", exename.drivename());
	ei.add_ev("MY_DIR", exename.dirname());
	ei.add_ev("MY", exename.dirname());

	s = WindowsAPI::GetComputerName();										// SOMEONESPC
	ei.add_ev("SYS_NAME", s);
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_WINDOWS);						// C:\WINDOWS
	ei.add_ev("SYS_ROOT", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_SYSTEM);						// C:\WINDOWS\system32
	ei.add_ev("SYS_DRIVE", s.drivename());
	ei.add_ev("SYS_DIR", s.backslash());
	ei.add_ev("SYS", s.backslash());

	s = WindowsAPI::GetUserName();											// someone
	ei.add_ev("USER_NAME", s);
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_PROFILE);						// C:\Documents and Settings\someone
	ei.add_ev("USER_DRIVE", s.drivename());
	ei.add_ev("USER_DIR", s.backslash());
	ei.add_ev("USER", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_PERSONAL);						// C:\Documents and Settings\someone\My Documents
	ei.add_ev("USER_DOC", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_DESKTOPDIRECTORY);				// C:\Documents and Settings\someone\�f�X�N�g�b�v
	ei.add_ev("USER_DESKTOP", s.backslash());

	s = WindowsAPI::SHGetSpecialFolder(CSIDL_COMMON_DOCUMENTS).dirname();	// C:\Documents and Settings\All Users\Documents
	ei.add_ev("ALL_USER_DRIVE", s.drivename());
	ei.add_ev("ALL_USER_DIR", s.backslash());
	ei.add_ev("ALL_USER", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_COMMON_DOCUMENTS);				// C:\Documents and Settings\All Users\Documents
	ei.add_ev("ALL_USER_DOC", s.backslash());
	s = WindowsAPI::SHGetSpecialFolder(CSIDL_COMMON_DESKTOPDIRECTORY);		// C:\Documents and Settings\All Users\�f�X�N�g�b�v
	ei.add_ev("ALL_USER_DESKTOP", s.backslash());

	s = WindowsAPI::SHGetSpecialFolder(CSIDL_PROGRAM_FILES);				// C:\Program Files
	ei.add_ev("PROGRAM_DRIVE", s.drivename());
	ei.add_ev("PROGRAM_DIR", s.backslash());
	ei.add_ev("PROGRAM", s.backslash());

	s = WindowsAPI::GetTempPath();											// C:\DOCUME~1\SOMEONE\LOCALS~1\Temp
	ei.add_ev("TMP_DRIVE", s.drivename());
	ei.add_ev("TMP_DIR", s.backslash());
	ei.add_ev("TMP", s.backslash());
}

void do_inifile_option(IniFileStream &ifs, ExecuteInfo &e, const String &aline)
{
	if(ifs.is_key(aline, "HELP")){
		do_help();
		error = true;	// exec�t�F�[�Y�����s���Ȃ�
		rcode = 0;		// ���G���[�ł͂Ȃ��̂�rcode��0��Ԃ�
	}else if(ifs.is_key(aline, "ARG")){
		e.set_arg(ifs.get_value_String(aline));
	}else if(ifs.is_key(aline, "CHDIR")){
		e.set_chdir(ifs.get_value_String(aline));
	}else if(ifs.is_key(aline, "IMPORT_ENV")){
		e.add_import_env(ifs.get_value_String(aline));
	}else if(ifs.is_key(aline, "EXPORT_ENV")){
		e.add_export_env(ifs.get_value_String(aline));
	}else if(ifs.is_key(aline, "VERBOSE")){
		e.verbose(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, "INTERNAL")){
		e.internal(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, "USE_PATH")){
		e.use_path(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, "GUI")){
		e.gui(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, "WAIT")){
		e.wait(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, "HIDE")){
		e.hide(ifs.get_value_bool(aline));
	}else if(ifs.is_key(aline, "MAXIMIZE")){
		e.maximize(ifs.get_value_bool(aline));
		e.minimize(false);
	}else if(ifs.is_key(aline, "MINIMIZE")){
		e.minimize(ifs.get_value_bool(aline));
		e.maximize(false);
	}else{
		putcout(PGM_ERR "invalid option: ", aline);
		error = true;
		rcode = -3;
	}
}

void load_inifile(ExecuteInfo &execinfo_default, ExecuteInfos &execinfos, const String &ininame)
{
	enum section_id{
		SECTION_NONE,
		SECTION_END,
		SECTION_GLOBAL,
		SECTION_OPTION,
		SECTION_EXEC,
	};

	execinfo_default.verbose_out("--- prepare ---");

	IniFileStream
		ifs;
	String
		aline;
	section_id
		section = SECTION_EXEC;

	execinfos.clear();
	execinfos.push_back(execinfo_default);

	ifs.open(ininame);

	if(!ifs.is_open()){
		putcerr(PGM_ERR "ini file open failed: ", ininame);
		error = true;
		rcode = -4;
		return;
	}

	execinfo_default.verbose_out("ini filename: " + ifs.filename());

	while(ifs.getline(aline)){
		if(ifs.is_comment(aline)){
			; // �R�����gor��s�Ȃ̂ŃX�L�b�v
		}else{
			if(ifs.is_section(aline)){
				if(ifs.is_section(aline, "END")){
					execinfo_default.verbose_out("end section detected");
					break;
				}else if(ifs.is_section(aline, "GLOBAL")){
					section = SECTION_GLOBAL;
				}else if(ifs.is_section(aline, "OPTION")){
					section = SECTION_OPTION;
					if(execinfos.back().exs().size() > 0)
						execinfos.push_back(execinfo_default);
					else
						execinfos.back() = execinfo_default;
				}else if(ifs.is_section(aline, "EXEC")){
					section = SECTION_EXEC;
					if(execinfos.back().exs().size() > 0)
						execinfos.push_back(execinfo_default);
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
					do_inifile_option(ifs, execinfos.back(), aline);
					break;
				case SECTION_EXEC:
					execinfos.back().add_exs(aline);
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

void wrapexec_main(const String &exename)
{
	ExecuteInfo
		execinfo_default;
	ExecuteInfos
		execinfos;
	String
		ininame;

	ininame = IniFileStream::determine_filename(exename);

	setup_expandvalues(execinfo_default, exename, ininame);

	load_inifile(execinfo_default, execinfos, ininame);

	for(ExecuteInfos::iterator i = execinfos.begin() ; !error && i != execinfos.end() ; ++i)
		i->execute();
}

#ifdef __MINGW32__

sint main(sint ac, char *[])
{
	wchar_t
		**av = ::CommandLineToArgvW(::GetCommandLine(), &ac);

	wrapexec_main(av[0]); // BUG : �t���p�X������Ă��Ȃ�

	GlobalFree(av);

	return rcode;
}

#else // __MINGW32__

#ifdef __CONSOLE__

sint wmain(sint, wchar_t *av[])
{
	wrapexec_main(av[0]);

	return rcode;
}

#else // __CONSOLE__

extern "C"
sint WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR cl, int)
{
	sint
		ac = 0;
	wchar_t
		**av = ::CommandLineToArgvW(cl, &ac);

	wrapexec_main(av[0]);

	return rcode;
}

#endif // __CONSOLE__

#endif // __MINGW32__
