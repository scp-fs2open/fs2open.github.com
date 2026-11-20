// fieldscan: generates the field lists the checkpoint codec uses to visit structs.
//
// Usage: fieldscan <source root> <struct list> <output header>
//
// The struct list has one "<struct name> <header relative to the source root>" pair per line, or
// "enum <enum name> <header>" for enums; '#' starts a comment.  For every listed struct the tool writes
//
//     CHECKPOINT_FIELDS_BEGIN(name)
//     CHECKPOINT_FIELD(name, member)
//     ...
//     CHECKPOINT_FIELDS_END(name)
//
// and for every listed enum (enum, enum class or FLAG_LIST; the name may be qualified, e.g. Ship::Ship_Flags)
//
//     CHECKPOINT_ENUM_BEGIN(name)
//     CHECKPOINT_ENUM_VALUE(name, enumerator)
//     ...
//     CHECKPOINT_ENUM_END(name)
//
// preceded by includes of the listed headers and followed by CHECKPOINT_SCANNED_STRUCTS(X),
// CHECKPOINT_SCANNED_ENUMS(X) and CHECKPOINT_SCANNED_FLAGS(X), X-macros listing all scanned structs, all scanned enums
// and the enums among them that are flag lists (FLAG_LIST).
//
// Members are saved unless marked.  A line comment at the end of a member's line containing
//
//     @checkpoint skip: <reason>
//
// (on its own, or after the member's existing comment) leaves out the members declared by the statement that ends on
// that line; the generated list notes them as "// skipped: member".  The reason is required.  A marker that belongs to
// no data member of a listed struct (a line without one, a method, a struct that isn't in the list) is an error, as is
// an unknown kind.
//
// Only member names are extracted.  Their types are deduced by the compiler from member pointers, so a wrong or
// inaccessible name is a compile error rather than a silent mistake.  The scanner understands the subset of C++ used
// in engine struct definitions: data members (including several declarators per line, arrays, bit fields, default
// initializers and function pointers), methods, nested types, access specifiers, and anonymous unions or structs that
// declare members.

#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using std::string;

namespace {

bool is_ident_char(char c)
{
	return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

// Marks a "// @checkpoint" comment in the stripped source: MARKER_BEGIN, the line number, MARKER_END.
constexpr char MARKER_BEGIN = '\x01';
constexpr char MARKER_END = '\x02';

struct Diagnostics {
	string header;
	int errors = 0;

	void error(int line, const string& message)
	{
		std::cerr << header << ":" << line << ": fieldscan: " << message << "\n";
		++errors;
	}
};

string trim(const string& s);

// Turns the text of a line comment into a marker if it is a valid "@checkpoint" annotation.
void parse_marker(const string& comment, int line, string& out, std::set<int>& markers, Diagnostics& diag)
{
	const string prefix = "@checkpoint";
	auto at = comment.find(prefix);
	if (at == string::npos || (at > 0 && !std::isspace(static_cast<unsigned char>(comment[at - 1]))))
		return;
	auto text = comment.substr(at + prefix.size());
	if (!text.empty() && !std::isspace(static_cast<unsigned char>(text[0])))
		return;
	text = trim(text);
	auto colon = text.find(':');
	auto kind = trim(text.substr(0, colon));
	if (kind != "skip") {
		diag.error(line, "unknown @checkpoint kind \"" + kind + "\"; the only kind is \"skip\"");
		return;
	}
	if (colon == string::npos || trim(text.substr(colon + 1)).empty()) {
		diag.error(line, "@checkpoint skip needs a reason: @checkpoint skip: <reason>");
		return;
	}
	out += MARKER_BEGIN;
	out += std::to_string(line);
	out += MARKER_END;
	markers.insert(line);
}

// Removes comments and preprocessor lines and blanks out string and character literals, keeping the line structure.
// "// @checkpoint" comments become markers.
string strip_source(const string& s, std::set<int>& markers, Diagnostics& diag)
{
	string out;
	out.reserve(s.size());
	bool line_start = true;
	int line = 1;

	for (size_t i = 0; i < s.size();) {
		char c = s[i];
		if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') {
			auto end = s.find('\n', i);
			if (end == string::npos)
				end = s.size();
			parse_marker(s.substr(i + 2, end - i - 2), line, out, markers, diag);
			i = end;
			continue;
		}
		if (c == '/' && i + 1 < s.size() && s[i + 1] == '*') {
			auto end = s.find("*/", i + 2);
			end = (end == string::npos) ? s.size() : end + 2;
			out += ' ';
			for (; i < end; ++i) {
				if (s[i] == '\n') {
					out += '\n';
					++line;
				}
			}
			continue;
		}
		if (c == '"' || (c == '\'' && (i == 0 || !is_ident_char(s[i - 1])))) {
			char quote = c;
			for (++i; i < s.size() && s[i] != quote && s[i] != '\n'; ++i) {
				if (s[i] == '\\')
					++i;
			}
			++i;
			out += '0';
			continue;
		}
		if (line_start && c == '#') {
			// preprocessor directive, including continuation lines
			while (i < s.size() && s[i] != '\n') {
				if (s[i] == '\\' && i + 1 < s.size() && s[i + 1] == '\n') {
					out += '\n';
					++line;
					++i;
				}
				++i;
			}
			continue;
		}

		if (c == '\n') {
			line_start = true;
			++line;
		} else if (!std::isspace(static_cast<unsigned char>(c))) {
			line_start = false;
		}

		out += c;
		++i;
	}
	return out;
}

// Removes the markers from a piece of stripped source.
string without_markers(const string& s)
{
	string out;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == MARKER_BEGIN) {
			while (i < s.size() && s[i] != MARKER_END)
				++i;
			continue;
		}
		out += s[i];
	}
	return out;
}

