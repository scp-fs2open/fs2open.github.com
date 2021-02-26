
#include "ExpressionParser.h"

#include "actions/expression/nodes/FunctionCallExpression.h"
#include "actions/expression/nodes/LiteralExpression.h"
#include "actions/expression/nodes/RandomRangeExpression.h"
#include "actions/expression/nodes/VariableReferenceExpression.h"
#include "actions/expression/nodes/VectorConstructorExpression.h"
#include "libs/antlr/ErrorListener.h"

PUSH_SUPPRESS_WARNINGS
#include "action_expression/generated/ActionExpressionLexer.h"
#include "action_expression/generated/ActionExpressionParser.h"
#include "action_expression/generated/ActionExpressionVisitor.h"
POP_SUPPRESS_WARNINGS

namespace actions {
namespace expression {

namespace {

class ExpressionBuilderVisitor : public ActionExpressionVisitor {
  public:
	antlrcpp::Any visitExpression_main(ActionExpressionParser::Expression_mainContext* context) override
	{
		return visit(context->expression());
	}
	antlrcpp::Any visitExpression(ActionExpressionParser::ExpressionContext* context) override
	{
		if (context->value_expression() != nullptr) {
			return visit(context->value_expression());
		}
		if (context->parenthesis_expression()) {
			return visit(context->parenthesis_expression());
		}
		if (context->random_range_expression()) {
			return visit(context->random_range_expression());
		}
		if (context->variable_reference_expression()) {
			return visit(context->variable_reference_expression());
		}

		// This is an operator so treat it as one.
		antlr4::Token* operatorTok;
		SCP_string operatorText;
		if (context->PLUS() != nullptr) {
			operatorTok = context->PLUS()->getSymbol();
			operatorText = context->PLUS()->getText();
		} else {
			operatorTok = context->MINUS()->getSymbol();
			operatorText = context->MINUS()->getText();
		}

		const auto leftExpr = visit(context->expression(0)).as<std::shared_ptr<nodes::AbstractExpression>>();
		const auto rightExpr = visit(context->expression(1)).as<std::shared_ptr<nodes::AbstractExpression>>();

		auto operatorExpression = std::make_shared<nodes::FunctionCallExpression>(operatorTok,
			true,
			std::move(operatorText),
			SCP_vector<std::shared_ptr<nodes::AbstractExpression>>{leftExpr, rightExpr});
		return std::static_pointer_cast<nodes::AbstractExpression>(operatorExpression);
	}
	antlrcpp::Any visitVariable_reference_expression(
		ActionExpressionParser::Variable_reference_expressionContext* context) override
	{
		const auto identifiers = context->IDENTIFIER();

		SCP_vector<SCP_string> partNames;
		partNames.reserve(identifiers.size());

		std::transform(identifiers.begin(),
			identifiers.end(),
			std::back_inserter(partNames),
			[](antlr4::tree::TerminalNode* node) { return node->getText(); });

		SCP_vector<antlr4::Token*> partTokens;
		partTokens.reserve(identifiers.size());

		std::transform(identifiers.begin(),
			identifiers.end(),
			std::back_inserter(partTokens),
			[](antlr4::tree::TerminalNode* node) { return node->getSymbol(); });

		auto expression =
			std::make_shared<nodes::VariableReferenceExpression>(std::move(partNames), std::move(partTokens));
		return std::static_pointer_cast<nodes::AbstractExpression>(expression);
	}
	antlrcpp::Any visitRandom_range_expression(ActionExpressionParser::Random_range_expressionContext* context) override
	{
		auto expression = std::make_shared<nodes::RandomRangeExpression>(context->RAND_L_PAREN()->getSymbol(),
			visit(context->expression(0)).as<std::shared_ptr<nodes::AbstractExpression>>(),
			visit(context->expression(1)).as<std::shared_ptr<nodes::AbstractExpression>>());
		return std::static_pointer_cast<nodes::AbstractExpression>(expression);
	}
	antlrcpp::Any visitLiteral_expression(ActionExpressionParser::Literal_expressionContext* context) override
	{
		std::shared_ptr<nodes::LiteralExpression> expression;

		if (context->FLOAT() != nullptr) {
			expression = std::make_shared<nodes::LiteralExpression>(context->FLOAT()->getSymbol(),
				Value(std::stof(context->FLOAT()->getText())));
		} else if (context->INT() != nullptr) {
			expression = std::make_shared<nodes::LiteralExpression>(context->INT()->getSymbol(),
				Value(std::stoi(context->INT()->getText())));
		} else {
			const auto idText = context->STRING()->getText();

			// Cut off the first and last characters since those are the < > braces
			expression = std::make_shared<nodes::LiteralExpression>(context->STRING()->getSymbol(),
				Value(idText.substr(1, idText.size() - 2)));
		}

		return std::static_pointer_cast<nodes::AbstractExpression>(expression);
	}
	antlrcpp::Any visitParenthesis_expression(ActionExpressionParser::Parenthesis_expressionContext* context) override
	{
		return visit(context->expression());
	}
	antlrcpp::Any visitValue_expression(ActionExpressionParser::Value_expressionContext* context) override
	{
		return visitChildren(context);
	}
	antlrcpp::Any visitVec3d_constructor(ActionExpressionParser::Vec3d_constructorContext* context) override
	{
		auto xExpression = visit(context->expression(0)).as<std::shared_ptr<nodes::AbstractExpression>>();
		auto yExpression = visit(context->expression(1)).as<std::shared_ptr<nodes::AbstractExpression>>();
		auto zExpression = visit(context->expression(2)).as<std::shared_ptr<nodes::AbstractExpression>>();

		auto expression = std::make_shared<nodes::VectorConstructorExpression>(context->L_PAREN()->getSymbol(),
			xExpression,
			yExpression,
			zExpression);

		return std::static_pointer_cast<nodes::AbstractExpression>(expression);
	}
};

} // namespace

ExpressionParser::ExpressionParser(SCP_string expressionText) : m_expressionText(std::move(expressionText)) {}

std::shared_ptr<nodes::AbstractExpression> ExpressionParser::parse(const ParseContext& context)
{
	antlr4::ANTLRInputStream input(m_expressionText);
	ActionExpressionLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);

	::ActionExpressionParser parser(&tokens);
	// By default we log to stderr which we do not want
	parser.removeErrorListeners();
	libs::antlr::ErrorListener errListener;
	parser.addErrorListener(&errListener);

	antlr4::tree::ParseTree* tree = parser.expression_main();

	std::shared_ptr<nodes::AbstractExpression> exprReturn;
	if (errListener.diagnostics.empty()) {
		// Only do semantic checking if syntax is valid
		ExpressionBuilderVisitor builder;
		exprReturn = tree->accept(&builder).as<std::shared_ptr<nodes::AbstractExpression>>();

		if (!exprReturn->validate(&parser, context)) {
			exprReturn = nullptr;
		} else {
			exprReturn->validationDone();
		}
	}

	if (!errListener.diagnostics.empty()) {
		SCP_stringstream outStream;
		for (const auto& diag : errListener.diagnostics) {
			SCP_string tokenUnderline;
			if (diag.tokenLength > 1) {
				tokenUnderline = SCP_string(diag.tokenLength - 1, '~');
			}
			outStream << m_expressionText << "\n"
					  << SCP_string(diag.columnInLine, ' ') << "^" << tokenUnderline << "\n"
					  << diag.errorMessage << "\n";
		}

		m_errorText = outStream.str();
		return nullptr;
	}

	return exprReturn;
}

const SCP_string& ExpressionParser::getErrorText() const
{
	return m_errorText;
}

} // namespace expression
} // namespace actions
