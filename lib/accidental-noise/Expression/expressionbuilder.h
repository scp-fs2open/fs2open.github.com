#ifndef EXPRESSIONBUILDER_H
#define EXPRESSIONBUILDER_H

#include "parsing.h"
#include "../VM/kernel.h"
#include "../VM/random_gen.h"
#include <vector>
#include <string>
#include <map>

namespace anl
{
class CExpressionBuilder
{
public:
    CExpressionBuilder(CKernel &kernel);
    ~CExpressionBuilder();

    CInstructionIndex eval(const std::string &expr);
    CInstructionIndex evalAndStore(const std::string &expr);
    void store(CInstructionIndex i);
    std::vector<Token> getPostfix(const std::string &expr);
    void setRandomSeed(unsigned int seed);

    CInstructionIndex evalAndStoreVar(const std::string &varname, const std::string &expr);
    void storeVar(const std::string &varname, CInstructionIndex idx);
    CInstructionIndex retrieveVar(const std::string &varname);

private:
    CKernel &kernel_;
    std::vector<CInstructionIndex> index_;
    std::map<std::string, int> f_;  // Functions
    std::vector<std::string> vars_;
    std::map<std::string, CInstructionIndex> storedvars_;
    KISS prng_;

    void buildFunction(const std::string &token, std::stack<CInstructionIndex> &stk);
    void buildVar(const std::string &token, std::stack<CInstructionIndex> &stk);
};
};

#endif
