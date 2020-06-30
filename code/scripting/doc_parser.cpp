#include "doc_parser.h"

#include "parse/parselo.h"

#undef TRUE
#undef FALSE

PUSH_SUPPRESS_WARNINGS
#include "arg_parser/generated/ArgumentListLexer.h"
#include "arg_parser/generated/ArgumentListParser.h"
#include "arg_parser/generated/ArgumentListVisitor.h"
POP_SUPPRESS_WARNINGS

namespace scripting {

namespace {

struct Diagnostic {
	size_t line;
	size_t columnInLine;
	size_t tokenLength;
	SCP_string errorMessage;
};

struct ErrorListener : public antlr4::BaseErrorListener {
	SCP_vector<Diagnostic> diagnostics;

	void syntaxError(antlr4::Recognizer* /*recognizer*/,
	                 antlr4::Token* offendingSymbol,
	                 size_t line,
	                 size_t charPositionInLine,
	                 const std::string& msg,
	                 std::exception_ptr /*e*/) override
	{
		// Apparently stop < start is a thing?
		const auto left   = std::min(offendingSymbol->getStartIndex(), offendingSymbol->getStopIndex());
		const auto right  = std::max(offendingSymbol->getStartIndex(), offendingSymbol->getStopIndex());
		const auto length = right - left + 1;

		diagnostics.push_back(Diagnostic{line, charPositionInLine, length, msg});
	}
};

ade_type_info merge_alternatives(const ade_type_info& left, const ade_type_info& right)
{
	SCP_vector<ade_type_info> elements;
	if (left.isSimple()) {
		elements.push_back(left);
	} else {
		elements.insert(elements.end(), left.elements().begin(), left.elements().end());
	}
	if (right.isSimple()) {
		elements.push_back(right);
	} else {
		elements.insert(elements.end(), right.elements().begin(), right.elements().end());
	}

	return ade_type_info(ade_type_alternative(elements));
}

class BaseVisitor : public ArgumentListVisitor {
  public:
	antlrcpp::Any visitArg_list(ArgumentListParser::Arg_listContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitSimple_type(ArgumentListParser::Simple_typeContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitType(ArgumentListParser::TypeContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitBoolean(ArgumentListParser::BooleanContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitValue(ArgumentListParser::ValueContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitActual_argument(ArgumentListParser::Actual_argumentContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitOptional_argument(ArgumentListParser::Optional_argumentContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitArgument(ArgumentListParser::ArgumentContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitFunc_arg(ArgumentListParser::Func_argContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitFunc_arglist(ArgumentListParser::Func_arglistContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
	antlrcpp::Any visitFunction_type(ArgumentListParser::Function_typeContext* /*context*/) override
	{
		throw std::runtime_error("Unhandled node encountered!");
	}
};

class ValueVisitor : public BaseVisitor {
  public:
	antlrcpp::Any visitBoolean(ArgumentListParser::BooleanContext* context) override { return visitChildren(context); }
	antlrcpp::Any visitValue(ArgumentListParser::ValueContext* context) override { return visitChildren(context); }
	antlrcpp::Any visitTerminal(antlr4::tree::TerminalNode* node) override { return node->getText(); }
};

class ArglistVisitor : public BaseVisitor {
  public:
	antlrcpp::Any visitFunc_arg(ArgumentListParser::Func_argContext* context) override;
	antlrcpp::Any visitFunc_arglist(ArgumentListParser::Func_arglistContext* context) override
	{
		SCP_vector<ade_type_info> argTypes;

		size_t n = context->children.size();
		for (size_t i = 0; i < n; i++) {
			antlrcpp::Any childResult = context->children[i]->accept(this);

			if (childResult.isNotNull()) {
				argTypes.push_back(childResult.as<ade_type_info>());
			}
		}

		return argTypes;
	}

  protected:
	antlrcpp::Any aggregateResult(antlrcpp::Any any, const antlrcpp::Any& nextResult) override
	{
		if (any.isNull()) {
		}
		return AbstractParseTreeVisitor::aggregateResult(any, nextResult);
	}
};

class TypeVisitor : public BaseVisitor {
  public:
	antlrcpp::Any visitSimple_type(ArgumentListParser::Simple_typeContext* context) override
	{
		if (context->NIL() != nullptr) {
			return ade_type_info("nil");
		} else {
			return ade_type_info(context->ID()->getText());
		}
	}
	antlrcpp::Any visitType(ArgumentListParser::TypeContext* context) override {
		return visitChildren(context);
	}

	antlrcpp::Any visitFunction_type(ArgumentListParser::Function_typeContext* context) override
	{
		auto retType = visit(context->type()).as<ade_type_info>();

		ArglistVisitor arglistVisitor;
		auto argTypes = context->func_arglist()->accept(&arglistVisitor).as<SCP_vector<ade_type_info>>();

		return ade_type_info(ade_type_function(std::move(retType), std::move(argTypes)));
	}

	antlrcpp::Any visitErrorNode(antlr4::tree::ErrorNode* /*node*/) override { return ade_type_info("<error type>"); }

  protected:
	antlrcpp::Any aggregateResult(antlrcpp::Any previous, const antlrcpp::Any& nextResult) override
	{
		// This happens while visiting terminals, ignore those
		if (nextResult.isNull()) {
			return previous;
		}

		if (previous.isNotNull()) {
			const auto& previousType = previous.as<ade_type_info>();
			const auto& nextType     = nextResult.as<ade_type_info>();
			return merge_alternatives(previousType, nextType);
		} else {
			return nextResult.as<ade_type_info>();
		}
	}
};

antlrcpp::Any scripting::ArglistVisitor::visitFunc_arg(ArgumentListParser::Func_argContext* context)
{
	TypeVisitor typeVisit;
	auto argType = context->type()->accept(&typeVisit).as<ade_type_info>();
	argType.setName(context->ID()->getText());

	return argType;
}

SCP_string getCommentContent(const SCP_string& content) {
	auto base = content.substr(2, content.length() - 4); // Strip leading and trailing delimiters

	drop_white_space(base);

	return base;
}

class ArgumentCollectorVisitor : public BaseVisitor {
	bool saw_optional = false;

  public:
	SCP_vector<argument_def> args;

	antlrcpp::Any visitArg_list(ArgumentListParser::Arg_listContext* context) override
	{
		visitChildren(context);
		return antlrcpp::Any();
	}
	antlrcpp::Any visitActual_argument(ArgumentListParser::Actual_argumentContext* context) override
	{
		argument_def argdef;
		argdef.optional = saw_optional;

		TypeVisitor typeVisit;
		const auto typeAny = context->type()->accept(&typeVisit);
		argdef.type        = typeAny.as<ade_type_info>();

		if (context->ID() != nullptr) {
			argdef.name = context->ID()->getText();
		}

		if (context->value() != nullptr) {
			ValueVisitor valueVisit;
			const auto valueAny = context->value()->accept(&valueVisit);
			argdef.def_val      = valueAny.as<SCP_string>();
			argdef.optional     = true;
		}

		if (context->ARG_COMMENT() != nullptr) {
			argdef.comment = getCommentContent(context->ARG_COMMENT()->getText());
		}

		args.push_back(std::move(argdef));

		if (context->argument()) {
			context->argument()->accept(this);
		}

		return antlrcpp::Any();
	}
	antlrcpp::Any visitOptional_argument(ArgumentListParser::Optional_argumentContext* context) override
	{
		saw_optional = true;

		visitChildren(context);
		return antlrcpp::Any();
	}
	antlrcpp::Any visitArgument(ArgumentListParser::ArgumentContext* context) override
	{
		visitChildren(context);
		return antlrcpp::Any();
	}
};

class TypeCheckVisitor : public BaseVisitor {
	ArgumentListParser* _parser = nullptr;
	const SCP_unordered_set<SCP_string>& _validTypeNames;

  public:
	TypeCheckVisitor(ArgumentListParser* parser, const SCP_unordered_set<SCP_string>& validTypeNames)
		: _parser(parser), _validTypeNames(validTypeNames)
	{
	}

	antlrcpp::Any visitSimple_type(ArgumentListParser::Simple_typeContext* context) override
	{
		// Nil is always valid
		if (context->NIL() != nullptr) {
			return antlrcpp::Any();
		}

		if (_validTypeNames.find(context->ID()->getText()) == _validTypeNames.end()) {
			_parser->notifyErrorListeners(context->ID()->getSymbol(),
				"Invalid type name <" + context->ID()->getText() + ">",
				nullptr);
		}

		return antlrcpp::Any();
	}
	antlrcpp::Any visitType(ArgumentListParser::TypeContext* context) override { return visitChildren(context); }

	antlrcpp::Any visitFunc_arg(ArgumentListParser::Func_argContext* context) override
	{
		visit(context->type());
		return antlrcpp::Any();
	}
	antlrcpp::Any visitFunc_arglist(ArgumentListParser::Func_arglistContext* context) override
	{
		visitChildren(context);
		return antlrcpp::Any();
	}

	antlrcpp::Any visitFunction_type(ArgumentListParser::Function_typeContext* context) override
	{
		visitChildren(context);
		return antlrcpp::Any();
	}

	antlrcpp::Any visitActual_argument(ArgumentListParser::Actual_argumentContext* context) override
	{
		visit(context->type());

		// Now also look at the remaining arguments
		if (context->argument() != nullptr)
		{
			visit(context->argument());
		}
		return antlrcpp::Any();
	}

	antlrcpp::Any visitOptional_argument(ArgumentListParser::Optional_argumentContext* context) override
	{
		visitChildren(context);
		return antlrcpp::Any();
	}
	antlrcpp::Any visitArgument(ArgumentListParser::ArgumentContext* context) override
	{
		visitChildren(context);
		return antlrcpp::Any();
	}
	antlrcpp::Any visitArg_list(ArgumentListParser::Arg_listContext* context) override
	{
		visitChildren(context);
		return antlrcpp::Any();
	}
};

} // namespace

argument_list_parser::argument_list_parser(const SCP_vector<SCP_string>& validTypeNames)
	: _validTypeNames(validTypeNames.begin(), validTypeNames.end())
{
}

bool argument_list_parser::parse(const SCP_string& argumentList)
{
	antlr4::ANTLRInputStream input(argumentList);
	ArgumentListLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);

	ArgumentListParser parser(&tokens);
	// By default we log to stderr which we do not want
	parser.removeErrorListeners();
	ErrorListener errListener;
	parser.addErrorListener(&errListener);

	antlr4::tree::ParseTree* tree = parser.arg_list();

	TypeCheckVisitor typeChecker(&parser, _validTypeNames);
	tree->accept(&typeChecker);

	// If there were errors, output them
	if (!errListener.diagnostics.empty()) {
		mprintf(("Scripting documentation: Error while parsing\n"));

		for (const auto& diag : errListener.diagnostics) {
			SCP_string tokenUnderline;
			if (diag.tokenLength > 1) {
				tokenUnderline = SCP_string(diag.tokenLength - 1, '~');
			}
			mprintf(("%s\n%s^%s\n%s\n",
				argumentList.c_str(),
				SCP_string(diag.columnInLine, ' ').c_str(),
				tokenUnderline.c_str(),
				diag.errorMessage.c_str()));
		}

		mprintf(("(This is only relevant for coders)\n\n"));
	} else {
		// Only look at the parameters if we know that we have valid input to avoid some of the error handling while
		// traversing the parse tree
		ArgumentCollectorVisitor argCollector;
		tree->accept(&argCollector);

		_argList = argCollector.args;
	}

	return errListener.diagnostics.empty();
}
const SCP_vector<scripting::argument_def>& argument_list_parser::getArgList() const { return _argList; }

} // namespace scripting
