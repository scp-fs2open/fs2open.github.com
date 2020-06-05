#include "ade_doc.h"

#include "globalincs/pstypes.h"

#include <regex>
#include <utility>

namespace scripting {

ade_type_info::ade_type_info(std::initializer_list<ade_type_info> tuple_types)
	: _type(ade_type_info_type::Tuple), _elements(tuple_types)
{
	Assertion(tuple_types.size() >= 1, "Tuples must have more than one element!");
}
ade_type_info::ade_type_info(const char* single_type) : _type(ade_type_info_type::Simple)
{
	if (single_type != nullptr) {
		_simple_name.assign(single_type);
	}

	if (single_type == nullptr) {
		// It would be better to handle this via an overload with std::nullptr_t but there are a lot of NULL usages left
		// so this is the soltuion that requires fewer changes
		_type = ade_type_info_type::Empty;
		return;
	}

	// Otherwise, make sure no one tries to specify a tuple with a single string.
	Assertion(strchr(single_type, ',') == nullptr,
		"Type string '%s' may not contain commas! Use the intializer list instead.",
		single_type);
}
ade_type_info::ade_type_info(const ade_type_array& listType)
	: _type(ade_type_info_type::Array), _elements{listType.getElementType()}
{
}
ade_type_info::ade_type_info(const ade_type_map& listType)
	: _type(ade_type_info_type::Map), _elements{listType.getKeyType(), listType.getValueType()}
{
}
ade_type_info::ade_type_info(const ade_type_iterator& iteratorType)
	: _type(ade_type_info_type::Iterator), _elements{iteratorType.getElementType()}
{
}
ade_type_info::ade_type_info(const ade_type_alternative& iteratorType)
	: _type(ade_type_info_type::Alternative), _elements(iteratorType.getElementTypes())
{
}
bool ade_type_info::isEmpty() const { return _type == ade_type_info_type::Empty; }
bool ade_type_info::isTuple() const { return _type == ade_type_info_type::Tuple; }
bool ade_type_info::isSimple() const { return _type == ade_type_info_type::Simple; }
bool ade_type_info::isArray() const { return _type == ade_type_info_type::Array; }
const SCP_vector<ade_type_info>& ade_type_info::elements() const { return _elements; }
const char* ade_type_info::getSimpleName() const { return _simple_name.c_str(); }
ade_type_info_type ade_type_info::getType() const { return _type; }
const ade_type_info& ade_type_info::arrayType() const { return _elements.front(); }

ade_type_array::ade_type_array(ade_type_info elementType) : _element_type(std::move(elementType)) {}
const ade_type_info& ade_type_array::getElementType() const { return _element_type; }

ade_type_map::ade_type_map(ade_type_info keyType, ade_type_info valueType)
	: _key_type(std::move(keyType)), _value_type(std::move(valueType))
{
}
const ade_type_info& ade_type_map::getKeyType() const { return _key_type; }
const ade_type_info& ade_type_map::getValueType() const { return _value_type; }

ade_type_iterator::ade_type_iterator(ade_type_info elementType) : _element_type(std::move(elementType)) {}
const ade_type_info& ade_type_iterator::getElementType() const { return _element_type; }

ade_type_alternative::ade_type_alternative(SCP_vector<ade_type_info> elements) : _elements(std::move(elements)) {}
const SCP_vector<ade_type_info>& ade_type_alternative::getElementTypes() const { return _elements; }

namespace {

struct arglist_parser_regexes {
	std::regex identifier{R"(^[\w:_-%]+)"};
	std::regex number{R"(^-?\d+(\.\d*)?)"};
	std::regex string{R"(^"[^"]*")"};
	std::regex tableInit{R"(^\{[^\}]*\})"};
};

arglist_parser_regexes& regexes()
{
	// Keep a single instance of this to keep the construction cost down. Also use the static initializer since this
	// code will be needed at program startup
	static arglist_parser_regexes instance;
	return instance;
}

enum class TokenType {
	Equals,
	LeftBracket,
	RightBracket,
	Comma,
	TypeAlternative,

