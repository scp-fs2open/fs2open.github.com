namespace anl
{

CKernel::CKernel()
{
    pi_=constant(3.14159265358979323846);
    e_=constant(2.71828182845904523536);
    one_=constant(1.0);
    zero_=constant(0.0);
    point5_=constant(0.5);
    sqrt2_=constant(sqrt(2.0));
}

CKernel::CKernel(const CKernel &rhs)
{
	kernel_=rhs.kernel_;
	pi_=rhs.pi_;
	e_=rhs.e_;
	one_=rhs.one_;
	zero_=rhs.zero_;
	point5_=rhs.point5_;
	sqrt2_=rhs.sqrt2_;
	vars_=rhs.vars_;
}

CInstructionIndex CKernel::pi()
{
    return pi_;
}

CInstructionIndex CKernel::e()
{
    return e_;
}

CInstructionIndex CKernel::one()
{
    return one_;
}

CInstructionIndex CKernel::zero()
{
    return zero_;
}

CInstructionIndex CKernel::point5()
{
    return point5_;
}

CInstructionIndex CKernel::sqrt2()
{
    return sqrt2_;
}


CInstructionIndex CKernel::constant(double val)
{
    anl::SInstruction i;
    i.outfloat_=val;
    i.opcode_=anl::OP_Constant;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::seed(unsigned int val)
{
    anl::SInstruction i;
    i.outfloat_=(double)val;
    i.opcode_=anl::OP_Seed;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::seeder(CInstructionIndex sd, CInstructionIndex src)
{
	anl::SInstruction i;
	i.sources_[0]=sd.index_;
	i.sources_[1]=src.index_;
	i.opcode_=anl::OP_Seeder;
	kernel_.push_back(i);
	return lastIndex();
}

CInstructionIndex CKernel::valueBasis(CInstructionIndex interpindex, CInstructionIndex seed)
{
    anl::SInstruction i;

    i.opcode_=anl::OP_ValueBasis;
    i.sources_[0]=interpindex.index_;
    i.sources_[1]=seed.index_;
    //i.seed_=seed;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::gradientBasis(CInstructionIndex interp, CInstructionIndex seed)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_GradientBasis;
    i.sources_[0]=interp.index_;
    i.sources_[1]=seed.index_;
    //i.seed_=seed;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::simplexBasis(CInstructionIndex seed)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_SimplexBasis;
    i.sources_[0]=seed.index_;
    //i.seed_=seed;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::fractal(CInstructionIndex seed, CInstructionIndex layer, CInstructionIndex persistence, CInstructionIndex lacunarity, CInstructionIndex numoctaves, CInstructionIndex frequency)
{
	anl::SInstruction i;
	i.opcode_=anl::OP_Fractal;
	i.sources_[0]=seed.index_;
	i.sources_[1]=layer.index_;
	i.sources_[2]=persistence.index_;
	i.sources_[3]=lacunarity.index_;
	i.sources_[4]=numoctaves.index_;
	i.sources_[5]=frequency.index_;
	kernel_.push_back(i);
	return lastIndex();
}

CInstructionIndex CKernel::randomize(CInstructionIndex seed, CInstructionIndex low, CInstructionIndex high)
{
	anl::SInstruction i;
	i.opcode_=anl::OP_Randomize;
	i.sources_[0]=seed.index_;
	i.sources_[1]=low.index_;
	i.sources_[2]=high.index_;
	kernel_.push_back(i);
	return lastIndex();
}

CInstructionIndex CKernel::cellularBasis(CInstructionIndex f1, CInstructionIndex f2, CInstructionIndex f3, CInstructionIndex f4, CInstructionIndex d1, CInstructionIndex d2, CInstructionIndex d3, CInstructionIndex d4, CInstructionIndex dist, CInstructionIndex seed)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_CellularBasis;
    //i.seed_=seed;
    i.sources_[0]=dist.index_;
    i.sources_[1]=f1.index_;
    i.sources_[2]=f2.index_;
    i.sources_[3]=f3.index_;
    i.sources_[4]=f4.index_;
    i.sources_[5]=d1.index_;
    i.sources_[6]=d2.index_;
    i.sources_[7]=d3.index_;
    i.sources_[8]=d4.index_;
    i.sources_[9]=seed.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::add(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Add;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::subtract(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Subtract;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::multiply(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Multiply;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::divide(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Divide;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::maximum(CInstructionIndex s1index, CInstructionIndex s2index)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Max;
    i.sources_[0]=s1index.index_;
    i.sources_[1]=s2index.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::minimum(CInstructionIndex s1index, CInstructionIndex s2index)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Min;
    i.sources_[0]=s1index.index_;
    i.sources_[1]=s2index.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::abs(CInstructionIndex sindex)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Abs;
    i.sources_[0]=sindex.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::pow(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Pow;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::bias(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Bias;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::gain(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Gain;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::cos(CInstructionIndex sindex)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Cos;
    i.sources_[0]=sindex.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::sin(CInstructionIndex sindex)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Sin;
    i.sources_[0]=sindex.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::tan(CInstructionIndex sindex)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Tan;
    i.sources_[0]=sindex.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::acos(CInstructionIndex sindex)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ACos;
    i.sources_[0]=sindex.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::asin(CInstructionIndex sindex)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ASin;
    i.sources_[0]=sindex.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::atan(CInstructionIndex sindex)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ATan;
    i.sources_[0]=sindex.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::tiers(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Tiers;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::smoothTiers(CInstructionIndex s1, CInstructionIndex s2)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_SmoothTiers;
    i.sources_[0]=s1.index_;
    i.sources_[1]=s2.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::scaleDomain(CInstructionIndex srcindex, CInstructionIndex scale)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ScaleDomain;
    i.sources_[0]=srcindex.index_;
    i.sources_[1]=scale.index_;
    kernel_.push_back(i);
    return lastIndex();
}


CInstructionIndex CKernel::scaleX(CInstructionIndex src, CInstructionIndex scale)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ScaleX;
    i.sources_[0]=src.index_;
    i.sources_[1]=scale.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::scaleY(CInstructionIndex src, CInstructionIndex scale)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ScaleY;
    i.sources_[0]=src.index_;
    i.sources_[1]=scale.index_;
    kernel_.push_back(i);
    return lastIndex();
}
CInstructionIndex CKernel::scaleZ(CInstructionIndex src, CInstructionIndex scale)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ScaleZ;
    i.sources_[0]=src.index_;
    i.sources_[1]=scale.index_;
    kernel_.push_back(i);
    return lastIndex();
}
CInstructionIndex CKernel::scaleW(CInstructionIndex src, CInstructionIndex scale)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ScaleW;
    i.sources_[0]=src.index_;
    i.sources_[1]=scale.index_;
    kernel_.push_back(i);
    return lastIndex();
}
CInstructionIndex CKernel::scaleU(CInstructionIndex src, CInstructionIndex scale)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ScaleU;
    i.sources_[0]=src.index_;
    i.sources_[1]=scale.index_;
    kernel_.push_back(i);
    return lastIndex();
}
CInstructionIndex CKernel::scaleV(CInstructionIndex src, CInstructionIndex scale)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_ScaleV;
    i.sources_[0]=src.index_;
    i.sources_[1]=scale.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::translateDomain(CInstructionIndex srcindex, CInstructionIndex trans)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_TranslateDomain;
    i.sources_[0]=srcindex.index_;
    i.sources_[1]=trans.index_;
    kernel_.push_back(i);
    return lastIndex();
}


CInstructionIndex CKernel::translateX(CInstructionIndex src, CInstructionIndex trans)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_TranslateX;
    i.sources_[0]=src.index_;
    i.sources_[1]=trans.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::translateY(CInstructionIndex src, CInstructionIndex trans)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_TranslateY;
    i.sources_[0]=src.index_;
    i.sources_[1]=trans.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::translateZ(CInstructionIndex src, CInstructionIndex trans)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_TranslateZ;
    i.sources_[0]=src.index_;
    i.sources_[1]=trans.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::translateW(CInstructionIndex src, CInstructionIndex trans)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_TranslateW;
    i.sources_[0]=src.index_;
    i.sources_[1]=trans.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::translateU(CInstructionIndex src, CInstructionIndex trans)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_TranslateU;
    i.sources_[0]=src.index_;
    i.sources_[1]=trans.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::translateV(CInstructionIndex src, CInstructionIndex trans)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_TranslateV;
    i.sources_[0]=src.index_;
    i.sources_[1]=trans.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::rotateDomain(CInstructionIndex src, CInstructionIndex angle, CInstructionIndex ax, CInstructionIndex ay, CInstructionIndex az)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_RotateDomain;
    i.sources_[0]=src.index_;
    i.sources_[1]=angle.index_;
    i.sources_[2]=ax.index_;
    i.sources_[3]=ay.index_;
    i.sources_[4]=az.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::addSequence(CInstructionIndex baseindex, unsigned int number, unsigned int stride)
{
    if(number<=1) return lastIndex();

    CInstructionIndex s1=baseindex;
    CInstructionIndex s2=baseindex+stride;
    CInstructionIndex addstart=add(s1,s2);

    if(number==2)
    {
        return lastIndex();
    }

    s1=addstart;
    s2+=stride;
    for(unsigned int c=0; c<number-2; ++c)
    {
        add(s1,s2);
        ++s1;
        s2+=stride;
    }

    return lastIndex();
}

CInstructionIndex CKernel::multiplySequence(CInstructionIndex baseindex, unsigned int number, unsigned int stride)
{
    if(number<=1) return lastIndex();

    CInstructionIndex s1=baseindex;
    CInstructionIndex s2=baseindex+stride;

    CInstructionIndex addstart=multiply(s1,s2);

    if(number==2)
    {
        return lastIndex();
    }

    s1=addstart;
    s2+=stride;
    for(unsigned int c=0; c<number-2; ++c)
    {
        multiply(s1,s2);
        ++s1;
        s2+=stride;
    }

    return lastIndex();
}

CInstructionIndex CKernel::maxSequence(CInstructionIndex baseindex, unsigned int number, unsigned int stride)
{
    if(number<=1) return lastIndex();

    CInstructionIndex s1=baseindex;
    CInstructionIndex s2=baseindex+stride;

    CInstructionIndex addstart=maximum(s1,s2);

    if(number==2)
    {
        return lastIndex();
    }

    s1=addstart;
    s2+=stride;
    for(unsigned int c=0; c<number-2; ++c)
    {
        maximum(s1,s2);
        ++s1;
        s2+=stride;
    }

    return lastIndex();
}

CInstructionIndex CKernel::minSequence(CInstructionIndex baseindex, unsigned int number, unsigned int stride)
{
    if(number<=1) return lastIndex();

    CInstructionIndex s1=baseindex;
    CInstructionIndex s2=baseindex+stride;

    CInstructionIndex addstart=minimum(s1,s2);

    if(number==2)
    {
        return lastIndex();
    }

    s1=addstart;
    s2+=stride;
    for(unsigned int c=0; c<number-2; ++c)
    {
        minimum(s1,s2);
        ++s1;
        s2+=stride;
    }

    return lastIndex();
}

CInstructionIndex CKernel::mix(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Blend;
    i.sources_[0]=low.index_;
    i.sources_[1]=high.index_;
    i.sources_[2]=control.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::select(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control, CInstructionIndex threshold, CInstructionIndex falloff)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Select;
    i.sources_[0]=low.index_;
    i.sources_[1]=high.index_;
    i.sources_[2]=control.index_;
    i.sources_[3]=threshold.index_;
    i.sources_[4]=falloff.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::step(CInstructionIndex val, CInstructionIndex control)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Step;
    i.sources_[0]=val.index_;
    i.sources_[1]=control.index_;
    kernel_.push_back(i);
    return lastIndex();
}
CInstructionIndex CKernel::linearStep(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_LinearStep;
    i.sources_[0]=low.index_;
    i.sources_[1]=high.index_;
    i.sources_[2]=control.index_;
    kernel_.push_back(i);
    return lastIndex();
}
CInstructionIndex CKernel::smoothStep(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_SmoothStep;
    i.sources_[0]=low.index_;
    i.sources_[1]=high.index_;
    i.sources_[2]=control.index_;
    kernel_.push_back(i);
    return lastIndex();
}
CInstructionIndex CKernel::smootherStep(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_SmootherStep;
    i.sources_[0]=low.index_;
    i.sources_[1]=high.index_;
    i.sources_[2]=control.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::curveSection(CInstructionIndex lowv, CInstructionIndex t0, CInstructionIndex t1, CInstructionIndex v0, CInstructionIndex v1, CInstructionIndex control)
{
	anl::SInstruction i;
	i.opcode_=anl::OP_CurveSection;
	i.sources_[0]=lowv.index_;
	i.sources_[1]=t0.index_;
	i.sources_[2]=t1.index_;
	i.sources_[3]=v0.index_;
	i.sources_[4]=v1.index_;
	i.sources_[5]=control.index_;
	kernel_.push_back(i);
	return lastIndex();
}

CInstructionIndex CKernel::simpleFractalLayer(unsigned int basistype, CInstructionIndex interpindex, double layerscale, double layerfreq, unsigned int s, bool rot,
        double angle, double ax, double ay, double az)
{
	CInstructionIndex myseed=seed(s);
    CInstructionIndex base=nextIndex();
    switch(basistype)
    {
    case anl::OP_ValueBasis:
        valueBasis(interpindex, myseed);
        break;
    case anl::OP_GradientBasis:
        gradientBasis(interpindex, myseed);
        break;
    case anl::OP_SimplexBasis:
        simplexBasis(myseed);
        break;
    default:
        gradientBasis(interpindex, myseed);
        break;
    }
    constant(layerscale);
    multiply(base,base+1);
    constant(layerfreq);
    CInstructionIndex sd=scaleDomain(base+2, lastIndex());
    if(rot)
    {
        double len=std::sqrt(ax*ax+ay*ay+az*az);
        constant(angle);
        constant(ax/len);
        constant(ay/len);
        constant(az/len);
        rotateDomain(sd, sd+1, sd+2, sd+3, sd+4);
    }
    return lastIndex();
}

CInstructionIndex CKernel::simpleRidgedLayer(unsigned int basistype, CInstructionIndex interpindex, double layerscale, double layerfreq, unsigned int s, bool rot,
        double angle, double ax, double ay, double az)
{
	CInstructionIndex myseed=seed(s);
    CInstructionIndex base=nextIndex();
    switch(basistype)
    {
    case anl::OP_ValueBasis:
        valueBasis(interpindex, myseed);
        break;
    case anl::OP_GradientBasis:
        gradientBasis(interpindex, myseed);
        break;
    case anl::OP_SimplexBasis:
        simplexBasis(myseed);
        break;
    default:
        gradientBasis(interpindex, myseed);
        break;
    }
    base=abs(base);
    constant(1.0);
    base=subtract(lastIndex(), base);
    constant(layerscale);
    multiply(base,base+1);
    constant(layerfreq);
    CInstructionIndex sd=scaleDomain(base+2, lastIndex());
    if(rot)
    {
        double len=std::sqrt(ax*ax+ay*ay+az*az);
        constant(angle);
        constant(ax/len);
        constant(ay/len);
        constant(az/len);
        rotateDomain(sd, sd+1, sd+2, sd+3, sd+4);
    }
    return lastIndex();
}

CInstructionIndex CKernel::simpleBillowLayer(unsigned int basistype, CInstructionIndex interpindex, double layerscale, double layerfreq, unsigned int s, bool rot,
        double angle, double ax, double ay, double az)
{
	CInstructionIndex myseed=seed(s);
    CInstructionIndex base=nextIndex();
    switch(basistype)
    {
    case anl::OP_ValueBasis:
        valueBasis(interpindex, myseed);
        break;
    case anl::OP_GradientBasis:
        gradientBasis(interpindex, myseed);
        break;
    case anl::OP_SimplexBasis:
        simplexBasis(myseed);
        break;
    default:
        gradientBasis(interpindex, myseed);
        break;
    }
    base=abs(base);
    base=multiply(base,constant(2.0));
    base=subtract(base,one());

    constant(layerscale);
    multiply(base,base+1);
    constant(layerfreq);
    CInstructionIndex sd=scaleDomain(base+2, lastIndex());
    if(rot)
    {
        double len=std::sqrt(ax*ax+ay*ay+az*az);
        constant(angle);
        constant(ax/len);
        constant(ay/len);
        constant(az/len);
        rotateDomain(sd, sd+1, sd+2, sd+3, sd+4);
    }
    return lastIndex();
}

CInstructionIndex CKernel::simpleRidgedMultifractal(unsigned int basistype, unsigned int interptype, unsigned int numoctaves, double frequency, unsigned int seed, bool rot)
{
    if(numoctaves<1) return 0;

    CInstructionIndex interpindex=constant(interptype);
    KISS rnd;
    rnd.setSeed(seed);
    simpleRidgedLayer(basistype, interpindex, 1.0, 1.0*frequency, seed+10,rot,
                      rnd.get01()*3.14159265, rnd.get01(), rnd.get01(), rnd.get01());
    CInstructionIndex lastlayer=lastIndex();

    for(unsigned int c=0; c<numoctaves-1; ++c)
    {
        CInstructionIndex nextlayer=simpleRidgedLayer(basistype, interpindex, 1.0/std::pow(2.0, (double)(c)), std::pow(2.0, (double)(c))*frequency, seed+10+c*1000,rot,
                                    rnd.get01()*3.14159265, rnd.get01(), rnd.get01(), rnd.get01());
        lastlayer=add(lastlayer,nextlayer);
    }
    return lastIndex();
}

CInstructionIndex CKernel::simplefBm(unsigned int basistype, unsigned int interptype, unsigned int numoctaves, double frequency, unsigned int seed, bool rot)
{
    if(numoctaves<1) return 0;

    CInstructionIndex interpindex=constant(interptype);
    KISS rnd;
    rnd.setSeed(seed);
    simpleFractalLayer(basistype, interpindex, 1.0, 1.0*frequency, seed+10,rot,
                       rnd.get01()*3.14159265, rnd.get01(), rnd.get01(), rnd.get01());
    CInstructionIndex lastlayer=lastIndex();

    for(unsigned int c=0; c<numoctaves-1; ++c)
    {
        CInstructionIndex nextlayer=simpleFractalLayer(basistype, interpindex, 1.0/std::pow(2.0, (double)(c)), std::pow(2.0, (double)(c))*frequency, seed+10+c*1000,rot,
                                    rnd.get01()*3.14159265, rnd.get01(), rnd.get01(), rnd.get01());
        lastlayer=add(lastlayer,nextlayer);
    }
    return lastIndex();
}

CInstructionIndex CKernel::simpleBillow(unsigned int basistype, unsigned int interptype, unsigned int numoctaves, double frequency, unsigned int seed, bool rot)
{
    if(numoctaves<1) return 0;

    CInstructionIndex interpindex=constant(interptype);
    KISS rnd;
    rnd.setSeed(seed);
    simpleBillowLayer(basistype, interpindex, 1.0, 1.0*frequency, seed+10,rot,
                      rnd.get01()*3.14159265, rnd.get01(), rnd.get01(), rnd.get01());
    CInstructionIndex lastlayer=lastIndex();

    for(unsigned int c=0; c<numoctaves-1; ++c)
    {
        CInstructionIndex nextlayer=simpleBillowLayer(basistype, interpindex, 1.0/std::pow(2.0, (double)(c)), std::pow(2.0, (double)(c))*frequency, seed+10+c*1000,rot,
                                    rnd.get01()*3.14159265, rnd.get01(), rnd.get01(), rnd.get01());
        lastlayer=add(lastlayer,nextlayer);
    }
    return lastIndex();
}

CInstructionIndex CKernel::x()
{
    anl::SInstruction i;
    i.opcode_=anl::OP_X;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::y()
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Y;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::z()
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Z;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::w()
{
    anl::SInstruction i;
    i.opcode_=anl::OP_W;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::u()
{
    anl::SInstruction i;
    i.opcode_=anl::OP_U;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::v()
{
    anl::SInstruction i;
    i.opcode_=anl::OP_V;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::dx(CInstructionIndex src, CInstructionIndex spacing)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_DX;
    i.sources_[0]=src.index_;
    i.sources_[1]=spacing.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::dy(CInstructionIndex src, CInstructionIndex spacing)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_DY;
    i.sources_[0]=src.index_;
    i.sources_[1]=spacing.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::dz(CInstructionIndex src, CInstructionIndex spacing)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_DZ;
    i.sources_[0]=src.index_;
    i.sources_[1]=spacing.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::dw(CInstructionIndex src, CInstructionIndex spacing)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_DW;
    i.sources_[0]=src.index_;
    i.sources_[1]=spacing.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::du(CInstructionIndex src, CInstructionIndex spacing)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_DU;
    i.sources_[0]=src.index_;
    i.sources_[1]=spacing.index_;
    kernel_.push_back(i);
    return lastIndex();
}
//
CInstructionIndex CKernel::dv(CInstructionIndex src, CInstructionIndex spacing)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_DV;
    i.sources_[0]=src.index_;
    i.sources_[1]=spacing.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::sigmoid(CInstructionIndex src)
{
    CInstructionIndex center=zero();
    CInstructionIndex ramp=one();
    return sigmoid(src,center,ramp);
}

CInstructionIndex CKernel::sigmoid(CInstructionIndex src, CInstructionIndex center, CInstructionIndex ramp)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Sigmoid;
    i.sources_[0]=src.index_;
    i.sources_[1]=center.index_;
    i.sources_[2]=ramp.index_;
    kernel_.push_back(i);
    return lastIndex();
}


CInstructionIndex CKernel::scaleOffset(CInstructionIndex src, double scale, double offset)
{
    CInstructionIndex c=constant(scale);
    CInstructionIndex o=constant(offset);
    CInstructionIndex m=multiply(src,c);
    add(m,o);
    return lastIndex();
}

CInstructionIndex CKernel::radial()
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Radial;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::clamp(CInstructionIndex src, CInstructionIndex low, CInstructionIndex high)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Clamp;
    i.sources_[0]=src.index_;
    i.sources_[1]=low.index_;
    i.sources_[2]=high.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::color(SRGBA c)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Color;
    i.outrgba_=c;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::color(float r, float g, float b, float a)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_Color;
    i.outrgba_=SRGBA(r,g,b,a);
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::combineRGBA(CInstructionIndex r, CInstructionIndex g, CInstructionIndex b, CInstructionIndex a)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_CombineRGBA;
    i.sources_[0]=r.index_;
    i.sources_[1]=g.index_;
    i.sources_[2]=b.index_;
    i.sources_[3]=a.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::combineHSVA(CInstructionIndex r, CInstructionIndex g, CInstructionIndex b, CInstructionIndex a)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_CombineHSVA;
    i.sources_[0]=r.index_;
    i.sources_[1]=g.index_;
    i.sources_[2]=b.index_;
    i.sources_[3]=a.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::hexTile(CInstructionIndex seed)
{
    anl::SInstruction i;
    i.opcode_=anl::OP_HexTile;
    //i.seed_=seed;
    i.sources_[0]=seed.index_;
    kernel_.push_back(i);
    return lastIndex();
}

CInstructionIndex CKernel::hexBump()
{
    anl::SInstruction i;
    i.opcode_=anl::OP_HexBump;
    kernel_.push_back(i);
    return lastIndex();
}

void CKernel::setVar(const std::string name,double val)
{
    auto i=vars_.find(name);
    if(i==vars_.end())
    {
        vars_.insert(std::pair<std::string, CInstructionIndex>(name,constant(val)));
    }
    else
    {
        unsigned s=(*i).second.index_;
        if(s>=kernel_.size()) return;
        kernel_[s].outfloat_=val;
    }
}

CInstructionIndex CKernel::getVar(const std::string name)
{
    auto i=vars_.find(name);
    if(i==vars_.end())
    {
        return zero();
    }
    else
    {
        unsigned int s=(*i).second.index_;
        if(s>=kernel_.size()) return zero();
        return (*i).second;
    }
}
};
