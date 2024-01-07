#ifndef SPHEREH
#define SPHEREH

#include "hitable.h"
#include "material.h"

class sphere: public hitable
{
    public:
        // 球体的球心和半径
        vec3 center;
        double radius;
        // 材质信息指针
        material* mat_ptr;
        sphere() {}
        sphere(vec3 cen, double r, material* mptr) : mat_ptr(mptr), center(cen), radius(r) {}
        virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const override;
};

bool sphere::hit(const ray& r, double tmin, double tmax, hit_record& rec) const
{
    vec3 oc = r.origin() - center;
    double a = dot(r.direction(), r.direction());
    double b = 2*dot(oc, r.direction());
    double c = dot(oc, oc) - radius*radius;

    // 二元一次方程的delta，大于零表示两个解，即视线与球体相交
    double discriminant = b*b - 4*a*c;

    // 视线与球体有两个交点
    if(discriminant > 0.0)
    {
        // 视线与球的两个交点
        double t1 = (-b - sqrt(discriminant))/(2*a), t2 = (-b + sqrt(discriminant))/(2*a);
        if(t1 < tmax && t1 > tmin)
        {
            rec.t = t1;
            rec.p = r.point_at_parameter(rec.t);
            // 两个向量相减得到法线：相机到球体上一点 减去 相机到球心
            rec.normal = unit_vector(rec.p - center);
            rec.mat_ptr = mat_ptr;
            return true;
        }
        if(t2 < tmax && t2 > tmin)
        {
            rec.t = t2;
            rec.p = r.point_at_parameter(rec.t);
            // 两个向量相减得到法线：相机到球体上一点 减去 相机到球心
            rec.normal = unit_vector(rec.p - center);
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }
    return false;
}

#endif
