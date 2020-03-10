#ifndef VERTEX_H_
#define VERTEX_H_

#include "structs.h"
#include "vector3.h"

/*! \def NO_TEXTURE_COORDS
\brief Po�et texturovac�ch sou�adnic.
*/
#define NO_TEXTURE_COORDS 1

//class Surface;

/*! \struct Vertex
\brief Struktura popisuj�c� v�echny atributy vertexu.

\author Tom� Fabi�n
\version 1.0
\date 2013
*/
struct /*ALIGN*/ Vertex
{
public:
	vec3f position; /*!< Pozice vertexu. */
	vec3f normal; /*!< Norm�la vertexu. */
	vec3f color; /*!< RGB barva vertexu <0, 1>^3. */
	Coord2f texture_coords[NO_TEXTURE_COORDS]; /*!< Texturovac� sou�adnice. */
	vec3f tangent; /*!< Prvn� osa sou�adn�ho syst�mu tangenta-bitangenta-norm�la. */

	char pad[8]; // dopln�n� na 64 byt�, m�lo by to m�t alespo� 4 byty, aby se sem ve�el 32-bitov� ukazatel

	//! V�choz� konstruktor.
	/*!
	Inicializuje v�echny slo�ky vertexu na hodnotu nula.
	*/
	Vertex() { }

	//! Obecn� konstruktor.
	/*!
	Inicializuje vertex podle zadan�ch hodnot parametr�.

	\param position pozice vertexu.
	\param normal norm�la vertexu.
	\param color barva vertexu.
	\param texture_coords nepovinn� ukazatel na pole texturovac�ch sou�adnic.
	*/
	Vertex( const vec3f position, const vec3f normal, vec3f color, Coord2f * texture_coords = NULL );

	//void Print();
};

#endif
