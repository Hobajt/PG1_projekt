#include "stdafx.h"
#include "vertex.h"

Vertex::Vertex( const vec3f position, const vec3f normal, vec3f color, Coord2f * texture_coords )
{
	this->position = position;
	this->normal = normal;
	this->color = color;

	if ( texture_coords != NULL )
	{
		for ( int i = 0; i < NO_TEXTURE_COORDS; ++i )
		{
			this->texture_coords[i] = texture_coords[i];
		}
	}	
}
