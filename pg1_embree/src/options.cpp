#include "stdafx.h"
#include "options.h"

#include <iostream>
#include <fstream>

Options Options::instance = Options::Load("res/options.cfg");

Options Options::Load(std::string_view path) {
	Options opts = {};

	std::ifstream file(path.data());
	if (file.is_open()) {
		try {
			std::string line;
			while (std::getline(file, line)) {
				std::string_view value = std::string_view(line.c_str() + line.find_first_of(' ') + 1);
				std::string_view tmp;
				if (line._Starts_with("width ")) {
					opts.width = atoi(value.data());
				}
				else if (line._Starts_with("height ")) {
					opts.height = atoi(value.data());
				}
				else if (line._Starts_with("sampleCount ")) {
					opts.sampleCount = atoi(value.data());
				}
				else if (line._Starts_with("maxDepth ")) {
					opts.maxDepth = atoi(value.data());
				}
				else if (line._Starts_with("path ")) {
					tmp = std::string_view(line.c_str() + 5);
					value = std::string_view(tmp.data() + tmp.find_first_of(' ') + 1);
					if (tmp._Starts_with("model ")) {
						opts.path_model = std::string(value);
					}
					else if (tmp._Starts_with("background ")) {
						opts.path_background = std::string(value);
					}
				}
				else if (line._Starts_with("viewFrom ")) {
					sscanf(value.data(), "%f %f %f", &opts.viewFrom.x, &opts.viewFrom.y, &opts.viewFrom.z);
				}
				else if (line._Starts_with("viewAt ")) {
					sscanf(value.data(), "%f %f %f", &opts.viewAt.x, &opts.viewAt.y, &opts.viewAt.z);
				}
				else if (line._Starts_with("light_pos ")) {
					sscanf(value.data(), "%f %f %f", &opts.omnilight_pos.x, &opts.omnilight_pos.y, &opts.omnilight_pos.z);
				}
				else if (line._Starts_with("fov ")) {
					opts.fov = atof(value.data());
				}
				else if (line._Starts_with("convertToLinear ")) {
					opts.materialToLinear = (bool)(atoi(value.data()));
				}
				else if (line._Starts_with("no_background ")) {
					opts.noBackground = (bool)(atoi(value.data()));
				}
				else if (line._Starts_with("direct_lighting ")) {
					opts.directLighting = (bool)(atoi(value.data()));
				}
				else if (line._Starts_with("use_pathtracing ")) {
					opts.usePathtraching = (bool)(atoi(value.data()));
				}
			}
			printf("Loaded options from '%s'\n", path.data());
		}
		catch (std::exception&) {
			printf("Failed during options parsing, loading default options.\n");
			opts = Options();
		}
	}
	else {
		printf("Failed to load options, using default ones.\n");
	}

	return opts;
}
