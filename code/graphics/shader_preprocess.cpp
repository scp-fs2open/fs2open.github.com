#include "graphics/shader_preprocess.h"

#include "cfile/cfile.h"
#include "def_files/def_files.h"
#include "graphics/2d.h"
#include "mod_table/mod_table.h"

static void handle_includes_impl(SCP_vector<SCP_string>& include_stack,
                                 SCP_stringstream& output,
                                 int& include_counter,
                                 const SCP_string& filename,
                                 const SCP_string& original)
{
	include_stack.emplace_back(filename);
	auto current_source_number = include_counter + 1;

	const char* INCLUDE_STRING = "#include";
	const char* CONDITIONAL_INCLUDE_STRING = "#conditional_include";
	SCP_stringstream input(original);

	int line_num = 1;
	for (SCP_string line; std::getline(input, line);) {
		auto include_start = line.find(CONDITIONAL_INCLUDE_STRING);

		if (include_start != SCP_string::npos) {
			include_start += strlen(CONDITIONAL_INCLUDE_STRING) + 1;
			bool require_capability = true;

			switch (line.at(include_start)) {
			case '+':
				require_capability = true;
				break;
			case '-':
				require_capability = false;
				break;
			default:
				Error(LOCATION,
				      "Shader %s:%d: Malformed conditional_include line. Expected + or -, got %c.",
				      filename.c_str(), line_num, line.at(include_start));
				break;
			}

			auto first_quote = line.find('"', include_start);
			auto second_quote = line.find('"', first_quote + 1);

			if (first_quote == SCP_string::npos || second_quote == SCP_string::npos) {
				Error(LOCATION,
				      "Shader %s:%d: Malformed conditional_include line. Could not find both quote characters for capability.",
				      filename.c_str(), line_num);
			}
			auto condition = line.substr(first_quote + 1, second_quote - first_quote - 1);
			auto capability = std::find_if(&gr_capabilities[0], &gr_capabilities[gr_capabilities_num],
			                               [condition](const gr_capability_def& ext_pair) {
				                               return !stricmp(ext_pair.parse_name, condition.c_str());
			                               });
			if (capability == &gr_capabilities[gr_capabilities_num]) {
				Error(LOCATION,
				      "Shader %s:%d: Malformed conditional_include line. Capability %s does not exist.",
				      filename.c_str(), line_num, condition.c_str());
			}

			if (gr_is_capable(capability->capability) == require_capability)
				include_start = second_quote + 1 - strlen(INCLUDE_STRING);
			else
				include_start = SCP_string::npos - 1;
		} else {
			include_start = line.find(INCLUDE_STRING);
		}

		if (include_start != SCP_string::npos && include_start != SCP_string::npos - 1) {
			auto first_quote = line.find('"', include_start + strlen(INCLUDE_STRING));
			auto second_quote = line.find('"', first_quote + 1);

			if (first_quote == SCP_string::npos || second_quote == SCP_string::npos) {
				Error(LOCATION,
				      "Shader %s:%d: Malformed include line. Could not find both quote characters.",
				      filename.c_str(), line_num);
			}

			auto file_name = line.substr(first_quote + 1, second_quote - first_quote - 1);
			auto existing_name =
			    std::find_if(include_stack.begin(), include_stack.end(),
			                 [&file_name](const SCP_string& str) { return str == file_name; });
			if (existing_name != include_stack.end()) {
				SCP_stringstream stack_string;
				for (auto& name : include_stack) {
					stack_string << "\t" << name << "\n";
				}

				Error(LOCATION,
				      "Shader %s:%d: Detected cyclic include! Previous includes (top level file first):\n%s",
				      filename.c_str(), line_num, stack_string.str().c_str());
			}

			++include_counter;
			output << "#line 1 " << include_counter + 1 << "\n";

			handle_includes_impl(include_stack, output, include_counter, file_name,
			                     shader_load_source(file_name));

			output << "#line " << line_num + 1 << " " << current_source_number << "\n";
		} else if (include_start != SCP_string::npos - 1) {
			output << line << "\n";
		}

		++line_num;
	}

	include_stack.pop_back();
}

