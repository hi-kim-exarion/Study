#pragma once

#include "ray.h"

class hit_record
{
public:
	point3 p;
	vec3 normal;
	Double t;
};

class hittable
{
public:
	virtual ~hittable() = default;
	virtual bool hit(const ray& r, Double ray_tmin, Double ray_tmax, hit_record& rec) const = 0;
};
