#ifndef TEMP_VECTOR_3D
#define TEMP_VECTOR_3D

#include <cmath>

template <class T>
class TVec3D
{
public:
    TVec3D()
    {
        vec[0]=(T)0;
        vec[1]=(T)0;
        vec[2]=(T)0;
    };

    TVec3D(int a)
    {
        vec[0]=(T)a;
        vec[1]=(T)a;
        vec[2]=(T)a;
    };

    TVec3D(float a)
    {
        vec[0]=(T)a;
        vec[1]=(T)a;
        vec[2]=(T)a;
    };

    TVec3D(const TVec3D<T> &tvec)
    {
        vec[0]=tvec[0];
        vec[1]=tvec[1];
        vec[2]=tvec[2];
    };

    TVec3D(const T t1, const T t2, const T t3)
    {
        vec[0]=t1;
        vec[1]=t2;
        vec[2]=t3;
    };

    TVec3D(const T *tptr)
    {
        vec[0]=tptr[0];
        vec[1]=tptr[1];
        vec[2]=tptr[2];
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

    T z()
    {
        return vec[2];
    };
    const T z() const
    {
        return vec[2];
    };

    T &operator[](const int i)
    {
        assert(i>=0 && i<3);
        return vec[i];
    };

    const T &operator[](const int i) const
    {
        assert(i>=0 && i<3);
        return vec[i];
    };

    void set(T v1, T v2, T v3)
    {
        vec[0]=v1;
        vec[1]=v2;
        vec[2]=v3;
    }
    inline T get_x()
    {
        return vec[0];
    }
    inline T get_y()
    {
        return vec[1];
    }
    inline T get_z()
    {
        return vec[2];
    }

    inline void set_x(T x)
    {
        vec[0]=x;
    }
    inline void set_y(T y)
    {
        vec[1]=y;
    }
    inline void set_z(T z)
    {
        vec[2]=z;
    }

    bool operator ==(const TVec3D<T> &tvec) const
    {
        if(vec[0]==tvec[0] && vec[1]==tvec[1] && vec[2]==tvec[2]) return true;
        return false;
    };

    bool operator !=(const TVec3D<T> &tvec) const
    {
        if(vec[0]==tvec[0] && vec[1]==tvec[1] && vec[2]==tvec[2]) return false;
        return true;
    };

    TVec3D<T> operator +(const TVec3D<T> &tvec) const
    {
        return TVec3D<T>(vec[0]+tvec[0], vec[1]+tvec[1], vec[2]+tvec[2]);
    };

    TVec3D<T> operator -(const TVec3D<T> &tvec) const
    {
        return TVec3D<T>(vec[0]-tvec[0], vec[1]-tvec[1], vec[2]-tvec[2]);
    };

    TVec3D<T> operator *(const TVec3D<T> &tvec) const
    {
        return TVec3D<T>(vec[0]*tvec[0], vec[1]*tvec[1], vec[2]*tvec[2]);
    };

    TVec3D<T> operator /(const TVec3D<T> &tvec) const
    {
        return TVec3D<T>(vec[0]/tvec[0], vec[1]/tvec[1], vec[2]/tvec[2]);
    };

    TVec3D<T> operator -() const
    {
        return TVec3D<T>(-vec[0], -vec[1], -vec[2]);
    };

    TVec3D<T> &operator +=(const TVec3D<T> &tvec)
    {
        vec[0] += tvec[0];
        vec[1] += tvec[1];
        vec[2] += tvec[2];
        return (*this);
    };

    TVec3D<T> &operator -=(const TVec3D<T> &tvec)
    {
        vec[0] -= tvec[0];
        vec[1] -= tvec[1];
        vec[2] -= tvec[2];
        return (*this);
    };

    TVec3D<T> &operator *=(const TVec3D<T> &tvec)
    {
        vec[0] *= tvec[0];
        vec[1] *= tvec[1];
        vec[2] *= tvec[2];
        return (*this);
    };

    TVec3D<T> &operator /=(const TVec3D<T> &tvec)
    {
        vec[0] /= tvec[0];
        vec[1] /= tvec[1];
        vec[2] /= tvec[2];
        return (*this);
    };

    TVec3D<T> operator *(const double &rhs) const
    {
        return TVec3D<T>(vec[0]*rhs, vec[1]*rhs, vec[2]*rhs);
    }

    TVec3D<T> operator /(const double &rhs) const
    {
        return TVec3D<T>(vec[0]/rhs, vec[1]/rhs, vec[2]/rhs);
    }

    TVec3D<T> fmul(const float &fval) const
    {
        return TVec3D<T>((T)(vec[0]*fval), (T)(vec[1]*fval), (T)(vec[2]*fval));
    };

    TVec3D<T> dmul(const double fval) const
    {
        return TVec3D<T>((T)(vec[0]*fval), (T)(vec[1]*fval), (T)(vec[2]*fval));
    };

    /*TVec3D<T> operator *(const float &fval) const
    {
        return TVec3D<T>((T)(vec[0]*fval), (T)(vec[1]*fval), (T)(vec[2]*fval));
    };

    TVec3D<T> operator *(const double fval) const
    {
        return TVec3D<T>((T)(vec[0]*fval), (T)(vec[1]*fval), (T)(vec[2]*fval));
    };*/

    TVec3D<T> operator +(const T &val) const
    {
        return TVec3D<T>(vec[0]+val, vec[1]+val, vec[2]+val);
    };

    TVec3D<T> operator -(const T &val) const
    {
        return TVec3D<T>(vec[0]-val, vec[1]-val, vec[2]-val);
    };

    /*TVec3D<T> operator *(const T &val) const
    {
        return TVec3D<T>(vec[0]*val, vec[1]*val, vec[2]*val);
    };*/
    template <typename multiplier> TVec3D<T> operator *(const multiplier &val) const
    {
        return TVec3D<T>(T(vec[0]*val), T(vec[1]*val), T(vec[2]*val));
    };

    template <typename multiplier> TVec3D<T> &operator *=(const multiplier &val)
    {
        vec[0]*=val;
        vec[1]*=val;
        vec[2]*=val;
        return (*this);
    };

    TVec3D<T> operator /(const T &val) const
    {
        return TVec3D<T>(vec[0]/val, vec[1]/val, vec[2]/val);
    };

    TVec3D<T> &operator +=(const T &val)
    {
        vec[0]+=val;
        vec[1]+=val;
        vec[2]+=val;
        return (*this);
    };

    TVec3D<T> &operator -=(const T &val)
    {
        vec[0]-=val;
        vec[1]-=val;
        vec[2]-=val;
        return (*this);
    };

    TVec3D<T> &operator *=(const T &val)
    {
        vec[0]*=val;
        vec[1]*=val;
        vec[2]*=val;
        return (*this);
    }

    TVec3D<T> &operator /=(const T &val)
    {
        vec[0]/=val;
        vec[1]/=val;
        vec[2]/=val;
        return (*this);
    };

    const T dotprod(const TVec3D<T> &tvec)
    {
        return (vec[0]*tvec[0] + vec[1]*tvec[1] +vec[2]*tvec[2]);
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





protected:

    T vec[3];
};




#endif
