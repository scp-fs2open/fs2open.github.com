#include <iostream>

CoordPair closest_point(double vx, double vy, double x, double  y)
{
    double len=std::sqrt(vx*vx+vy*vy);
    double u=(x*vx+y*vy)/len;
    CoordPair c;
    c.x=u*vx;
    c.y=u*vy;
    return c;
}

double deg_to_rad(double deg)
{
    return deg*(3.14159265/180.0);
}

double rad_to_deg(double rad)
{
    return rad*(180.0/3.14159265);
}

double hex_function(double x, double y)
{
    if(x==0 && y==0) return 1.0;

    double len=std::sqrt(x*x+y*y);
    double dx=x/len, dy=y/len;
    double angle_degrees=rad_to_deg(std::atan2(dy,dx));

    double angleincrement=60;
    double t=(angle_degrees/angleincrement);
    double a1=std::floor(t)*angleincrement;
    double a2=a1+angleincrement;

    double ax1=std::cos(deg_to_rad(a1));
    double ay1=std::sin(deg_to_rad(a1));
    double ax2=std::cos(deg_to_rad(a2));
    double ay2=std::sin(deg_to_rad(a2));

    CoordPair p1=closest_point(ax1,ay1,x,y);
    CoordPair p2=closest_point(ax2,ay2,x,y);

    double dist1=std::sqrt((x-p1.x)*(x-p1.x)+(y-p1.y)*(y-p1.y));
    double dist2=std::sqrt((x-p2.x)*(x-p2.x)+(y-p2.y)*(y-p2.y));

    if(dist1<dist2)
    {
        double d1=std::sqrt(p1.x*p1.x+p1.y*p1.y);
        return d1/0.86602540378443864676372317075294;
    }
    else
    {
        double d1=std::sqrt(p2.x*p2.x+p2.y*p2.y);
        return d1/0.86602540378443864676372317075294;
    }

}

