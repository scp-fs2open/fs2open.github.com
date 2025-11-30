#include "ShaderPreprocessor.h"

#include "globalincs/systemvars.h"

#include <algorithm>
#include <memory>

namespace graphics {

ShaderPreprocessor::ShaderPreprocessor(ShaderPreprocessorCallbacks callbacks)
    : m_callbacks(std::move(callbacks))
{
	Assertion(m_callbacks.loadSource != nullptr, "ShaderPreprocessor requires a loadSource callback");
	// checkCapability can be null if conditional includes are not used
}

void ShaderPreprocessor::handleIncludesImpl(SCP_vector<SCP_string>& includeStack,
                                            SCP_stringstream& output,
                                            int& includeCounter,
                                            const SCP_string& filename,
                                            const SCP_string& source)
{
	includeStack.emplace_back(filename);
	auto currentSourceNumber = includeCounter + 1;

	const char* INCLUDE_STRING = "#include";
	const char* CONDITIONAL_INCLUDE_STRING = "#conditional_include";
	SCP_stringstream input(source);

	int lineNum = 1;
	for (SCP_string line; std::getline(input, line);) {
		auto includeStart = line.find(CONDITIONAL_INCLUDE_STRING);

		if (includeStart != SCP_string::npos) {
			// This is a conditional include. Figure out whether to include, or whether not to.
			// Conditional include syntax: #conditional_include (+|-)"capability" "filename"
			// On +, include if capability is available, on -, include if not available
			includeStart += strlen(CONDITIONAL_INCLUDE_STRING) + 1;
			bool requireCapability = true;

			switch (line.at(includeStart)) {
			case '+':
				requireCapability = true;
				break;
			case '-':
				requireCapability = false;
				break;
			default:
				Error(LOCATION,
				      "Shader %s:%d: Malformed conditional_include line. Expected + or -, got %c.",
				      filename.c_str(),
				      lineNum,
				      line.at(includeStart));
				break;
			}

			auto firstQuote = line.find('"', includeStart);
			auto secondQuote = line.find('"', firstQuote + 1);

			if (firstQuote == SCP_string::npos || secondQuote == SCP_string::npos) {
				Error(LOCATION,
				      "Shader %s:%d: Malformed conditional_include line. Could not find both quote characters for "
				      "capability.",
				      filename.c_str(),
				      lineNum);
			}

			auto condition = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);

			// Use the callback to check capability
			bool capabilitySupported = false;
			if (m_callbacks.checkCapability) {
				capabilitySupported = m_callbacks.checkCapability(condition);
			} else {
				// If no capability checker, treat all capabilities as unavailable
				// This is useful for Vulkan which may not use conditional includes
				mprintf(("ShaderPreprocessor: No capability checker provided, treating '%s' as unavailable\n",
				         condition.c_str()));
				capabilitySupported = false;
			}

			// Prepare for including if capability matches requirement, skip otherwise
			if (capabilitySupported == requireCapability) {
				includeStart = secondQuote + 1 - strlen(INCLUDE_STRING);
			} else {
				includeStart = SCP_string::npos - 1;
			}
		} else {
			// Only search for normal includes if it's not a conditional include
			includeStart = line.find(INCLUDE_STRING);
		}

		if (includeStart != SCP_string::npos && includeStart != SCP_string::npos - 1) {
			auto firstQuote = line.find('"', includeStart + strlen(INCLUDE_STRING));
			auto secondQuote = line.find('"', firstQuote + 1);

			if (firstQuote == SCP_string::npos || secondQuote == SCP_string::npos) {
				Error(LOCATION,
				      "Shader %s:%d: Malformed include line. Could not find both quote characters.",
				      filename.c_str(),
				      lineNum);
			}

			auto includeFilename = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);
			auto existingName = std::find_if(includeStack.begin(),
			                                 includeStack.end(),
			                                 [&includeFilename](const SCP_string& str) { return str == includeFilename; });

			if (existingName != includeStack.end()) {
				SCP_stringstream stackString;
				for (auto& name : includeStack) {
					stackString << "\t" << name << "\n";
				}

				Error(LOCATION,
				      "Shader %s:%d: Detected cyclic include! Previous includes (top level file first):\n%s",
				      filename.c_str(),
				      lineNum,
				      stackString.str().c_str());
			}

			++includeCounter;
			// The second parameter defines which source string we are currently working with.
			// We keep track of how many includes have been in the file so far to specify this
			output << "#line 1 " << includeCounter + 1 << "\n";

			// Load the included file using the callback
			auto includeContent = m_callbacks.loadSource(includeFilename.c_str());
			if (!includeContent.empty()) {
				handleIncludesImpl(includeStack, output, includeCounter, includeFilename, includeContent);
			} else {
				Error(LOCATION,
				      "Shader %s:%d: Failed to load include file: %s",
				      filename.c_str(),
				      lineNum,
				      includeFilename.c_str());
			}

			// We are done with the include file so now we can return to the original file
			output << "#line " << lineNum + 1 << " " << currentSourceNumber << "\n";
		} else if (includeStart != SCP_string::npos - 1) {
			output << line << "\n";
		}

