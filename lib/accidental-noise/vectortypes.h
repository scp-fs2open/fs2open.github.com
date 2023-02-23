#ifndef COMMON_VECTOR_TYPES
#define COMMON_VECTOR_TYPES

namespace anl
{
//typedef rhs4D<float> SRGBA;

struct SRGBA
{
    SRGBA() : r(0), g(0), b(0), a(0) {}
    SRGBA(const  SRGBA &rhs) : r(rhs.r), g(rhs.g), b(rhs.b), a(rhs.a) {}
    SRGBA(const float t1, const float t2, const float t3, const float t4) : r(t1), g(t2), b(t3), a(t4) {}
    ~SRGBA() {}

    bool operator ==(const SRGBA &rhs) const
    {
        if(r==rhs.r && g==rhs.g && b==rhs.b && a==rhs.a) return true;
        return false;
    };

    bool operator !=(const SRGBA &rhs) const
    {
        if(r==rhs.r && g==rhs.g && b==rhs.b && a==rhs.a) return false;
        return true;
    };

    SRGBA operator +(const SRGBA &rhs) const
    {
        return SRGBA(r+rhs.r, g+rhs.g, b+rhs.b, a+rhs.a);
    };

    SRGBA operator -(const SRGBA &rhs) const
    {
        return SRGBA(r-rhs.r, g-rhs.g, b-rhs.b, a-rhs.a);
    };

    SRGBA operator *(const SRGBA &rhs) const
    {
        return SRGBA(r*rhs.r, g*rhs.g, b*rhs.b, a*rhs.a);
    };

    SRGBA operator /(const SRGBA &rhs) const
    {
        return SRGBA(r/rhs.r, g/rhs.g, b/rhs.b, a/rhs.a);
    };

    SRGBA operator -() const
    {
        return SRGBA(-r, -g, -b, -a);
    };

    SRGBA &operator +=(const SRGBA &rhs)
    {
        r += rhs.r;
        g += rhs.g;
        b += rhs.b;
        a += rhs.a;
        return (*this);
    };

    SRGBA &operator -=(const SRGBA &rhs)
    {
        r -= rhs.r;
        g -= rhs.g;
        b -= rhs.b;
        a -= rhs.a;
        return (*this);
    };

    SRGBA &operator *=(const SRGBA &rhs)
    {
        r *= rhs.r;
        g *= rhs.g;
        b *= rhs.b;
        a *= rhs.a;
        return (*this);
    };

    SRGBA &operator /=(const SRGBA &rhs)
    {
        r /= rhs.r;
        g /= rhs.g;
        b /= rhs.b;
        a /= rhs.a;
        return (*this);
    };



    SRGBA operator +(const float &val) const
    {
        return SRGBA(r+val, g+val, b+val, a+val);
    };

    SRGBA operator -(const float &val) const
    {
        return SRGBA(r-val, g-val, b-val, a-val);
    };

    SRGBA operator *(const float &val) const
    {
        return SRGBA(r*val, g*val, b*val, a*val);
    };

    SRGBA operator /(const float &val) const
    {
        return SRGBA(r/val, g/val, b/val, a/val);
    };

    SRGBA &operator +=(const float &val)
    {
        r+=val;
        g+=val;
        b+=val;
        a+=val;
        return (*this);
    };

    SRGBA &operator -=(const float &val)
    {
        r-=val;
        g-=val;
        b-=val;
        a-=val;
        return (*this);
    };

    SRGBA &operator *=(const float &val)
    {
        r*=val;
        g*=val;
        b*=val;
        a*=val;
        return (*this);
    }

    SRGBA &operator /=(const float &val)
    {
        r/=val;
        g/=val;
        b/=val;
        a/=val;
        return (*this);
    };

    float r;
    float g;
    float b;
    float a;
};
};



#endif
