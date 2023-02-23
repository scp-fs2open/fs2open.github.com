
namespace anl
{
Token::Token() : type_(Token::NONE), token_("") {}
Token::Token(ETokenTypes t, const std::string token) : type_(t), token_(token) {}
Token::Token(const Token &rhs)
{
    type_=rhs.type_;
    token_=rhs.token_;
}
Token::~Token() {}

const Token::ETokenTypes Token::GetType() const
{
    return type_;
}
const std::string &Token::GetToken() const
{
    return token_;
}


Tokenizer::Tokenizer(const std::string expr, const std::map<std::string, int> &fmap, const std::vector<std::string> &vars) :
    expression_(expr), pos_(expression_.begin()), lastToken_(Token::NONE, ""), functions_(fmap), vars_(vars)
{
}

bool Tokenizer::HasNext()
{
    if(expression_.length()==0 || pos_ == expression_.end()) return false;
    return true;
}

Token Tokenizer::NextToken()
{
    char ch=*pos_;
    while (ch==' ' || ch=='\n' || ch=='\t') ch=*(++pos_);

    if(IsDigit(ch) || ch=='.' || (ch=='-' && (lastToken_.GetType()==Token::NONE || lastToken_.GetType()==Token::OPENPARENS ||
                                  lastToken_.GetType()==Token::COMMA || lastToken_.GetType()==Token::OPERATOR)))
    {

        return ParseNumberToken(ch);
    }
    else if (ch==',') return ParseComma(ch);
    else if (ch=='(')
    {
        if(lastToken_.GetType() != Token::NONE && lastToken_.GetType() != Token::OPERATOR && lastToken_.GetType()!=Token::UNARYOPERATOR
                && lastToken_.GetType() != Token::OPENPARENS && lastToken_.GetType() != Token::FUNCTION && lastToken_.GetType() != Token::COMMA)
        {
            std::stringstream ss;
            ss << "Error: Parentheses can not follow token " << lastToken_.GetToken() << " of type " << lastToken_.GetType();
            lastToken_=Token(Token::INVALID, ss.str());
            return lastToken_;
        }
        return ParseParentheses(true);
    }
    else if (ch==')') return ParseParentheses(false);
    else if (IsValidOperator(ch)) return ParseOperator(ch);
    else if (IsAlphabetic(ch) || ch=='_')
    {
        if(lastToken_.GetType() != Token::NONE && lastToken_.GetType() != Token::OPERATOR && lastToken_.GetType()!=Token::UNARYOPERATOR
                && lastToken_.GetType() != Token::OPENPARENS && lastToken_.GetType() != Token::FUNCTION && lastToken_.GetType() != Token::COMMA)
        {
            std::stringstream ss;
            ss << "Error: Unable to parse token at " << ch;
            lastToken_=Token(Token::INVALID, ss.str());
            return lastToken_;
        }
        return ParseFunctionOrVariable(ch);
    }
    std::stringstream ss;
    ss << "Unable to parse char '" << ch << "' Code:" << (int)ch;
    lastToken_=Token(Token::INVALID, ss.str());
    return lastToken_;
}

bool Tokenizer::IsValidOperator(char ch)
{
    return (ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='^');
}

bool Tokenizer::IsDigit(char ch)
{
    return (ch=='1' || ch=='2' || ch=='3' || ch=='4' || ch=='5' || ch=='6' || ch=='7' || ch=='8' || ch=='9' || ch=='0');
}

bool Tokenizer::IsAlphabetic(char ch)
{
    return (ch>=65 && ch<=90) || (ch>=97 && ch<=122);
}

Token Tokenizer::ParseNumberToken(char ch)
{
    std::string::iterator offset=pos_;
    int len=1;
    pos_++;
    std::stringstream ss;
    if(expression_.end()==(offset+len))
    {
        ss<<ch;
        lastToken_=Token(Token::NUMBER, ss.str());
        return lastToken_;
    }

    while(expression_.end() != (offset+len) && IsNumeric(*(offset+len), *(offset+len-1)=='e' || *(offset+len-1)=='E'))
    {
        len++;
        pos_++;
    }
    if (*(offset+len-1)=='e' || *(offset+len-1)=='E')
    {
        len--;
        pos_--;
    }
    for(std::string::iterator i=offset; i!=pos_; ++i) ss << *i;
    lastToken_=Token(Token::NUMBER, ss.str());
    return lastToken_;
}

Token Tokenizer::ParseComma(char ch)
{
    pos_++;
    lastToken_=Token(Token::COMMA, ",");
    return lastToken_;
}

Token Tokenizer::ParseParentheses(bool which)
{
    if(which) lastToken_=Token(Token::OPENPARENS, "(");
    else lastToken_=Token(Token::CLOSEPARENS, ")");
    pos_++;
    return lastToken_;
}

Token Tokenizer::ParseOperator(char ch)
{
    std::stringstream ss;
    ss<<ch;
    if(ch=='-')
    {
        if (lastToken_.GetType()==Token::NONE || lastToken_.GetType()==Token::OPERATOR || lastToken_.GetType()==Token::OPENPARENS
                || lastToken_.GetType()==Token::COMMA)
        {
            // Unary
            lastToken_=Token(Token::UNARYOPERATOR, "-");
            pos_++;
            return lastToken_;
        }
        lastToken_=Token(Token::OPERATOR, "-");
        pos_++;
        return lastToken_;
    }
    else
    {
        pos_++;
        if(ch=='+' || ch=='*' || ch=='^' || ch=='/')
        {
            lastToken_=Token(Token::OPERATOR, ss.str());
            return lastToken_;
        }
        lastToken_=Token(Token::INVALID, "Invalid operator: "+ss.str());
        return lastToken_;
    }

}

Token Tokenizer::ParseFunctionOrVariable(char ch)
{
    std::string::iterator offset=pos_;
    if(offset==expression_.end()) pos_++;
    std::stringstream tok;

    while((offset)!=expression_.end() &&
            (IsAlphabetic(*(offset)) || IsDigit(*(offset)) || *(offset)=='_'))
    {
        tok << *(offset);
        ++offset;
    }

    if(IsFunctionName(tok.str()))
    {
        lastToken_=Token(Token::FUNCTION, tok.str());
    }
    else
    {
        lastToken_=Token(Token::VAR, tok.str());
    }

    pos_=offset;
    return lastToken_;
}

bool Tokenizer::IsNumeric(char ch, bool lastCharE)
{
    return (ch=='1' || ch=='2' || ch=='3' || ch=='4' || ch=='5' || ch=='6' || ch=='7' || ch=='8' || ch=='9' || ch=='0' ||
            ch=='.' || ch=='e' || ch=='E' || (lastCharE && (ch=='-' || ch=='+')));
}

bool Tokenizer::IsSpecialToken(const std::string &t)
{
    for(auto i : vars_) if(t==i) return true;
    return false;
}

bool Tokenizer::IsFunctionName(const std::string &t)
{
    return (functions_.find(t) != functions_.end());
}


ExpressionToPostfix::ExpressionToPostfix(const std::string &expr, const std::map<std::string, int> &f, const std::vector<std::string> &v) : expr_(expr),
    f_(f), vars_(v)
{


}

std::vector<Token> ExpressionToPostfix::ToPostfix()
{
    Tokenizer tz(expr_, f_, vars_);
    std::stack<Token> stk;
    std::vector<Token> output;

    while(tz.HasNext())
    {
        Token tk=tz.NextToken();
        switch(tk.GetType())
        {
        case Token::NUMBER:
        case Token::VAR:
            output.push_back(tk);
            break;
        case Token::FUNCTION:
            stk.push(tk);
            break;
        case Token::COMMA:
            while(!stk.empty() && stk.top().GetType() != Token::OPENPARENS)
            {
                output.push_back(stk.top());
                stk.pop();
            }
            if(stk.empty() || stk.top().GetType()!=Token::OPENPARENS)
            {
                output.push_back(Token(Token::INVALID, "Misplaced function separator or mismatched parens"));
                return output;
            }
            break;
        case Token::UNARYOPERATOR:
            while(!stk.empty() && (stk.top().GetType()==Token::OPERATOR || stk.top().GetType()==Token::UNARYOPERATOR))
            {
                if(stk.top().GetType()==Token::OPERATOR) break;
                else if((IsLeftAssociative(tk) && GetPrecedence(tk) <= GetPrecedence(stk.top())) || (GetPrecedence(tk) < GetPrecedence(stk.top())))
                {
                    output.push_back(stk.top());
                    stk.pop();
                }
                else break;
            }
            stk.push(tk);
            break;
        case Token::OPERATOR:
            while(!stk.empty() && ((stk.top().GetType()==Token::OPERATOR || stk.top().GetType()==Token::UNARYOPERATOR) && GetPrecedence(stk.top())>=GetPrecedence(tk)))
            {
                output.push_back(stk.top());
                stk.pop();
            }
            stk.push(tk);
            break;
        case Token::OPENPARENS:
            stk.push(tk);
            break;
        case Token::CLOSEPARENS:
            if(stk.empty())
            {
                output.push_back(Token(Token::INVALID, "Error: Encountered close parens without corresponding open."));
                return output;
            }
            while(!stk.empty() && stk.top().GetType() != Token::OPENPARENS)
            {
                output.push_back(stk.top());
                stk.pop();
            }
            stk.pop();
            if(!stk.empty() && stk.top().GetType() == Token::FUNCTION)
            {
                output.push_back(stk.top());
                stk.pop();
            }
            break;
        default:
            output.push_back(Token(Token::INVALID, "Unknown token type encountered."));
            return output;
        }
    }

    while(!stk.empty())
    {
        output.push_back(stk.top());
        stk.pop();
    }
    return output;
}

int ExpressionToPostfix::GetNumOperands(const Token &tk)
{
    if(tk.GetType()==Token::UNARYOPERATOR) return 1;
    else if(tk.GetType()==Token::OPERATOR) return 2;
    return 0;
}

bool ExpressionToPostfix::IsLeftAssociative(const Token &tk)
{
    const std::string tok=tk.GetToken();
    if(tok=="+" || tok=="-" || tok=="/" || tok=="*") return true;
    return false;
}

int ExpressionToPostfix::GetPrecedence(const Token &tk)
{
    std::string c=tk.GetToken();
    if(c == "^")
    {
        return 3;
    }
    if(c == "*" || c == "/")
    {
        return 2;
    }
    if(c== "+" || c == "-")
    {
        return 1;
    }
    else return 0;
}

};
