#include "ini_file.h"
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <boost/algorithm/string.hpp>

using std::string;
using std::vector;
using std::map;
using boost::lexical_cast;
using boost::shared_ptr;
#include <iostream>

ini_file::ini_file() {
}

const string ini_section::nullvalue = "";

ini_file::ini_file(const string& path) : name(path) {
	std::ifstream input(path.c_str(), std::ios::binary);
	parse(input);
	input.close();
}

ini_file::ini_file(const boost::shared_ptr<File> f) {
	parse(f);
}

int ini_file::output(const string& path) const {
	std::ofstream of(path.c_str(), std::ios::binary);
	of.clear();
	int num_bytes_written = output(of);
	of.close();
	return num_bytes_written;
}

int ini_file::output() const {
	return output(name);
}

int ini_file::output(std::ostream& os) const {
	int num_bytes_written = 0;
	sectionmap::const_iterator it;
	for (it = sections.begin(); it != sections.end(); ++it) {
		os << '[' << it->first << ']' << "\r\n";
		num_bytes_written += it->first.length() + 4;
		shared_ptr<ini_section> s = it->second;
		num_bytes_written += s->output(os);
		os << "\r\n";
		num_bytes_written += 2;
	}
	return num_bytes_written;
}

int ini_file::parse(std::ifstream& f) {
	if (!f.is_open() || f.bad())
		return 0;

	int num = 0;

	while (!f.eof()) {
		string line;
		getline(f, line);
		// remove linefeeds
		string::const_reverse_iterator it = line.rbegin();
		int newlen = line.length();
		while (it != line.rend()) {
			if ((*it == '\r') || (*it == '\n'))
				newlen--;
			else
				break;
			it++;
		}
		line.resize(newlen);
		num += process_line(line);
	}
	return num;
}

int ini_file::parse(const shared_ptr<File>& f) {
	if (!f) return 0;

	vector<unsigned char> buf;
	f->read(buf, f->size());
	name = f->name();

	int first_non_ws = 0;
	int last_non_ws;
	for (int pos = 0; pos < f->size(); pos++) {
		switch (buf[pos]) {
			case '\r':
				last_non_ws = pos - 1;
				if (pos < buf.size() && buf[++pos] == '\n')
					pos++;
				process_line(string(buf.begin() + first_non_ws, buf.begin() + last_non_ws + 1));
				first_non_ws = pos;
				break;
			case '\n':
				last_non_ws = pos - 1;
				process_line(string(buf.begin() + first_non_ws, buf.begin() + last_non_ws + 1) );
				first_non_ws = pos + 1;
				break;
          }
     }
     last_non_ws = buf.size() - 1;
     process_line( string(buf.begin() + first_non_ws, buf.begin() + last_non_ws + 1));

	//while (!f->eof())
	//	process_line(f->readline());
	//vector<string> lines = vector<string>();
	//f->read_all_lines(lines);
	//for (vector<string>::iterator i = lines.begin(); i != lines.end(); i++) {
	//	process_line(*i);
	//}
	return 0;
}

int ini_file::process_line(string& line) {
	fix_line(line); // remove clutter

	if (line.length() == 0) 
		return 0;
	// Test if this line contains start of new section i.e. matches [*]
	if ((line[0] == '[') && (line[line.length() - 1] == ']')) {
		string section = line.substr(1, line.length() - 2);
		
		boost::algorithm::to_lower(section);

		if (sections.find(section) == sections.end()) {
			sections.insert(
				std::pair<string, shared_ptr<ini_section>>(section, shared_ptr<ini_section>(new ini_section(section))));
		}

		sectionmap::iterator kmi = sections.find(section);
		if (kmi == sections.end()) {
			cur_section = shared_ptr<ini_section>();
		}
		else {
			cur_section = shared_ptr<ini_section>(kmi->second);
		}
	}
	else if (cur_section) {
		return cur_section->parse_line(line);
	}
	return 0;
}

string ini_file::get_key(const string& section, const string& key) {
	// Load section if it's not loaded yet
	if (!cur_section || cur_section->get_name() != section) {
		std::string section_(section);
		boost::algorithm::to_lower(section_);
		cur_section = shared_ptr<ini_section>();
		sectionmap::iterator kmi = sections.find(section_);
		if (kmi == sections.end()) return ini_section::nullvalue;
		cur_section = kmi->second;
	}
	// Return key in loaded section
	return get_key(key);
}

string ini_file::get_key(const string& key) const {
	// ensure exists
	assert(cur_section != NULL);
	std::string key_(key);
	boost::to_lower(key_);
	return cur_section->read_string(key_);
}

bool ini_file::set_current_section(const string& section) {
	if (!cur_section || cur_section->get_name() != section) {
		cur_section = shared_ptr<ini_section>();
		std::string section_(section);
		boost::algorithm::to_lower(section_);
		sectionmap::iterator kmi = sections.find(section_);

		if (kmi == sections.end()) {
			sections.insert(
				std::pair<string, shared_ptr<ini_section>>(section_, shared_ptr<ini_section>(new ini_section(section_))));
			kmi = sections.find(section_);
		}
		cur_section = kmi->second;
	}
	// @ cur_section->get_name() == section
	return true;
}

string ini_file::get_name() const {
	return name;
}

ini_section& ini_file::get_current_section() {
	return *cur_section;
}

const ini_section& ini_file::get_current_section() const {
	return *cur_section;
}

ini_section& ini_file::get_section(const string& section) {
	set_current_section(section);
	return get_current_section();
}

sectionmap::const_iterator ini_file::begin() const {
	return sections.begin();
}

sectionmap::const_iterator ini_file::end() const {
	return sections.end();
}