		++lineNum;
	}

	includeStack.pop_back();
}

SCP_string ShaderPreprocessor::handleIncludes(const char* filename, const SCP_string& source)
{
	SCP_stringstream output;
	SCP_vector<SCP_string> includeStack;
	auto includeCounter = 0;

	handleIncludesImpl(includeStack, output, includeCounter, filename, source);

	return output.str();
}

SCP_string ShaderPreprocessor::handlePredefines(const char* filename, const SCP_string& source)
{
	SCP_stringstream output;
	SCP_unordered_map<SCP_string, SCP_string> defines;

	const char* PREDEFINE_STRING = "#predefine";
	const char* PREREPLACE_STRING = "#prereplace";

	SCP_stringstream input(source);
	for (SCP_string line; std::getline(input, line);) {
		auto predefineStart = line.find(PREDEFINE_STRING);
		auto prereplaceStart = line.find(PREREPLACE_STRING);

		if (predefineStart != SCP_string::npos) {
			predefineStart += strlen(PREDEFINE_STRING);

			auto tokenStart = line.find(' ', predefineStart);
			auto tokenEnd = line.find(' ', tokenStart + 1);

			if (tokenStart == SCP_string::npos || tokenEnd == SCP_string::npos) {
				Error(LOCATION, "Shader %s: Malformed predefine line. Could not find define token.", filename);
			}

			auto token = line.substr(tokenStart + 1, tokenEnd - tokenStart - 1);
			auto replaceWith = line.substr(tokenEnd + 1);

			auto replaceStrToken = replaceWith.find("%s");
			if (replaceStrToken == SCP_string::npos ||
			    replaceWith.find("%s", replaceStrToken + 1) != SCP_string::npos) {
				Error(LOCATION,
				      "Shader %s: Malformed predefine line. Replacing string must have exactly one %%s.",
				      filename);
			}
			if (defines.find(token) != defines.end()) {
				Error(LOCATION,
				      "Shader %s: Malformed predefine line. Token %s is already defined.",
				      filename,
				      token.c_str());
			}

			defines.emplace(std::move(token), std::move(replaceWith));

			output << "\n"; // Don't mess with the line count
		} else if (prereplaceStart != SCP_string::npos) {
			prereplaceStart += strlen(PREREPLACE_STRING);

			auto tokenStart = line.find(' ', prereplaceStart);
			auto tokenEnd = line.find(' ', tokenStart + 1);

			if (tokenStart == SCP_string::npos || tokenEnd == SCP_string::npos) {
				Error(LOCATION, "Shader %s: Malformed prereplace line. Could not find define token.", filename);
			}

			auto token = line.substr(tokenStart + 1, tokenEnd - tokenStart - 1);
			auto replaceArg = line.substr(tokenEnd + 1);

			auto replaceWithIt = defines.find(token);
			if (replaceWithIt == defines.end()) {
				Error(LOCATION,
				      "Shader %s: Malformed prereplace line. Could not find token %s.",
				      filename,
				      token.c_str());
			}

			size_t size = replaceWithIt->second.length() - 1 + replaceArg.size();
			std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);

			snprintf(buffer.get(), size, replaceWithIt->second.c_str(), replaceArg.c_str());
			buffer[size - 1] = '\0';

			output << buffer.get() << "\n";
		} else {
			output << line << "\n";
		}
	}

	return output.str();
}

SCP_string ShaderPreprocessor::preprocess(const char* filename, const SCP_string& source)
{
	auto withIncludes = handleIncludes(filename, source);
	return handlePredefines(filename, withIncludes);
}

} // namespace graphics