namespace anl
{
//CNoiseExecutor::CNoiseExecutor(CKernel *kernel) : kernel_(kernel->getKernel()), evaluated_(kernel->getKernel()->size(), false), coordcache_(kernel->getKernel()->size()), cache_(kernel->getKernel()->size())
CNoiseExecutor::CNoiseExecutor(CKernel &kernel) : kernel_(kernel)
{
}

CNoiseExecutor::~CNoiseExecutor()
{

}

double CNoiseExecutor::evaluateScalar(double x, double y, CInstructionIndex idx)
{
    CCoordinate c(x,y);
    return evaluateAt(c,idx).outfloat_;
}

double CNoiseExecutor::evaluateScalar(double x, double y, double z, CInstructionIndex idx)
{
    CCoordinate c(x,y,z);
    return evaluateAt(c,idx).outfloat_;
}

double CNoiseExecutor::evaluateScalar(double x, double y, double z, double w, CInstructionIndex idx)
{
    CCoordinate c(x,y,z,w);
    return evaluateAt(c,idx).outfloat_;
}

double CNoiseExecutor::evaluateScalar(double x, double y, double z, double w, double u, double v, CInstructionIndex idx)
{
    CCoordinate c(x,y,z,w,u,v);
    return evaluateAt(c,idx).outfloat_;
}


SRGBA CNoiseExecutor::evaluateColor(double x, double y, CInstructionIndex idx)
{
    CCoordinate c(x,y);
    return evaluateAt(c,idx).outrgba_;
}

SRGBA CNoiseExecutor::evaluateColor(double x, double y, double z, CInstructionIndex idx)
{
    CCoordinate c(x,y);
    return evaluateAt(c,idx).outrgba_;
}

SRGBA CNoiseExecutor::evaluateColor(double x, double y, double z, double w, CInstructionIndex idx)
{
    CCoordinate c(x,y);
    return evaluateAt(c,idx).outrgba_;
}

SRGBA CNoiseExecutor::evaluateColor(double x, double y, double z, double w, double u, double v, CInstructionIndex idx)
{
    CCoordinate c(x,y);
    return evaluateAt(c,idx).outrgba_;
}

InstructionListType *CNoiseExecutor::prepare()
{
    InstructionListType *k=kernel_.getKernel();
    if(!k || k->size()==0) return 0;
    if(k->size() != evaluated_.size()) evaluated_.resize(k->size());
    if(k->size() != coordcache_.size()) coordcache_.resize(k->size());
    if(k->size() != cache_.size()) cache_.resize(k->size());

    // clear evaluated flags
    for(auto i=evaluated_.begin(); i!=evaluated_.end(); ++i) *i=false;

    return k;
}


SVMOutput CNoiseExecutor::evaluate(CCoordinate &coord)
{
    SVMOutput out;
    // Evaluate the last one to start the chain
    InstructionListType *k=prepare();
    if(!k) return out;

    evaluateInstruction(*k, evaluated_, coordcache_, cache_, k->size()-1, coord);

    return cache_[k->size()-1];
}

SVMOutput CNoiseExecutor::evaluateAt(CCoordinate &coord, CInstructionIndex index)
{
    SVMOutput out;
    // Evaluate the instruction at the specified index
    InstructionListType *k=prepare();
    if(!k) return out;

    evaluateInstruction(*k, evaluated_, coordcache_, cache_, index.index_, coord);
    return cache_[index.index_];
}

double CNoiseExecutor::evaluateParameter(InstructionListType &kernel, EvaluatedType &evaluated, CoordCacheType &coordcache, CacheType &cache, unsigned int index, CCoordinate &coord)
{
    if(index>=kernel.size()) return 0;

    evaluateInstruction(kernel, evaluated, coordcache, cache, index, coord);
    return cache[index].outfloat_;
}

SRGBA CNoiseExecutor::evaluateRGBA(InstructionListType &kernel, EvaluatedType &evaluated, CoordCacheType &coordcache, CacheType &cache, unsigned int index, CCoordinate &coord)
{
    if(index>=kernel.size()) return SRGBA();

    evaluateInstruction(kernel, evaluated, coordcache, cache, index, coord);
    //return kernel[index].outfloat_;
    return cache[index].outrgba_;
}

SVMOutput CNoiseExecutor::evaluateBoth(InstructionListType &kernel, EvaluatedType &evaluated, CoordCacheType &coordcache, CacheType &cache, unsigned int index, CCoordinate &coord)
{
    if(index>=kernel.size()) return SVMOutput(0);

    evaluateInstruction(kernel, evaluated, coordcache, cache, index, coord);
    //return kernel[index].outfloat_;
    return cache[index];
}

void CNoiseExecutor::seedSource(InstructionListType &kernel, EvaluatedType &evaluated, unsigned int index, unsigned int &seed)
{
	//std::cout << "Seed: " << seed << std::endl;
	SInstruction &i=kernel[index];
	evaluated[index]=false;
	if(i.opcode_==OP_Seed)
	{
		i.outfloat_=(float)seed++;
		return;
	}
	else
	{
		switch(i.opcode_)
		{
			case OP_NOP:
			case OP_Seed:
			case OP_Constant: return; break;
			case OP_Seeder: for(int c=0; c<2; ++c) seedSource(kernel, evaluated, i.sources_[c], seed); return; break;
			case OP_ValueBasis: seedSource(kernel, evaluated, i.sources_[1], seed); return; break;
			case OP_GradientBasis: seedSource(kernel, evaluated, i.sources_[1], seed); return; break;
			case OP_SimplexBasis: seedSource(kernel, evaluated, i.sources_[0], seed); return; break;
			case OP_CellularBasis: for(int c=0; c<10; ++c) seedSource(kernel, evaluated, i.sources_[c], seed); return; break;
			case OP_Add:
			case OP_Subtract:
			case OP_Multiply:
			case OP_Divide:
			case OP_ScaleDomain: 
			case OP_ScaleX:
			case OP_ScaleY:
			case OP_ScaleZ:
			case OP_ScaleW:
			case OP_ScaleU:
			case OP_ScaleV:
			case OP_TranslateDomain:
			case OP_TranslateX:
			case OP_TranslateY:
			case OP_TranslateZ:
			case OP_TranslateW:
			case OP_TranslateU:
			case OP_TranslateV: seedSource(kernel, evaluated, i.sources_[0], seed); seedSource(kernel, evaluated, i.sources_[1], seed); return; break;
			case OP_RotateDomain: for(int c=0; c<5; ++c) seedSource(kernel, evaluated, i.sources_[c], seed); return; break;
			case OP_Blend: for(int c=0; c<3; ++c) seedSource(kernel, evaluated, i.sources_[c], seed); return; break;
			case OP_Select: for(int c=0; c<5; ++c) seedSource(kernel, evaluated, i.sources_[c], seed); return; break;
			case OP_Min:
			case OP_Max: seedSource(kernel, evaluated, i.sources_[0], seed); seedSource(kernel, evaluated, i.sources_[1], seed); return; break;
			case OP_Abs: seedSource(kernel, evaluated, i.sources_[0], seed); return; break;
			case OP_Pow: seedSource(kernel, evaluated, i.sources_[0], seed); seedSource(kernel, evaluated, i.sources_[1], seed); return; break;
			case OP_Clamp: for(int c=0; c<3; ++c) seedSource(kernel, evaluated, i.sources_[c], seed); return; break;
			case OP_Radial: return; break;
			case OP_Sin:
			case OP_Cos:
			case OP_Tan:
			case OP_ASin:
			case OP_ACos:
			case OP_ATan: seedSource(kernel, evaluated, i.sources_[0], seed); return; break;
			case OP_Bias:
			case OP_Gain: 
			case OP_Tiers:
			case OP_SmoothTiers: seedSource(kernel, evaluated, i.sources_[0], seed); seedSource(kernel, evaluated, i.sources_[1], seed); return; break;
			case OP_X:
			case OP_Y:
			case OP_Z:
			case OP_W:
			case OP_U:
			case OP_V: return; break;
			case OP_DX:
			case OP_DY:
			case OP_DZ:
			case OP_DW:
			case OP_DU:
			case OP_DV: seedSource(kernel, evaluated, i.sources_[0], seed); seedSource(kernel, evaluated, i.sources_[1], seed); return; break;
			case OP_Sigmoid: for(int c=0; c<3; ++c) seedSource(kernel, evaluated, i.sources_[c], seed); return; break;
			case OP_Fractal: for(int c=0; c<6; ++c) seedSource(kernel,evaluated,i.sources_[c],seed); return; break;//i.outfloat_=(double)seed++; return; break;
			case OP_Randomize: 
			case OP_SmoothStep: 
			case OP_SmootherStep:
			case OP_LinearStep: for(int c=0; c<3; ++c) seedSource(kernel,evaluated,i.sources_[c],seed); return; break;
			case OP_Step: for(int c=0; c<2; ++c) seedSource(kernel,evaluated,i.sources_[c],seed); return; break;
			case OP_CurveSection: for(int c=0; c<6; ++c) seedSource(kernel,evaluated,i.sources_[c],seed); return; break;
			case OP_HexTile: seedSource(kernel, evaluated, i.sources_[0], seed); return; break;
			case OP_HexBump: return; break;
			case OP_Color: return; break;
			case OP_ExtractRed:
			case OP_ExtractGreen:
			case OP_ExtractBlue:
			case OP_ExtractAlpha:
			case OP_Grayscale: return; break;
			case OP_CombineRGBA:
			case OP_CombineHSVA: for(int c=0; c<4; ++c) seedSource(kernel, evaluated, i.sources_[c], seed); return; break;
			default: return; break;
		}
	}
}

void CNoiseExecutor::evaluateInstruction(InstructionListType &kernel, EvaluatedType &evaluated, CoordCacheType &coordcache, CacheType &cache, unsigned int index, CCoordinate &coord)
{
    if(index>=kernel.size()) return;
    SInstruction &i=kernel[index];

    if(evaluated[index]==true && coordcache[index]==coord) return;

    coordcache[index]=coord;

    switch(i.opcode_)
    {
    case OP_NOP:
    case OP_Seed:
    case OP_Constant:
        evaluated[index]=true;
        cache[index].set(i.outfloat_);
        return;
        break;
	case OP_Seeder:
	{
		// Need to iterate through source chain and set seeds based on current seed.
		unsigned int seed=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
		seedSource(kernel,evaluated,i.sources_[1],seed);
		evaluated[index]=true;
		SVMOutput s1;
        s1=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        cache[index].set(s1);
		return;
		break;
	}
		
    case OP_ValueBasis:
    {
        // Parameters
        // 0=Interpolation
        int interp=(int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        unsigned int seed=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        switch(coord.dimension_)
        {
        case 2:
            switch(interp)
            {
            case 0:
                cache[index].set(value_noise2D(coord.x_,coord.y_,seed,noInterp));
                break;
            case 1:
                cache[index].set(value_noise2D(coord.x_,coord.y_,seed,linearInterp));
                break;
            case 2:
                cache[index].set(value_noise2D(coord.x_,coord.y_,seed,hermiteInterp));
                break;
            default:
                cache[index].set(value_noise2D(coord.x_,coord.y_,seed,quinticInterp));
                break;
            };
            break;
        case 3:
            switch(interp)
            {
            case 0:
                cache[index].set(value_noise3D(coord.x_,coord.y_,coord.z_,seed,noInterp));
                break;
            case 1:
                cache[index].set(value_noise3D(coord.x_,coord.y_,coord.z_,seed,linearInterp));
                break;
            case 2:
                cache[index].set(value_noise3D(coord.x_,coord.y_,coord.z_,seed,hermiteInterp));
                break;
            default:
                cache[index].set(value_noise3D(coord.x_,coord.y_,coord.z_,seed,quinticInterp));
                break;
            };
            break;
        case 4:
            switch(interp)
            {
            case 0:
                cache[index].set(value_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,noInterp));
                break;
            case 1:
                cache[index].set(value_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,linearInterp));
                break;
            case 2:
                cache[index].set(value_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,hermiteInterp));
                break;
            default:
                cache[index].set(value_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,quinticInterp));
                break;
            };
            break;
        default:
            switch(interp)
            {
            case 0:
                cache[index].set(value_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,noInterp));
                break;
            case 1:
                cache[index].set(value_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,linearInterp));
                break;
            case 2:
                cache[index].set(value_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,hermiteInterp));
                break;
            default:
                cache[index].set(value_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,quinticInterp));
                break;
            };
            break;
        }

        evaluated[index]=true;
        return;
        break;
    }
    case OP_GradientBasis:
    {
        // Parameters
        // 0=Interpolation
        int interp=(int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        unsigned int seed=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        switch(coord.dimension_)
        {
        case 2:
            switch(interp)
            {
            case 0:
                cache[index].set(gradient_noise2D(coord.x_,coord.y_,seed,noInterp));
                break;
            case 1:
                cache[index].set(gradient_noise2D(coord.x_,coord.y_,seed,linearInterp));
                break;
            case 2:
                cache[index].set(gradient_noise2D(coord.x_,coord.y_,seed,hermiteInterp));
                break;
            default:
                cache[index].set(gradient_noise2D(coord.x_,coord.y_,seed,quinticInterp));
                break;
            };
            break;
        case 3:
            //std::cout << "(" << coord.x_ << "," << coord.y_ << "," << coord.z_ << std::endl;
            switch(interp)
            {
            case 0:
                cache[index].set(gradient_noise3D(coord.x_,coord.y_,coord.z_,seed,noInterp));
                break;
            case 1:
                cache[index].set(gradient_noise3D(coord.x_,coord.y_,coord.z_,seed,linearInterp));
                break;
            case 2:
                cache[index].set(gradient_noise3D(coord.x_,coord.y_,coord.z_,seed,hermiteInterp));
                break;
            default:
                cache[index].set(gradient_noise3D(coord.x_,coord.y_,coord.z_,seed,quinticInterp));
                break;
            };
            break;
        case 4:
            switch(interp)
            {
            case 0:
                cache[index].set(gradient_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,noInterp));
                break;
            case 1:
                cache[index].set(gradient_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,linearInterp));
                break;
            case 2:
                cache[index].set(gradient_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,hermiteInterp));
                break;
            default:
                cache[index].set(gradient_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,quinticInterp));
                break;
            };
            break;
        default:
            switch(interp)
            {
            case 0:
                cache[index].set(gradient_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,noInterp));
                break;
            case 1:
                cache[index].set(gradient_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,linearInterp));
                break;
            case 2:
                cache[index].set(gradient_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,hermiteInterp));
                break;
            default:
                cache[index].set(gradient_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,quinticInterp));
                break;
            };
            break;
        }
        evaluated[index]=true;
        return;
        break;
    }
    case OP_SimplexBasis:
    {
        // Parameters

        // Simplex noise isn't interpolated, so interp does nothing
        unsigned int seed=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        switch(coord.dimension_)
        {
        case 2:
            cache[index].set(simplex_noise2D(coord.x_,coord.y_,seed,noInterp));
            break;
        case 3:
            cache[index].set(simplex_noise3D(coord.x_,coord.y_,coord.z_,seed,noInterp));
            break;
        case 4:
            cache[index].set(simplex_noise4D(coord.x_,coord.y_,coord.z_,coord.w_,seed,noInterp));
            break;
        default:
            cache[index].set(simplex_noise6D(coord.x_,coord.y_,coord.z_,coord.w_,coord.u_,coord.v_,seed,noInterp));
            break;
        };
        evaluated[index]=true;
        return;
        break;
    }
    case OP_CellularBasis:
    {
        unsigned int dist=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double f1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double f2=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);
        double f3=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[3],coord);
        double f4=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[4],coord);
        double d1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[5],coord);
        double d2=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[6],coord);
        double d3=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[7],coord);
        double d4=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[8],coord);
        unsigned int seed=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[9],coord);
        double f[4], d[4];
        switch(coord.dimension_)
        {
        case 2:
            switch(dist)
            {
            case 0:
                cellular_function2D(coord.x_, coord.y_, seed, f, d, distEuclid2);
                break;
            case 1:
                cellular_function2D(coord.x_, coord.y_, seed, f, d, distManhattan2);
                break;
            case 2:
                cellular_function2D(coord.x_, coord.y_, seed, f, d, distGreatestAxis2);
                break;
            case 3:
                cellular_function2D(coord.x_, coord.y_, seed, f, d, distLeastAxis2);
                break;
            default:
                cellular_function2D(coord.x_, coord.y_, seed, f, d, distEuclid2);
                break;
            };
            break;
        case 3:
            switch(dist)
            {
            case 0:
                cellular_function3D(coord.x_, coord.y_, coord.z_, seed, f, d, distEuclid3);
                break;
            case 1:
                cellular_function3D(coord.x_, coord.y_, coord.z_, seed, f, d, distManhattan3);
                break;
            case 2:
                cellular_function3D(coord.x_, coord.y_, coord.z_, seed, f, d, distGreatestAxis3);
                break;
            case 3:
                cellular_function3D(coord.x_, coord.y_, coord.z_, seed, f, d, distLeastAxis3);
                break;
            default:
                cellular_function3D(coord.x_, coord.y_, coord.z_, seed, f, d, distEuclid3);
                break;
            };
            break;
        case 4:
            switch(dist)
            {
            case 0:
                cellular_function4D(coord.x_, coord.y_, coord.z_, coord.w_, seed, f, d, distEuclid4);
                break;
            case 1:
                cellular_function4D(coord.x_, coord.y_, coord.z_, coord.w_, seed, f, d, distManhattan4);
                break;
            case 2:
                cellular_function4D(coord.x_, coord.y_, coord.z_, coord.w_, seed, f, d, distGreatestAxis4);
                break;
            case 3:
                cellular_function4D(coord.x_, coord.y_, coord.z_, coord.w_, seed, f, d, distLeastAxis4);
                break;
            default:
                cellular_function4D(coord.x_, coord.y_, coord.z_, coord.w_, seed, f, d, distEuclid4);
                break;
            };
            break;
        default:
            switch(dist)
            {
            case 0:
                cellular_function6D(coord.x_, coord.y_, coord.z_, coord.w_, coord.u_, coord.v_, seed, f, d, distEuclid6);
                break;
            case 1:
                cellular_function6D(coord.x_, coord.y_, coord.z_, coord.w_, coord.u_, coord.v_, seed, f, d, distManhattan6);
                break;
            case 2:
                cellular_function6D(coord.x_, coord.y_, coord.z_, coord.w_, coord.u_, coord.v_, seed, f, d, distGreatestAxis6);
                break;
            case 3:
                cellular_function6D(coord.x_, coord.y_, coord.z_, coord.w_, coord.u_, coord.v_, seed, f, d, distLeastAxis6);
                break;
            default:
                cellular_function6D(coord.x_, coord.y_, coord.z_, coord.w_, coord.u_, coord.v_, seed, f, d, distEuclid6);
                break;
            };
            break;
        };

        cache[index].set(f1*f[0]+f2*f[1]+f3*f[2]+f4*f[3]+d1*d[0]+d2*d[1]+d3*d[2]+d4*d[3]);
        return;
        break;
    }
    case OP_Add:
    {
        SVMOutput s1, s2;
        s1=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        s2=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        cache[index].set(s1+s2);
        evaluated[index]=true;
        return;
        break;
    }
    case OP_Subtract:
    {
        SVMOutput s1, s2;
        s1=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        s2=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        cache[index].set(s1-s2);
        evaluated[index]=true;
        return;
    }
    break;
    case OP_Multiply:
    {
        SVMOutput s1, s2;
        s1=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        s2=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[1],coord);

        cache[index].set(s1*s2);
        evaluated[index]=true;
        return;
        break;
    }
    case OP_Divide:
    {
        SVMOutput s1, s2;
        s1=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        s2=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        cache[index].set(s1/s2);
        evaluated[index]=true;
        return;
        break;
    }
    break;
    case OP_Bias:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double s2=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        s1=std::max(0.0,std::min(1.0,s1));
        s2=std::max(0.0,std::min(1.0,s2));
        cache[index].set(bias(s1,s2));
        evaluated[index]=true;
        return;
        break;
    }
    break;
    case OP_Gain:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double s2=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        s1=std::max(0.0,std::min(1.0,s1));
        s2=std::max(0.0,std::min(1.0,s2));
        cache[index].set(gain(s1,s2));
        evaluated[index]=true;
        return;
        break;
    }
    break;
    case OP_Max:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double s2=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        cache[index].set(std::max(s1,s2));
        evaluated[index]=true;
        return;
        break;
    }
    case OP_Min:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double s2=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        cache[index].set(std::min(s1,s2));
        evaluated[index]=true;
        return;
        break;
    }
    case OP_Abs:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        cache[index].set(std::abs(s1));
        evaluated[index]=true;
        return;
        break;
    }
    case OP_Pow:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double s2=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        evaluated[index]=true;
        cache[index].set(std::pow(s1,s2));
        return;
        break;
    }
    case OP_Cos:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        evaluated[index]=true;
        cache[index].set(std::cos(s1));
        return;
    }
    break;
    case OP_Sin:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        evaluated[index]=true;
        cache[index].set(std::sin(s1));
        return;
    }
    break;
    case OP_Tan:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        evaluated[index]=true;
        cache[index].set(std::tan(s1));
        return;
    }
    break;
    case OP_ACos:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        evaluated[index]=true;
        cache[index].set(std::acos(s1));
        return;
    }
    break;
    case OP_ASin:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        evaluated[index]=true;
        cache[index].set(std::asin(s1));
        return;
    }
    break;
    case OP_ATan:
    {
        double s1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        evaluated[index]=true;
        cache[index].set(std::atan(s1));
        return;
    }
    break;
    case OP_Tiers:
    {
        int numsteps=(int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double Tb=std::floor(val*(double)numsteps);
        evaluated[index]=true;
        cache[index].set(Tb/(double)numsteps);
    }
    break;
    case OP_SmoothTiers:
    {
        int numsteps=(int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord)-1;
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double Tb=std::floor(val*(double)numsteps);
        double Tt=Tb+1.0;
        double t=quintic_blend(val*(double)numsteps-Tb);
        Tb/=(double)numsteps;
        Tt/=(double)numsteps;
        evaluated[index]=true;
        cache[index].set(Tb+t*(Tt-Tb));
    }
    break;
    case OP_ScaleDomain:
    {
        CCoordinate scale(1,1,1,1,1,1);
        double sc=evaluateParameter(kernel, evaluated, coordcache, cache, i.sources_[1], coord);
        switch(coord.dimension_)
        {
        case 2:
        {
            scale.set(sc,sc);
            break;
        }
        case 3:
        {
            scale.set(sc,sc,sc);
            break;
        }
        case 4:
        {
            scale.set(sc,sc,sc,sc);
            break;
        }
        default:
        {
            scale.set(sc,sc,sc,sc,sc,sc);
            break;
        }
        };

        CCoordinate c=coord*scale;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], c));
        evaluated[index]=true;
        return;
        break;
    }
    case OP_ScaleX:
    {
        double s=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate scale(s,1,1,1,1,1);
        CCoordinate c=coord*scale;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache, cache,i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_ScaleY:
    {
        double s=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate scale(1,s,1,1,1,1);
        CCoordinate c=coord*scale;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_ScaleZ:
    {
        double s=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[1], coord);
        CCoordinate scale(1,1,s,1,1,1);
        CCoordinate c=coord*scale;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache, cache,i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_ScaleW:
    {
        double s=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate scale(1,1,1,s,1,1);
        CCoordinate c=coord*scale;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_ScaleU:
    {
        double s=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[1], coord);
        CCoordinate scale(1,1,1,1,s,1);
        CCoordinate c=coord*scale;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_ScaleV:
    {
        double s=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[1], coord);
        CCoordinate scale(1,1,1,1,1,s);
        CCoordinate c=coord*scale;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_TranslateX:
    {
        double t=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate trans(t,0,0,0,0,0);
        CCoordinate c=coord+trans;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_TranslateY:
    {
        double t=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate trans(0,t,0,0,0,0);
        CCoordinate c=coord+trans;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache, cache,i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_TranslateZ:
    {
        double t=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate trans(0,0,t,0,0,0);
        CCoordinate c=coord+trans;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache, cache,i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_TranslateW:
    {
        double t=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate trans(0,0,0,t,0,0);
        CCoordinate c=coord+trans;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache, cache,i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_TranslateU:
    {
        double t=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate trans(0,0,0,0,t,0);
        CCoordinate c=coord+trans;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache, cache,i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_TranslateV:
    {
        double t=evaluateParameter(kernel, evaluated, coordcache, cache,i.sources_[1], coord);
        CCoordinate trans(0,0,0,0,0,t);
        CCoordinate c=coord+trans;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache, cache,i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;


    case OP_TranslateDomain:
    {
        CCoordinate scale(1,1,1,1,1,1);
        double sc=evaluateParameter(kernel, evaluated, coordcache,cache,i.sources_[1],coord);
        switch(coord.dimension_)
        {
        case 2:
        {
            scale.set(sc,sc);
            break;
        }
        case 3:
        {
            scale.set(sc,sc,sc);
            break;
        }
        case 4:
        {
            scale.set(sc,sc,sc,sc);
            break;
        }
        default:
        {
            scale.set(sc,sc,sc,sc,sc,sc);
            break;
        }
        };

        CCoordinate c=coord+scale;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], c));
        evaluated[index]=true;
        return;
    }
    break;
    case OP_RotateDomain:
    {
        double angle=evaluateParameter(kernel, evaluated, coordcache, cache, i.sources_[1], coord);
        double ax=evaluateParameter(kernel, evaluated, coordcache, cache, i.sources_[2], coord);
        double ay=evaluateParameter(kernel, evaluated, coordcache, cache, i.sources_[3], coord);
        double az=evaluateParameter(kernel, evaluated, coordcache, cache, i.sources_[4], coord);

        double len=std::sqrt(ax*ax+ax*ay+az*az);
        ax/=len;
        ay/=len;
        az/=len;

        double cosangle=cos(angle);
        double sinangle=sin(angle);

        double rotmatrix[3][3];

        rotmatrix[0][0] = 1.0 + (1.0-cosangle)*(ax*ax-1.0);
        rotmatrix[1][0] = -az*sinangle+(1.0-cosangle)*ax*ay;
        rotmatrix[2][0] = ay*sinangle+(1.0-cosangle)*ax*az;

        rotmatrix[0][1] = az*sinangle+(1.0-cosangle)*ax*ay;
        rotmatrix[1][1] = 1.0 + (1.0-cosangle)*(ay*ay-1.0);
        rotmatrix[2][1] = -ax*sinangle+(1.0-cosangle)*ay*az;

        rotmatrix[0][2] = -ay*sinangle+(1.0-cosangle)*ax*az;
        rotmatrix[1][2] = ax*sinangle+(1.0-cosangle)*ay*az;
        rotmatrix[2][2] = 1.0 + (1.0-cosangle)*(az*az-1.0);

        double nx, ny, nz;
        nx = (rotmatrix[0][0]*coord.x_) + (rotmatrix[1][0]*coord.y_) + (rotmatrix[2][0]*coord.z_);
        ny = (rotmatrix[0][1]*coord.x_) + (rotmatrix[1][1]*coord.y_) + (rotmatrix[2][1]*coord.z_);
        nz = (rotmatrix[0][2]*coord.x_) + (rotmatrix[1][2]*coord.y_) + (rotmatrix[2][2]*coord.z_);
        CCoordinate newcoord=CCoordinate(nx,ny,nz,coord.w_,coord.u_,coord.v_);
        newcoord.dimension_=coord.dimension_;
        cache[index].set(evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], newcoord));
        evaluated[index]=true;
    }
    break;
    case OP_Blend:
    {
        SVMOutput low,high;
        double control;
        low=evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[0], coord);
        high=evaluateBoth(kernel, evaluated, coordcache,cache, i.sources_[1], coord);
        control=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[2], coord);
        //cache[index].set(low+(high-low)*control);
		cache[index].set(low*(1.0-control) + high*control);
        evaluated[index]=true;
        return;
    }
    break;
    case OP_Select:
    {
        evaluated[index]=true;
        double control,threshold,falloff;
        control=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);
        threshold=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[3],coord);
        falloff=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[4],coord);

        if(falloff>0.0)
        {
            if(control<(threshold-falloff))
            {
                SVMOutput low;
                low=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
                cache[index].set(low);
            }
            else if(control>(threshold+falloff))
            {
                SVMOutput high;
                high=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
                cache[index].set(high);
            }
            else
            {
                SVMOutput low, high;
                low=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
                high=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
                double lower=threshold-falloff;
                double upper=threshold+falloff;
                double blend=quintic_blend((control-lower)/(upper-lower));
                //cache[index].set(lerp(blend,low,high));
                cache[index].set(low+(high-low)*blend);
            }
        }
        else
        {
            if (control<threshold)
            {
                SVMOutput low;
                low=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
                cache[index].set(low);
            }
            else
            {
                SVMOutput high;
                high=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
                cache[index].set(high);
            }
        }
    }
    break;

    case OP_X:
    {
        cache[index].set(coord.x_);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_Y:
    {
        cache[index].set(coord.y_);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_Z:
    {
        cache[index].set(coord.z_);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_W:
    {
        cache[index].set(coord.w_);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_U:
    {
        cache[index].set(coord.u_);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_V:
    {
        cache[index].set(coord.v_);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_DX:
    {
        double spacing=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        CCoordinate newcoord(coord.x_+spacing, coord.y_, coord.z_, coord.w_, coord.u_, coord.v_);
        newcoord.dimension_=coord.dimension_;
        double v1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],newcoord);
        cache[index].set((val-v1)/spacing);
        evaluated[index]=true;
        return;
    }
    break;
    case OP_DY:
    {
        double spacing=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        CCoordinate newcoord(coord.x_, coord.y_+spacing, coord.z_, coord.w_, coord.u_, coord.v_);
        newcoord.dimension_=coord.dimension_;
        double v1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],newcoord);
        cache[index].set((val-v1)/spacing);
        evaluated[index]=true;
        return;
    }
    break;
    case OP_DZ:
    {
        double spacing=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        CCoordinate newcoord(coord.x_, coord.y_, coord.z_+spacing, coord.w_, coord.u_, coord.v_);
        newcoord.dimension_=coord.dimension_;
        double v1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],newcoord);
        cache[index].set((val-v1)/spacing);
        evaluated[index]=true;
        return;
    }
    break;
    case OP_DW:
    {
        double spacing=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        CCoordinate newcoord(coord.x_, coord.y_, coord.z_, coord.w_+spacing, coord.u_, coord.v_);
        newcoord.dimension_=coord.dimension_;
        double v1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],newcoord);
        cache[index].set((val-v1)/spacing);
        evaluated[index]=true;
        return;
    }
    break;
    case OP_DU:
    {
        double spacing=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        CCoordinate newcoord(coord.x_, coord.y_, coord.z_, coord.w_, coord.u_+spacing, coord.v_);
        newcoord.dimension_=coord.dimension_;
        double v1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],newcoord);
        cache[index].set((val-v1)/spacing);
        evaluated[index]=true;
        return;
    }
    break;
    case OP_DV:
    {
        double spacing=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        CCoordinate newcoord(coord.x_, coord.y_, coord.z_, coord.w_, coord.u_, coord.v_+spacing);
        newcoord.dimension_=coord.dimension_;
        double v1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],newcoord);
        cache[index].set((val-v1)/spacing);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_Sigmoid:
    {
        double s=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double c=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double r=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);
        cache[index].set(1.0 / (1.0 + std::exp(-r*(s-c))));
        evaluated[index]=true;
        return;
    }
    break;
	
	case OP_Fractal:
	{
		//layer,pers,lac,octaves
		unsigned int numoctaves=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[4],coord);
		unsigned int seed=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
		double val=0.0f;
		double freq=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[5],coord);
		double lac=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[3],coord);
		double pers=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);
		double amp=1.0;
		CCoordinate mycoord=coord;
		mycoord=mycoord*freq;
		
		for(unsigned int c=0; c<numoctaves; ++c)
		{
			seedSource(kernel,evaluated,i.sources_[1],seed);
			double v=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],mycoord);
			val+=v*amp;
			//val=v;
			amp*=pers;
			mycoord=mycoord*lac;
		}
		cache[index].set(val);
		evaluated[index]=true;
		return;
	}
	
	case OP_Randomize:  // Randomize a value range based on seed
	{
		unsigned int seed=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
		KISS rnd;
		rnd.setSeed(seed);
		double low=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
		double high=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);
		cache[index].set(low+rnd.get01()*(high-low));
		evaluated[index]=true;
		return;
	}
	
	case OP_SmoothStep:
	{
		double low,high;
        double control;
        low=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[0], coord);
        high=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[1], coord);
        control=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[2], coord);
		double t=std::min(1.0, std::max(0.0, (control-low)/(high-low)));
		t=t*t*(3.0-2.0*t);
        cache[index].set(t);
		evaluated[index]=true;
		return;
	}
	
	case OP_SmootherStep:
	{
		double low,high;
        double control;
        low=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[0], coord);
        high=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[1], coord);
        control=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[2], coord);
		double t=std::min(1.0, std::max(0.0, (control-low)/(high-low)));
		t=t*t*t*(t*(t*6 - 15) + 10);
        cache[index].set(t);
		evaluated[index]=true;
		return;
	}
	
	case OP_LinearStep:
	{
		double low,high;
        double control;
        low=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[0], coord);
        high=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[1], coord);
        control=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[2], coord);
		double t=(control-low)/(high-low);
		cache[index].set(std::min(1.0, std::max(0.0, t)));
		evaluated[index]=true;
		return;
	}
	
	case OP_Step:
	{
		double val;
        double control;
        val=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[0], coord);
        control=evaluateParameter(kernel, evaluated, coordcache,cache, i.sources_[1], coord);
        cache[index].set(control<val ? 0.0 : 1.0);
		evaluated[index]=true;
		return;
	}
	
	case OP_CurveSection:
	{
		SVMOutput lowv=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
		double control=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[5],coord);
		double t0=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
		double t1=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);
		
		if(control<t0)
		{
			cache[index].set(lowv);
			evaluated[index]=true;
		}
		else
		{
			double interp=(control-t0)/(t1-t0);
			interp=interp*interp*interp*(interp*(interp*6.0-15.0)+10.0);
			interp=std::min(1.0, std::max(0.0, interp));
			SVMOutput v0=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[3],coord);
			SVMOutput v1=evaluateBoth(kernel,evaluated,coordcache,cache,i.sources_[4],coord);
		
			cache[index].set(v0 + (v1-v0)*interp);
			evaluated[index]=true;
		}
		return;
	}

    case OP_Radial:
    {
        double len=0;
        switch(coord.dimension_)
        {
        case 2:
            len=std::sqrt(coord.x_*coord.x_+coord.y_*coord.y_);
            break;
        case 3:
            len=std::sqrt(coord.x_*coord.x_+coord.y_*coord.y_+coord.z_*coord.z_);
            break;
        case 4:
            len=std::sqrt(coord.x_*coord.x_+coord.y_*coord.y_+coord.z_*coord.z_+coord.w_*coord.w_);
            break;
        default:
            len=std::sqrt(coord.x_*coord.x_+coord.y_*coord.y_+coord.z_*coord.z_+coord.w_*coord.w_+coord.u_*coord.u_+coord.v_*coord.v_);
            break;
        };
        cache[index].set(len);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_Clamp:
    {
        double val=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double low=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double high=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);

        cache[index].set(std::max(low,std::min(high,val)));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_HexTile:
    {
        TileCoord tile=calcHexPointTile(coord.x_, coord.y_);
        unsigned int seed=(unsigned int)evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        unsigned int hash=hash_coords_2(tile.x, tile.y, seed);
        cache[index].set((double)hash/255.0);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_HexBump:
    {
        TileCoord tile=calcHexPointTile(coord.x_, coord.y_);
        CoordPair center=calcHexTileCenter(tile.x, tile.y);
        double dx=coord.x_-center.x;
        double dy=coord.y_-center.y;
        cache[index].set(hex_function(dx,dy));
        evaluated[index]=true;
        return;
    }
    break;

    case OP_Color:
        evaluated[index]=true;
        cache[index].set(i.outrgba_);
        return;
        break;

    case OP_ExtractRed:
    {
        SRGBA c=evaluateRGBA(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        cache[index].set(c.r);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_ExtractGreen:
    {
        SRGBA c=evaluateRGBA(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        cache[index].set(c.g);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_ExtractBlue:
    {
        SRGBA c=evaluateRGBA(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        cache[index].set(c.b);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_ExtractAlpha:
    {
        SRGBA c=evaluateRGBA(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        cache[index].set(c.a);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_Grayscale:
    {
        double v=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        cache[index].set(v);
        evaluated[index]=true;
        return;
    }
    break;

    case OP_CombineRGBA:
    {
        double r=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double g=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double b=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);
        double a=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[3],coord);
        cache[index].set(SRGBA(r,g,b,a));
        evaluated[index]=true;
        return;
    }
    break;
	
	case OP_CombineHSVA:
    {
        double h=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[0],coord);
        double s=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[1],coord);
        double v=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[2],coord);
        double a=evaluateParameter(kernel,evaluated,coordcache,cache,i.sources_[3],coord);
		SRGBA col;
		
		double P,Q,T,fract;
		if (h>=360.0) h=0.0;
		else h=h/60.0;
		fract = h - std::floor(h);
		
		P = v*(1.0-s);
		Q = v*(1.0-s*fract);
		T = v*(1.0-s*(1.0-fract));
		
		if (h>=0 && h<1) col=SRGBA(v,T,P,1);
		else if (h>=1 && h<2) col=SRGBA(Q,v,P,a);
		else if (h>=2 && h<3) col=SRGBA(P,v,T,a);
		else if (h>=3 && h<4) col=SRGBA(P,Q,v,a);
		else if (h>=4 && h<5) col=SRGBA(T,P,v,a);
		else if (h>=5 && h<6) col=SRGBA(v,P,Q,a);
		else col=SRGBA(0,0,0,a);
	
        cache[index].set(col);
        evaluated[index]=true;
        return;
    }
    break;

    default:
        evaluated[index]=true;
        return;
        break;
    };
}

TileCoord CNoiseExecutor::calcHexPointTile(float px, float py)
{
    TileCoord tile;
    float rise=0.5f;
    float slope=rise/0.8660254f;
    int X=(int)(px/1.732051f);
    int Y=(int)(py/1.5f);

    float offsetX=px-(float)X*1.732051f;
    float offsetY=py-(float)Y*1.5f;

    if(fmod(Y, 2)==0)
    {
        if(offsetY < (-slope * offsetX+rise))
        {
            --X;
            --Y;
        }
        else if(offsetY < (slope*offsetX-rise))
        {
            --Y;
        }
    }
    else
    {
        if(offsetX >= 0.8660254f)
        {
            if(offsetY <(-slope*offsetX+rise*2))
            {
                --Y;
            }
        }
        else
        {
            if(offsetY<(slope*offsetX))
            {
                --Y;
            }
            else
            {
                --X;
            }
        }
    }
    tile.x=X;
    tile.y=Y;
    return tile;
}

CoordPair CNoiseExecutor::calcHexTileCenter(int tx, int ty)
{
    CoordPair origin;
    float ymod=fmod(ty, 2.0f);
    if(ymod==1.0f) ymod=0.8660254;
    else ymod=0.0f;
    origin.x=(float)tx*1.732051+ymod;
    origin.y=(float)ty*1.5;
    CoordPair center;
    center.x=origin.x+0.8660254;
    center.y=origin.y+1.0;
    //std::cout << "Hex Coords: " << tx << ", " << ty << "  Center: " << center[0] << ", " << center[1] << std::endl;
    return center;
}
};
