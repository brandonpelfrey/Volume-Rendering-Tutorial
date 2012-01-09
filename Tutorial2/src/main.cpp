#include <cstdio>
#include "VolumeRenderer.h"
#include "Grid.h"

void populateGrid(Grid* grid, float T) {
	const int* dims = grid->getDimensions();
	for(int i=0;i<dims[0]; ++i)
		for(int j=0;j<dims[1]; ++j)
			for(int k=0;k<dims[2]; ++k) {

				Vec3 voxelPosition;
				voxelPosition.x = (float)i / (dims[0]-1) * 2.f + -1.f;
				voxelPosition.y = (float)j / (dims[1]-1) * 2.f + -1.f;
				voxelPosition.z = (float)k / (dims[2]-1) * 2.f + -1.f;

				// Fill the grid with a solid sphere with a very dense inner sphere
				float dfield2 = (voxelPosition-Vec3(0,0,0)).norm();
				float value = dfield2 < 1.f ? 1.f : 0.f;

				float inner = (voxelPosition-Vec3(0,.5f + .25f*cos(T*2*M_PI),0)).norm();
				if(inner < .15f)
					value = 20.f;

				grid->setVoxel(i,j,k,value);	
			}
}

int main(int /*argc*/, char* const /*argv*/[]) {
	Grid grid(128,128,128, Vec3(-1,-1,-1), Vec3(1,1,1));	
	VolumeRenderer renderer(512,512, &grid);

	// Render some frames with the dense occluding ball moving
	for(int frame=0;frame<100;++frame) {

		// Let's fill the grid with something to render.
		populateGrid(&grid, (float)frame / 99);

		// Compute a DSM for faster light querying.
		// Light attenuation outside of the grid is none, so default 
		// value outside of the grid is exp(0) = 1.
		Grid dsm(128,128,128, Vec3(-1,-1,-1), Vec3(1,1,1), 1);	
		printf("Computing DSM ... "); fflush(stdout);
		dsm.computeDSM(grid, 1.f, .025, Vec3(0,1,0));
		printf("done.\n"); fflush(stdout);

		// Ray marching
		renderer.render(Vec3(1,1,1), Vec3(0,0,0), 2, 128, dsm);

		// Output an image to look at.
		char frame_path[256];
		sprintf(frame_path,"frame_%04d.png",frame);
		renderer.writeImage(frame_path);
		printf("Finished rendering frame %d / 100.\n", frame+1);
	}

	return 0;
}
