#ifndef TARRAYS_H
#define TARRAYS_H
#include <vector>
#include <stdexcept>
#include <cmath>
#include "../vectortypes.h"

namespace anl
{
template <class T>
class TArray2D
{
private:
    T *m_data;
    int m_width, m_height;
public:
    TArray2D(int w, int h) : m_data(0), m_width(w), m_height(h)
    {
        resize(w,h);
    }
    TArray2D()
    {
        m_width=0;
        m_height=0;
        m_data=0;
    };
    ~TArray2D()
    {
        destroy();
    };

    void resize(int w, int h)
    {
        destroy();
        if(w==0 || h==0) return;
        m_data = new T[w*h];
        m_width=w;
        m_height=h;
        fill(T());
    };

    void destroy()
    {
        if(m_data) delete[] m_data;
        m_data=0;
        m_width=0;
        m_height=0;
    };

    int width()
    {
        return m_width;
    };
    int height()
    {
        return m_height;
    };
    T *getData()
    {
        return m_data;
    };   // Hackification

    void set(int x, int y, T v)
    {
        if(x>=m_width || y>=m_height || x<0 || y<0) return;
        if(!m_data) return;

        m_data[y*m_width+x]=v;
    }

    T get(int x, int y)
    {
        if(x>=m_width || y>=m_height || !m_data) return T(0);

        return m_data[y*m_width+x];
    }

    T &getRef(int x, int y)
    {
        //if(x>=m_width || y>=m_height || !m_data) return T(0);

        return m_data[y*m_width+x];
    }

    T get(float x, float y)
    {
        int ix = (int)x;
        int iy = (int)y;
        int nx = ix+1;
        int ny = iy+1;

        if(ix>=m_width || iy>=m_height) return T(0);
        if(nx>=m_width) nx=ix;
        if(ny>=m_height) ny=iy;

        T v1=get(ix,iy);
        T v2=get(nx,iy);
        T v3=get(ix,ny);
        T v4=get(nx,ny);

        float s = x - (float)ix;
        float t = y - (float)iy;

        // interpolate v1->v2 and v3->v4 on s
        T v5 = v1 + (T)((v2-v1)*s);
        T v6 = v3 + (T)((v4-v3)*s);

        // interpolated v5->v6 on t
        T final = v5 + (T)((v6-v5)*t);
        return final;
    }

    T getBilinear(float x, float y)
    {
        x=x*(float)(m_width-1);
        y=y*(float)(m_height-1);
        int ix = (int)x;
        int iy = (int)y;
        int nx = ix+1;
        int ny = iy+1;

        if(ix>=m_width || iy>=m_height) return T(0);
        if(nx>=m_width) nx=ix;
        if(ny>=m_height) ny=iy;

        T v1=get(ix,iy);
        T v2=get(nx,iy);
        T v3=get(ix,ny);
        T v4=get(nx,ny);

        float s = x - (float)ix;
        float t = y - (float)iy;

        // interpolate v1->v2 and v3->v4 on s
        T v5 = v1 + (T)((v2-v1)*s);
        T v6 = v3 + (T)((v4-v3)*s);

        // interpolated v5->v6 on t
        T final = v5 + (T)((v6-v5)*t);
        return final;
    }

    T getIndexed(int c)
    {
        if(c >= m_height*m_width + m_width || c<0) return T(0);
        return m_data[c];
    }

    void setIndexed(int c, T v)
    {
        if(c>=m_height*m_width+m_width || c<0) return;
        m_data[c]=v;
    }