	Identifier,
	Number,
	String,
	TableInitializer,

	EndOfFile,
};

struct regex_token {
	TokenType type;
	const std::regex& regex;
};

using regex_tokens = std::array<regex_token, 4>;
regex_tokens getRegexTokens()
{
	const auto& regex = regexes();

	return {
		regex_token{TokenType::Number, regex.number},
		regex_token{TokenType::String, regex.string},
		regex_token{TokenType::Identifier, regex.identifier},
		regex_token{TokenType::TableInitializer, regex.tableInit},
	};
}

const char* to_string(TokenType t)
{
	switch (t) {
	case TokenType::Equals:
		return "'='";
	case TokenType::LeftBracket:
		return "'['";
	case TokenType::RightBracket:
		return "']'";
	case TokenType::Comma:
		return "','";
	case TokenType::TypeAlternative:
		return "'|' or '/'";
	case TokenType::EndOfFile:
		return ">End Of Input<";
	case TokenType::Identifier:
		return "<identifier>";
	case TokenType::String:
		return "<string>";
	case TokenType::Number:
		return "<number>";
	case TokenType::TableInitializer:
		return "<table initializer>";
	}

	UNREACHABLE("Unknown token type %d", static_cast<int>(t));
	return "UNKNOWN";
}

struct Token {
	TokenType type;
	SCP_string content;
	size_t offset;
};

class token_error : public std::exception {
	static SCP_string formatTokenMessage(const Token& t)
	{
		SCP_string str = to_string(t.type);
		if (!t.content.empty()) {
			str += " (\"";
			str += t.content;
			str += "\")";
		}
		return str;
	}

  public:
	token_error(SCP_string errMsg, Token token) : _errMsg(std::move(errMsg)), _token(std::move(token))
	{
		_errMsg += ": Found ";
		_errMsg += formatTokenMessage(_token);
	}
	~token_error() override = default;

	const char* what() const noexcept override { return _errMsg.c_str(); }

	const Token& getToken() const { return _token; }

  private:
	SCP_string _errMsg;
	Token _token;
};

// What follows is a recursive decent parser for a LL(1) language that should parse most argument lists we have that
// don't involve too complicated constructs. This should be handled by an actual parser generator but I don't want to
// pull is bison/flex for such a simple language...
using lookahead_array = std::array<Token, 1>;

namespace detail {
bool checkType(TokenType) { return false; }
template <typename T, typename... Rest>
bool checkType(TokenType t, T testVal, Rest... rest)
{
	return t == testVal || checkType(t, rest...);
}

template <typename T>
void appendExpectString(SCP_stringstream& stream, bool toplevel, T enumVal)
{
	if (!toplevel) {
		stream << "or ";
	}
	stream << to_string(enumVal);
}
template <typename T, typename... Rest>
void appendExpectString(SCP_stringstream& stream, bool, T enumVal, Rest... rest)
{
	stream << to_string(enumVal) << ", ";
	appendExpectString(stream, false, rest...);
}
} // namespace detail

template <TokenType... ValidTypes>
bool isTypeValid(TokenType t)
{
	return detail::checkType(t, ValidTypes...);
}

template <TokenType... ValidTypes>
SCP_string constructExpectString()
{
	SCP_stringstream stream;
	detail::appendExpectString(stream, true, ValidTypes...);
	return stream.str();
}

struct lexer_state {
	SCP_string parse_text;

	Token parseToken()
	{
		fillLookahead();
		Assertion(lookeahead_size > 0, "Lookahead was not filled after fillLookahead!");
		Token parsed = std::move(lookahead[0]);
		--lookeahead_size;
		return parsed;
	}

	template <TokenType... ValidTypes>
	Token parseAndValidateToken()
	{
		auto parsed = parseToken();

		if (!isTypeValid<ValidTypes...>(parsed.type)) {
			throw token_error("Unexpected token. Expected " + constructExpectString<ValidTypes...>(), parsed);
		}

		return parsed;
	}

