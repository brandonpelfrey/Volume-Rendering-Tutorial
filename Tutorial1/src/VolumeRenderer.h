#ifndef VolumeRenderer_h
#define VolumeRenderer_h

#include "Grid.h"
#include "Vec3.h"

class VolumeRenderer {
private:
	int width, height;
	float *image;
	Grid* volumeData;

public:
	/*! Set up a simple renderer with a certain width and height for the final image.
		* @param[in]  _width The width of the output image.
	 * @param[in]  _height The height of the output image.
	 * @param[in]  dataSource The grid from which we will read our data.
		*/
	VolumeRenderer(int _width, int _height, Grid* dataSource);
	~VolumeRenderer();

	/*! The meat of the system. This function does volume ray marching 
	 *  using approximate single scattering.
		*		@param[in] cameraPosition The position of the camera rendering the scene.
		*		@param[in] cameraFocus A point in space which will be the center of the image.
		*		@param[in] marchDistance The maximum distance that will be marched away from the camera.
		*		@param[in] marchingStepCount The number of steps to march along the path from the camera into the scene.
		*/
	void render(const Vec3& cameraPosition, const Vec3& cameraFocus, float marchDistance, int marchingStepCount);

	/*!
		*	Compute the light attenuation from a point in space to a light. The result
		*	is in the range (0,1].
		*	@param[in] x The position we in question
		*	@param[in] lightPosition The position of the light in 3D space.
		*	@param[in] stepSize The step size in integrating the light attenuation.
		*/
	float sampleLighting(const Vec3& x, const Vec3& lightPosition, float stepSize);

	/*! Writes the contents of the image buffer to a file.
	 *  @param[out] path The location where the image file will be written to.
		*/
	void writeImage(const char *path);
};

#endif