    void fill(T v)
    {
        if(!m_data) return;

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                m_data[y*m_width+x]=v;
            }
        }
    }

    void copyFrom(TArray2D<T> *b)
    {
        if(!m_data) return;
        int w,h;

        if(!b) return;
        w=b->width();
        h=b->height();
        if(w != m_width || h != m_height) return;  // Must be same size

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                set(x,y,b->get(x,y) );
            }
        }
    }

    void copyFromSub(TArray2D<T> *b, int x, int y)
    {
        if(!m_data) return;
        int w,h;

        if(!b) return;
        w=b->width();
        h=b->height();
        if(w==0 || h==0 ) return;
        for(int cx=0; cx<w; ++cx)
        {
            for(int cy=0; cy<h; ++cy)
            {
                set(x+cx, y+cy, b->get(cx,cy));
            }
        }
    }

    void addArray(TArray2D<T> *b)
    {
        if(!m_data) return;
        int w,h;

        if(!b) return;
        w=b->width();
        h=b->height();
        if(w != m_width || h != m_height) return;  // Must be same size

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T temp = get(x,y);
                set(x,y,b->get(x,y)+temp );
            }
        }
    }

    void addArraySub(TArray2D<T> *b, int x, int y)
    {
        if(!m_data) return;
        int w,h;

        if(!b) return;
        w=b->width();
        h=b->height();
        if(w==0 || h==0) return;

        for(int cx=0; cx<w; ++cx)
        {
            for(int cy=0; cy<h; ++cy)
            {
                T temp = get(cx+x, cy+y);
                set(cx+x, cy+y, b->get(cx,cy)+temp);
            }
        }
    }

    void subtractArray(TArray2D<T> *b)
    {
        if(!m_data) return;
        int w,h;

        if(!b) return;
        w=b->width();
        h=b->height();
        if(w != m_width || h != m_height) return;  // Must be same size

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T temp = get(x,y);
                set(x,y,temp-b->get(x,y) );
            }
        }
    }

    void subtractArraySub(TArray2D<T> *b, int x, int y)
    {
        if(!m_data) return;
        int w,h;

        if(!b) return;
        w=b->width();
        h=b->height();
        if(w==0 || h==0) return;

        for(int cx=0; cx<w; ++cx)
        {
            for(int cy=0; cy<h; ++cy)
            {
                T temp = get(cx+x, cy+y);
                set(cx+x, cy+y, temp-b->get(cx,cy));
            }
        }
    }
    void multiplyArray(TArray2D<T> *b)
    {
        if(!m_data) return;
        int w,h;

        if(!b) return;
        w=b->width();
        h=b->height();
        if(w != m_width || h != m_height) return;  // Must be same size

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T temp = get(x,y);
                set(x,y,b->get(x,y)*temp );
            }
        }
    }

    void multiplyArraySub(TArray2D<T> *b, int x, int y)
    {
        if(!m_data) return;
        int w,h;

        if(!b) return;
        w=b->width();
        h=b->height();
        if(w==0 || h==0) return;

        for(int cx=0; cx<w; ++cx)
        {
            for(int cy=0; cy<h; ++cy)
            {
                T temp = get(cx+x, cy+y);
                set(cx+x, cy+y, b->get(cx,cy)*temp);
            }
        }
    }

    void scale(T s)
    {
        if(!m_data) return;
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T temp=get(x,y);
                set(x,y,temp*s);
            }
        }
    }

    void blendFromArrays(TArray2D<T> *b1, TArray2D<T> *b2, TArray2D<float> *b3)
    {
        if(!m_data) return;
        if(!b1 || !b2 || !b3) return;
        if(b1->width() != m_width || b2->width() != m_width || b3->width() != m_width) return;
        if(b1->height() != m_height || b2->height() != m_height || b3->height() != m_height) return;

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                float t=b3->get(x,y);
                T v1=b1->get(x,y);
                T v2=b2->get(x,y);
                //T val = (T) ( (float)v1*(1-t) + (float)v2*t );
                T val = v1 + (T)((v2-v1)*t);
                set(x,y,val);
            }
        }
    }

    void selectFromArrays(TArray2D<T> *b1, TArray2D<T> *b2, TArray2D<float> *b3, float threshold)
    {
        if(!m_data) return;
        if(!b1 || !b2 || !b3) return;
        if(b1->width() != m_width || b2->width() != m_width || b3->width() != m_width) return;
        if(b1->height() != m_height || b2->height() != m_height || b3->height() != m_height) return;

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                float t=b3->get(x,y);
                if(t<threshold) set(x,y,b1->get(x,y));
                else set(x,y,b2->get(x,y));
            }
        }
    }

    T getMax()
    {
        if(!m_data) return (T)0;
        T m = m_data[0];
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T v=get(x,y);
                if(v > m) m=v;
            }
        }

        return m;
    }

    T getMin()
    {
        if(!m_data) return (T)0;
        T m = m_data[0];
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T v=get(x,y);
                if(v < m) m=v;
            }
        }

        return m;
    }

    void scaleToRange(T low, T high)
    {
        if(!m_data) return;
        T max=getMax();
        T min=getMin();

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T temp = get(x,y);
                temp = temp - min;
                //float ftemp = (float)temp / ( (float)max - (float)min);
                T ftemp = temp / (max-min);
                T val = ftemp * (high-low);
                //T val = (T)(ftemp * ((float)high-(float)low));
                val = val + low;
                set(x,y,val);
            }
        }
    }
	
	void bias(T b)
	{
		if(!m_data) return;
      
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
				T temp = get(x,y);
				set(x,y,pow(temp, log(b)/log(0.5)));
			}
		}
	}
	
	void gain(T b)
	{
		if(!m_data) return;
      
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
				T temp = get(x,y);
				if(temp<0.5)
				{
					set(x,y,pow(2.0*temp, log(1.0-b)/log(0.5))/2.0);
					
				}
				else
				{
					set(x,y,1.0-pow(2.0-2.0*temp, log(1.0-b)/log(0.5))/2.0);
					
				}
			}
		}
	}

    void wrapCoords(int &x, int &y)
    {
        if(m_width==0 || m_height==0) return;
        while(x >= m_width) x -= m_width;
        while(y >= m_height) y -= m_height;
    }

    void wrapCoords(float &x, float &y)
    {
        if(m_width==0 || m_height==0) return;
        while(x>=(float)m_width) x-=(float)m_width;
        while(y>=(float)m_height) y-=(float)m_height;
    }

    void offset(int ox, int oy)
    {
        if(!m_data) return;
        // Offset buffer
        TArray2D<T> tempbuf;
        tempbuf.resize(m_width, m_height);
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                int nx=x-ox;
                int ny=y-oy;
                wrapCoords(nx,ny);
                tempbuf.set(x,y,get(nx,ny));
            }
        }
        copyFrom(&tempbuf);
    }

    void flipVertical()
    {
        if(!m_data) return;
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T c1=get(x,y);
                T c2=get(x,(m_height-1)-y);
                set(x,y,c2);
                set(x,(m_height-1)-y,c1);
            }
        }
    }

    void flipHorizontal()
    {
        if(!m_data) return;
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                T c1=get(x,y);
                T c2=get((m_width-1)-x,y);
                set(x,y,c2);
                set((m_width-1)-x,y,c1);
            }
        }
    }

    T sampleRow(TArray2D<T> *src, TArray2D<double> *blur, float x, float y)
    {
        // Apply sample kernel to the rows of the source image to sample down
        int bw=blur->width();
        T total=T(0);
        float totalweight=0.0f;
        for(int xx=0; xx<bw; ++xx)
        {
            float ox=(float)(xx-bw/2);
            float nx=x+ox;
            if(nx>=0 && nx<src->width())
            {
                float k=(float)blur->get(xx,0);
                T v=src->get(nx,y);
                total=total+v*k;
                totalweight+=k;
            }
        }
        return total/totalweight;
    }

    T sampleCol(TArray2D<T> *src, TArray2D<double> *blur, float x, float y)
    {
        // Apply sample kernel to the rows of the source image to sample down
        int bw=blur->width();
        T total=T(0);
        float totalweight=0.0f;
        for(int yy=0; yy<bw; ++yy)
        {
            float oy=(float)(yy-bw/2);
            float ny=y+oy;
            if(ny>=0 && ny<src->height())
            {
                float k=(float)blur->get(yy,0);
                T v=src->get(x,ny);
                total=total+v*k;
                totalweight+=k;
            }
        }
        return total/totalweight;
    }

    void buildSampleKernel(TArray2D<double> *kernel, int size)
    {
        kernel->resize(size,1);
        double c=(double)size / 2.0;
        double total=0;
        for(int x=0; x<size; ++x)
        {
            double d=std::abs((double)x-c) / c;
            d=1.0-d;
            kernel->set(x,0,d);
            total+=d;
        }
        for(int x=0; x<size; ++x)
        {
            kernel->set(x,0,kernel->get(x,0)/total);
        }
    }

    void get4x4Neighborhood(T *block, int x, int y)
    {
        // Copy out a 4x4 block of values. Any values that go beyond the borders are clamped.
        // Neighborhood occupies the range [x-1,x+2][y-1,y+2]
        for(int xx=-1; xx<=2; ++xx)
        {
            for(int yy=-1; yy<=2; ++yy)
            {
                int bx=xx+1;
                int by=yy+1;

                int nx=x+xx;
                if(nx<0) nx=0;
                if(nx>=m_width) nx=m_width-1;

                int ny=y+yy;
                if(ny<0) ny=0;
                if(ny>=m_height) ny=m_height-1;

                block[by*4+bx]=get(nx,ny);
            }
        }
    }

    T splineVector(T *vec, float parm)
    {
        T c = (vec[2]-vec[0]) * 0.5f;
        T v = (vec[1]-vec[2]);
        T w = c+v;
        T a = w+v+(vec[3]-vec[2])*0.5f;
        T b = w+a;
        return ((((a*parm)-b)*parm + c) * parm + vec[2]);

        // Catmull-Rom interpolation
        /*
        float t=parm;
        float t2=parm*parm;
        float t3=t2*parm;
        return ((vec[1]*2.0f)+(vec[2]-vec[1])*t+(vec[0]*2.0f - vec[1]*5.0f + vec[2]*4.0f - vec[3])*t2 + (vec[1]*3-vec[2]*3-vec[0]+vec[3])*t3)*0.5f;
        */

    }

    void subtractFilter(TArray2D<T> *fil)
    {
        // Subtract a buffer from this one. The buffer being subtracted is sampled bilinearly, and can be of any size
        for(int y=0; y<m_height; ++y)
        {
            for(int x=0; x<m_width; ++x)
            {
                float nx=(float)x/(float)m_width;
                float ny=(float)y/(float)m_height;
                T val=fil->getBilinear(nx,ny);
                set(x,y,get(x,y)-val);
            }
        }
    }

    void addFilter(TArray2D<T> *fil)
    {
        // Add a buffer to this one. The buffer being subtracted is sampled bilinearly, and can be of any size
        for(int y=0; y<m_height; ++y)
        {
            for(int x=0; x<m_width; ++x)
            {
                float nx=(float)x/(float)m_width;
                float ny=(float)y/(float)m_height;
                T val=fil->getBilinear(nx,ny);
                set(x,y,get(x,y)+val);
            }
        }
    }

    T splineNeighborhood(T *block, float s, float t)
    {
        // Apply spline interpolation to neighborhood
        T a[4];
        for(int y=0; y<4; ++y)
        {
            // Spline the rows
            a[y]=splineVector(&block[y*4], s);
        }
        // Spline the columns
        return splineVector(a,t);
    }
    void scaleTo(TArray2D<T> *dst)
    {
        if(!dst || !m_data) return;
        int mw=dst->width(), mh=dst->height();
        T neighborhood[16];

        for(int x=0; x<mw; ++x)
        {
            for(int y=0; y<mh; ++y)
            {
                float s=(float)x/(float)mw;
                float t=(float)y/(float)mh;
                float nx=s*(float)m_width;
                float ny=t*(float)m_height;

                float p=nx - floorf(nx);
                float q=ny - floorf(ny);
                get4x4Neighborhood(neighborhood, (int)nx, (int)ny);
                T val=splineNeighborhood(neighborhood, p, q);
                dst->set(x,y,val);
            }
        }
    }

    void applyBlurRow(TArray2D<T> *src, TArray2D<T> *dest, TArray2D<double> *blur, int x, int y, bool seamless)
    {
        int bw=blur->width();
        T total=T(0);
        double totalweight=0;
        for(int xx=0; xx<bw; ++xx)
        {
            int nx=xx-bw/2;
            double k=blur->get(xx,0);
            T v;

            int cx=nx+x;
            if(seamless)
            {
                // Wrap coords
                //wrapCoords(cx,y);
                while(cx<0) cx+=m_width;
                while(cx>=m_width) cx-=m_width;
                totalweight+=k;
                v=src->get(cx,y);
            }
            else
            {
                if(cx>=0 && cx<src->width())
                {
                    totalweight+=k;
                    v=src->get(cx,y);
                }
                else v=T(0);
            }

            //T v=src->get(cx,y);
            total=total+(T)(v*k);

        }
        dest->set(x,y,(T)total/totalweight);
    }

    void applyBlurCol(TArray2D<T> *src, TArray2D<T> *dest, TArray2D<double> *blur, int x, int y, bool seamless)
    {
        int bw=blur->width();
        T total=T(0);
        double totalweight=0;
        for(int yy=0; yy<bw; ++yy)
        {
            int ny=yy-bw/2;
            int cy=y+ny;
            double k=blur->get(yy,0);
            T v;
            if(seamless)
            {
                //wrapCoords(x,cy);
                while(cy<0) cy+=m_height;
                while(cy>=m_height) cy-=m_height;
                totalweight+=k;
                v=src->get(x,cy);
            }
            else
            {
                if(cy>=0 && cy<src->height())
                {
                    totalweight+=k;
                    v=src->get(x,cy);
                }
                else
                {
                    v=T(0);
                }
            }
            //T v=src->get(x,cy);
            total=total+(T)(v*k);

        }
        dest->set(x,y,(T)total/totalweight);
    }


    void blur(float blur_size, bool seamless)
    {
        if(!m_data) return;
        TArray2D<T> temp;
        temp.resize(m_width, m_height);
        TArray2D<double> kernelx, kernely;
        buildSampleKernel(&kernelx, (int)(blur_size*m_width));
        buildSampleKernel(&kernely, (int)(blur_size*m_height));

        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                applyBlurRow(this, &temp, &kernelx, x, y, seamless);
            }
        }
        for(int x=0; x<m_width; ++x)
        {
            for(int y=0; y<m_height; ++y)
            {
                applyBlurCol(&temp, this, &kernely, x, y, seamless);
            }
        }
    }
};

