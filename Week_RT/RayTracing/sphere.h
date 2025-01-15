#pragma once

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable
{
public:
	sphere(const point3& center, Double radius) : center(center), radius(std::fmax(0,radius)) {}

	// hittable을(를) 통해 상속됨
	bool hit(const ray& r, Double ray_tmin, Double ray_tmax, hit_record& rec) const override
	{
		vec3 oc = center - r.origin();
		Double a = r.direction().length_squared();
		Double h = dot(r.direction(), oc);
		Double c = oc.length_squared() - radius * radius;

		Double discriminant = h * h - a * c;
		if (discriminant < 0)
			return false;

		Double sqrtd = std::sqrt(discriminant);

		// Find the nearest root that lies in the acceptable range.
		Double root = (h - sqrtd) / a;
		if (root <= ray_tmin || ray_tmax <= root)
		{
			root = (h + sqrtd) / a;
			if (root <= ray_tmin || ray_tmax <= root)
			{
				return false;
			}
		}

		rec.t = root;
		rec.p = r.at(rec.t);
		rec.normal = (rec.p - center) / radius;

		return true;
	}


private:
	point3 center;
	Double radius;

};