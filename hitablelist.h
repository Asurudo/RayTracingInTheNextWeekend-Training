#ifndef HITABLELISTH
#define HITABLELISTH

#include "hitable.h"
#include "sphere.h"
#include <vector>

class hitable_list
{
    public:
        std::vector<sphere*> list;
        hitable_list() {}
        hitable_list(const std::vector<sphere*>& l): list(l) {};
        hitable_list& operator=(const hitable_list &world){
	        list = world.list;
            return *this;
        }
        virtual bool hitanything(const ray& r, double tmin, double tmax, hit_record& rec) const;
};

bool hitable_list::hitanything(const ray& r, double tmin, double tmax, hit_record& rec) const
{
    hit_record tempRec;
    bool hitAnything = false;
    // closestSoFar 保证每次查询的最远距离：如果先遇到近的以后就不会考虑远的，从而不会把远近搞错。
    double closestSoFar = tmax;
    for(const auto& h: list)
        if(h->hit(r, tmin, closestSoFar, tempRec))
        {
            hitAnything = true;
            closestSoFar = tempRec.t;
            rec = tempRec;
        }
    return hitAnything;
}

#endif