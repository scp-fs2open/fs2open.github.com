namespace anl
{
CCoordinate::CCoordinate()
{
    set(0,0);
}

CCoordinate::CCoordinate(double x, double y)
{
    set(x,y);
}

CCoordinate::CCoordinate(double x, double y, double z)
{
    set(x,y,z);
}

CCoordinate::CCoordinate(double x, double y, double z, double w)
{
    set(x,y,z,w);
}

CCoordinate::CCoordinate(double x, double y, double z, double w, double u, double v)
{
    set(x,y,z,w,u,v);
}

CCoordinate::CCoordinate(const CCoordinate &c)
{
    dimension_=c.dimension_;
    x_=c.x_;
    y_=c.y_;
    z_=c.z_;
    w_=c.w_;
    u_=c.u_;
    v_=c.v_;
}

void CCoordinate::set(double x, double y)
{
    dimension_=2;
    x_=x;
    y_=y;
    z_=0;
    w_=0;
    u_=0;
    v_=0;
}

void CCoordinate::set(double x, double y, double z)
{
    dimension_=3;
    x_=x;
    y_=y;
    z_=z;
    w_=0;
    u_=0;
    v_=0;
}

void CCoordinate::set(double x, double y, double z, double w)
{
    dimension_=4;
    x_=x;
    y_=y;
    z_=z;
    w_=w;
    u_=0;
    v_=0;
}

void CCoordinate::set(double x, double y, double z, double w, double u, double v)
{
    dimension_=6;
    x_=x;
    y_=y;
    z_=z;
    w_=w;
    u_=u;
    v_=v;
}

CCoordinate CCoordinate::operator *(double rhs)
{
    CCoordinate ret(0,0);
    ret.dimension_=dimension_;
    ret.x_=x_*rhs;
    ret.y_=y_*rhs;
    ret.z_=z_*rhs;
    ret.w_=w_*rhs;
    ret.u_=u_*rhs;
    ret.v_=v_*rhs;
    return ret;
}

CCoordinate CCoordinate::operator *(CCoordinate &rhs)
{
    CCoordinate ret(0,0);
    ret.dimension_=dimension_;
    ret.x_=x_*rhs.x_;
    ret.y_=y_*rhs.y_;
    ret.z_=z_*rhs.z_;
    ret.w_=w_*rhs.w_;
    ret.u_=u_*rhs.u_;
    ret.v_=v_*rhs.v_;
    return ret;
}

CCoordinate CCoordinate::operator +(CCoordinate &rhs)
{
    CCoordinate ret(0,0);
    ret.dimension_=dimension_;
    ret.x_=x_+rhs.x_;
    ret.y_=y_+rhs.y_;
    ret.z_=z_+rhs.z_;
    ret.w_=w_+rhs.w_;
    ret.u_=u_+rhs.u_;
    ret.v_=v_+rhs.v_;
    return ret;
}

CCoordinate &CCoordinate::operator =(const CCoordinate &c)
{
    dimension_=c.dimension_;
    x_=c.x_;
    y_=c.y_;
    z_=c.z_;
    w_=c.w_;
    u_=c.u_;
    v_=c.v_;

    return *this;
}

bool CCoordinate::operator ==(const CCoordinate &c)
{
    switch(dimension_)
    {
    case 2:
        return x_==c.x_ && y_==c.y_;
        break;
    case 3:
        return x_==c.x_ && y_==c.y_ && z_==c.z_;
        break;
    case 4:
        return x_==c.x_ && y_==c.y_ && z_==c.z_ && w_==c.w_;
        break;
    default:
        return x_==c.x_ && y_==c.y_ && z_==c.z_ && w_==c.w_ && u_==c.u_ && v_==c.v_;
        break;
    };
}
};
