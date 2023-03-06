#ifndef TEMP_VECTOR_2D
#define TEMP_VECTOR_2D

#include <cmath>
#include <cassert>

template <class T>
class TVec2D
{
public:
    TVec2D()
    {
        vec[0]=(T)0;
        vec[1]=(T)0;
    };

    TVec2D(int a)
    {
        vec[0]=(T)a;
        vec[1]=(T)a;
    };

    TVec2D(const TVec2D<T> &tvec)
    {
        vec[0]=tvec[0];
        vec[1]=tvec[1];
    };

    TVec2D(const T t1, const T t2)
    {
        vec[0]=t1;
        vec[1]=t2;
    };

    TVec2D(const T *tptr)
    {
        vec[0]=tptr[0];
        vec[1]=tptr[1];
    };

    T x()
    {
        return vec[0];
    };
    const T x() const
    {
        return vec[0];
    };

    T y()
    {
        return vec[1];
    };
    const T y() const
    {
        return vec[1];
    };

    T &operator[](const int i)
    {
        assert(i>=0 && i<2);
        return vec[i];
    };

    const T &operator[](const int i) const
    {
        assert(i>=0 && i<2);
        return vec[i];
    };

    bool operator ==(const TVec2D<T> &tvec) const
    {
        if(vec[0]==tvec[0] && vec[1]==tvec[1]) return true;
        return false;
    };

    bool operator !=(const TVec2D<T> &tvec) const
    {
        if(vec[0]==tvec[0] && vec[1]==tvec[1]) return false;
        return true;
    };

    TVec2D<T> operator +(const TVec2D<T> &tvec) const
    {
        return TVec2D<T>(vec[0]+tvec[0], vec[1]+tvec[1]);
    };

    TVec2D<T> operator -(const TVec2D<T> &tvec) const
    {
        return TVec2D<T>(vec[0]-tvec[0], vec[1]-tvec[1]);
    };

    TVec2D<T> operator *(const TVec2D<T> &tvec) const
    {
        return TVec2D<T>(vec[0]*tvec[0], vec[1]*tvec[1]);
    };

    TVec2D<T> operator /(const TVec2D<T> &tvec) const
    {
        return TVec2D<T>(vec[0]/tvec[0], vec[1]/tvec[1]);
    };

    TVec2D<T> operator -() const
    {
        return TVec2D<T>(-vec[0], -vec[1]);
    };

    TVec2D<T> &operator +=(const TVec2D<T> &tvec)
    {
        vec[0] += tvec[0];
        vec[1] += tvec[1];
        return (*this);
    };

    TVec2D<T> &operator -=(const TVec2D<T> &tvec)
    {
        vec[0] -= tvec[0];
        vec[1] -= tvec[1];
        return (*this);
    };

    TVec2D<T> &operator *=(const TVec2D<T> &tvec)
    {
        vec[0] *= tvec[0];
        vec[1] *= tvec[1];
        return (*this);
    };

    TVec2D<T> &operator /=(const TVec2D<T> &tvec)
    {
        vec[0] /= tvec[0];
        vec[1] /= tvec[1];
        return (*this);
    };



    TVec2D<T> operator +(const T &val) const
    {
        return TVec2D<T>(vec[0]+val, vec[1]+val);
    };

    TVec2D<T> operator -(const T &val) const
    {
        return TVec2D<T>(vec[0]-val, vec[1]-val);
    };

    TVec2D<T> operator *(const T &val) const
    {
        return TVec2D<T>(vec[0]*val, vec[1]*val);
    };

    TVec2D<T> operator /(const T &val) const
    {
        return TVec2D<T>(vec[0]/val, vec[1]/val);
    };

    TVec2D<T> &operator +=(const T &val)
    {
        vec[0]+=val;
        vec[1]+=val;
        return (*this);
    };

    TVec2D<T> &operator -=(const T &val)
    {
        vec[0]-=val;
        vec[1]-=val;
        return (*this);
    };

    TVec2D<T> &operator *=(const T &val)
    {
        vec[0]*=val;
        vec[1]*=val;
        return (*this);
    }

    TVec2D<T> &operator /=(const T &val)
    {
        vec[0]/=val;
        vec[1]/=val;
        return (*this);
    };

    const T dotprod(const TVec2D<T> &tvec)
    {
        return (vec[0]*tvec[0] + vec[1]*tvec[1]);
    };

    const T length()
    {
        T dot = dotprod((*this));
        double sqr = sqrt((double)dot);
        return (T)sqr;
    };

    void normalize()
    {
        T lng = length();
        (*this) /= lng;
    };

    inline T get_x()
    {
        return vec[0];
    }
    inline T get_y()
    {
        return vec[1];
    }


    inline void set_x(T x)
    {
        vec[0]=x;
    }
    inline void set_y(T y)
    {
        vec[1]=y;
    }

    inline void set(T x, T y)
    {
        vec[0]=x;
        vec[1]=y;
    }





protected:

    T vec[2];
};




#endif
