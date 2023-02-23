#ifndef T_ARRAY3_H
#define T_ARRAY3_H

#include <cassert>
#include <algorithm>

namespace anl
{
template<typename T>
class TArray3D
{
public:
    typedef T value_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

    TArray3D(size_t width=1, size_t height=1, size_t depth=1) : width_(0), height_(0), depth_(0), data_(0)
    {
        resize(width, height, depth);
    }

    TArray3D(const TArray3D<T>& a)
    {
        resize(a.width(), a.height(), a.depth());
        std::copy(a.begin(), a.end(), data_);
    }

    TArray3D& operator = (const TArray3D<T>& a)
    {
        resize(a.width(), a.height(), a.depth());
        std::copy(a.begin(), a.end(), data_);
        return *this;
    }

    ~TArray3D()
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
    inline size_t depth() const
    {
        return depth_;
    }
    inline size_t size() const
    {
        return width_*height_*depth_;
    }
    inline size_t bytes() const
    {
        return size()*sizeof(value_type);
    }

    void fill(value_type val)
    {
        std::uninitialized_fill_n(data_, size(), val);
    }

    void swap(TArray3D<T>& a)
    {
        std::swap(width_, a.width_);
        std::swap(height_, a.height_);
        std::swap(depth_, a.depth_);
        std::swap(data_, a.data_);
    }

    void resize(size_t width, size_t height, size_t depth)
    {
        size_t nelements=width*height*depth;
        assert(nelements>0);

        if(data_!=0)
        {
            if(width==width_ && height==height_ && depth==depth_) return;
            delete[] data_;
            data_=0;
        }

        data_=new value_type[nelements];
        width_=width;
        height_=height;
        depth_=depth;
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
    inline const T& operator () (size_t i, size_t j, size_t k) const
    {
        return data_[checkedIndex(i,j,k)];
    }
    inline T& operator () (size_t i)
    {
        return data_[checkedIndex(i)];
    }
    inline T& operator () (size_t i, size_t j, size_t k)
    {
        return data_[checkedIndex(i,j,k)];
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

    size_t checkedIndex(size_t i, size_t j, size_t k) const
    {
        size_t s=k*width_*depth_+width_*j+i;
        assert(s<size());
        return s;
    }

    size_t width_, height_;
    T* data_;
};
};

#endif