SCP_string shader_load_source(const SCP_string& filename)
{
	SCP_string content;

	// Check external shaders first (modding support)
	if (Enable_external_shaders) {
		CFILE* cf_shader = cfopen(filename.c_str(), "rt", CF_TYPE_EFFECTS);
		if (cf_shader != nullptr) {
			int len = cfilelength(cf_shader);
			content.resize(len);
			cfread(content.data(), len + 1, 1, cf_shader);
			cfclose(cf_shader);
			return content;
		}
	}

	// Fall back to embedded defaults
	auto def_shader = defaults_get_file(filename.c_str());
	if (def_shader.data != nullptr && def_shader.size > 0) {
		content.assign(reinterpret_cast<const char*>(def_shader.data), def_shader.size);
	} else {
		mprintf(("shader_load_source: Could not load shader source: %s\n", filename.c_str()));
	}

	return content;
}

SCP_string shader_preprocess_includes(const SCP_string& filename, const SCP_string& source)
{
	SCP_stringstream output;
	SCP_vector<SCP_string> include_stack;
	auto include_counter = 0;

	handle_includes_impl(include_stack, output, include_counter, filename, source);

	return output.str();
}

SCP_string shader_preprocess_defines(const SCP_string& filename, const SCP_string& source)
{
	SCP_stringstream output;
	SCP_unordered_map<SCP_string, SCP_string> defines;

	//In any shader, define GLOBAL_FAR_Z
	output << "#define GLOBAL_FAR_Z " << std::fixed << std::setprecision(2) << Max_draw_distance << std::defaultfloat << '\n';

	const char* PREDEFINE_STRING = "#predefine";
	const char* PREREPLACE_STRING = "#prereplace";

	SCP_stringstream input(source);
	for (SCP_string line; std::getline(input, line);) {
		auto predefine_start = line.find(PREDEFINE_STRING);
		auto prereplace_start = line.find(PREREPLACE_STRING);

		if (predefine_start != SCP_string::npos) {
			predefine_start += strlen(PREDEFINE_STRING);

			auto token_start = line.find(' ', predefine_start);
			auto token_end = line.find(' ', token_start + 1);

			if (token_start == SCP_string::npos || token_end == SCP_string::npos) {
				Error(LOCATION, "Shader %s: Malformed predefine line. Could not find define token.",
				      filename.c_str());
			}

			auto token = line.substr(token_start + 1, token_end - token_start - 1);
			auto replaceWith = line.substr(token_end + 1);

			auto replaceStrToken = replaceWith.find("%s");
			if (replaceStrToken == SCP_string::npos ||
			    replaceWith.find("%s", replaceStrToken + 1) != SCP_string::npos) {
				Error(LOCATION,
				      "Shader %s: Malformed predefine line. Replacing string must have exactly one %%s.",
				      filename.c_str());
			}
			if (defines.find(token) != defines.end()) {
				Error(LOCATION,
				      "Shader %s: Malformed predefine line. Token %s is already defined.",
				      filename.c_str(), token.c_str());
			}

			defines.emplace(std::move(token), std::move(replaceWith));

			output << "\n"; // Preserve line count
		} else if (prereplace_start != SCP_string::npos) {
			prereplace_start += strlen(PREREPLACE_STRING);

			auto token_start = line.find(' ', prereplace_start);
			auto token_end = line.find(' ', token_start + 1);

			if (token_start == SCP_string::npos || token_end == SCP_string::npos) {
				Error(LOCATION, "Shader %s: Malformed prereplace line. Could not find define token.",
				      filename.c_str());
			}

			auto token = line.substr(token_start + 1, token_end - token_start - 1);
			auto replaceArg = line.substr(token_end + 1);

			auto replaceWithIt = defines.find(token);
			if (replaceWithIt == defines.end()) {
				Error(LOCATION, "Shader %s: Malformed prereplace line. Could not find token %s.",
				      filename.c_str(), token.c_str());
			}

			size_t size = replaceWithIt->second.length() - 1 + replaceArg.size();
			std::unique_ptr<char[]> buffer = make_unique<char[]>(size);

			snprintf(buffer.get(), size, replaceWithIt->second.c_str(), replaceArg.c_str());
			buffer[size - 1] = '\0';

			output << buffer.get() << "\n";
		} else {
			output << line << "\n";
		}
	}

	return output.str();
}
