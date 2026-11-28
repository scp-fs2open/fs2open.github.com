// fieldscan: generates the field lists the checkpoint codec uses to visit structs.
//
// Usage: fieldscan <source root> <header list> <output header>
//
// The header list names one header per line, absolute or relative to the source root.  The structs and enums the
// codec visits are marked where they are defined, with a line comment at the end of the line that names them:
//
//     class ship	// @checkpoint fields
//     FLAG_LIST(Ship_Flags) {	// @checkpoint fields
//     } debris_genre_t;	// @checkpoint fields		(typedef enum { ... } name;)
//
// For every marked struct the tool writes
//
//     CHECKPOINT_FIELDS_BEGIN(name)
//     CHECKPOINT_FIELD(name, member)
//     ...
//     CHECKPOINT_FIELDS_END(name)
//
// and for every marked enum (enum, enum class or FLAG_LIST)
//
//     CHECKPOINT_ENUM_BEGIN(name)
//     CHECKPOINT_ENUM_VALUE(name, enumerator)
//     ...
//     CHECKPOINT_ENUM_END(name)
//
// with names qualified by their enclosing namespaces and classes (e.g. Ship::Ship_Flags), preceded by includes of the
// headers and followed by CHECKPOINT_SCANNED_STRUCTS(X), CHECKPOINT_SCANNED_ENUMS(X) and CHECKPOINT_SCANNED_FLAGS(X),
// X-macros listing all scanned structs, all scanned enums and the enums among them that are flag lists.
//
// Members are saved unless marked.  A line comment at the end of a member's line containing
//
//     @checkpoint skip: <reason>
//
// (on its own, or after the member's existing comment) leaves out the members declared by the statement that ends on
// that line; the generated list notes them as "// skipped: member".  The reason is required.  A marker that belongs to
// no data member of a marked struct (a line without one, a method, a struct that isn't marked), a "fields" marker on a
// line that defines nothing, a name marked twice and an unknown kind are errors.
//
// Only member names are extracted.  Their types are deduced by the compiler from member pointers, so a wrong or
// inaccessible name is a compile error rather than a silent mistake.  The scanner understands the subset of C++ used
// in engine struct definitions: data members (including several declarators per line, arrays, bit fields, default
// initializers and function pointers), methods, nested types, access specifiers, and anonymous unions or structs that
// declare members.

#include <algorithm>
#include <cctype>
#include <cstring>
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

// Marks a "// @checkpoint" comment in the stripped source: MARKER_BEGIN, the kind, the line number, MARKER_END.
constexpr char MARKER_BEGIN = '\x01';
constexpr char MARKER_END = '\x02';
constexpr char SKIP = 'S';
constexpr char FIELDS = 'F';

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
void parse_marker(const string& comment, int line, string& out, std::map<int, char>& markers, Diagnostics& diag)
{
	const string prefix = "@checkpoint";
	auto at = comment.find(prefix);
	if (at == string::npos || (at > 0 && !std::isspace(static_cast<unsigned char>(comment[at - 1]))))
		return;
	// a comment quoting code with a marker, e.g. in documentation
	if (comment.rfind("//", at) != string::npos)
		return;
	auto text = comment.substr(at + prefix.size());
	if (!text.empty() && !std::isspace(static_cast<unsigned char>(text[0])))
		return;
	text = trim(text);
	auto colon = text.find(':');
	auto kind = trim(text.substr(0, colon));
	char code;
	if (kind == "fields") {
		code = FIELDS;
	} else if (kind == "skip") {
		if (colon == string::npos || trim(text.substr(colon + 1)).empty()) {
			diag.error(line, "@checkpoint skip needs a reason: @checkpoint skip: <reason>");
			return;
		}
		code = SKIP;
	} else {
		diag.error(line, "unknown @checkpoint kind \"" + kind + "\"; the kinds are \"fields\" and \"skip\"");
		return;
	}
	out += MARKER_BEGIN;
	out += code;
	out += std::to_string(line);
	out += MARKER_END;
	markers[line] = code;
}

// Removes comments and preprocessor lines and blanks out string and character literals, keeping the line structure.
// "// @checkpoint" comments become markers.
string strip_source(const string& s, std::map<int, char>& markers, Diagnostics& diag)
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
			char kind = s[pos];
			int marker = std::stoi(s.substr(pos + 1, end - pos - 1));
			pos = end + 1;
			if (kind != SKIP)
				continue;	// the definition of a nested type
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

