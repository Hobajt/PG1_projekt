#include "stdafx.h"
#include "material.h"
#include "utils.h"
#include "surface.h"
#include "mymath.h"

bool MaterialExists(std::vector<Material*>& materials, char* material_name) {
	for (std::vector<Material*>::const_iterator iter = materials.begin();
		 iter != materials.end(); ++iter) {
		Material* material = *iter;

		if (material->get_name().compare(material_name) == 0) {
			return true;
		}
	}

	return false;
}

Texture* TextureProxy(const std::string& full_name, std::map<std::string, Texture*>& already_loaded_textures,
					  const int flip = -1, const bool single_channel = false) {
	std::map<std::string, Texture*>::iterator already_loaded_texture = already_loaded_textures.find(full_name);
	Texture* texture = NULL;
	if (already_loaded_texture != already_loaded_textures.end()) {
		texture = already_loaded_texture->second;
	}
	else {
		texture = new Texture(full_name.c_str());// , flip, single_channel);
		already_loaded_textures[full_name] = texture;
	}

	return texture;
}

int LoadMTL(const char* file_name, const char* path, std::vector<Material*>& materials) {
	FILE* file = fopen(file_name, "rt");
	if (file == NULL) {
		printf("File %s not found.\n", file_name);

		return -1;
	}

	size_t file_size = static_cast<size_t>(GetFileSize64(file_name));
	char* buffer = new char[file_size + 1]; 
	char* buffer_backup = new char[file_size + 1];

	printf("Loading materials from '%s' (%0.1f KB)...\n", file_name, file_size / 1024.0f);

	size_t number_of_items_read = fread(buffer, sizeof(*buffer), file_size, file);

	if (!feof(file) && (number_of_items_read != file_size)) {
		printf("Unexpected end of file encountered.\n");

		fclose(file);
		file = NULL;

		return -1;
	}

	buffer[number_of_items_read] = 0; 

	fclose(file); 
	file = NULL;

	memcpy(buffer_backup, buffer, file_size + 1); 

	printf("Done.\n\n");

	printf("Parsing mesh data...\n");

	char material_name[128] = { 0 };
	char image_file_name[256] = { 0 };

	const char delim[] = "\n";
	char* line = strtok(buffer, delim);

	std::map<std::string, Texture*> already_loaded_textures;

	Material* material = NULL;

	while (line != NULL) {
		if (line[0] != '#') {
			if (strstr(line, "newmtl") == line) {
				if (material != NULL) {
					material->set_name(material_name);
					if (!MaterialExists(materials, material_name)) {
						materials.push_back(material);
						printf("\r%I64u material(s)\t\t", materials.size());
					}
				}
				material = NULL;

				sscanf(line, "%*s %s", &material_name);			

				material = new Material();
			}
			else {
				char* tmp = Trim(line);
				if (strstr(tmp, "Ka") == tmp) // ambient color of the material
				{
					sscanf(tmp, "%*s %f %f %f", &material->ambient.x, &material->ambient.y, &material->ambient.z);
				}
				if (strstr(tmp, "Kd") == tmp) // diffuse color of the material
				{
					sscanf(tmp, "%*s %f %f %f", &material->diffuse.x, &material->diffuse.y, &material->diffuse.z);
				}
				if (strstr(tmp, "At") == tmp) // ATTENUATION
				{
					sscanf(tmp, "%*s %f %f %f", &material->attenuation.x, &material->attenuation.y, &material->attenuation.z);
				}
				if (strstr(tmp, "Ks") == tmp) // specular color of the material
				{
					sscanf(tmp, "%*s %f %f %f", &material->specular.x, &material->specular.y, &material->specular.z);
				}
				if (strstr(tmp, "Ke") == tmp)
				{
					sscanf(tmp, "%*s %f %f %f", &material->emission.x, &material->emission.y, &material->emission.z);
				}
				if (strstr(tmp, "Ns") == tmp) 
				{
					sscanf(tmp, "%*s %f", &material->shininess);
				}
				if (strstr(tmp, "map_Kd") == tmp)
				{
					sscanf(tmp, "%*s %s", image_file_name);
					std::string full_name = std::string(path).append(image_file_name);
					material->set_texture(Material::kDiffuseMapSlot, TextureProxy(full_name, already_loaded_textures));
				}
				if (strstr(tmp, "map_Ks") == tmp) 
				{
					sscanf(tmp, "%*s %s", image_file_name);
					std::string full_name = std::string(path).append(image_file_name);
					material->set_texture(Material::kSpecularMapSlot, TextureProxy(full_name, already_loaded_textures));
				}
				if (strstr(tmp, "map_bump") == tmp) 
				{
					float bm = 0;
					sscanf(tmp, "%*s %*s %f %s", &bm, image_file_name);
					std::string full_name = std::string(path).append(image_file_name);
					material->set_texture(Material::kNormalMapSlot, TextureProxy(full_name, already_loaded_textures));
				}
				if (strstr(tmp, "map_D") == tmp) 
				{
					sscanf(tmp, "%*s %s", image_file_name);
					std::string full_name = std::string(path).append(image_file_name);
					material->set_texture(Material::kOpacityMapSlot, TextureProxy(full_name, already_loaded_textures, -1, true));
				}
				if (strstr(tmp, "Ni") == tmp) 
				{
					sscanf(tmp, "%*s %f", &material->ior);
				}
				if (strstr(tmp, "shader") == tmp) 
				{
					sscanf(tmp, "%*s %d", &material->shader);
				}
			}
		}

		line = strtok(NULL, delim); 
	}

	if (material != NULL) {
		material->set_name(material_name);
		materials.push_back(material);
		printf("\r%I64u material(s)\t\t", materials.size());
	}
	material = NULL;

	SAFE_DELETE_ARRAY(buffer_backup);
	SAFE_DELETE_ARRAY(buffer);

	printf("\n");

	return 0;
}

