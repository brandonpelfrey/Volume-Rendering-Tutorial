#include "VolumeRenderer.h"
#include "Grid.h"

void populateGrid(Grid* grid) {
	const int* dims = grid->getDimensions();
	for(int i=0;i<dims[0]; ++i)
		for(int j=0;j<dims[1]; ++j)
			for(int k=0;k<dims[2]; ++k) {

				Vec3 voxelPosition;
				voxelPosition.x = (float)i / (dims[0]-1) * 2.f + -1.f;
				voxelPosition.y = (float)j / (dims[1]-1) * 2.f + -1.f;
				voxelPosition.z = (float)k / (dims[2]-1) * 2.f + -1.f;

				// Fill the grid with a solid sphere with a very dense inner sphere
				float dfield2 = voxelPosition.normSquared();
				float value = dfield2 < 1.f ? 1.f : 0.f;

				if(dfield2<.25f)
					value = 20.f;

				grid->setVoxel(i,j,k,value);	
			}
}

int main(int argc, char* const argv[]) {
	Grid grid(128,128,128, Vec3(-1,-1,-1), Vec3(1,1,1));	
	VolumeRenderer renderer(512,512, &grid);

	// Let's fill the grid with something to render.
	populateGrid(&grid);

	// Ray marching
	renderer.render(Vec3(1,1,1), Vec3(0,0,0), 2, 128);

	// Output an image to look at.
	renderer.writeImage("output.png");

	return 0;
}
