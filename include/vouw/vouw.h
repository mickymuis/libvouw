/*
 * VOUW - Spatial, compression-based pattern mining on matrices
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2018, Leiden Institute for Advanced Computer Science
 */

#pragma once

#define VOUW_NAMESPACE_BEGIN namespace Vouw {
#define VOUW_NAMESPACE_END };

VOUW_NAMESPACE_BEGIN

const double pseudoCount =.5;

/** Returns the length of the Universal Code for Integers
  * It is defined as log2*(n) + log2(c) 
  * See `Universal Prior for Integers' by Jorma Rissanen */ 
double uintCodeLength( unsigned int n );

double binom( unsigned int n, unsigned int k );


// DEPRECATED
enum DirT {
    DirNone =0,
    DirHoriz =1,
    DirVert =1<<1,
    DirDiagL = 1<<2,
    DirDiagR = 1<<3,
    DirAll = (1<<4)-1
};

VOUW_NAMESPACE_END