bool word_at(const string& s, size_t pos, const string& word)
{
	if (s.compare(pos, word.size(), word) != 0)
		return false;
	if (pos > 0 && is_ident_char(s[pos - 1]))
		return false;
	auto end = pos + word.size();
	return end >= s.size() || !is_ident_char(s[end]);
}

bool starts_with_word(const string& s, const string& word)
{
	return word_at(s, 0, word);
}

bool contains_word(const string& s, const string& word)
{
	for (auto pos = s.find(word); pos != string::npos; pos = s.find(word, pos + 1)) {
		if (word_at(s, pos, word))
			return true;
	}
	return false;
}

string trim(const string& s)
{
	auto first = s.find_first_not_of(" \t\r\n");
	if (first == string::npos)
		return {};
	auto last = s.find_last_not_of(" \t\r\n");
	return s.substr(first, last - first + 1);
}

// Finds the opening brace of the definition of "struct name" or "class name" and returns the position after it.
size_t find_definition(const string& s, const string& name)
{
	for (const char* keyword : {"struct", "class"}) {
		string kw = keyword;
		for (auto pos = s.find(kw); pos != string::npos; pos = s.find(kw, pos + 1)) {
			if (!word_at(s, pos, kw))
				continue;
			auto p = pos + kw.size();
			while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p])))
				++p;
			if (!word_at(s, p, name))
				continue;
			p += name.size();
			while (p < s.size() && s[p] != '{' && s[p] != ';' && s[p] != '(')
				++p;
			if (p < s.size() && s[p] == '{')
				return p + 1;
		}
	}
	return string::npos;
}

// Extracts the name from one declarator: the last identifier before an initializer, array bound or bit field width,
// or the name inside "(*name)" for function pointers.
string declarator_name(string decl)
{
	auto fp = decl.find("(*");
	if (fp != string::npos) {
		string name;
		for (auto p = fp + 2; p < decl.size() && is_ident_char(decl[p]); ++p)
			name += decl[p];
		return name;
	}

	int depth = 0;
	for (size_t i = 0; i < decl.size(); ++i) {
		char c = decl[i];
		if (c == '<') {
			++depth;
		} else if (c == '>') {
			--depth;
		} else if (depth == 0 && (c == '=' || c == '[')) {
			decl.resize(i);
			break;
		} else if (depth == 0 && c == ':' && (i + 1 >= decl.size() || decl[i + 1] != ':') && (i == 0 || decl[i - 1] != ':')) {
			decl.resize(i);
			break;
		}
	}

	decl = trim(decl);
	auto begin = decl.size();
	while (begin > 0 && is_ident_char(decl[begin - 1]))
		--begin;
	return decl.substr(begin);
}

