#ifndef TEMP_VECTOR_4D
#define TEMP_VECTOR_4D

#include <cmath>

template <class T>
class TVec4D
{
public:
    TVec4D()
    {
        vec[0]=(T)0;
        vec[1]=(T)0;
        vec[2]=(T)0;
        vec[3]=(T)0;
    };

    TVec4D(int a)
    {
        vec[0]=(T)a;
        vec[1]=(T)a;
        vec[2]=(T)a;
        vec[3]=(T)a;
    };

    TVec4D(const TVec4D<T> &tvec)
    {
        vec[0]=tvec[0];
        vec[1]=tvec[1];
        vec[2]=tvec[2];
        vec[3]=tvec[3];
    };

    TVec4D(const T t1, const T t2, const T t3, const T t4)
    {
        vec[0]=t1;
        vec[1]=t2;
        vec[2]=t3;
        vec[3]=t4;
    };

    TVec4D(const T *tptr)
    {
        vec[0]=tptr[0];
        vec[1]=tptr[1];
        vec[2]=tptr[2];
        vec[3]=tptr[3];
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

    T w()
    {
        return vec[3];
    };
    const T w() const
    {
        return vec[3];
    };


    T &operator[](const int i)
    {
        assert(i>=0 && i<4);
        return vec[i];
    };

    const T &operator[](const int i) const
    {
        assert(i>=0 && i<4);
        return vec[i];
    };

    bool operator ==(const TVec4D<T> &tvec) const
    {
        if(vec[0]==tvec[0] && vec[1]==tvec[1] && vec[2]==tvec[2] && vec[3]==tvec[3]) return true;
        return false;
    };

    bool operator !=(const TVec4D<T> &tvec) const
    {
        if(vec[0]==tvec[0] && vec[1]==tvec[1] && vec[2]==tvec[2] && vec[3]==tvec[3]) return false;
        return true;
    };

    TVec4D<T> operator +(const TVec4D<T> &tvec) const
    {
        return TVec4D<T>(vec[0]+tvec[0], vec[1]+tvec[1], vec[2]+tvec[2], vec[3]+tvec[3]);
    };

    TVec4D<T> operator -(const TVec4D<T> &tvec) const
    {
        return TVec4D<T>(vec[0]-tvec[0], vec[1]-tvec[1], vec[2]-tvec[2], vec[3]-tvec[3]);
    };

    TVec4D<T> operator *(const TVec4D<T> &tvec) const
    {
        return TVec4D<T>(vec[0]*tvec[0], vec[1]*tvec[1], vec[2]*tvec[2], vec[3]*tvec[3]);
    };

    TVec4D<T> operator *(const double &rhs) const
    {
        return TVec4D<T>(vec[0]*rhs, vec[1]*rhs, vec[2]*rhs, vec[3]*rhs);
    }

    TVec4D<T> operator /(const TVec4D<T> &tvec) const
    {
        return TVec4D<T>(vec[0]/tvec[0], vec[1]/tvec[1], vec[2]/tvec[2], vec[3]/tvec[3]);
    };

    TVec4D<T> operator /(const double &rhs) const
    {
        return TVec4D<T>(vec[0]/rhs, vec[1]/rhs, vec[2]/rhs, vec[3]/rhs);
    }

    TVec4D<T> operator -() const
    {
        return TVec4D<T>(-vec[0], -vec[1], -vec[2], -vec[3]);
    };

    TVec4D<T> &operator +=(const TVec4D<T> &tvec)
    {
        vec[0] += tvec[0];
        vec[1] += tvec[1];
        vec[2] += tvec[2];
        vec[3] += tvec[3];
        return (*this);
    };

    TVec4D<T> &operator -=(const TVec4D<T> &tvec)
    {
        vec[0] -= tvec[0];
        vec[1] -= tvec[1];
        vec[2] -= tvec[2];
        vec[3] -= tvec[3];
        return (*this);
    };

    TVec4D<T> &operator *=(const TVec4D<T> &tvec)
    {
        vec[0] *= tvec[0];
        vec[1] *= tvec[1];
        vec[2] *= tvec[2];
        vec[3] *= tvec[3];
        return (*this);
    };

    TVec4D<T> &operator /=(const TVec4D<T> &tvec)
    {
        vec[0] /= tvec[0];
        vec[1] /= tvec[1];
        vec[2] /= tvec[2];
        vec[3] /= tvec[3];
        return (*this);
    };



    TVec4D<T> operator +(const T &val) const
    {
        return TVec4D<T>(vec[0]+val, vec[1]+val, vec[2]+val, vec[3]+val);
    };

    TVec4D<T> operator -(const T &val) const
    {
        return TVec4D<T>(vec[0]-val, vec[1]-val, vec[2]-val, vec[3]-val);
    };

    TVec4D<T> operator *(const T &val) const
    {
        return TVec4D<T>(vec[0]*val, vec[1]*val, vec[2]*val, vec[3]*val);
    };

    TVec4D<T> operator /(const T &val) const
    {
        return TVec4D<T>(vec[0]/val, vec[1]/val, vec[2]/val, vec[3]/val);
    };

    TVec4D<T> &operator +=(const T &val)
    {
        vec[0]+=val;
        vec[1]+=val;
        vec[2]+=val;
        vec[3]+=val;
        return (*this);
    };

    TVec4D<T> &operator -=(const T &val)
    {
        vec[0]-=val;
        vec[1]-=val;
        vec[2]-=val;
        vec[3]-=val;
        return (*this);
    };

    TVec4D<T> &operator *=(const T &val)
    {
        vec[0]*=val;
        vec[1]*=val;
        vec[2]*=val;
        vec[3]*=val;
        return (*this);
    }

    TVec4D<T> &operator /=(const T &val)
    {
        vec[0]/=val;
        vec[1]/=val;
        vec[2]/=val;
        vec[3]/=val;
        return (*this);
    };

    const T dotprod(const TVec4D<T> &tvec)
    {
        return (vec[0]*tvec[0] + vec[1]*tvec[1] +vec[2]*tvec[2]+vec[3]*tvec[3]);
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

    void set(T x, T y, T z, T w)
    {
        vec[0]=x;
        vec[1]=y;
        vec[2]=z;
        vec[3]=w;
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
    inline T get_w()
    {
        return vec[3];
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
    inline void set_w(T w)
    {
        vec[3]=w;
    }






protected:

    T vec[4];
};




#endif
