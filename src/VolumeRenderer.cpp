#include "VolumeRenderer.h"
#include <Magick++.h>

VolumeRenderer::VolumeRenderer(int _width, int _height, Grid* dataSource)
	: width(_width), height(_height), volumeData(dataSource)
{
	int N = width * height * 4;

	// Allocate pixels for our output image (RGBA)
	image = new float[N];

	// Clear the image to black
	for(int i=0;i<N;++i) {
		image[i] = 0.f;
	}
}

VolumeRenderer::~VolumeRenderer() {
	// Deconstructor should clean up image.
	delete[] image;
}

void VolumeRenderer::render(const Vec3& cameraPosition, const Vec3& cameraFocus, float marchDistance, int marchingStepCount) {
	// Compute a few things for the camera set up.
	// Given the camera's position and a point to focus on,
	// we can construct the view space.
	
	Vec3 lookDirection = (cameraFocus - cameraPosition).normalized();
	Vec3 camRight = lookDirection ^ Vec3(0,1,0); // Assume there is no camera "roll".
	Vec3 camUp = camRight ^ lookDirection;

	// compute distance to the camera plane from the field of view, and aspect ratio
	const float fov = 80.f;
	float cameraPlaneDistance = 1.f / (2.f * tanf(fov*3.141592659f/180.f*.5f));
	const float aspect = (float)width / height;

	// Compute the step size for the ray marching.
	const float ds = marchDistance / marchingStepCount;
	
	// For indicating progress during render.
	int pi=0;

	// For each pixel...
	#pragma omp parallel for
	for(int px=0;px<width;++px) {
		for(int py=0;py<height;++py) {

				// Pixel index into the image array.
				int pndx = 4*(py*width+px);

				// Compute the ray direction for this pixel. Same as in standard ray tracing.
				float u = -.5f + (px+.5f) / (float)(width-1);
				u *= aspect; // Correction for square pixels on non-square image aspect ratio.
				float v =  .5f - (py+.5f) / (float)(height-1);

				Vec3 rayDirection = lookDirection * cameraPlaneDistance + camRight*u + camUp*v;
				rayDirection.normalize();

				// Initialize transmissivity to 1. We haven't "seen" anything yet.
				// Remember this is 1-alpha.
				float T = 1.f;

				// Scattering coefficient for this medium. Assumed to be homogenous.
				// More on this in later tutorials. Play with this and see what happens...
				const float kappa = 1.f;

				// We're going to cheat a little bit and store the color as a 3-component vector.
				// Code re-use!
				Vec3 finalColor(0,0,0);

				// For now, we'll assume the density has one homogenous color... White.
				Vec3 densityColor(1,1,1);

				// Distance along the ray. All rays start at the camera.
				for(float s=0.f; s<marchDistance; s += ds) {
					Vec3 rayPosition = cameraPosition + s * rayDirection;

					// Sample the density from a grid. Later, we'll play with how to render
					// things other than just grids.
					float density = volumeData->read(rayPosition);

					// Compute the amount of light that reaches this point.
					// We'll sample the light twice as coarsely as the main integral for now.
					float lightValue = sampleLighting(rayPosition, Vec3(0,1,0), ds);

					// Hint at the future... We can play with the light absorption. (You can ignore this if you want.)
					lightValue = powf(lightValue, 3.f);

					// Let's also boost the amount of light so that everything is brighter.
					lightValue *= 5;

					// Numerical integration of the path integral. Theory will be covered in
					// tutorial two.
					float dT = expf(density * -ds * kappa);
					T *= dT;
					finalColor += ((1.f-dT)*T/kappa)*densityColor*lightValue;
				}

				// Write out the output of our ray march into the image buffer.
				// We'll composite this image on top of some other color,
				// just to demonstrate
				Vec3 backgroundColor(0,0,0);
				finalColor = (1.f-T)*finalColor + T*backgroundColor;

				for(int c=0;c<3;++c) {
					image[pndx + c] = finalColor[c];
				}

				// For the purposes of this tutorial, the final alpha for this image will be 1.
				image[pndx+3] = 1.f;
			}

			// Print out some progress information
			#pragma omp critical 
			{
				printf("\rRender progress: %.02f%%", 100.f*pi++/(width-1));
				fflush(stdout);
			}
	}

	printf("\n");
}

float VolumeRenderer::sampleLighting(const Vec3& x, const Vec3& lightPosition, float stepSize) {
	Vec3 lightDirection = lightPosition - x;
	float lightDistance = lightDirection.norm();

	// Normalize the direction vector that we will now march along.
	lightDirection *= 1.f/lightDistance;

	float densitySum = 0.f;
	for(float s=0; s<lightDistance; s+=stepSize) {
		Vec3 samplePosition = x + s * lightDirection;
		densitySum += volumeData->read(	samplePosition );
	}

	return exp(-stepSize * 1.f * densitySum);
}

void VolumeRenderer::writeImage(const char *path) {
	// We'll use ImageMagick's c++ bindings here to make life way simpler.
	// This gives us support for PNGs, JPEGs, BMP, TGA, etc for free.
	Magick::Image output;
	output.read(width,height,"RGBA",Magick::FloatPixel,image);
	output.write(path);
}