// Turns one member declaration statement into the names of the data members it declares (none for methods etc.).
void collect_members(string stmt, std::vector<string>& out)
{
	stmt = trim(stmt);
	for (bool changed = true; changed;) {
		changed = false;
		for (const char* access : {"public", "protected", "private"}) {
			if (starts_with_word(stmt, access)) {
				auto colon = stmt.find(':');
				if (colon != string::npos) {
					stmt = trim(stmt.substr(colon + 1));
					changed = true;
				}
			}
		}
	}
	if (stmt.empty())
		return;

	for (const char* skip : {"using", "typedef", "friend", "static", "enum", "template", "explicit", "virtual", "constexpr", "inline", "static_assert"}) {
		if (starts_with_word(stmt, skip))
			return;
	}
	if (contains_word(stmt, "operator"))
		return;

	// A '(' before any initializer means a method, unless it is a function pointer.
	auto init = stmt.find_first_of("={");
	auto paren = stmt.find('(');
	if (paren != string::npos && (init == string::npos || paren < init) && stmt.compare(paren, 2, "(*") != 0)
		return;

	// Split the declarators at top-level commas.
	std::vector<string> declarators;
	string current;
	int depth = 0;
	for (char c : stmt) {
		if (c == '<' || c == '(' || c == '[' || c == '{')
			++depth;
		else if (c == '>' || c == ')' || c == ']' || c == '}')
			--depth;
		if (c == ',' && depth == 0) {
			declarators.push_back(current);
			current.clear();
		} else {
			current += c;
		}
	}
	declarators.push_back(current);

	for (auto& decl : declarators) {
		auto name = declarator_name(decl);
		if (!name.empty() && !std::isdigit(static_cast<unsigned char>(name[0])))
			out.push_back(name);
	}
}

struct Member {
	string name;
	bool skipped;
};

int line_at(const string& s, size_t pos)
{
	int line = 1;
	for (size_t i = 0; i < pos && i < s.size(); ++i) {
		if (s[i] == '\n')
			++line;
	}
	return line;
}

// Collects the data members of the struct whose body starts at pos, applying the markers in it.
std::vector<Member> scan_members(const string& s, size_t pos, const string& struct_name, std::set<int>& consumed, Diagnostics& diag)
{
	std::vector<Member> members;
	string stmt;
	int paren = 0;
	int line = line_at(s, pos);

	size_t last_begin = 0;		// the members of the last statement
	size_t last_end = 0;
	int last_line = -1;			// the line the last statement ended on

	auto misplaced = [&](int marker) {
		diag.error(marker, "@checkpoint marker in " + struct_name + " does not belong to a data member");
		consumed.insert(marker);
	};

	// the statement that just ended declared the members from begin on
	auto finish_statement = [&](size_t begin) {
		last_begin = begin;
		last_end = members.size();
		last_line = line;
	};

	auto collect = [&](const string& statement) {
		std::vector<string> names;
		collect_members(statement, names);
		for (auto& name : names)
			members.push_back({name, false});
	};

	while (pos < s.size()) {
		char c = s[pos++];

		if (c == MARKER_BEGIN) {
			auto end = s.find(MARKER_END, pos);
			int marker = std::stoi(s.substr(pos, end - pos));
			pos = end + 1;
			// a marker belongs to the statement that ended on its line
			if (last_line != line || !trim(stmt).empty() || last_begin == last_end) {
				misplaced(marker);
			} else {
				for (auto i = last_begin; i < last_end; ++i)
					members[i].skipped = true;
				consumed.insert(marker);
			}
			continue;
		}

		if (c == '\n')
			++line;

		if (c == '}')
			break; // end of the struct

		if (c == '{') {
			bool is_type = contains_word(stmt, "struct") || contains_word(stmt, "class") || contains_word(stmt, "union") || contains_word(stmt, "enum");
			bool is_method = !is_type && stmt.find('(') != string::npos && stmt.find('=') == string::npos;

			if (!is_type && !is_method) {
				// a braced initializer: keep the statement and continue
				for (int depth = 1; depth > 0 && pos < s.size(); ++pos) {
					if (s[pos] == '{')
						++depth;
					else if (s[pos] == '}')
						--depth;
					else if (s[pos] == '\n')
						++line;
				}
				continue;
			}

			// skip the braced block
			for (int depth = 1; depth > 0 && pos < s.size(); ++pos) {
				if (s[pos] == '{')
					++depth;
				else if (s[pos] == '}')
					--depth;
				else if (s[pos] == '\n')
					++line;
			}

			auto begin = members.size();
			if (is_type) {
				// "union { ... } a, b;" declares members, "struct x { ... };" does not
				auto semicolon = s.find(';', pos);
				if (semicolon == string::npos)
					break;
				auto tail = s.substr(pos, semicolon - pos);
				for (char t : tail) {
					if (t == '\n')
						++line;
				}
				tail = trim(without_markers(tail));
				if (!tail.empty())
					collect("nested " + tail);
				pos = semicolon + 1;
			}
			stmt.clear();
			finish_statement(begin);
			continue;
		}

		if (c == '(')
			++paren;
		else if (c == ')')
			--paren;

		if (c == ';' && paren == 0) {
			auto begin = members.size();
			collect(stmt);
			stmt.clear();
			finish_statement(begin);
			continue;
		}
		stmt += c;
	}

	return members;
}

