#include "stdafx.h"
#include "application.h"

#include "vector3.h"
#include "mymath.h"

#include "options.h"

#include "raytracer.h"
#include "pathtracer.h"
#include "background.h"

void InitRaytracer() {
	Options& opt = Options::Get();
	const char* config = "threads=0,verbose=0";

	Raytracer raytracer(opt.width, opt.height, deg2rad(opt.fov), opt.viewFrom, opt.viewAt, config);
	if (!opt.noBackground)
		raytracer.SetBackground(std::make_unique<BackgroundSkydome>(opt.path_background));
	raytracer.LoadScene(opt.path_model);
	raytracer.MainLoop();
}

void InitPathtracer() {
	Options& opt = Options::Get();
	const char* config = "threads=0,verbose=0";

	Pathtracer pathtracer(opt.width, opt.height, deg2rad(opt.fov), opt.viewFrom, opt.viewAt, config);
	if(!opt.noBackground)
		pathtracer.SetBackground(std::make_unique<BackgroundSkydome>(opt.path_background));
	pathtracer.LoadScene(opt.path_model);
	pathtracer.MainLoop();
}