template<class T> class TArray3D
{
public:
    TArray3D() : m_width(0), m_height(0), m_depth(0) {}
    TArray3D(int width, int height, int depth)
    {
        resize(width,height,depth);
    }
    ~TArray3D() {}

    int width()
    {
        return m_width;
    }
    int height()
    {
        return m_height;
    }
    int depth()
    {
        return m_depth;
    }

    void resize(int width, int height, int depth)
    {
        m_array.resize(width*height*depth);
        m_width=width;
        m_height=height;
        m_depth=depth;
        for(int c=0; c<m_width*m_height*depth; ++c) m_array[c]=T(0);
    }

    void set(int x, int y, int z, T val)
    {
        if(x>=m_width || y>=m_height || z>=m_depth) throw(std::out_of_range("Array index out of range"));
        m_array[z*(m_width*m_height)+y*m_width+x]=val;
    }

    T get(int x, int y, int z)
    {
        if(x>=m_width || y>=m_height || z>=m_depth) throw(std::out_of_range("Array index out of range"));
        return m_array[z*(m_width*m_height)+y*m_width+x];
    }

    TArray3D &operator=(const TArray2D<T> &v)
    {
        resize(v.width(), v.height(), v.depth());
        for(int c=0; c<m_width*m_height*m_depth; ++c)
        {
            m_array[c]=v.m_array[c];
        }
        return *this;
    }

    TArray3D &operator=(const T& v)
    {
        for(int c=0; c<m_width*m_height*m_depth; ++c) m_array[c]=v;
    }

    T *getData()
    {
        return m_array.size()>0 ? &m_array[0] : 0;
    }

protected:
    int m_width, m_height, m_depth;
    std::vector<T> m_array;

};
};
#endif
