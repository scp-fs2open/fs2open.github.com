#ifndef KERNEL_H
#define KERNEL_H

#include "instruction.h"
#include "vm.h"
#include <string>
#include <map>


namespace anl
{
enum InterpolationTypes
{
    INTERP_NONE,
    INTERP_LINEAR,
    INTERP_HERMITE,
    INTERP_QUINTIC
};

enum DistanceTypes
{
    DISTANCE_EUCLID,
    DISTANCE_MANHATTAN,
    DISTANCE_LEASTAXIS,
    DISTANCE_GREATESTAXIS
};

enum BasisTypes
{
    BASIS_VALUE,
    BASIS_GRADIENT,
    BASIS_SIMPLEX
};

class CKernel
{
public:
    CKernel();
	CKernel(const CKernel &rhs);
	
	

    CInstructionIndex pi();
    CInstructionIndex e();
    CInstructionIndex one();
    CInstructionIndex zero();
    CInstructionIndex point5();
    CInstructionIndex sqrt2();

    CInstructionIndex constant(double val);
    CInstructionIndex seed(unsigned int val);
	CInstructionIndex seeder(CInstructionIndex sd, CInstructionIndex src);
    CInstructionIndex valueBasis(CInstructionIndex interpindex, CInstructionIndex seed);
    CInstructionIndex gradientBasis(CInstructionIndex interpindex, CInstructionIndex seed);
    CInstructionIndex simplexBasis(CInstructionIndex seed);
    CInstructionIndex cellularBasis(CInstructionIndex f1, CInstructionIndex f2, CInstructionIndex f3, CInstructionIndex f4, CInstructionIndex d1, CInstructionIndex d2, CInstructionIndex d3, CInstructionIndex d4, CInstructionIndex dist, CInstructionIndex seed);
    CInstructionIndex add(CInstructionIndex s1index, CInstructionIndex s2index);
    CInstructionIndex subtract(CInstructionIndex s1, CInstructionIndex s2);
    CInstructionIndex multiply(CInstructionIndex s1index, CInstructionIndex s2index);
    CInstructionIndex divide(CInstructionIndex s1, CInstructionIndex s2);
    CInstructionIndex maximum(CInstructionIndex s1index, CInstructionIndex s2index);
    CInstructionIndex minimum(CInstructionIndex s1index, CInstructionIndex s2index);
    CInstructionIndex abs(CInstructionIndex sindex);
    CInstructionIndex pow(CInstructionIndex s1, CInstructionIndex s2);
    CInstructionIndex bias(CInstructionIndex s1, CInstructionIndex s2);
    CInstructionIndex gain(CInstructionIndex s1, CInstructionIndex s2);

    CInstructionIndex scaleDomain(CInstructionIndex srcindex, CInstructionIndex scale);

    CInstructionIndex scaleX(CInstructionIndex src, CInstructionIndex scale);
    CInstructionIndex scaleY(CInstructionIndex src, CInstructionIndex scale);
    CInstructionIndex scaleZ(CInstructionIndex src, CInstructionIndex scale);
    CInstructionIndex scaleW(CInstructionIndex src, CInstructionIndex scale);
    CInstructionIndex scaleU(CInstructionIndex src, CInstructionIndex scale);
    CInstructionIndex scaleV(CInstructionIndex src, CInstructionIndex scale);

    CInstructionIndex translateDomain(CInstructionIndex srcindex, CInstructionIndex trans);

    CInstructionIndex translateX(CInstructionIndex src, CInstructionIndex trans);
    CInstructionIndex translateY(CInstructionIndex src, CInstructionIndex trans);
    CInstructionIndex translateZ(CInstructionIndex src, CInstructionIndex trans);
    CInstructionIndex translateW(CInstructionIndex src, CInstructionIndex trans);
    CInstructionIndex translateU(CInstructionIndex src, CInstructionIndex trans);
    CInstructionIndex translateV(CInstructionIndex src, CInstructionIndex trans);

    CInstructionIndex rotateDomain(CInstructionIndex src, CInstructionIndex angle, CInstructionIndex ax, CInstructionIndex ay, CInstructionIndex az);

    CInstructionIndex addSequence(CInstructionIndex baseindex, unsigned int number, unsigned int stride);
    CInstructionIndex multiplySequence(CInstructionIndex baseindex, unsigned int number, unsigned int stride);
    CInstructionIndex maxSequence(CInstructionIndex baseindex, unsigned int number, unsigned int stride);
    CInstructionIndex minSequence(CInstructionIndex baseindex, unsigned int number, unsigned int stride);

    CInstructionIndex mix(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control);
    CInstructionIndex select(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control, CInstructionIndex threshold, CInstructionIndex falloff);
    CInstructionIndex clamp(CInstructionIndex src, CInstructionIndex low, CInstructionIndex high);

