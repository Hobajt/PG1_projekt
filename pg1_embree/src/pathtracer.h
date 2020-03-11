#pragma once
#include "simpleguidx11.h"
#include "surface.h"
#include "camera.h"

#include "sumstructs.h"
#include "background.h"

#include <vector>

struct Options;

class Pathtracer : public SimpleGuiDX11 {
public:
	Pathtracer(const int width, const int height,
			  const float fov_y, const vec3f view_from, const vec3f view_at,
			  const char* config = "threads=0,verbose=3");
	~Pathtracer();

	int InitDeviceAndScene(const char* config);
	int ReleaseDeviceAndScene();
	void LoadScene(const std::string file_name);
	int Ui();
	RTCRay PrepareRay(vec3f& rOrg, vec3f& rDir);
	inline void SetBackground(std::unique_ptr<Background> bg) { background = std::move(bg); }



	clr4f get_pixel(const int x, const int y, const float t = 0.0f) override;

	//Recursive pathtracing (more straightforward)
	clr3f TraceRay(RTCRay& ray, int depth = 0, float n1 = 1.f);
	//Non-recursive (roulette might not be done properly though).
	clr3f TraceRay2(RTCRay& ray);

	//Helper methods.
	clr3f DirectLighting(IntersectionEmbree& data);
	clr3f GlassShading(IntersectionEmbree& data, int depth, float n1);
private:
	std::vector<Surface*> surfaces_;
	std::vector<Material*> materials_;

	RTCDevice device_;
	SceneData scene;
	Camera camera_;

	int samples;
	float _1_samples;
	int maxDepth;

	std::unique_ptr<Background> background;
	static clr3f defaultBackground;
};
