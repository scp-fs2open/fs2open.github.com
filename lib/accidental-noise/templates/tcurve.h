#ifndef TCURVE_H
#define TCURVE_H
#include <vector>
#include <cmath>


namespace anl
{
template<class T> class TCurve
{
public:
    TCurve() {}
    ~TCurve() {}

    void pushPoint(double t, T point)
    {
        tIterType iter=findControlPoint(t);
        m_points.insert(iter,SControlPoint(t,point));
        return;
    }

    void clear()
    {
        m_points.clear();
    };

    T noInterp(double t)
    {
        if(m_points.size() < 2) return T(0);
        if( t <= m_points[0].m_t) return m_points[0].m_value;
        if( t >= m_points[m_points.size()-1].m_t) return m_points[m_points.size()-1].m_value;

        tIterType iter=findControlPoint(t);
        if(iter==m_points.end())
        {
            // Something weird happened
            return T(0);
        }

        tIterType prev_iter = iter-1;
        if(prev_iter==m_points.end())
        {
            // Again, something weird happened
            return T(0);
        }
        return prev_iter->m_value;
    }

    T linearInterp(double t)
    {
        if(m_points.size() < 2) return T(0);
        if( t <= m_points[0].m_t) return m_points[0].m_value;
        if( t >= m_points[m_points.size()-1].m_t) return m_points[m_points.size()-1].m_value;

        tIterType iter=findControlPoint(t);
        if(iter==m_points.end())
        {
            // Something weird happened
            return T(0);
        }

        tIterType prev_iter = iter-1;
        if(prev_iter==m_points.end())
        {
            // Again, something weird happened
            return T(0);
        }

        double t0=(*prev_iter).m_t;
        double t1=(*iter).m_t;
        double interp=(t-t0)/(t1-t0);

        T v0=(*prev_iter).m_value;
        T v1=(*iter).m_value;
        return v0 + T((v1-v0)*interp);
    }

    T cubicInterp(double t)
    {
        if(m_points.size() < 2) return T(0);
        if( t <= m_points[0].m_t) return m_points[0].m_value;
        if( t >= m_points[m_points.size()-1].m_t) return m_points[m_points.size()-1].m_value;

        tIterType iter=findControlPoint(t);
        if(iter==m_points.end())
        {
            // Something weird happened
            return T(0);
        }

        tIterType prev_iter = iter-1;
        if(prev_iter==m_points.end())
        {
            // Again, something weird happened
            return T(0);
        }

        double t0=(*prev_iter).m_t;
        double t1=(*iter).m_t;
        double interp=(t-t0)/(t1-t0);
        interp=(interp*interp*(3-2*interp));

        T v0=(*prev_iter).m_value;
        T v1=(*iter).m_value;
        return v0 + T((v1-v0)*interp);
    }

    T quinticInterp(double t)
    {
        if(m_points.size() < 2) return T(0);
        if( t <= m_points[0].m_t) return m_points[0].m_value;
        if( t >= m_points[m_points.size()-1].m_t) return m_points[m_points.size()-1].m_value;

        tIterType iter=findControlPoint(t);
        if(iter==m_points.end())
        {
            // Something weird happened
            return T(0);
        }

        tIterType prev_iter = iter-1;
        if(prev_iter==m_points.end())
        {
            // Again, something weird happened
            return T(0);
        }

        double t0=(*prev_iter).m_t;
        double t1=(*iter).m_t;
        double interp=(t-t0)/(t1-t0);
        interp=interp*interp*interp*(interp*(interp*6-15)+10);

        T v0=(*prev_iter).m_value;
        T v1=(*iter).m_value;
        return v0 + T((v1-v0)*interp);
    }


protected:
    struct SControlPoint
    {
        SControlPoint(double t, T v)
        {
            m_t=t;
            m_value=v;
        };
        double m_t;
        T m_value;
    };
    typedef typename std::vector<SControlPoint>::iterator tIterType;
    std::vector<SControlPoint> m_points;

    tIterType findControlPoint(double t)
    {
        // Find the first control point where p.t > t
        tIterType iter=m_points.begin();
        while(iter != m_points.end())
        {
            if(t<=(*iter).m_t)
            {
                return iter;
            }
            ++iter;
        }

        return m_points.end();
    }

};
};


#endif
