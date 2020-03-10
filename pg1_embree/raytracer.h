#pragma once
#include "simpleguidx11.h"
#include "surface.h"
#include "camera.h"

#include "sumstructs.h"

struct Options;

/*! \class Raytracer
\brief General ray tracer class.

\author Tomáš Fabián
\version 0.1
\date 2018
*/
class Raytracer : public SimpleGuiDX11
{
public:
	Raytracer( const int width, const int height, 
		const float fov_y, const vec3f view_from, const vec3f view_at,
		const char * config = "threads=0,verbose=3" );
	~Raytracer();

	int InitDeviceAndScene( const char * config );
	int ReleaseDeviceAndScene();
	void LoadScene( const std::string file_name );
	int Ui();


	clr4f get_pixel( const int x, const int y, const float t = 0.0f ) override;

	clr3f TraceRay(const RTCRay& ray, int depth = 0, float n1 = 1.f);

private:
	std::vector<Surface *> surfaces_;
	std::vector<Material *> materials_;

	RTCDevice device_;
	SceneData scene;
	Camera camera_;

	int samples;
	float _1_samples;
	Options* opt;
};
