#ifndef T_ARRAY1_H
#define T_ARRAY1_H

#include <cassert>
#include <algorithm>

namespace anl
{
template<typename T>
class TArray1D
{
public:
    typedef T value_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

    TArray1D() : size_(0), capacity_(0), data_(0)
    {
    }

    TArray1D(size_t size, T value): size_(0), capacity_(0), data_(0)
    {
        resize(size, 0);
        fill(value);
    }

    TArray1D(size_t size) : size_(0), capacity_(0), data_(0)
    {
        resize(size,0);
    }

    TArray1D(const TArray1D<T>& a) : size_(0), capacity_(0), data_(0)
    {
        resize(a.size(),0);
    }
    TArray1D(const T* data, size_t size)  : size_(0), capacity_(0), data_(0)
    {
        resize(size,data);
    }

    TArray1D& operator = (const TArray1D<T>& a)
    {
        resize(a.size(), a.data());
        return *this;
    }

    ~TArray1D()
    {
        delete[] data_;
    }

    inline size_t size() const
    {
        return size_;
    }
    inline size_t capacity() const
    {
        return capacity_;
    }
    inline size_t bytes() const
    {
        return size()*sizeof(value_type);
    }

    void fill(value_type val)
    {
        //std::fill_n(data_, size(), val);
        assert(data_);
        for(unsigned int i=0; i<size(); ++i) data_[i]=val;
    }

    void swap(TArray1D<T>& a)
    {
        std::swap(size_, a.size_);
        std::swap(capacity_, a.capacity_);
        std::swap(data_, a.data_);
    }

    inline const_iterator begin() const
    {
        return data_;
    }
    inline const_iterator end() const
    {
        return data_+size();
    }
    inline iterator begin()
    {
        return data_;
    }
    inline iterator end()
    {
        return data_+size();
    }

    inline const T& operator [] (size_t i) const
    {
        return data_[checkedIndex(i)];
    }
    inline T& operator [] (size_t i)
    {
        return data_[checkedIndex(i)];
    }

    inline T& at(size_t i)
    {
        return data_[checkedIndex(i)];
    }

    inline const T* data() const
    {
        return data_;
    }
    inline T* data()
    {
        return data_;
    }
    void resize(size_t width)
    {
        resize(width,0);
    }
    void reserve(size_t cap)
    {
        if(cap<size_) cap=size_;
        if(cap!=capacity_)
        {
            T* newbuf=0;
            capacity_=cap;
            if(capacity_)
            {
                newbuf=new value_type[capacity_];
                std::copy(begin(), end(), newbuf);
                std::swap(data_, newbuf);
                delete[] newbuf;
            }
        }
    }
    inline bool empty()
    {
        return size_==0;
    }
    inline void push_back(const T& value)
    {
        resize(size_+1, &value);
    }
    const T& front() const
    {
        assert(size_);
        return data_[0];
    }
    const T& back() const
    {
        assert(size_);
        return (data_+size_)[0];
    }

private:

    void resize(size_t size, const T* src)
    {
        if(size>=size_)
        {
            if(size>capacity_)
            {
                if(!capacity_) capacity_=size;
                else
                {
                    while(capacity_<size) capacity_+=(capacity_+1) >> 1;
                }

                T* newbuf=new value_type[capacity_];
                std::copy(begin(), end(), newbuf);
                std::swap(data_, newbuf);
                delete[] newbuf;
            }
            if(src) std::copy(src,src+(size-size_),data_+size_);
        }
        size_=size;
    }

    inline size_t checkedIndex(size_t s)
    {
        assert(s<size_);
        return s;
    }

    size_t size_, capacity_;
    T* data_;
};
};

#endif

