#pragma once

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include "File.h"

using std::string;
using std::wstring;
using std::map;

typedef unsigned char byte;

struct t_csf_header {
	__int32 id;
	__int32 flags1;
	__int32 numlabels;
	__int32 numextravalues;
	__int32 zero;
	__int32 language;
};

const __int32 CSF_File_id = 'CSF ';
const __int32 csf_label_id = 'LBL ';
const __int32 csf_string_id = 'STR ';
const __int32 csf_string_w_id = 'STRW';

const enum LANGUAGE { US, ZERO1, GERMAN, FRENCH, ZERO2, ZERO3,
						ZERO4, ZERO5, KOREAN, CHINESE
					};

class CSF_File {
public:
	struct t_map_entry {
		wstring value;
		string extra_value;
	};

	CSF_File(byte* data, int size) : data(data), cb(size) {}
	CSF_File(boost::shared_ptr<File> f);

	byte* get_data() const {
		return data;
	}

	int get_size() const {
		return cb;
	}

	const t_csf_header* get_header() const {
		return reinterpret_cast<const t_csf_header*>(get_data());
	}

	int get_language() const {
		return get_header()->language;
	}

	typedef map<string, t_map_entry> t_map;

	int post_open();
	void erase_value(const string& name);
	string get_converted_value(const string& name) const;
	void set_value(const string& name, const wstring& value, const string& extra_value);
	static string convert2string(const wstring& s);
	static wstring convert2wstring(const string& s);

	bool is_valid() const {
		const t_csf_header& header = *get_header();
		int size = get_size();
		return !(sizeof(t_csf_header) > size ||
						 header.id != CSF_File_id);
	}

	int get_c_strings() const {
		return m_map.size();
	}

	const t_map& get_map() const {
		return m_map;
	}

	wstring get_value(const string& name) const {
		return m_map.find(name)->second.value;
	}

	string get_extra_value(const string& name) const {
		return m_map.find(name)->second.extra_value;
	}

	bool has_name(const string& name) const {
		return m_map.find(name) != m_map.end();
	}

	int get_write_size() const;
	void write(byte* d) const;

	int add_csf_entries(char* path);

private:
	t_map m_map;
	byte* data;
	int cb;
};

void write_wstring(byte*& w, wstring v);
void write_string(byte*& w, string v);
wstring read_wstring(const byte*& r);
string read_string(const byte*& r);
void write_int(byte*& w, int v);
int read_int(const byte*& r);