/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, 2018, 2019, Leiden Institute for Advanced Computer Science
 */

#pragma once

#include "matrix.h"
#include "pattern.h"
#include <map>

VOUW_NAMESPACE_BEGIN

typedef std::map<Coord2D, Matrix2D::ElementT> ErrorMapT;

double errorCodeLength( unsigned int mwidth, unsigned int mheight, int uniqueElems );

int errorCount( const Pattern& p1, const Pattern& p2 );

int errorMapDelta( ErrorMapT& map, const Pattern& from, const Pattern& to, const Coord2D& pivot ); 

VOUW_NAMESPACE_END