	const Token& peekToken()
	{
		fillLookahead();

		Assertion(lookeahead_size > 0, "Lookahead was not filled after fillLookahead!");
		return lookahead[0];
	}

  private:
	size_t offset = 0;

	lookahead_array lookahead;
	size_t lookeahead_size = 0;

	void fillLookahead()
	{
		while (lookeahead_size < lookahead.size()) {
			lookahead[lookeahead_size] = parseAndAdvance();
			++lookeahead_size;
		}
	}

	Token parseAndAdvance()
	{
		// Skip whitespace
		while (offset < parse_text.size() && parse_text[offset] == ' ') {
			++offset;
		}
		if (offset >= parse_text.size()) {
			return {TokenType::EndOfFile, SCP_string(), offset};
		}

		const auto tokenOff = offset;

		switch (parse_text[offset]) {
		case '[':
			++offset;
			return {TokenType::LeftBracket, SCP_string(), tokenOff};
		case ']':
			++offset;
			return {TokenType::RightBracket, SCP_string(), tokenOff};
		case ',':
			++offset;
			return {TokenType::Comma, SCP_string(), tokenOff};
		case '=':
			++offset;
			return {TokenType::Equals, SCP_string(), tokenOff};
		case '|':
		case '/':
			++offset;
			return {TokenType::TypeAlternative, SCP_string(), tokenOff};
		}

		// Check if this a regex based token
		for (const auto& regexTok : getRegexTokens()) {
			std::smatch match;
			if (!std::regex_search(parse_text.cbegin() + offset, parse_text.cend(), match, regexTok.regex)) {
				continue;
			}

			offset += match.length(0);
			return {regexTok.type, match.str(0), tokenOff};
		}

		throw std::runtime_error("Unrecognized input: " + parse_text.substr(offset));
	}
};

struct parser_state {
	lexer_state lexer;

	SCP_vector<argument_def> arguments;

	void main_arg_list()
	{
		const auto& next = lexer.peekToken();
		if (next.type == TokenType::EndOfFile) {
			// Empty parameter list is valid
			return;
		}

		arg_list();
	}

	SCP_string value()
	{
		auto t = lexer.parseAndValidateToken<TokenType::Identifier,
			TokenType::Number,
			TokenType::String,
			TokenType::TableInitializer>();
		return t.content;
	}

	ade_type_info simple_type()
	{
		Token t = lexer.parseAndValidateToken<TokenType::Identifier>();
		return ade_type_info(t.content.c_str());
	}

	ade_type_info type()
	{
		auto firstType = simple_type();

		auto& next = lexer.peekToken();
		if (next.type != TokenType::TypeAlternative) {
			// Not something for us
			return firstType;
		}
		lexer.parseToken(); // Remove peeked token from stream

		auto rest = type();

		SCP_vector<ade_type_info> typeList;
		typeList.reserve(rest.elements().size() + 1);

		typeList.push_back(firstType);
		if (rest.getType() == ade_type_info_type::Alternative) {
			typeList.insert(typeList.end(), rest.elements().begin(), rest.elements().end());
		} else {
			typeList.push_back(rest);
		}

		return ade_type_alternative(typeList);
	}

