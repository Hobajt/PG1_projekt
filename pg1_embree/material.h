#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "vector3.h"
#include "texture.h"

#define NO_TEXTURES 4

#define IOR_AIR 1.000293f

#define IOR_WATER 1.33f

#define IOR_GLASS 1.5f

class Material {
public:
	Material();

	Material(std::string& name, const vec3f& ambient, const vec3f& diffuse, const vec3f& specular, const vec3f& emission, const float reflectivity, const float shininess, const float ior, Texture** textures = NULL, const int no_textures = 0);

	~Material();

	void set_name(const char* name);

	std::string get_name() const;

	void set_texture(const int slot, Texture* texture);

	Texture* get_texture(const int slot) const;

	static Material* GetDefault();

public:
	vec3f ambient{ vec3f{0.f, 0.f, 0.f} };
	vec3f diffuse{ vec3f{0.f, 0.f, 0.f} };
	vec3f specular{ vec3f{0.f, 0.f, 0.f} };

	vec3f emission{ vec3f{0.f, 0.f, 0.f} };
	vec3f attenuation{ vec3f{0.f, 0.f, 0.f} };

	float shininess; 

	float reflectivity; 
	float ior;

	int shader;

	static const char kDiffuseMapSlot; 
	static const char kSpecularMapSlot;
	static const char kNormalMapSlot; 
	static const char kOpacityMapSlot; 

private:
	Texture* textures_[NO_TEXTURES];

	std::string name_;

	static Material* defaultMat;
};

#endif
