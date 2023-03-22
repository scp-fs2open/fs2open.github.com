/*
Copyright (c) 2008 Joshua Tippetts

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef RANDOM_GEN_H
#define RANDOM_GEN_H
#include <ctime>
#include <algorithm>
#include <climits>


namespace anl
{

class CBasePRNG
{
public:
    virtual ~CBasePRNG() {};

    virtual unsigned int get()=0;
    virtual void setSeed(unsigned int s)=0;

    void setSeedTime()
    {
        setSeed(static_cast<unsigned int>(std::time(0)));
    }

    unsigned int getTarget(unsigned int t)
    {
        double v=get01();
        return (unsigned int)(v*(double)t);
    }

    unsigned int getRange(unsigned int low, unsigned int high)
    {
        if(high < low) std::swap(low, high);
        double range = (double)((high - low)+1);
        double val = (double)low + get01()*range;
        return (unsigned int)(val);
    }

    double get01()
    {
        return ((double)get() / (double)(UINT_MAX));
    }
};

class LCG : public CBasePRNG
{
public:
    LCG()
    {
        setSeed(10000);
    }

    void setSeed(unsigned int s)
    {
        m_state=s;
    }

    unsigned int get()
    {
        return (m_state=69069*m_state+362437);
    }

protected:
    unsigned int m_state;
};




// The following generators are based on generators created by George Marsaglia
// They use the an LCG object for seeding, to initialize various
// state and tables. Seeding them is a bit more involved than an LCG.
class Xorshift : public CBasePRNG
{
public:
    Xorshift()
    {
        setSeed(10000);
    }

    void setSeed(unsigned int s)
    {
        LCG lcg;
        lcg.setSeed(s);
        m_x=lcg.get();
        m_y=lcg.get();
        m_z=lcg.get();
        m_w=lcg.get();
        m_v=lcg.get();
    }

    unsigned int get()
    {
        unsigned int t;
        t=(m_x^(m_x>>7));
        m_x=m_y;
        m_y=m_z;
        m_z=m_w;
        m_w=m_v;
        m_v=(m_v^(m_v<<6))^(t^(t<<13));
        return (m_y+m_y+1)*m_v;
    }

protected:
    unsigned int m_x, m_y, m_z, m_w, m_v;
};

class MWC256 : public CBasePRNG
{
public:
    MWC256()
    {
        setSeed(10000);
    }

    void setSeed(unsigned int s)
    {
        LCG lcg;
        lcg.setSeed(s);
        for(int i=0; i<256; ++i)
        {
            m_Q[i]=lcg.get();
        }
        c=lcg.getTarget(809430660);
        m_i=255;
    }

    unsigned int get()
    {
        unsigned long long int t,a=809430660LL;
        //static unsigned char i=255;
        t=a*m_Q[++m_i]+c;
        c=(t>>32);
        return(m_Q[m_i]=(unsigned int)t);
    }

protected:
    unsigned int m_Q[256], c;
    unsigned char m_i;
};

class CMWC4096 : public CBasePRNG
{
public:
    CMWC4096()
    {
        setSeed(10000);
    }

    void setSeed(unsigned int s)
    {
        LCG lcg;
        lcg.setSeed(s);   // Seed the global random source

        // Seed the table
        for(int i=0; i<4096; ++i)
        {
            m_Q[i]=lcg.get();
        }

        c=lcg.getTarget(18781);
        m_i=2095;
    }

    unsigned int get()
    {
        unsigned long long int t, a=18782LL, b=4294967295UL;
        //static unsigned int i=2095;
        unsigned int r=(unsigned int)(b-1);

        m_i=(m_i+1)&4095;
        t=a*m_Q[m_i]+c;
        c=(t>>32);
        t=(t&b)+c;
        if(t>r)
        {
            c++;
            t=t-b;
        }
        return (m_Q[m_i]=(unsigned int)(r-t));
    }

protected:
    unsigned int m_Q[4096], c;
    unsigned int m_i;
};

class KISS : public CBasePRNG
{
public:
    KISS()
    {
        setSeed(10000);
    }

    void setSeed(unsigned int s)
    {
        LCG lcg;
        lcg.setSeed(s);
        z=lcg.get();
        w=lcg.get();
        jsr=lcg.get();
        jcong=lcg.get();
    }

    unsigned int get()
    {
        z=36969*(z&65535)+(z>>16);
        w=18000*(w&65535)+(w>>16);
        unsigned int mwc = (z<<16)+w;

        jcong=69069*jcong+1234567;

        jsr^=(jsr<<17);
        jsr^=(jsr>>13);
        jsr^=(jsr<<5);

        return ((mwc^jcong) + jsr);
    }

public:
    unsigned int z,w,jsr,jcong;
};
};

#endif
