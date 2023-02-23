#ifndef PARSING_H
#define PARSING_H
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <stack>

namespace anl
{
class Token
{
public:
    enum ETokenTypes
    {
        COMMA,
        CLOSEPARENS,
        OPENPARENS,
        NUMBER,
        FUNCTION,
        OPERATOR,
        UNARYOPERATOR,
        VAR,
        INVALID,
        NONE,
    };

    Token();
    Token(ETokenTypes t, const std::string token);
    Token(const Token &rhs);
    virtual ~Token();

    const ETokenTypes GetType() const;
    const std::string &GetToken() const;

protected:
    ETokenTypes type_;
    std::string token_;
};

class Tokenizer
{
public:
    Tokenizer(const std::string expr, const std::map<std::string, int> &fmap, const std::vector<std::string> &vars);
    bool HasNext();
    Token NextToken();

protected:
    std::string expression_;
    std::string::iterator pos_;
    Token lastToken_;
    std::map<std::string, int> functions_;
    std::vector<std::string> vars_;

    bool IsValidOperator(char ch);
    bool IsDigit(char ch);
    bool IsAlphabetic(char ch);
    Token ParseNumberToken(char ch);
    Token ParseComma(char ch);
    Token ParseParentheses(bool which);
    Token ParseOperator(char ch);
    Token ParseFunctionOrVariable(char ch);
    bool IsNumeric(char ch, bool lastCharE);
    bool IsSpecialToken(const std::string &t);
    bool IsFunctionName(const std::string &t);
};

class ExpressionToPostfix
{
public:
    ExpressionToPostfix(const std::string &expr, const std::map<std::string, int> &f, const std::vector<std::string> &v);
    std::vector<Token> ToPostfix();

protected:
    std::string expr_;
    const std::map<std::string, int> &f_;
    const std::vector<std::string> &vars_;

    int GetNumOperands(const Token &tk);
    bool IsLeftAssociative(const Token &tk);
    int GetPrecedence(const Token &tk);
};
};
#endif