// Finds the opening brace of the definition of an enum ("enum name", "enum class name", "FLAG_LIST(name)" or
// "typedef enum { ... } name;") and returns the position after it.
size_t find_enum_definition(const string& s, const string& name, bool& is_flag_list)
{
	is_flag_list = false;
	for (auto pos = s.find(name); pos != string::npos; pos = s.find(name, pos + 1)) {
		if (!word_at(s, pos, name))
			continue;

		// typedef enum { ... } name;
		auto close = s.find_last_not_of(" \t\r\n", pos == 0 ? 0 : pos - 1);
		if (close != string::npos && s[close] == '}') {
			int depth = 0;
			size_t open = close;
			for (; open != string::npos; --open) {
				if (s[open] == '}')
					++depth;
				else if (s[open] == '{' && --depth == 0)
					break;
				if (open == 0) {
					open = string::npos;
					break;
				}
			}
			if (open != string::npos) {
				auto head = s.substr(open > 64 ? open - 64 : 0, open > 64 ? 64 : open);
				if (contains_word(head, "enum"))
					return open + 1;
			}
			continue;
		}

		// what precedes the name must introduce an enum
		auto before = s.substr(0, pos);
		auto last = before.find_last_not_of(" \t\r\n(");
		if (last == string::npos)
			continue;
		auto start = before.find_last_of(" \t\r\n(", last);
		auto word = before.substr(start == string::npos ? 0 : start + 1, last - (start == string::npos ? 0 : start + 1) + 1);
		if (word != "enum" && word != "class" && word != "struct" && word != "FLAG_LIST")
			continue;
		if ((word == "class" || word == "struct") && !contains_word(before.substr(before.size() > 32 ? before.size() - 32 : 0), "enum"))
			continue;

		auto p = pos + name.size();
		while (p < s.size() && s[p] != '{' && s[p] != ';' && s[p] != '(')
			++p;
		if (p < s.size() && s[p] == '{') {
			is_flag_list = (word == "FLAG_LIST");
			return p + 1;
		}
	}
	return string::npos;
}

// Extracts the enumerator names of an enum body; values and the NUM_VALUES marker of flag lists are left out.
std::vector<string> scan_enumerators(const string& s, size_t pos)
{
	std::vector<string> names;
	string current;
	int depth = 0;
	auto finish = [&]() {
		auto decl = trim(without_markers(current));
		auto equals = decl.find('=');
		if (equals != string::npos)
			decl = trim(decl.substr(0, equals));
		if (!decl.empty() && decl != "NUM_VALUES")
			names.push_back(decl);
		current.clear();
	};
	while (pos < s.size()) {
		char c = s[pos++];
		if (c == '(' || c == '{')
			++depth;
		else if (c == ')')
			--depth;
		else if (c == '}') {
			if (depth == 0)
				break;
			--depth;
		}
		if (c == ',' && depth == 0)
			finish();
		else
			current += c;
	}
	finish();
	return names;
}

bool read_file(const string& path, string& out)
{
	std::ifstream in(path, std::ios::binary);
	if (!in)
		return false;
	std::stringstream buffer;
	buffer << in.rdbuf();
	out = buffer.str();
	return true;
}

} // namespace

