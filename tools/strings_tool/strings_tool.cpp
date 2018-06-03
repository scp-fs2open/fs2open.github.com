// Declares clang::SyntaxOnlyAction.
#include <cstdint>
#include <iostream>
#include <mutex>
#include <set>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"

#ifdef USE_BOOST_REGEX
#include <boost/regex.hpp>
namespace regex = boost;
#else
#include <regex>
namespace regex = std::regex;
#endif

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace llvm;

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
}

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory StringsToolCategory("strings_tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nExtracts translatable strings (in the form of XSTR calls) from the source code and "
                              "writes them in the strings.tbl format.\n");

auto xstrMatcher = callExpr(callee(functionDecl(hasName("XSTR"))), hasArgument(0, stringLiteral().bind("text")),
                            hasArgument(1, integerLiteral().bind("id")))
                       .bind("xstrCall");

auto uiXstrMatcher =
    initListExpr(hasType(cxxRecordDecl(hasName("::UI_XSTR"))), has(ignoringParenImpCasts(stringLiteral().bind("text"))),
                 has(integerLiteral().bind("id")))
        .bind("uiXstr");

std::string getEscapedStringLiteralContent(const clang::StringLiteral* lit) {
	if (!lit->isAscii() && !lit->isUTF8()) {
		// We only support strings with ASCII/Utf-8 encoding
		return "";
	}

	auto text_str = lit->getString().str();

	// Escape new lines
	replaceAll(text_str, "\n", "\\n");
	// Escape tab characters
	replaceAll(text_str, "\t", "\\t");
	// Escape string characters
	replaceAll(text_str, "\"", "\\\"");

	return text_str;
}

int64_t getIntValue(const clang::IntegerLiteral* lit) {
	if (!lit->getValue().isSignedIntN(64)) {
		// Only signed int of up to 64-bit are supported
		throw std::runtime_error("Only up to 64-bit integers are supported!");
	}

	return lit->getValue().getSExtValue();
}

std::string getSourceFileName(SourceLocation loc, SourceManager* manager) {
	if (loc.isMacroID()) {
		// Get the start/end expansion locations
		std::pair<SourceLocation, SourceLocation> expansionRange = manager->getExpansionRange(loc);

		// We're just interested in the start location
		loc = expansionRange.first;
	}

	return manager->getFilename(loc).str();
}

class XstrManager {

  public:
	struct XstrInfo {
		int64_t id;
		clang::SourceRange def;
		std::string text;
		std::string file;

		bool operator<(const XstrInfo& other) {
			if (file != other.file) {
				return file < other.file;
			}
			return id < other.id;
		}
	};

  public:
	void addXstr(int64_t id, const std::string& text, ASTContext* ctx, const clang::Stmt* sourceStmt) {
		if (id < 0) {
			// Ignore invalid ids
			return;
		}

		std::unique_lock<std::mutex> guard(_mapping_lock);

		auto iter = _mapping.find(id);

		XstrInfo info;
		info.id   = id;
		info.def  = sourceStmt->getSourceRange();
		info.text = text;
		info.file = getSourceFileName(sourceStmt->getLocStart(), &ctx->getSourceManager());

		if (iter == _mapping.end()) {
			_mapping.emplace(id, info);
			return;
		}

		if (iter->second.text != text) {
			ctx->getDiagnostics().Report(sourceStmt->getLocStart(),
			                             ctx->getDiagnostics().getDiagnosticIDs()->getCustomDiagID(
			                                 DiagnosticIDs::Warning,
			                                 "Found duplicate XSTR id usage with "
			                                 "different text! Previous text was: "
			                                 "\"%0\" but new text is \"%1\". Previous text was used in \"%2\"."))
			    << iter->second.text << text << iter->second.file;
		}
	}

	const std::vector<XstrInfo> getStrings() const {
		std::vector<XstrInfo> out;
		for (auto& entry : _mapping) {
			out.push_back(entry.second);
		}
		std::sort(out.begin(), out.end());
		return out;
	}

  private:
	std::mutex _mapping_lock;
	std::unordered_map<int64_t, XstrInfo> _mapping;
};