// The identifier that starts at pos, or after the whitespace at pos.
string identifier_at(const string& s, size_t pos)
{
	while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos])))
		++pos;
	string name;
	for (; pos < s.size() && is_ident_char(s[pos]); ++pos)
		name += s[pos];
	return name;
}

// The names of the namespaces and classes enclosing pos, outermost first.
std::vector<string> scopes_at(const string& s, size_t pos)
{
	std::vector<string> scopes;
	string stmt;
	for (size_t i = 0; i < pos; ++i) {
		char c = s[i];
		if (c == '{') {
			auto head = trim(without_markers(stmt));
			string name;
			if (starts_with_word(head, "namespace")) {
				name = identifier_at(head, 9);
			} else if (!contains_word(head, "enum") && head.find('(') == string::npos) {
				for (const char* keyword : {"struct", "class"}) {
					for (auto at = head.find(keyword); name.empty() && at != string::npos; at = head.find(keyword, at + 1)) {
						if (word_at(head, at, keyword))
							name = identifier_at(head, at + strlen(keyword));
					}
				}
			}
			scopes.push_back(name);
			stmt.clear();
		} else if (c == '}') {
			if (!scopes.empty())
				scopes.pop_back();
			stmt.clear();
		} else if (c == ';') {
			stmt.clear();
		} else {
			stmt += c;
		}
	}
	return scopes;
}

struct Definition {
	string name;		// qualified
	size_t body = string::npos;	// the position after the opening brace
	bool is_enum = false;
	bool is_flag_list = false;
};

