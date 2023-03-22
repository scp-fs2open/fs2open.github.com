#ifndef VM_H
#define VM_H

#include "instruction.h"
#include "coordinate.h"
#include "noise_gen.h"

#include <vector>
#include "../templates/tarray1.h"
#include "../vectortypes.h"

struct TileCoord
{
    unsigned int x,y;
};

struct CoordPair
{
    double x,y;
};

namespace anl
{
struct SVMOutput
{
    float outfloat_;
    SRGBA outrgba_;

    SVMOutput() : outfloat_(0), outrgba_(0,0,0,0)
    {

    }

    SVMOutput(float v) : outfloat_(v), outrgba_(v,v,v,1)
    {
    }

    SVMOutput(float v, SRGBA rgba)
    {
        outfloat_=v;
        outrgba_=rgba;
    }

    SVMOutput(const SVMOutput &rhs) : outfloat_(rhs.outfloat_), outrgba_(rhs.outrgba_)
    {
    }

    void set(float v)
    {
        outfloat_=v;
        outrgba_.r=outrgba_.g=outrgba_.b=v;
        outrgba_.a=1;
    }

    void set(SRGBA v)
    {
        outrgba_=v;
        outfloat_=0.2126f*v.r + 0.7152f*v.g + 0.0722f*v.b;
    }

    SVMOutput operator-(const SVMOutput &rhs) const
    {
        return SVMOutput(outfloat_-rhs.outfloat_, outrgba_-rhs.outrgba_);
    }

    SVMOutput operator+(const SVMOutput &rhs) const
    {
        return SVMOutput(outfloat_+rhs.outfloat_, outrgba_+rhs.outrgba_);
    }

    SVMOutput operator*(const SVMOutput &rhs) const
    {
        return SVMOutput(outfloat_*rhs.outfloat_, outrgba_*rhs.outrgba_);
    }

    SVMOutput operator/(const SVMOutput &rhs) const
    {
        return SVMOutput(outfloat_/rhs.outfloat_, outrgba_/rhs.outrgba_);
    }

    SVMOutput operator*(float rhs) const
    {
        return SVMOutput(outfloat_*rhs, outrgba_*rhs);
    }


    void set(const SVMOutput &rhs)
    {
        outfloat_=rhs.outfloat_;
        outrgba_=rhs.outrgba_;
    }
};

typedef std::vector<SInstruction> InstructionListType;
typedef std::vector<bool> EvaluatedType;
typedef std::vector<CCoordinate> CoordCacheType;
typedef std::vector<SVMOutput> CacheType;

class CNoiseExecutor
{
public:
    CNoiseExecutor(CKernel &kernel);
    ~CNoiseExecutor();

    SVMOutput evaluate(CCoordinate &coord);
    SVMOutput evaluateAt(CCoordinate &coord, CInstructionIndex index);

    double evaluateScalar(double x, double y, CInstructionIndex idx);
    double evaluateScalar(double x, double y, double z, CInstructionIndex idx);
    double evaluateScalar(double x, double y, double z, double w, CInstructionIndex idx);
    double evaluateScalar(double x, double y, double z, double w, double u, double v, CInstructionIndex idx);

    SRGBA evaluateColor(double x, double y, CInstructionIndex idx);
    SRGBA evaluateColor(double x, double y, double z, CInstructionIndex idx);
    SRGBA evaluateColor(double x, double y, double z, double w, CInstructionIndex idx);
    SRGBA evaluateColor(double x, double y, double z, double w, double u, double v, CInstructionIndex idx);
private:
	void seedSource(InstructionListType &kernel, EvaluatedType &evaluated, unsigned int index, unsigned int &seed);
    void evaluateInstruction(InstructionListType &kernel, EvaluatedType &evaluated, CoordCacheType &coordcache, CacheType &cache, unsigned int index, CCoordinate &coord);
    double evaluateParameter(InstructionListType &kernel, EvaluatedType &evaluated, CoordCacheType &coordcache, CacheType &cache, unsigned int index, CCoordinate &coord);
    SVMOutput evaluateBoth(InstructionListType &kernel, EvaluatedType &evaluated, CoordCacheType &coordcache, CacheType &cache, unsigned int index, CCoordinate &coord);
    SRGBA evaluateRGBA(InstructionListType &kernel, EvaluatedType &evaluated, CoordCacheType &coordcache, CacheType &cache, unsigned int index, CCoordinate &coord);
    TileCoord calcHexPointTile(float px, float py);
    CoordPair calcHexTileCenter(int tx, int ty);
    InstructionListType *prepare();

    //InstructionListType *kernel_;
    CKernel &kernel_;
    EvaluatedType evaluated_;
    CoordCacheType coordcache_;
    CacheType cache_;

};
};


#endif