sectionmap::iterator ini_file::begin() {
	return sections.begin();
}

sectionmap::iterator ini_file::end() {
	return sections.end();
}

void ini_file::set_value(const std::string& section, const std::string& key, const std::string& value) {
	set_current_section(section);
	set_value(key, value);
}

void ini_file::set_value(const std::string& key, const std::string& value) {
	cur_section->set_value(key, value);
}

/*
ini_section class
Methods implementations
*/

void fix_line(string &line) {
	string::const_iterator start = line.begin();
	string::const_iterator end;

	size_t comment_start = line.find(";");
	if (comment_start == string::npos) 
		end = line.end();
	else end = line.begin() + comment_start;
	
	// remove leading spaces/tabs/newline
	while (start != end) {
		// usefull data starts here
		if (*start != ' ' && *start != '\t' && *start != '\n' && *start != '\r')
			break;

		start++;
	}

	// remove useless trailing stuff
	while (end > start) {
		end--;
		if (*end != ' ' && *end != '\t') {
			end++;
			break;
		}
	}
	line = start < end ? string(start, end) : string();
}

ini_section::ini_section(const string& section_Name) {
	this->section_name = section_Name;
}

int ini_section::parse_lines(const vector<string>& buffer) {
	int num = 0;
	vector<string>::const_iterator it;

	for (it = buffer.begin(); it != buffer.end(); ++it) {
		num += parse_line(*it);
	}
	return num;
}

int ini_section::parse_line(const string& line) {
	// ignore comments
	if (line[0] == ';') return 0;

	string key, value;
	int pos = line.find("=");
	if (pos != string::npos) {
		key = line.substr(0, pos);
		value = line.substr(pos + 1);
		// remove unnecessary garbage from value
		fix_line(key);
		boost::to_lower(key);

		fix_line(value);

		entries[key] = value;
		return 1;
	}
	return 0;
}

void ini_section::clear() {
	entries.clear();
}

int ini_section::output(std::ostream& os) const {
	int num = 0;
	for(keymap::const_iterator it = entries.begin(); it != entries.end(); ++it) {
		os << it->first << '=' << it->second << "\r\n";
		num += it->first.length() + it->second.length() + 3;
	}
	return num;
}

bool ini_section::read_bool(const string& key, bool assume) const {
	// Hacky, not using operator[] so const function is allowed so ini_section's
	// can be passed const without losing read_string() functionality
	std::string key_(key);
	boost::to_lower(key_);
	keymap::const_iterator iter = entries.find(key_);
	if (iter == entries.end())
		return assume;
	string val = iter->second;
	boost::algorithm::to_lower(val);
	if (iter->second == "true" || iter->second == "yes" || iter->second == "1")
		return true;
	else if (iter->second == "false" || iter->second == "no" || iter->second == "0")
		return false;
	return assume;
}

const string& ini_section::read_string(const string& key, const std::string& assume) const {
	// Not using operator[] so const function is allowed so ini_section's
	// can be passed const without losing read_string() functionality
	std::string key_(key);
	boost::to_lower(key_);
	keymap::const_iterator iter = entries.find(key_);
	return iter == entries.end() ? assume : iter->second;
}

double ini_section::read_double(const string& key, double assume) const {
	// Hacky, not using operator[] so const function is allowed so ini_section's
	// can be passed const without losing read_string() functionality
	std::string key_(key);
	boost::to_lower(key_);
	keymap::const_iterator iter = entries.find(key_);
	if (iter == entries.end())
		return assume;

	string lexical = iter->second;
	boost::algorithm::replace_all(lexical, ",", ".");

	try {
		return lexical_cast<double>(lexical);
	}
	catch (boost::bad_lexical_cast) {
		return assume;
	}
}

keymap::const_iterator ini_section::begin() const {
	return entries.begin();
}

keymap::const_iterator ini_section::end() const {
	return entries.end();
}

keymap::iterator ini_section::begin() {
	return entries.begin();
}

keymap::iterator ini_section::end() {
	return entries.end();
}

int ini_section::read_int(const string& key, int assume) const {
	std::string key_(key);
	boost::to_lower(key_);
	keymap::const_iterator iter = entries.find(key_);
	if (iter == entries.end())
		return assume;
	try {
		return lexical_cast<int>(iter->second);
	}

	catch (boost::bad_lexical_cast) {
		return 0;
	}
}

void ini_section::set_value(const string& key, const string& value) {
	std::string key_(key);
	boost::to_lower(key_);
	entries[key_] = value;
}

const std::string& ini_section::lookup(const std::string& value) const {
	for (keymap::const_iterator iter = entries.begin(); iter != entries.end(); iter++)
		if (iter->second == value)
			return iter->first;
	return nullvalue;
}

string ini_section::get_concatenated_values(Range R) const {
	int low = R.first,
		high = R.second;
	std::ostringstream os;
	for (int i = low; i <= high; i++) {
		os << read_string(lexical_cast<string>(i));
	}
	return os.str();
}

string ini_section::get_concatenated_values() const {
	return get_concatenated_values( std::pair<int, int>(1, entries.size()) );
}

string ini_section::get_name() const {
	return section_name;
}

string ini_section::deconcatenate_values(const string& input) {
	string::const_iterator si = input.begin();
	std::stringstream os;

	int line_i = 1;
	while (si < input.end()) {
		char line[80];
		int cb_line = std::min((int)(input.end() - si), 70);
		memcpy(line, &*si, cb_line);
		line[cb_line] = 0;
		si += cb_line;
		os << line_i++ << '=' << line << std::endl;
	}
	os << std::endl;
	return os.str();
}