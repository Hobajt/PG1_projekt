#include "stdafx.h"
#include "application.h"

#include "vector3.h"
#include "mymath.h"

#include "options.h"

#include "raytracer.h"

void InitRaytracer() {
	Options& opt = Options::Get();
	const char* config = "threads=0,verbose=0";

	Raytracer raytracer(opt.width, opt.height, deg2rad(opt.fov), opt.viewFrom, opt.viewAt, config);
	raytracer.LoadScene(opt.path_model);
	raytracer.MainLoop();
}
