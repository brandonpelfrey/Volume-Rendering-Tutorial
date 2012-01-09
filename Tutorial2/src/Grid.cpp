#include "Grid.h"
#include <cstdio>

#define GRID_INDEX(X,Y,Z) ( (X) + (Y)*res[0] + (Z)*res[0]*res[1] )

Grid::Grid(int _nx, int _ny, int _nz, Vec3 _bmin, Vec3 _bmax, float _default_value) : default_value(_default_value)
{
	bbox[0] = _bmin;
	bbox[1] = _bmax;
	bbox_size = bbox[1] - bbox[0];
	res[0] = _nx;
	res[1] = _ny;
	res[2] = _nz;
	int N = res[0] * res[1] * res[2];

	printf("Allocating grid of size %dx%dx%d (%.02f MB)...\n", 
		_nx,_ny,_nz, N*sizeof(float)/(1024.f*1024.f));

	data = new float[N];
}

void Grid::setVoxel(int x, int y, int z, float value) {
	data[GRID_INDEX(x,y,z)] = value;
}

void Grid::computeDSM(const Grid& density, const float kappa, const float ds, const Vec3& lightPosition) {

	// Repeated calculation brought out of the loop
	const Vec3 step = bbox_size.cdiv(Vec3(res[0]-1,res[1]-1,res[2]-1));

	// For each voxel...
	#pragma omp parallel for
	for(int i=0;i<res[0];++i)
		for(int j=0;j<res[1];++j)
			for(int k=0;k<res[2];++k) {
				// Compute the location of this voxel
				Vec3 x = bbox[0] + step.cmul(Vec3(i+.5f,j+.5f,k+.5f));
				
				// Compute a vector to the light
				Vec3 lightDirection = lightPosition - x;

				// How far will we need to ray march?
				const float smax = lightDirection.norm();

				// Make a unit-length direction vector to step in.
				lightDirection *= 1.f / smax;

				// Ray march and accumulate density from x -> light
				float density_sum = 0;
				for(float s=0; s < smax; s += ds) {
					const Vec3 sample = x + lightDirection * s;
					density_sum += density.read(sample);
				}
				setVoxel(i,j,k, expf(-kappa*ds*density_sum) );
			}
}

float Grid::read(const Vec3& x) const {
	// Note: A lot of stuff in here is usually optimized. 
	// Left here for clarity.

	// Compute the relative position inside the grid
	Vec3 relative = x - bbox[0];

	relative.x *= res[0] / (bbox_size.x);
	relative.y *= res[1] / (bbox_size.y);
	relative.z *= res[2] / (bbox_size.z);

	int i = relative.x;
	int j = relative.y;
	int k = relative.z;

	// If we're outside the grid, then return zero (as a convention).
	if(i<0 || j<0 || k<0 || i>=res[0]-1 || j>=res[1]-1 || k>=res[2]-1)
		return 0.f;

	// We're inside the box! Let's do trilinear interpolation.
	
	// Compute weights
	Vec3 w = relative - Vec3(i,j,k);
	Vec3 wi = Vec3(1.f,1.f,1.f) - w;

	return 
		data[GRID_INDEX(i  ,j  ,k  )] * wi.x * wi.y * wi.z +
		data[GRID_INDEX(i+1,j  ,k  )] * w.x * wi.y * wi.z +
		data[GRID_INDEX(i  ,j+1,k  )] * wi.x * w.y * wi.z +
		data[GRID_INDEX(i+1,j+1,k  )] * w.x * w.y * wi.z +
		data[GRID_INDEX(i  ,j  ,k+1)] * wi.x * wi.y * w.z +
		data[GRID_INDEX(i+1,j  ,k+1)] * w.x * wi.y * w.z +
		data[GRID_INDEX(i  ,j+1,k+1)] * wi.x * w.y * w.z +
		data[GRID_INDEX(i+1,j+1,k+1)] * w.x * w.y * w.z ;
}
