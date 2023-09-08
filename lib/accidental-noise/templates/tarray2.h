#ifndef T_ARRAY2_H
#define T_ARRAY2_H

#include <cassert>
#include <algorithm>

namespace anl
{
template<typename T>
class TArray2D
{
public:
    typedef T value_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

    TArray2D(size_t width=1, size_t height=1) : width_(0), height_(0), data_(0)
    {
        resize(width, height);
    }

    TArray2D(const TArray2D<T>& a)
    {
        resize(a.width(), a.height());
        std::copy(a.begin(), a.end(), data_);
    }

    TArray2D& operator = (const TArray2D<T>& a)
    {
        resize(a.width(), a.height());
        std::copy(a.begin(), a.end(), data_);
        return *this;
    }

    ~TArray2D()
    {
        delete[] data_;
    }

    inline size_t width() const
    {
        return width_;
    }
    inline size_t height() const
    {
        return height_;
    }
    inline size_t size() const
    {
        return width_*height_;
    }
    inline size_t bytes() const
    {
        return size()*sizeof(value_type);
    }

    void fill(value_type val)
    {
        std::uninitialized_fill_n(data_, size(), val);
    }

    void swap(TArray2D<T>& a)
    {
        std::swap(width_, a.width_);
        std::swap(height_, a.height_);
        std::swap(data_, a.data_);
    }

    void resize(size_t width, size_t height)
    {
        size_t nelements=width*height;
        assert(nelements>0);

        if(data_!=0)
        {
            if(width==width_ && height==height_) return;
            delete[] data_;
            data_=0;
        }

        data_=new value_type[nelements];
        width_=width;
        height_=height;
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

    inline const T& operator () (size_t i) const
    {
        return data_[checkedIndex(i)];
    }
    inline const T& operator () (size_t i, size_t j) const
    {
        return data_[checkedIndex(i,j)];
    }
    inline T& operator () (size_t i)
    {
        return data_[checkedIndex(i)];
    }
    inline T& operator () (size_t i, size_t j)
    {
        return data_[checkedIndex(i,j)];
    }

    inline const T* c_data() const
    {
        return data_;
    }
    inline T* c_data()
    {
        return data_;
    }

private:

    size_t checkedIndex(size_t i) const
    {
        assert(i<size());
        return i;
    }

    size_t checkedIndex(size_t i, size_t j) const
    {
        size_t s=width_*i+j;
        assert(s<size());
        return s;
    }

    size_t width_, height_;
    T* data_;
};
};

#endif
