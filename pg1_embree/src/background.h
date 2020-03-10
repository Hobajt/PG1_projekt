#pragma once

#include <string>
#include <memory>
#include <array>
#include "structs.h"
#include "vector3.h"

class Texture;

class Background {
public:
	virtual ~Background() = default;

	virtual clr3f GetPixelColor(vec3f& rayDir) const = 0;
protected:
	static float M_1_2PI;
};

class BackgroundStatic : public Background {
public:
	BackgroundStatic(const clr3f& color);
	virtual clr3f GetPixelColor(vec3f& rayDir) const override;
public:
	clr3f color;
};

class BackgroundSkydome : public Background {
public:
	BackgroundSkydome(const std::string& texturePath);

	virtual clr3f GetPixelColor(vec3f& rayDir) const override;
private:
	std::unique_ptr<Texture> texture;
};

class BackgroundSkybox : public Background {
public:
	BackgroundSkybox(const std::string& texturePathBase);

	virtual clr3f GetPixelColor(vec3f& rayDir) const override;
private:
	std::array<std::unique_ptr<Texture>, 6> textures;
};