#pragma once

#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "File.h"

// Describes classes ini_file, ini_section, ini_Entry
class ini_file; class ini_section; class ini_Entry;

// typedefs
typedef std::pair<int, int> Range;
typedef std::map<std::string, std::string> keymap;
typedef std::map<std::string, boost::shared_ptr<ini_section>> sectionmap;

// ini_file consists of a number of ini sections
class ini_file {
private:

	sectionmap sections;
	std::string name;

	// Points to currently processed section (if any)
	boost::shared_ptr<ini_section> cur_section;
	int process_line(std::string& line);

public:

	ini_file(const std::string& path);
	ini_file(const boost::shared_ptr<File> f);
	ini_file();
	~ini_file() {
		sections.clear();
	}
	// Writes the ini file in a way such that reloading it would result in the
	// same state, yet also a readable format

	// output to new file
	int output(const std::string& path) const;

	// output to ostream
	int output(std::ostream& os) const;

	// output to this->Name
	int output() const;

	// load from ifstream
	int parse(std::ifstream& in);

	// load from vfs::file
	int parse(const boost::shared_ptr<File>& f);

	// return filename
	std::string get_name() const;

	// returns value of (key,value) of pair indexed by key in section
	// not const, might modify current section
	std::string get_key(const std::string& section, const std::string& key);

	// returns value of (key,value) pair indexed by key of last used section
	std::string get_key(const std::string& key) const;

	// sets sections, gives confirmation
	bool set_current_section(const std::string& section);

	// sets sections, gives confirmation
	ini_section& get_current_section();

	// sets sections, gives confirmation
	const ini_section& get_current_section() const;

	// sets sections, gives confirmation
	ini_section& get_section(const std::string& section);

	void set_value(const std::string& section, const std::string& key, const std::string& value);
	void set_value(const std::string& key, const std::string& value);

	sectionmap::const_iterator begin() const;
	sectionmap::const_iterator end() const;
	sectionmap::iterator begin() ;
	sectionmap::iterator end();
};

// ini_section is a number of ini_Entry's that is contained within a file
class ini_section {
private:

	// Internal representation is a mapping of pairs
	// (std::string key, std::string value)
	keymap entries;
	std::string section_name;

public:

	// Constructors
	ini_section(const std::string& section_name);
	~ini_section() {
		entries.clear();
	}

	// parses (key,value) pairs and adds them into the map
	// Returns number of added pairs
	int parse_line(const std::string& line);
	int parse_lines(const std::vector<std::string>& section_text);

	// clears all entries
	void clear();

	// Writes this section in file-format to a stream
	// Returns total number of bytes written
	int output(std::ostream& os) const;

	// Getter/setters
	std::string get_name() const;
	static const std::string nullvalue;
	const std::string& read_string(const std::string& key, const std::string& assume = "") const;
	const std::string& read_string(int num) const;
	bool read_bool(const std::string& key, bool assume = false) const;
	int read_int(const std::string& key, int assume = 0) const;
	double read_double(const std::string& key, double assume = 0.0) const;
	void set_value(const std::string& key, const std::string& value);

	// lookup which, if any, index has given value
	const std::string& lookup(const std::string& value) const;

	// Returns the concatenated values of keys in range R
	std::string get_concatenated_values(Range R) const;

	// Returns the concatenated values of keys in range < 1 .. get_numkeys >
	std::string get_concatenated_values() const;

	// Separates string every 70 characters
	static std::string deconcatenate_values(const std::string& input);

	// returns number of keys
	int num_keys() const {
		return entries.size();
	}

	// Iterators to go through all ini keys of a section
	// @TODO: find something better
	keymap::const_iterator begin() const;
	keymap::const_iterator end() const;
	keymap::iterator begin();
	keymap::iterator end();
};

// removes unimportant parts from line
void fix_line(std::string &line);