	void arg_list()
	{
		argument_def def;

		// An argument has the form ("[")? identifier (identifier ("=" value)? )? ("," arg_list)? ("]")?
		{
			const Token& next = lexer.peekToken();
			if (next.type == TokenType::LeftBracket) {
				lexer.parseToken();

				def.optional = true;
			}
		}

		// The first identifier is always the type name
		def.type = type();

		// We can either have an identifier (arg name) or a comma (another argument)
		auto t = lexer.parseToken();
		if (t.type == TokenType::EndOfFile) {
			// This is fine, rest is optional
			arguments.insert(arguments.cbegin(), std::move(def));
			return;
		}

		if (t.type == TokenType::Comma) {
			// Since there was a comma another argument must follow
			arg_list();

			// Since we parse the rest of the argument already we need to insert this at the front
			arguments.insert(arguments.cbegin(), std::move(def));
			return;
		}

		if (t.type != TokenType::Identifier) {
			// This is a valid point to terminate the parameter
			arguments.insert(arguments.cbegin(), std::move(def));
			return;
		}

		// token is an identifier which must be our argument name
		def.name = t.content;

		if (!def.optional) {
			// Make sure we do not consume the right bracket of our parent
			const auto& next = lexer.peekToken();

			if (next.type == TokenType::RightBracket) {
				// Ok, let our parent handle the right bracket
				arguments.insert(arguments.cbegin(), std::move(def));
				return;
			}
		}

		// We either expect a comma (next argument) or an equals (default value specifier)
		if (def.optional) {
			// If we saw the opening optional group then the right bracket is also valid here
			t = lexer.parseAndValidateToken<TokenType::EndOfFile,
				TokenType::Comma,
				TokenType::Equals,
				TokenType::RightBracket>();
		} else {
			t = lexer.parseAndValidateToken<TokenType::EndOfFile, TokenType::Comma, TokenType::Equals>();
		}
		if (t.type == TokenType::EndOfFile) {
			// This is fine, rest is optional
			arguments.insert(arguments.cbegin(), std::move(def));
			return;
		}

		if (t.type == TokenType::Comma) {
			// Since there was a comma another argument must follow
			arg_list();

			// Since we parse the rest of the argument already we need to insert this at the front
			arguments.insert(arguments.cbegin(), std::move(def));
			return;
		}

		if (def.optional && t.type == TokenType::RightBracket) {
			// We are done and nothing else is expected now
			arguments.insert(arguments.cbegin(), std::move(def));
			return;
		}

		// This must now be the default value
		def.def_val = value();

		bool sawClosingBracket = false;
		const auto& next       = lexer.peekToken();
		if (def.optional && next.type == TokenType::RightBracket) {
			// Properly terminated an optional parameter
			lexer.parseToken(); // Remove from stream
			sawClosingBracket = true;
		} else if (next.type == TokenType::Comma) {
			// There is another parameter
			lexer.parseToken(); // Remove from stream

			arg_list();
		}

		// Final check, if we didn't see the closing bracket yet it must be here
		if (!sawClosingBracket && def.optional) {
			lexer.parseAndValidateToken<TokenType::RightBracket>();
		}

		// Since we parse the rest of the argument already we need to insert this at the front
		arguments.insert(arguments.cbegin(), std::move(def));
	}

	void toplevel_arg_list()
	{
		const Token& next = lexer.peekToken();
		if (next.type == TokenType::EndOfFile) {
			// Valid to have an empty list
			return;
		}

		arg_list();
	}
};

} // namespace

bool argument_list_parser::parse(const SCP_string& argumentList)
{
	lexer_state lexer;
	lexer.parse_text = argumentList;

	parser_state parser;
	parser.lexer = std::move(lexer);

	try {
		// Kick off the parsing by expecting an argument list
		parser.toplevel_arg_list();

		const auto& next = lexer.peekToken();
		if (next.type != TokenType::EndOfFile) {
			throw token_error("Input remained after parsing.", next);
		}

		_argList = parser.arguments;
		return true;
	} catch (const token_error& token_error) {
		fprintf(stderr,
			"Error while parsing\n%s\n%s^\n%s\n\n",
			parser.lexer.parse_text.c_str(),
			std::string(token_error.getToken().offset, ' ').c_str(),
			token_error.what());

		return false;
	} catch (const std::exception& e) {
		fprintf(stderr, "Error while parsing\n%s\n%s\n\n", parser.lexer.parse_text.c_str(), e.what());

		return false;
	}
}
const SCP_vector<scripting::argument_def>& argument_list_parser::getArgList() const { return _argList; }

} // namespace scripting
