#include "expressionbuilder.h"

#include <iostream>

template<class Container, class V>
std::pair<typename Container::iterator, bool> insert_or_assign(Container & c, typename Container::key_type const & k, V && v)
{
    auto itor = c.find(k);
    if(itor == c.end())
    {
        return c.emplace(k, std::forward<V>(v));
    }
    else
    {
        itor->second = std::forward<V>(v);
        return std::make_pair(itor, false);
    }
}

template<class C, class V>
std::pair<typename C::iterator, bool> insert_or_assign(C & c, typename C::key_type && k, V && v)
{
    auto itor = c.find(k);
    if(itor == c.end())
    {
        return c.emplace(std::move(k), std::forward<V>(v));
    }
    else
    {
        itor->second = std::forward<V>(v);
        return std::make_pair(itor, false);
    }
}

namespace anl
{
CExpressionBuilder::CExpressionBuilder(CKernel &kernel) : kernel_(kernel)
{
    f_["valueBasis"]=2;
    f_["gradientBasis"]=2;
    f_["simplexBasis"]=1;
    f_["cellularBasis"]=10;
    f_["max"]=2;
    f_["min"]=2;
    f_["abs"]=1;
    f_["bias"]=2;
    f_["gain"]=2;
    f_["scale"]=2;
    f_["scaleX"]=2;
    f_["scaleY"]=2;
    f_["scaleZ"]=2;
    f_["scaleW"]=2;
    f_["scaleU"]=2;
    f_["scaleV"]=2;
    f_["translate"]=2;
    f_["translateX"]=2;
    f_["translateY"]=2;
    f_["translateZ"]=2;
    f_["translateW"]=2;
    f_["translateU"]=2;
    f_["translateV"]=2;
    f_["rotateDomain"]=5;
    f_["mix"]=3;
    f_["select"]=5;
    f_["clamp"]=3;
    f_["cos"]=1;
    f_["sin"]=1;
    f_["tan"]=1;
    f_["acos"]=1;
    f_["asin"]=1;
    f_["atan"]=1;
    f_["tiers"]=2;
    f_["smoothTiers"]=2;
    f_["dx"]=1;
    f_["dy"]=1;
    f_["dz"]=1;
    f_["dw"]=1;
    f_["du"]=1;
    f_["dv"]=1;
    f_["sigmoid"]=3;
	f_["randomize"]=3;
	f_["fractal"]=6;
	f_["step"]=2;
	f_["linearStep"]=3;
	f_["smoothStep"]=3;
	f_["smootherStep"]=3;
	f_["curveSection"]=5;
    f_["index"]=1;
    f_["rindex"]=1;

    f_["color"]=4;

    // Build vars
    vars_.push_back("rand");
    vars_.push_back("rand01");
    vars_.push_back("x");
    vars_.push_back("y");
    vars_.push_back("z");
    vars_.push_back("w");
    vars_.push_back("u");
    vars_.push_back("v");
    vars_.push_back("radial");

}
CExpressionBuilder::~CExpressionBuilder() {}

void CExpressionBuilder::setRandomSeed(unsigned int seed)
{
    prng_.setSeed(seed);
}

std::vector<Token> CExpressionBuilder::getPostfix(const std::string &expr)
{
    ExpressionToPostfix e(expr, f_, vars_);
    return e.ToPostfix();
}

CInstructionIndex CExpressionBuilder::eval(const std::string &expr)
{
    ExpressionToPostfix e(expr, f_, vars_);

    auto p=e.ToPostfix();
    std::stack<CInstructionIndex> stk;

    for(auto i : p)
    {
        if(i.GetType()==Token::NUMBER)
        {
            stk.push(kernel_.constant(std::stod(i.GetToken())));
        }
        else if(i.GetType()==Token::OPERATOR)
        {
            CInstructionIndex right=stk.top();
            stk.pop();
            CInstructionIndex left=stk.top();
            stk.pop();
            if(i.GetToken()=="+") stk.push(kernel_.add(left,right));
            else if(i.GetToken()=="-") stk.push(kernel_.subtract(left,right));
            else if(i.GetToken()=="*") stk.push(kernel_.multiply(left,right));
            else if(i.GetToken()=="/") stk.push(kernel_.divide(left,right));
            else if(i.GetToken()=="^") stk.push(kernel_.pow(left,right));
        }
        else if(i.GetType()==Token::UNARYOPERATOR)
        {
            CInstructionIndex o=stk.top();
            stk.pop();
            stk.push(kernel_.multiply(o, kernel_.constant(-1.0)));
        }
        else if(i.GetType()==Token::FUNCTION)
        {
            buildFunction(i.GetToken(), stk);
        }
        else if(i.GetType()==Token::VAR)
        {
            buildVar(i.GetToken(), stk);
        }
    }

    return stk.top();
}

CInstructionIndex CExpressionBuilder::evalAndStore(const std::string &expr)
{
    auto e=eval(expr);

    index_.push_back(e);

    return e;
}

CInstructionIndex CExpressionBuilder::evalAndStoreVar(const std::string &varname, const std::string &expr)
{
    auto e=eval(expr);
    insert_or_assign(storedvars_, varname, e);

    return e;
}

void CExpressionBuilder::store(CInstructionIndex i)
{
    index_.push_back(i);
}

CInstructionIndex CExpressionBuilder::retrieveVar(const std::string &varname)
{
    auto i=storedvars_.find(varname);
    if(i!=storedvars_.end()) return (*i).second;
    else return kernel_.zero();
}

void CExpressionBuilder::storeVar(const std::string &varname, CInstructionIndex i)
{
    insert_or_assign(storedvars_, varname, i);
}

void CExpressionBuilder::buildVar(const std::string &token, std::stack<CInstructionIndex> &stk)
{
    if(token=="rand")
    {
        stk.push(kernel_.seed(prng_.get()));
    }
    else if (token=="rand01")
    {
        stk.push(kernel_.constant(prng_.get01()));
    }
    else if (token=="x")
    {
        stk.push(kernel_.x());
    }
    else if (token=="y")
    {
        stk.push(kernel_.y());
    }
    else if (token=="z")
    {
        stk.push(kernel_.z());
    }
    else if (token=="w")
    {
        stk.push(kernel_.w());
    }
    else if (token=="u")
    {
        stk.push(kernel_.u());
    }
    else if (token=="v")
    {
        stk.push(kernel_.v());
    }
    else if (token=="radial")
    {
        stk.push(kernel_.radial());
    }
    else
    {
        // Not a pre-built token, let's search the list of stored vars
        auto i=storedvars_.find(token);
        if(i!=storedvars_.end())
        {
            stk.push((*i).second);
        }
        else
        {
            stk.push(kernel_.getVar(token));
        }
    }
}

void CExpressionBuilder::buildFunction(const std::string &token, std::stack<CInstructionIndex> &stk)
{
    if(token=="valueBasis")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.valueBasis(left,right));
    }
    else if(token=="gradientBasis")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.gradientBasis(left,right));
    }
    else if(token=="simplexBasis")
    {
        CInstructionIndex o=stk.top();
        stk.pop();
        stk.push(kernel_.simplexBasis(o));
    }
    else if(token=="cellularBasis")
    {
        CInstructionIndex c9=stk.top();
        stk.pop();
        CInstructionIndex c8=stk.top();
        stk.pop();
        CInstructionIndex c7=stk.top();
        stk.pop();
        CInstructionIndex c6=stk.top();
        stk.pop();
        CInstructionIndex c5=stk.top();
        stk.pop();
        CInstructionIndex c4=stk.top();
        stk.pop();
        CInstructionIndex c3=stk.top();
        stk.pop();
        CInstructionIndex c2=stk.top();
        stk.pop();
        CInstructionIndex c1=stk.top();
        stk.pop();
        CInstructionIndex c0=stk.top();
        stk.pop();
        stk.push(kernel_.cellularBasis(c0,c1,c2,c3,c4,c5,c6,c7,c8,c9));
    }
    else if(token=="max")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.maximum(left,right));
    }
    else if(token=="min")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.minimum(left,right));
    }
    else if(token=="abs")
    {
        CInstructionIndex o=stk.top();
        stk.pop();
        stk.push(kernel_.abs(o));
    }
    else if(token=="bias")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.bias(left,right));
    }
    else if(token=="gain")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.gain(left,right));
    }
    else if(token=="scale")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.scaleDomain(left, right));
    }
    else if(token=="scaleX")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.scaleX(left,right));
    }
    else if(token=="scaleY")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.scaleY(left,right));
    }
    else if(token=="scaleZ")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.scaleZ(left,right));
    }
    else if(token=="scaleW")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.scaleW(left,right));
    }
    else if(token=="scaleU")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.scaleU(left,right));
    }
    else if(token=="scaleV")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.scaleV(left,right));
    }
    else if(token=="translate")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.translateDomain(left,right));
    }
    else if(token=="translateX")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.translateX(left,right));
    }
    else if(token=="translateY")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.translateY(left,right));
    }
    else if(token=="translateZ")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.translateZ(left,right));
    }
    else if(token=="translateW")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.translateW(left,right));
    }
    else if(token=="translateU")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.translateU(left,right));
    }
    else if(token=="translateV")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.translateV(left,right));
    }
    else if(token=="rotateDomain")
    {
        CInstructionIndex c4=stk.top();
        stk.pop();
        CInstructionIndex c3=stk.top();
        stk.pop();
        CInstructionIndex c2=stk.top();
        stk.pop();
        CInstructionIndex c1=stk.top();
        stk.pop();
        CInstructionIndex c0=stk.top();
        stk.pop();
        stk.push(kernel_.rotateDomain(c0,c1,c2,c3,c4));
    }
    else if(token=="mix")
    {
        CInstructionIndex c2=stk.top();
        stk.pop();
        CInstructionIndex c1=stk.top();
        stk.pop();
        CInstructionIndex c0=stk.top();
        stk.pop();
        stk.push(kernel_.mix(c0,c1,c2));
    }
    else if(token=="select")
    {
        CInstructionIndex c4=stk.top();
        stk.pop();
        CInstructionIndex c3=stk.top();
        stk.pop();
        CInstructionIndex c2=stk.top();
        stk.pop();
        CInstructionIndex c1=stk.top();
        stk.pop();
        CInstructionIndex c0=stk.top();
        stk.pop();
        stk.push(kernel_.select(c0,c1,c2,c3,c4));
    }
    else if(token=="clamp")
    {
        CInstructionIndex c2=stk.top();
        stk.pop();
        CInstructionIndex c1=stk.top();
        stk.pop();
        CInstructionIndex c0=stk.top();
        stk.pop();
        stk.push(kernel_.clamp(c0,c1,c2));
    }
    else if(token=="cos")
    {
        CInstructionIndex o=stk.top();
        stk.pop();
        stk.push(kernel_.cos(o));
    }
    else if(token=="sin")
    {
        CInstructionIndex o=stk.top();
        stk.pop();
        stk.push(kernel_.sin(o));
    }
    else if(token=="tan")
    {
        CInstructionIndex o=stk.top();
        stk.pop();
        stk.push(kernel_.tan(o));
    }
    else if(token=="acos")
    {
        CInstructionIndex o=stk.top();
        stk.pop();
        stk.push(kernel_.acos(o));
    }
    else if(token=="asin")
    {
        CInstructionIndex o=stk.top();
        stk.pop();
        stk.push(kernel_.asin(o));
    }
    else if(token=="atan")
    {
        CInstructionIndex o=stk.top();
        stk.pop();
        stk.push(kernel_.atan(o));
    }
    else if(token=="tiers")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.tiers(left,right));
    }
    else if(token=="smoothTiers")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.smoothTiers(left,right));
    }
    else if(token=="dx")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.dx(left,right));
    }
    else if(token=="dy")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.dy(left,right));
    }
    else if(token=="dz")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.dz(left,right));
    }
    else if(token=="dw")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.dw(left,right));
    }
    else if(token=="du")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.du(left,right));
    }
    else if(token=="dv")
    {
        CInstructionIndex right=stk.top();
        stk.pop();
        CInstructionIndex left=stk.top();
        stk.pop();
        stk.push(kernel_.dv(left,right));
    }
    else if(token=="color")
    {
        CInstructionIndex a=stk.top();
        stk.pop();
        CInstructionIndex b=stk.top();
        stk.pop();
        CInstructionIndex g=stk.top();
        stk.pop();
        CInstructionIndex r=stk.top();
        stk.pop();

        stk.push(kernel_.combineRGBA(r,g,b,a));
    }
    else if(token=="sigmoid")
    {
        CInstructionIndex rmp=stk.top();
        stk.pop();
        CInstructionIndex cntr=stk.top();
        stk.pop();
        CInstructionIndex src=stk.top();
        stk.pop();

        stk.push(kernel_.sigmoid(src,cntr,rmp));
    }
	else if(token=="randomize")
	{
		CInstructionIndex seed=stk.top();
		stk.pop();
		CInstructionIndex low=stk.top(); stk.pop();
		CInstructionIndex high=stk.top(); stk.pop();
		stk.push(kernel_.randomize(seed,low,high));
	}
	else if(token=="step")
	{
		CInstructionIndex control=stk.top(); stk.pop();
		CInstructionIndex val=stk.top(); stk.pop();
		stk.push(kernel_.step(val,control));
	}
	else if(token=="linearStep")
	{
		CInstructionIndex control=stk.top(); stk.pop();
		CInstructionIndex high=stk.top(); stk.pop();
		CInstructionIndex low=stk.top(); stk.pop();
		stk.push(kernel_.linearStep(low,high,control));
	}
	else if(token=="smoothStep")
	{
		CInstructionIndex control=stk.top(); stk.pop();
		CInstructionIndex high=stk.top(); stk.pop();
		CInstructionIndex low=stk.top(); stk.pop();
		stk.push(kernel_.smoothStep(low,high,control));
	}
	else if(token=="smootherStep")
	{
		CInstructionIndex control=stk.top(); stk.pop();
		CInstructionIndex high=stk.top(); stk.pop();
		CInstructionIndex low=stk.top(); stk.pop();
		stk.push(kernel_.smootherStep(low,high,control));
	}
	else if(token=="curveSection")
	{
		CInstructionIndex control=stk.top(); stk.pop();
		CInstructionIndex v1=stk.top(); stk.pop();
		CInstructionIndex v0=stk.top(); stk.pop();
		CInstructionIndex t1=stk.top(); stk.pop();
		CInstructionIndex t0=stk.top(); stk.pop();
		CInstructionIndex lowv=stk.top(); stk.pop();
		stk.push(kernel_.curveSection(lowv,t0,t1,v0,v1,control));
	}
	else if(token=="fractal")
	{
		CInstructionIndex freq=stk.top(); stk.pop();
		CInstructionIndex octaves=stk.top(); stk.pop();
		CInstructionIndex lac=stk.top(); stk.pop();
		CInstructionIndex pers=stk.top(); stk.pop();
		CInstructionIndex layer=stk.top(); stk.pop();
		CInstructionIndex seed=stk.top(); stk.pop();
		stk.push(kernel_.fractal(seed,layer,pers,lac,octaves,freq));
	}
    else if(token=="index")
    {
        InstructionListType *il=kernel_.getKernel();
        CInstructionIndex i=stk.top();
        stk.pop();
        unsigned int id=i.index_;
        if(id >= il->size() || (*il)[id].opcode_!=OP_Constant)
        {
            // Parameter passed to index must be a number
            // TODO: Figure out how to convey error to user
            stk.push(kernel_.zero());
        }
        else
        {
            unsigned int ic=(unsigned int)(*il)[id].outfloat_;
            if(ic>=index_.size())
            {
                // Index out of range
                stk.push(kernel_.zero());
            }
            else
            {
                stk.push(index_[ic]);
            }
        }


    }
    else if(token=="rindex")
    {
        // Reverse index
        InstructionListType *il=kernel_.getKernel();
        CInstructionIndex i=stk.top();
        stk.pop();
        unsigned int id=i.index_;
        if(id >= il->size() || (*il)[id].opcode_!=OP_Constant)
        {
            // Parameter passed to index must be a number
            // TODO: Figure out how to convey error to user
            stk.push(kernel_.zero());
        }
        else
        {
            unsigned int ic=(unsigned int)(*il)[id].outfloat_;
            if(ic>=index_.size())
            {
                // Index out of range
                stk.push(kernel_.zero());
            }
            else
            {
                stk.push(index_[(index_.size()-1)-ic]);
            }
        }


    }

}
};
