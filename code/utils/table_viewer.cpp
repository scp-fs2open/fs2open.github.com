#include "table_viewer.h"

#include "cfile/cfile.h"
#include "parse/parselo.h"

namespace {

void append_header(SCP_string& output, const SCP_string& filename)
{
	output += "-- ";
	output += filename;
	output += "  -------------------------------\r\n";
}

void trim_newline(char* line)
{
	size_t len = strlen(line);
	while (len > 0 && line[len - 1] == '\n') {
		line[len - 1] = 0;
		--len;
	}
}

bool strip_and_match_entry_prefix(const char* line, int& comment, const char* entry_prefix, SCP_string* extracted_name = nullptr)
{
	char line_without_comments[256];
	int i;
	int j;

	for (i = j = 0; line[i] && j < static_cast<int>(sizeof(line_without_comments)) - 1; ++i) {
		if (line[i] == '/' && line[i + 1] == '/') {
			break;
		}

		if (line[i] == '/' && line[i + 1] == '*') {
			comment = 1;
			++i;
			continue;
		}

		if (line[i] == '*' && line[i + 1] == '/') {
			comment = 0;
			++i;
			continue;
		}

		if (!comment) {
			line_without_comments[j++] = line[i];
		}
	}

	line_without_comments[j] = 0;
	const auto prefix_len = static_cast<int>(strlen(entry_prefix));
	if (strnicmp(line_without_comments, entry_prefix, prefix_len) != 0) {
		return false;
	}

	drop_trailing_white_space(line_without_comments);
	i = prefix_len;
	while (line_without_comments[i] == ' ' || line_without_comments[i] == '\t' || line_without_comments[i] == '@') {
		++i;
	}

	if (extracted_name != nullptr) {
		*extracted_name = line_without_comments + i;
	}

	return true;
}

void append_file_contents(SCP_string& output, CFILE* fp)
{
	char line[256];
	while (cfgets(line, 255, fp)) {
		trim_newline(line);
		output += line;
		output += "\r\n";
	}
}

void append_matching_entry(SCP_string& output, CFILE* fp, const SCP_string& file_name, const char* entry_name, const char* entry_prefix)
{
	char line[256];
	int found = 0;
	int comment = 0;

	while (cfgets(line, 255, fp)) {
		trim_newline(line);

		SCP_string found_name;
		if (strip_and_match_entry_prefix(line, comment, entry_prefix, &found_name)) {
			found = 0;
			if (stricmp(found_name.c_str(), entry_name) == 0) {
				append_header(output, file_name);
				found = 1;
			}
		}

		if (found) {
			output += line;
			output += "\r\n";
		}
	}
}

} // namespace

namespace table_viewer {

SCP_string get_table_entry_text(const char* table_filename,
	const char* modular_pattern,
	const char* entry_name,
	const char* missing_table_message,
	const char* entry_prefix)
{
	SCP_string output;

	auto fp = cfopen(table_filename, "r");
	if (!fp) {
		return missing_table_message != nullptr ? SCP_string(missing_table_message) : SCP_string();
	}

	append_matching_entry(output, fp, table_filename, entry_name, entry_prefix);
	cfclose(fp);

	SCP_vector<SCP_string> table_files;
	const auto num_files = cf_get_file_list(table_files, CF_TYPE_TABLES, NOX(modular_pattern), CF_SORT_REVERSE);

	for (int n = 0; n < num_files; ++n) {
		table_files[n] += ".tbm";
		fp = cfopen(table_files[n].c_str(), "r");
		if (!fp) {
			continue;
		}

		append_matching_entry(output, fp, table_files[n], entry_name, entry_prefix);
		cfclose(fp);
	}

	return output;
}

SCP_string get_complete_table_text(const char* table_filename, const char* modular_pattern, const char* missing_table_message)
{
	SCP_string output;
	auto fp = cfopen(table_filename, "r");
	if (!fp) {
		return missing_table_message != nullptr ? SCP_string(missing_table_message) : SCP_string();
	}

	append_header(output, table_filename);
	append_file_contents(output, fp);
	cfclose(fp);

	SCP_vector<SCP_string> table_files;
	const auto num_files = cf_get_file_list(table_files, CF_TYPE_TABLES, NOX(modular_pattern), CF_SORT_REVERSE);

	for (int n = 0; n < num_files; ++n) {
		table_files[n] += ".tbm";
		fp = cfopen(table_files[n].c_str(), "r");
		if (!fp) {
			continue;
		}

		append_header(output, table_files[n]);
		append_file_contents(output, fp);
		cfclose(fp);
	}

	return output;
}

} // namespace table_viewer
