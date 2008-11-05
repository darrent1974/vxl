// This is brl/bbas/imesh/imesh_operations.h
#ifndef imesh_operations_h_
#define imesh_operations_h_
//:
// \file
// \brief Operations on meshes
// \author Matt Leotta (mleotta@lems.brown.edu)
// \date May 5, 2008
//
// \verbatim
//  Modifications
//   <none yet>
// \endverbatim


#include "imesh_mesh.h"
#include <vcl_set.h>


//: Subdivide mesh faces into triangle
vcl_auto_ptr<imesh_regular_face_array<3> >
imesh_triangulate(const imesh_face_array_base& faces);

//: Subdivide quadrilaterals into triangle
vcl_auto_ptr<imesh_regular_face_array<3> >
imesh_triangulate(const imesh_regular_face_array<4>& faces);


//: Triangulate the faces of the mesh (in place)
void
imesh_triangulate(imesh_mesh& mesh);


//: Subdivide faces into quadrilaterals (in place)
//  Add a vertex at the center of each edge
//  And a vertex at the center of each face
void
imesh_quad_subdivide(imesh_mesh& mesh);

//: Subdivide faces into quadrilaterals (in place)
//  Add a vertex at the center of each edge
//  And a vertex at the center of each face
//  Only subdivide the selected faces
void
imesh_quad_subdivide(imesh_mesh& mesh, const vcl_set<unsigned int>& sel_faces);


//: Extract a sub-mesh containing only the faces listed in sel_faces
imesh_mesh
imesh_submesh_from_faces(const imesh_mesh& mesh, const vcl_set<unsigned int>& sel_faces);


//: Flip the orientation of all mesh faces
void imesh_flip_faces( imesh_mesh& mesh );

//: Flip the orientation of the selected faces
void imesh_flip_faces( imesh_mesh& mesh, const vcl_set<unsigned int>& sel_faces);


//: Compute the dual mesh using face centroids for vertices
imesh_mesh dual_mesh(const imesh_mesh& mesh);


#endif // imesh_operations_h_