// The definition a "fields" marker at pos marks: the struct, class or enum named on the marker's line, or, for a line
// that closes a definition ("} name;"), the typedef'd struct or enum it closes.
bool definition_at(const string& s, size_t pos, Definition& def)
{
	auto line_start = s.rfind('\n', pos);
	line_start = (line_start == string::npos) ? 0 : line_start + 1;
	auto head = s.substr(line_start, pos - line_start);
	auto first = head.find_first_not_of(" \t\r");
	if (first == string::npos)
		return false;

	size_t name_from = string::npos;	// where the head's name starts
	size_t keyword_at = string::npos;	// where the definition starts, for its scope
	if (head[first] == '}') {
		// typedef struct/enum { ... } name;
		auto close = line_start + first;
		int depth = 0;
		size_t open = close;
		for (;; --open) {
			if (s[open] == '}')
				++depth;
			else if (s[open] == '{' && --depth == 0)
				break;
			if (open == 0)
				return false;
		}
		auto stmt_start = s.find_last_of(";{}", open == 0 ? 0 : open - 1);
		stmt_start = (stmt_start == string::npos) ? 0 : stmt_start + 1;
		auto intro = without_markers(s.substr(stmt_start, open - stmt_start));
		if (!contains_word(intro, "enum") && !contains_word(intro, "struct") && !contains_word(intro, "class"))
			return false;
		def.is_enum = contains_word(intro, "enum");
		def.body = open + 1;
		name_from = close + 1;
		keyword_at = stmt_start;
	} else {
		auto flag_list = head.find("FLAG_LIST(");
		if (flag_list != string::npos && word_at(head, flag_list, "FLAG_LIST")) {
			def.is_enum = def.is_flag_list = true;
			name_from = line_start + flag_list + 10;
		} else {
			for (const char* keyword : {"enum", "struct", "class"}) {
				for (auto at = head.find(keyword); name_from == string::npos && at != string::npos; at = head.find(keyword, at + 1)) {
					if (!word_at(head, at, keyword))
						continue;
					def.is_enum = (strcmp(keyword, "enum") == 0);
					auto after = at + strlen(keyword);
					// enum class name, enum struct name
					if (def.is_enum) {
						auto next = identifier_at(head, after);
						if (next == "class" || next == "struct")
							after = head.find(next, after) + next.size();
					}
					name_from = line_start + after;
				}
				if (name_from != string::npos)
					break;
			}
			if (name_from == string::npos)
				return false;
		}
		keyword_at = line_start + first;
		auto brace = s.find_first_of("{;", name_from);
		if (brace == string::npos || s[brace] != '{')
			return false;
		def.body = brace + 1;
	}

	def.name = identifier_at(s, name_from);
	if (def.name.empty())
		return false;
	string qualified;
	for (const auto& scope : scopes_at(s, keyword_at)) {
		if (!scope.empty())
			qualified += scope + "::";
	}
	def.name = qualified + def.name;
	return true;
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

// The path to include a header by: relative to the source root, with forward slashes.
string include_path(string path, string root)
{
	std::replace(path.begin(), path.end(), '\\', '/');
	std::replace(root.begin(), root.end(), '\\', '/');
	if (!root.empty() && root.back() != '/')
		root += '/';
	if (path.compare(0, root.size(), root) == 0)
		path = path.substr(root.size());
	return path;
}

int main(int argc, char** argv)
{
	if (argc != 4) {
		std::cerr << "usage: fieldscan <source root> <header list> <output header>\n";
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

	// sorted, so the output doesn't depend on the order of the list
	std::set<string> header_paths;
	{
		std::istringstream lines(list);
		string line;
		while (std::getline(lines, line)) {
			line = trim(line);
			if (!line.empty())
				header_paths.insert(include_path(line, root));
		}
	}

	std::ostringstream includes;
	std::ostringstream out;

	int errors = 0;
	std::vector<string> scanned;
	std::vector<string> scanned_enums;
	std::vector<string> scanned_flags;
	std::map<string, string> defined_in;	// marked name -> header

	for (const auto& header : header_paths) {
		auto path = header;
		if (header.find(':') == string::npos && header[0] != '/')
			path = string(root).append("/").append(header);
		string raw;
		if (!read_file(path, raw)) {
			std::cerr << "fieldscan: cannot read " << path << "\n";
			++errors;
			continue;
		}
		if (raw.find("@checkpoint") == string::npos)
			continue;

		std::map<int, char> markers;
		std::set<int> consumed;
		Diagnostics diag;
		diag.header = path;
		auto source = strip_source(raw, markers, diag);
		bool any = false;

		for (auto at = source.find(MARKER_BEGIN); at != string::npos; at = source.find(MARKER_BEGIN, at + 1)) {
			if (source[at + 1] != FIELDS)
				continue;
			int line = std::stoi(source.substr(at + 2, source.find(MARKER_END, at) - at - 2));
			consumed.insert(line);

			Definition def;
			if (!definition_at(source, at, def)) {
				diag.error(line, "@checkpoint fields must be at the end of the line that names a struct, class or enum definition");
				continue;
			}
			auto previous = defined_in.find(def.name);
			if (previous != defined_in.end()) {
				diag.error(line, def.name + " is already marked in " + previous->second);
				continue;
			}
			defined_in[def.name] = header;
			any = true;

			if (def.is_enum) {
				out << "CHECKPOINT_ENUM_BEGIN(" << def.name << ")\n";
				for (const auto& enumerator : scan_enumerators(source, def.body))
					out << "CHECKPOINT_ENUM_VALUE(" << def.name << ", " << enumerator << ")\n";
				out << "CHECKPOINT_ENUM_END(" << def.name << ")\n\n";
				scanned_enums.push_back(def.name);
				if (def.is_flag_list)
					scanned_flags.push_back(def.name);
				continue;
			}

			out << "CHECKPOINT_FIELDS_BEGIN(" << def.name << ")\n";
			for (const auto& member : scan_members(source, def.body, def.name, consumed, diag)) {
				if (member.skipped)
					out << "// skipped: " << member.name << "\n";
				else
					out << "CHECKPOINT_FIELD(" << def.name << ", " << member.name << ")\n";
			}
			out << "CHECKPOINT_FIELDS_END(" << def.name << ")\n\n";
			scanned.push_back(def.name);
		}

		if (any)
			includes << "#include \"" << header << "\"\n";

		// skip markers that no marked struct took, e.g. in a struct that isn't marked
		for (const auto& marker : markers) {
			if (consumed.count(marker.first) == 0)
				diag.error(marker.first, "@checkpoint skip outside the structs marked with @checkpoint fields");
		}
		errors += diag.errors;
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

	if (errors > 0)
		return 1;

	string result = "// Generated by fieldscan from the @checkpoint markers in the engine headers. Do not edit.\n\n"
		"#ifndef CHECKPOINT_CODEC_IMPLEMENTATION\n"
		"#error \"Only checkpoint/checkpoint_fields.cpp may include the generated checkpoint field lists\"\n"
		"#endif\n\n" + includes.str() + "\n" + out.str();

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
