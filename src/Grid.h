#ifndef Grid_h
#define Grid_h

#include "Vec3.h"

class Grid {
private:
	int res[3];
	float *data;
	Vec3 bbox[2];
	Vec3 bbox_size;

public:
	/*! Create a grid of some resolution, enclosed in the
	 *  axis-aligned bounding box of [bmin,bmax]
		*  @param[in] _nx X-axis resolution of the grid
		*  @param[in] _ny Y-axis resolution of the grid
		*  @param[in] _nz Z-axis resolution of the grid
		*  @param[in] _bmin The lower corner of the AABB for the grid
		*  @param[in] _bmax The upper corner of the AABB for the grid
		*/
	Grid(int _nx, int _ny, int _nz, Vec3 _bmin, Vec3 _bmax);

	//! Set the value of a single voxel in the grid
	void setVoxel(int x, int y, int z, float value);

	//! Read a value from the grid at a particular point
	//  in space. Returns 0.f if outside the grid.
	float read(const Vec3& x) const;

	const int* getDimensions() const { return res; }
};

#endif