int LoadOBJ(const char* file_name, std::vector<Surface*>& surfaces, std::vector<Material*>& materials,
			const bool flip_yz, const vec3f default_color) {
	FILE* file = fopen(file_name, "rt");
	if (file == NULL) {
		printf("File %s not found.\n", file_name);

		return -1;
	}

	char path[128] = { "" };
	const char* tmp = strrchr(file_name, '/');
	if (tmp != NULL) {
		memcpy(path, file_name, sizeof(char) * (tmp - file_name + 1));
	}

	/*const long long*/size_t file_size = static_cast<size_t>(GetFileSize64(file_name));
	char* buffer = new char[file_size + 1]; 
	char* buffer_backup = new char[file_size + 1];

	printf("Loading model from '%s' (%0.1f MB)...\n", file_name, file_size / sqr(1024.0f));

	size_t number_of_items_read = fread(buffer, sizeof(*buffer), file_size, file);

	if (!feof(file) && (number_of_items_read != file_size)) {
		printf("Unexpected end of file encountered.\n");

		fclose(file);
		file = NULL;

		return -1;
	}

	buffer[number_of_items_read] = 0; 

	fclose(file);
	file = NULL;

	memcpy(buffer_backup, buffer, file_size + 1); 

	printf("Done.\n\n");

	printf("Parsing material data...\n");

	char material_library[128] = { 0 };

	std::vector<std::string> material_libraries;

	const char delim[] = "\n";
	char* line = strtok(buffer, delim);

	while (line != NULL) {
		switch (line[0]) {
			case 'm': // mtllib
			{
				sscanf(line, "%*s %s", &material_library);
				printf("Material library: %s\n", material_library);
				material_libraries.push_back(std::string(path).append(std::string(material_library)));
			}
			break;
		}

		line = strtok(NULL, delim); 
	}

	memcpy(buffer, buffer_backup, file_size + 1); 

	for (int i = 0; i < static_cast<int>(material_libraries.size()); ++i) {
		LoadMTL(material_libraries[i].c_str(), path, materials);
	}

	std::vector<vec3f> vertices; 
	std::vector<vec3f> per_vertex_normals;
	std::vector<Coord2f> texture_coords;

	line = strtok(buffer, delim);

	while (line != NULL) {
		switch (line[0]) {
			case 'v': 		
			{
				switch (line[1]) {
					case ' ': 
					{
						vec3f vertex;
						if (flip_yz) {
							//float x, y, z;
							sscanf(line, "%*s %f %f %f", &vertex.x, &vertex.z, &vertex.y);
							vertex.y *= -1;
						}
						else {
							sscanf(line, "%*s %f %f %f", &vertex.x, &vertex.y, &vertex.z);
						}

						vertices.push_back(vertex);
					}
					break;

					case 'n': 
					{
						vec3f normal;
						if (flip_yz) {
							//float x, y, z;
							sscanf(line, "%*s %f %f %f", &normal.x, &normal.z, &normal.y);
							normal.y *= -1;
						}
						else {
							sscanf(line, "%*s %f %f %f", &normal.x, &normal.y, &normal.z);
						}
						normal.normalize();
						per_vertex_normals.push_back(normal);
					}
					break;

					case 't': 
					{
						Coord2f texture_coord;
						float z = 0;
						sscanf(line, "%*s %f %f %f",
							   &texture_coord.u, &texture_coord.v, &z);
						texture_coords.push_back(texture_coord);
					}
					break;
				}
			}
			break;
		}

		line = strtok(NULL, delim); 
	}

	memcpy(buffer, buffer_backup, file_size + 1); 

	printf("%I64u vertices, %I64u normals and %I64u texture coords.\n",
		   vertices.size(), per_vertex_normals.size(), texture_coords.size());
	
	char group_name[128];
	char material_name[128];
	char vertices_indices[4][8 * 3 + 2];	
	char vertex_indices[3][8];				

	std::vector<Vertex> face_vertices; 

	int no_surfaces = 0; 

	line = strtok(buffer, delim); // reset

	while (line != NULL) {
		switch (line[0]) {
			case 'g': // group
			{
				if (face_vertices.size() > 0) {
					surfaces.push_back(BuildSurface(std::string(group_name), face_vertices));
					printf("\r%I64u group(s)\t\t", surfaces.size());
					++no_surfaces;
					face_vertices.clear();

					for (int i = 0; i < static_cast<int>(materials.size()); ++i) {
						if (materials[i]->get_name().compare(material_name) == 0) {
							Surface* s = *--surfaces.end();
							s->set_material(materials[i]);
							break;
						}
					}
				}

				sscanf(line, "%*s %s", &group_name);
				//printf( "Group name: %s\n", group_name );				
			}
			break;

			case 'u': // usemtl			
			{
				sscanf(line, "%*s %s", &material_name);
				//printf( "Material name: %s\n", material_name );						
			}
			break;

			case 'f': // face
			{			
				int no_spaces = 0;
				for (int i = 0; i < static_cast<int>(strlen(line)); ++i) {
					if (line[i] == ' ') {
						++no_spaces;
					}
				}
				switch (no_spaces) {
					case 3: // triangles
						sscanf(line, "%*s %s %s %s",
							   &vertices_indices[0], &vertices_indices[1], &vertices_indices[2]);
						break;

					case 4: // quadrilaterals				
						sscanf(line, "%*s %s %s %s %s",
							   &vertices_indices[0], &vertices_indices[1], &vertices_indices[2], &vertices_indices[3]);
						break;
				}

				for (int i = 0; i < 3; ++i) {
					sscanf(vertices_indices[i], "%[0-9]/%[0-9]/%[0-9]",
						   &vertex_indices[0], &vertex_indices[1], &vertex_indices[2]);

					const int vertex_index = atoi(vertex_indices[0]) - 1;
					const int texture_coord_index = atoi(vertex_indices[1]) - 1;
					const int per_vertex_normal_index = atoi(vertex_indices[2]) - 1;

					face_vertices.push_back(Vertex(vertices[vertex_index],
												   per_vertex_normals[per_vertex_normal_index],
												   default_color, &texture_coords[texture_coord_index]));
				}

				if (no_spaces == 4) {
					const int i[] = { 0, 2, 3 };
					for (int j = 0; j < 3; ++j) {
						sscanf(vertices_indices[i[j]], "%[0-9]/%[0-9]/%[0-9]",
							   &vertex_indices[0], &vertex_indices[1], &vertex_indices[2]);

						const int vertex_index = atoi(vertex_indices[0]) - 1;
						const int texture_coord_index = atoi(vertex_indices[1]) - 1;
						const int per_vertex_normal_index = atoi(vertex_indices[2]) - 1;

						face_vertices.push_back(Vertex(vertices[vertex_index],
													   per_vertex_normals[per_vertex_normal_index],
													   default_color, &texture_coords[texture_coord_index]));
					}
				}
			}
			break;
		}

		line = strtok(NULL, delim); 
	}

	if (face_vertices.size() > 0) {
		surfaces.push_back(BuildSurface(std::string(group_name), face_vertices));
		printf("\r%I64u group(s)\t\t", surfaces.size());
		++no_surfaces;
		face_vertices.clear();

		for (int i = 0; i < static_cast<int>(materials.size()); ++i) {
			if (materials[i]->get_name().compare(material_name) == 0) {
				Surface* s = *--surfaces.end();
				s->set_material(materials[i]);
				break;
			}
		}
	}

	texture_coords.clear();
	per_vertex_normals.clear();
	vertices.clear();

	SAFE_DELETE_ARRAY(buffer_backup);
	SAFE_DELETE_ARRAY(buffer);

	printf("\nDone.\n\n");

	return no_surfaces;
}
