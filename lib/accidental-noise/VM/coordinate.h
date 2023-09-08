#ifndef COORDINATE_H
#define COORDINATE_H

namespace anl
{
struct CCoordinate
{
    CCoordinate();
    CCoordinate(double x, double y);
    CCoordinate(double x, double y, double z);
    CCoordinate(double x, double y, double z, double w);
    CCoordinate(double x, double y, double z, double w, double u, double v);
    CCoordinate(const CCoordinate &c);

    void set(double x, double y);
    void set(double x, double y, double z);
    void set(double x, double y, double z, double w);
    void set(double x, double y, double z, double w, double u, double v);

    CCoordinate operator *(double rhs);
    CCoordinate operator *(CCoordinate &rhs);
    CCoordinate operator +(CCoordinate &rhs);
    CCoordinate &operator =(const CCoordinate &rhs);
    bool operator ==(const CCoordinate &rhs);

    double x_, y_, z_, w_, u_, v_;
    unsigned int dimension_;
};
};

#endif