class UiXstrPrinter : public MatchFinder::MatchCallback {
	XstrManager* _manager = nullptr;

  public:
	UiXstrPrinter(XstrManager* manager) : _manager(manager) {}

	void run(const MatchFinder::MatchResult& Result) override {
		auto varDecls = Result.Nodes.getNodeAs<clang::InitListExpr>("uiXstr");
		auto text_lit = Result.Nodes.getNodeAs<clang::StringLiteral>("text");
		auto id_lit   = Result.Nodes.getNodeAs<clang::IntegerLiteral>("id");

		auto text = getEscapedStringLiteralContent(text_lit);
		int64_t id;
		try {
			id = getIntValue(id_lit);
		} catch (const std::runtime_error&) {
			return;
		}

		_manager->addXstr(id, text, Result.Context, varDecls);
	}
};

class XstrPrinter : public MatchFinder::MatchCallback {
	XstrManager* _manager = nullptr;

  public:
	XstrPrinter(XstrManager* manager) : _manager(manager) {}

	void run(const MatchFinder::MatchResult& Result) override {
		auto call_expr = Result.Nodes.getNodeAs<clang::CallExpr>("xstrCall");
		auto text_lit  = Result.Nodes.getNodeAs<clang::StringLiteral>("text");
		auto id_lit    = Result.Nodes.getNodeAs<clang::IntegerLiteral>("id");

		auto text = getEscapedStringLiteralContent(text_lit);
		if (text.empty()) {
			// We only support strings with ASCII/Utf-8 encoding
			return;
		}

		int64_t id;
		try {
			id = getIntValue(id_lit);
		} catch (const std::runtime_error&) {
			return;
		}

		_manager->addXstr(id, text, Result.Context, call_expr);
	}
};

class DiagnosticLogger : public SourceFileCallbacks {
  public:
	bool handleBeginSource(CompilerInstance& CI) override {
		for (auto& input : CI.getFrontendOpts().Inputs) {
			std::cerr << "Processing \"" << input.getFile().str() << "\"\n";
		}
		return true;
	}
};

std::vector<std::string> resolveRegexes(const CompilationDatabase& compilations,
                                        const std::vector<std::string>& regexes) {
	std::set<std::string> allFiles;
	auto compilationFiles = compilations.getAllFiles();

	for (auto& regex : regexes) {
		try {
			regex::regex file_regex(regex);

			for (auto& file : compilationFiles) {
				if (regex::regex_match(file, file_regex)) {
					allFiles.emplace(file);
				}
			}
		} catch (regex::regex_error& e) {
			// Syntax error in the regular expression
			std::cerr << "Skipping bad regex '" << regex << "': " << e.what() << std::endl;
		}
	}

	std::vector<std::string> outVec;
	std::copy(allFiles.begin(), allFiles.end(), std::back_inserter(outVec));
	return outVec;
}

void write_in_table_syntax(std::ostream& out, const std::vector<XstrManager::XstrInfo>& strings) {
	std::string last_file;

	out << "#English";

	for (auto& info : strings) {
		if (info.file != last_file) {
			out << "\n";
			out << ";-------------------------------------------------\n";
			out << "; File: " << llvm::sys::path::filename(info.file).str() << "\n";
			out << ";-------------------------------------------------\n";
			out << "\n";
			last_file = info.file;
		}

		out << info.id << ", \"" << info.text << "\"\n";
	}
}

int main(int argc, const char** argv) {
	CommonOptionsParser OptionsParser(argc, argv, StringsToolCategory, llvm::cl::OneOrMore);

	std::vector<std::string> resolved_files =
	    resolveRegexes(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

	ClangTool Tool(OptionsParser.getCompilations(), resolved_files);

	XstrManager xstrManager;

	XstrPrinter Printer(&xstrManager);
	UiXstrPrinter uiPrinter(&xstrManager);

	MatchFinder Finder;
	Finder.addMatcher(xstrMatcher, &Printer);
	Finder.addMatcher(uiXstrMatcher, &uiPrinter);

	DiagnosticLogger logger;
	auto res = Tool.run(newFrontendActionFactory(&Finder, &logger).get());

	write_in_table_syntax(std::cout, xstrManager.getStrings());

	return res;
}