int main(int argc, char** argv)
{
	if (argc != 4) {
		std::cerr << "usage: fieldscan <source root> <struct list> <output header>\n";
		return 2;
	}
	string root = argv[1];
	string list_path = argv[2];
	string output_path = argv[3];

	string list;
	if (!read_file(list_path, list)) {
		std::cerr << "fieldscan: cannot read " << list_path << "\n";
		return 1;
	}

	// the struct definitions come first, so the list needs no includes kept up to date elsewhere
	std::ostringstream includes;
	std::vector<string> included;
	std::ostringstream out;

	int errors = 0;
	std::vector<string> scanned;
	std::vector<string> scanned_enums;
	std::vector<string> scanned_flags;

	// each header is read and stripped once; markers are checked once all its structs have been scanned
	struct Header {
		string source;
		std::set<int> markers;
		std::set<int> consumed;
		Diagnostics diag;
	};
	std::map<string, Header> headers;
	std::istringstream lines(list);
	string line;
	while (std::getline(lines, line)) {
		auto hash = line.find('#');
		if (hash != string::npos)
			line.resize(hash);
		std::istringstream fields(line);
		string name, header;
		if (!(fields >> name))
			continue;
		bool is_enum = (name == "enum");
		if (is_enum && !(fields >> name)) {
			std::cerr << "fieldscan: no enum name given\n";
			++errors;
			continue;
		}
		if (!(fields >> header)) {
			std::cerr << "fieldscan: no header given for " << name << "\n";
			++errors;
			continue;
		}

		auto known = headers.find(header);
		if (known == headers.end()) {
			string header_path = root;
			header_path += '/';
			header_path += header;

			string raw;
			if (!read_file(header_path, raw)) {
				std::cerr << "fieldscan: cannot read " << header_path << "\n";
				++errors;
				continue;
			}
			Header parsed;
			parsed.diag.header = header_path;
			parsed.source = strip_source(raw, parsed.markers, parsed.diag);
			known = headers.emplace(header, std::move(parsed)).first;
		}
		auto& parsed = known->second;
		const auto& source = parsed.source;

		auto unqualified = name.substr(name.rfind(':') == string::npos ? 0 : name.rfind(':') + 1);
		bool is_flag_list = false;
		auto body = is_enum ? find_enum_definition(source, unqualified, is_flag_list) : find_definition(source, name);
		if (body == string::npos) {
			std::cerr << "fieldscan: no definition of " << name << " in " << header << "\n";
			++errors;
			continue;
		}

		bool seen = false;
		for (const auto& h : included)
			seen = seen || h == header;
		if (!seen) {
			included.push_back(header);
			includes << "#include \"" << header << "\"\n";
		}

		if (is_enum) {
			out << "CHECKPOINT_ENUM_BEGIN(" << name << ")\n";
			for (const auto& enumerator : scan_enumerators(source, body))
				out << "CHECKPOINT_ENUM_VALUE(" << name << ", " << enumerator << ")\n";
			out << "CHECKPOINT_ENUM_END(" << name << ")\n\n";
			scanned_enums.push_back(name);
			if (is_flag_list)
				scanned_flags.push_back(name);
			continue;
		}

		out << "CHECKPOINT_FIELDS_BEGIN(" << name << ")\n";
		for (const auto& member : scan_members(source, body, name, parsed.consumed, parsed.diag)) {
			if (member.skipped)
				out << "// skipped: " << member.name << "\n";
			else
				out << "CHECKPOINT_FIELD(" << name << ", " << member.name << ")\n";
		}
		out << "CHECKPOINT_FIELDS_END(" << name << ")\n\n";
		scanned.push_back(name);
	}

	// all scanned structs, for code that needs to handle each of them
	out << "#define CHECKPOINT_SCANNED_STRUCTS(X)";
	for (const auto& name : scanned)
		out << " \\\n\tX(" << name << ")";
	out << "\n";

	// all scanned enums
	out << "#define CHECKPOINT_SCANNED_ENUMS(X)";
	for (const auto& name : scanned_enums)
		out << " \\\n\tX(" << name << ")";
	out << "\n";

	// the scanned enums that are flag lists
	out << "#define CHECKPOINT_SCANNED_FLAGS(X)";
	for (const auto& name : scanned_flags)
		out << " \\\n\tX(" << name << ")";
	out << "\n";

	// markers that no listed struct took, e.g. in a struct that isn't in the list
	for (auto& entry : headers) {
		auto& parsed = entry.second;
		for (int marker : parsed.markers) {
			if (parsed.consumed.count(marker) == 0)
				parsed.diag.error(marker, "@checkpoint marker outside the listed structs of this header");
		}
		errors += parsed.diag.errors;
	}

	if (errors > 0)
		return 1;

	string result = "// Generated by fieldscan from " + list_path + ". Do not edit.\n\n" + includes.str() + "\n" + out.str();

	// only touch the output when it changed, so dependent sources don't rebuild needlessly
	string previous;
	if (read_file(output_path, previous) && previous == result)
		return 0;

	std::ofstream file(output_path, std::ios::binary | std::ios::trunc);
	if (!file) {
		std::cerr << "fieldscan: cannot write " << output_path << "\n";
		return 1;
	}
	file << result;
	return 0;
}