    CInstructionIndex cos(CInstructionIndex src);
    CInstructionIndex sin(CInstructionIndex src);
    CInstructionIndex tan(CInstructionIndex src);
    CInstructionIndex acos(CInstructionIndex src);
    CInstructionIndex asin(CInstructionIndex src);
    CInstructionIndex atan(CInstructionIndex src);

    CInstructionIndex tiers(CInstructionIndex src, CInstructionIndex numtiers);
    CInstructionIndex smoothTiers(CInstructionIndex src, CInstructionIndex numtiers);

    CInstructionIndex x();
    CInstructionIndex y();
    CInstructionIndex z();
    CInstructionIndex w();
    CInstructionIndex u();
    CInstructionIndex v();

    CInstructionIndex dx(CInstructionIndex src, CInstructionIndex spacing);
    CInstructionIndex dy(CInstructionIndex src, CInstructionIndex spacing);
    CInstructionIndex dz(CInstructionIndex src, CInstructionIndex spacing);
    CInstructionIndex dw(CInstructionIndex src, CInstructionIndex spacing);
    CInstructionIndex du(CInstructionIndex src, CInstructionIndex spacing);
    CInstructionIndex dv(CInstructionIndex src, CInstructionIndex spacing);

    CInstructionIndex sigmoid(CInstructionIndex src);
    CInstructionIndex sigmoid(CInstructionIndex src, CInstructionIndex center, CInstructionIndex ramp);

    CInstructionIndex radial();
	
	CInstructionIndex fractal(CInstructionIndex seed, CInstructionIndex layer, CInstructionIndex persistence, CInstructionIndex lacunarity, CInstructionIndex numoctaves, CInstructionIndex freq);
	CInstructionIndex randomize(CInstructionIndex seed, CInstructionIndex low, CInstructionIndex high);
	CInstructionIndex step(CInstructionIndex val, CInstructionIndex control);
	CInstructionIndex linearStep(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control);
	CInstructionIndex smoothStep(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control);
	CInstructionIndex smootherStep(CInstructionIndex low, CInstructionIndex high, CInstructionIndex control);
	
	CInstructionIndex curveSection(CInstructionIndex lowv, CInstructionIndex t0, CInstructionIndex t1, CInstructionIndex v0, CInstructionIndex v1, CInstructionIndex control);

    // Patterns
    CInstructionIndex hexTile(CInstructionIndex seed);
    CInstructionIndex hexBump();

    CInstructionIndex color(SRGBA c);
    CInstructionIndex color(float r, float g, float b, float a);

    CInstructionIndex combineRGBA(CInstructionIndex r, CInstructionIndex g, CInstructionIndex b, CInstructionIndex a);
	CInstructionIndex combineHSVA(CInstructionIndex h, CInstructionIndex s, CInstructionIndex v, CInstructionIndex a);

    CInstructionIndex scaleOffset(CInstructionIndex src, double scale, double offset);

    CInstructionIndex simpleFractalLayer(unsigned int basistype, CInstructionIndex interptypeindex, double layerscale, double layerfreq, unsigned int seed, bool rot=true,
                                         double angle=0.5, double ax=0, double ay=0, double az=1);
    CInstructionIndex simpleRidgedLayer(unsigned int basistype, CInstructionIndex interptypeindex, double layerscale, double layerfreq, unsigned int seed, bool rot=true,
                                        double angle=0.5, double ax=0, double ay=0, double az=1);
    CInstructionIndex simpleBillowLayer(unsigned int basistype, CInstructionIndex interptypeindex, double layerscale, double layerfreq, unsigned int seed, bool rot=true,
                                        double angle=0.5, double ax=0, double ay=0, double az=1);

    CInstructionIndex simplefBm(unsigned int basistype, unsigned int interptype, unsigned int numoctaves, double frequency, unsigned int seed, bool rot=true);
    CInstructionIndex simpleRidgedMultifractal(unsigned int basistype, unsigned int interptype, unsigned int numoctaves, double frequency, unsigned int seed, bool rot=true);
    CInstructionIndex simpleBillow(unsigned int basistype, unsigned int interptype, unsigned int numoctaves, double frequency, unsigned int seed, bool rot=true);

    InstructionListType *getKernel()
    {
        return &kernel_;
    }
    CInstructionIndex nextIndex()
    {
        return CInstructionIndex(static_cast<unsigned int>(kernel_.size()));
    }
    CInstructionIndex lastIndex()
    {
        return CInstructionIndex(static_cast<unsigned int>(kernel_.size()-1));
    }

    void setVar(const std::string name,double val);
    CInstructionIndex getVar(const std::string name);
	CKernel &operator =(const CKernel &in)
	{
		kernel_=in.kernel_;
		vars_=in.vars_;
		return *this;
	}
private:
    InstructionListType kernel_;

    CInstructionIndex pi_, e_, one_, zero_, point5_, sqrt2_;
    std::map<std::string, CInstructionIndex> vars_;
};

};

#endif
