// Some tests for vgl_closest_point
// Peter Vanroose, 5 June 2003
#include <testlib/testlib_test.h>
#include <vgl/vgl_closest_point.h>
#include <vgl/vgl_distance.h>
#include <vgl/vgl_homg_point_2d.h>
#include <vgl/vgl_homg_line_2d.h>
#include <vgl/vgl_homg_point_3d.h>
#include <vgl/vgl_homg_plane_3d.h>
#include <vgl/vgl_homg_line_3d_2_points.h>
#include <vgl/vgl_point_2d.h>
#include <vgl/vgl_line_2d.h>
#include <vgl/vgl_point_3d.h>
#include <vgl/vgl_line_3d_2_points.h>
#include <vgl/vgl_line_segment_3d.h>
#include <vgl/vgl_plane_3d.h>
#include <vcl_iostream.h>
#include <vcl_cmath.h> // for sqrt()

static void testHomgLine2DClosestPoint()
{
  vgl_homg_point_2d<double> p, q;
  vgl_homg_line_2d<double> l;

  // test for coincident
  l.set(0,1,-1); q.set(0,1); p = vgl_closest_point(l,q);
  TEST("2D coincident test", p, q);
  TEST("2D coincident test", vgl_closest_point(q,l), q);
  TEST_NEAR("Distance test", vgl_distance(l,q), 0.0, 1e-8);
  TEST_NEAR("Distance test", vgl_distance(q,l), 0.0, 1e-8);
  q.set(5,1); p = vgl_closest_point(l,q);
  TEST("2D coincident test", p, q);

  // test for non-coincident
  q.set(0,2); p.set(0,1);
  TEST("2D non-coincident test", vgl_closest_point(l,q), p);
  TEST_NEAR("Distance test", vgl_distance(l,q), 1.0, 1e-8);
  q.set(5,2); p.set(5,1);
  TEST("2D non-coincident test", vgl_closest_point(q,l), p);
  TEST_NEAR("Distance test", vgl_distance(q,l), 1.0, 1e-8);
}

// Test for closest points on two 3D lines, by Brendan McCane
static void testHomgLine3DClosestPoints()
{
  vgl_homg_point_3d<double> p1, p2;
  vgl_homg_line_3d_2_points<double> l1;
  vgl_homg_line_3d_2_points<double> l2;
  vcl_pair<vgl_homg_point_3d<double>, vgl_homg_point_3d<double> > pts;

  // test for parallel lines
  p1.set(0,1,0); p2.set(1,0,0,0); l1.set(p1, p2);
  p1.set(1,4,4); p2.set(1,0,0,0); l2.set(p1, p2);
  pts = vgl_closest_points(l1, l2);
  // result should be (1,0,0,0) for both pts
  if (pts.first!=pts.second)
    vcl_cout << "parallel test failed, points should be equal.\n"
             << "points are: " << pts.first << ' ' << pts.second
             << vcl_endl;
  else if (pts.first!=p2)
    vcl_cout << "parallel test failed, points should be " << p2 << '\n'
             << "points are: " << pts.first << ' ' << pts.second
             << vcl_endl;
  TEST("Parallel test", pts.first==pts.second && pts.first==p2, true);
  TEST_NEAR("Parallel distance test", vgl_distance(l1,l2), 5.0, 1e-8);
  TEST_NEAR("Parallel distance test", vgl_distance(l1,p1), 5.0, 1e-8);
  p1.set(0,1,0);
  TEST_NEAR("Parallel distance test", vgl_distance(p1,l2), 5.0, 1e-8);

  // test for intersecting lines.
  p1.set(0,0,0); p2.set(1,1,1); l1.set(p1,p2);
  p1.set(2,0,0); p2.set(1,1,1); l2.set(p1,p2);
  pts = vgl_closest_points(l1, l2);
  // result should be (1,1,1) for both pts
  if (pts.first != pts.second)
    vcl_cout << "Intersect test failed, points should be equal\n"
             << "points are: " << pts.first << ' ' << pts.second
             << vcl_endl;
  else if (pts.first != p2)
    vcl_cout << "Intersect test failed, points should be " << p2 << '\n'
             << "points are: " << pts.first << ' ' << pts.second
             << vcl_endl;
  TEST("Intersect test", pts.first==pts.second && pts.first==p2, true);
  TEST_NEAR("Intersect distance test", vgl_distance(l1,l2), 0.0, 1e-8);

  // now test for skew lines
  //
  // The lines are diagonals on neighbouring faces of a unit cube.
  // The diagonals are chosen so they do not meet. The closest
  // distance between the two lines is 1/sqrt(3). There is actually
  // a way of visualising the problem that makes the answer obvious
  // (see visualisation course by Geoff Wyvill and Bob Parslow).

  p1.set(0,0,0); p2.set(1,1,0); l1.set(p1,p2);
  p1.set(1,0,0); p2.set(1,1,1); l2.set(p1,p2);
  pts = vgl_closest_points(l1,l2);
  vcl_cout << "Closest points are: " << pts.first << ' ' << pts.second << '\n';
  TEST("Skew lines test", pts.first, vgl_homg_point_3d<double>(2,2,0,3));
  TEST("Skew lines test", pts.second,vgl_homg_point_3d<double>(3,1,1,3));
  TEST_NEAR("Skew lines distance test", vgl_distance(pts.first,pts.second),1/vcl_sqrt(3.0),1e-8);
  TEST_NEAR("Skew lines distance test", vgl_distance(l1,pts.second),1/vcl_sqrt(3.0),1e-8);
  TEST_NEAR("Skew lines distance test", vgl_distance(pts.first,l2),1/vcl_sqrt(3.0),1e-8);
}

static void testHomgPlane3DClosestPoint()
{
  vgl_homg_point_3d<double> p, q;
  vgl_homg_plane_3d<double> l;

  // test for coincident
  l.set(0,1,0,-1); q.set(0,1,2); p = vgl_closest_point(l,q);
  TEST("3D coincident test", p, q);
  TEST("3D coincident test", vgl_closest_point(q,l), q);
  TEST_NEAR("Distance test", vgl_distance(l,q), 0.0, 1e-8);
  TEST_NEAR("Distance test", vgl_distance(q,l), 0.0, 1e-8);
  q.set(5,1,3); p = vgl_closest_point(l,q);
  TEST("3D coincident test", p, q);

  // test for non-coincident
  q.set(0,2,3); p.set(0,1,3);
  TEST("3D non-coincident test", vgl_closest_point(l,q), p);
  TEST_NEAR("Distance test", vgl_distance(l,q), 1.0, 1e-8);
  q.set(5,2,3); p.set(5,1,3);
  TEST("3D non-coincident test", vgl_closest_point(q,l), p);
  TEST_NEAR("Distance test", vgl_distance(q,l), 1.0, 1e-8);
}

static void testLine2DClosestPoint()
{
  vgl_point_2d<double> p, q;
  vgl_line_2d<double> l;

  // test for coincident
  l.set(0,1,-1); q.set(0,1); p = vgl_closest_point(l,q);
  TEST("2D coincident test", p, q);
  TEST("2D coincident test", vgl_closest_point(q,l), q);
  TEST_NEAR("Distance test", vgl_distance(l,q), 0.0, 1e-8);
  TEST_NEAR("Distance test", vgl_distance(q,l), 0.0, 1e-8);
  q.set(5,1); p = vgl_closest_point(l,q);
  TEST("2D coincident test", p, q);

  // test for non-coincident
  q.set(0,2); p.set(0,1);
  TEST("2D non-coincident test", vgl_closest_point(l,q), p);
  TEST_NEAR("Distance test", vgl_distance(l,q), 1.0, 1e-8);
  q.set(5,2); p.set(5,1);
  TEST("2D non-coincident test", vgl_closest_point(q,l), p);
  TEST_NEAR("Distance test", vgl_distance(q,l), 1.0, 1e-8);
}

static void testLine3DClosestPoint()
{
  vgl_point_3d<double> p, q;
  vgl_line_3d_2_points<double> l;

  // test for coincident
  p.set(3,4,-1); q.set(-3,-2,5); l.set(p,q);
  q.set(0,1,2); p = vgl_closest_point(l,q);
  TEST("3D coincident test", p, q);
  TEST("3D coincident test", vgl_closest_point(q,l), q);
  TEST_NEAR("Distance test", vgl_distance(l,q), 0.0, 1e-8);
  TEST_NEAR("Distance test", vgl_distance(q,l), 0.0, 1e-8);

  // test for non-coincident
  q.set(0,2,7); p.set(0,5,3); l.set(p,vgl_point_3d<double>(0,1,0));
  TEST("3D non-coincident test", vgl_closest_point(l,q), p);
  TEST_NEAR("Distance test", vgl_distance(l,q), 5.0, 1e-8);
  q.set(1,2,7);
  TEST("3D non-coincident test", vgl_closest_point(q,l), p);
  TEST_NEAR("Distance test", vgl_distance(q,l), vcl_sqrt(26.0), 1e-8);
}

// Similar to test above, but uses parametric value.
static void test_line_3d_2_points_closest_point_t()
{
  vcl_cout << "-------------------------------------------------- \n";
  vcl_cout << "Testing vgl_closest_point_t(line_3d_2_points): \n";
  vcl_cout << "-------------------------------------------------- \n";

  vgl_point_3d<double> p, q, r;
  vgl_line_3d_2_points<double> l;
  double t;

  // test for coincident
  p.set(3,4,-1); q.set(-3,-2,5); l.set(p,q);
  r.set(0,1,2);  // midpt of line
  t = vgl_closest_point_t(l,r);
  TEST_NEAR("3D coincident test", t, 0.5, 1e-8);

  // test for non-coincident
  p.set(0,0,0); q.set(1,0,0); l.set(p,q);
  r.set(0.5, 2, 3); 
  t = vgl_closest_point_t(l,r);
  TEST_NEAR("3D non-coincident test", t, 0.5, 1e-8);
}

static void testPlane3DClosestPoint()
{
  vcl_cout << "-------------------------------------------------- \n";
  vcl_cout << "Testing vgl_closest_point(vgl_plane_3d): \n";
  vcl_cout << "-------------------------------------------------- \n";

  vgl_point_3d<double> p, q;
  vgl_plane_3d<double> l;

  // test for coincident
  l.set(0,1,0,-1); q.set(0,1,2); p = vgl_closest_point(l,q);
  TEST("3D coincident test", p, q);
  TEST("3D coincident test", vgl_closest_point(q,l), q);
  TEST_NEAR("Distance test", vgl_distance(l,q), 0.0, 1e-8);
  TEST_NEAR("Distance test", vgl_distance(q,l), 0.0, 1e-8);
  q.set(5,1,3); p = vgl_closest_point(l,q);
  TEST("3D coincident test", p, q);

  // test for non-coincident
  q.set(0,2,3); p.set(0,1,3);
  TEST("3D non-coincident test", vgl_closest_point(l,q), p);
  TEST_NEAR("Distance test", vgl_distance(l,q), 1.0, 1e-8);
  q.set(5,2,3); p.set(5,1,3);
  TEST("3D non-coincident test", vgl_closest_point(q,l), p);
  TEST_NEAR("Distance test", vgl_distance(q,l), 1.0, 1e-8);
}

static void testPoly2DClosestPoint()
{
  double poly_x[] = {0.0, 0.0, 2.0, 2.0};
  double poly_y[] = {0.0, 1.0, 1.0, 0.0}; // rectangle
  double px,py;

  // test for coincident with non-closed polygon
  int idx = vgl_closest_point_to_non_closed_polygon(px,py, poly_x,poly_y,4, 1.5,1.0);
  TEST("2D non-closed polygon coincident test: index", idx, 1);
  TEST("2D non-closed polygon coincident test: point x", px, 1.5);
  TEST("2D non-closed polygon coincident test: point y", py, 1.0);
  TEST_NEAR("Distance test", vgl_distance_to_non_closed_polygon(poly_x,poly_y,4, 1.5,1.0), 0.0, 1e-8);

  // test for non-coincident with non-closed polygon
  idx = vgl_closest_point_to_non_closed_polygon(px,py, poly_x,poly_y,4, 1.5,0.25);
  TEST("2D non-closed polygon non-coincident test: index", idx, 2);
  TEST("2D non-closed polygon non-coincident test: point x", px, 2.0);
  TEST("2D non-closed polygon non-coincident test: point y", py, 0.25);
  TEST_NEAR("Distance test", vgl_distance_to_non_closed_polygon(poly_x,poly_y,4, 1.5,0.25), 0.5, 1e-8);

  // test for coincident with closed polygon
  idx = vgl_closest_point_to_closed_polygon(px,py, poly_x,poly_y,4, 1.5,1.0);
  TEST("2D closed polygon coincident test: index", idx, 1);
  TEST("2D closed polygon coincident test: point x", px, 1.5);
  TEST("2D closed polygon coincident test: point y", py, 1.0);
  TEST_NEAR("Distance test", vgl_distance_to_closed_polygon(poly_x,poly_y,4, 1.5,1.0), 0.0, 1e-8);

  // test for non-coincident with closed polygon
  idx = vgl_closest_point_to_closed_polygon(px,py, poly_x,poly_y,4, 1.5,0.25);
  TEST("2D closed polygon non-coincident test: index", idx, 3);
  TEST("2D closed polygon non-coincident test: point x", px, 1.5);
  TEST("2D closed polygon non-coincident test: point y", py, 0.0);
  TEST_NEAR("Distance test", vgl_distance_to_closed_polygon(poly_x,poly_y,4, 1.5,0.25), 0.25, 1e-8);
}

static void testPoly3DClosestPoint()
{
  double poly_x[] = {0.0, 0.0, 2.0, 2.0};
  double poly_y[] = {0.0, 0.0, 2.0, 2.0};
  double poly_z[] = {0.0, 1.0, 1.0, 0.0}; // rectangle
  double px,py,pz;

  // test for coincident with non-closed polygon
  int idx = vgl_closest_point_to_non_closed_polygon(px,py,pz, poly_x,poly_y,poly_z,4, 1.5,1.5,1.0);
  TEST("3D non-closed polygon coincident test: index", idx, 1);
  TEST("3D non-closed polygon coincident test: point x", px, 1.5);
  TEST("3D non-closed polygon coincident test: point y", py, 1.5);
  TEST("3D non-closed polygon coincident test: point z", pz, 1.0);
  TEST_NEAR("Distance test", vgl_distance_to_non_closed_polygon(poly_x,poly_y,poly_z,4, 1.5,1.5,1.0), 0.0, 1e-8);

  // test for non-coincident with non-closed polygon
  idx = vgl_closest_point_to_non_closed_polygon(px,py,pz, poly_x,poly_y,poly_z,4, 1.5,1.5,0.25);
  TEST("3D non-closed polygon non-coincident test: index", idx, 2);
  TEST("3D non-closed polygon non-coincident test: point x", px, 2.0);
  TEST("3D non-closed polygon non-coincident test: point y", py, 2.0);
  TEST("3D non-closed polygon non-coincident test: point z", pz, 0.25);
  TEST_NEAR("Distance test", vgl_distance_to_non_closed_polygon(poly_x,poly_y,poly_z,4, 1.5,1.5,0.25), vcl_sqrt(0.5), 1e-8);

  // test for coincident with closed polygon
  idx = vgl_closest_point_to_closed_polygon(px,py,pz, poly_x,poly_y,poly_z,4, 1.5,1.5,1.0);
  TEST("3D closed polygon coincident test: index", idx, 1);
  TEST("3D closed polygon coincident test: point x", px, 1.5);
  TEST("3D closed polygon coincident test: point y", py, 1.5);
  TEST("3D closed polygon coincident test: point z", pz, 1.0);
  TEST_NEAR("Distance test", vgl_distance_to_closed_polygon(poly_x,poly_y,poly_z,4, 1.5,1.5,1.0), 0.0, 1e-8);

  // test for non-coincident with closed polygon
  idx = vgl_closest_point_to_closed_polygon(px,py,pz, poly_x,poly_y,poly_z,4, 1.5,1.5,0.25);
  TEST("3D closed polygon non-coincident test: index", idx, 3);
  TEST("3D closed polygon non-coincident test: point x", px, 1.5);
  TEST("3D closed polygon non-coincident test: point y", py, 1.5);
  TEST("3D closed polygon non-coincident test: point z", pz, 0.0);
  TEST_NEAR("Distance test", vgl_distance_to_closed_polygon(poly_x,poly_y,poly_z,4, 1.5,1.5,0.25), 0.25, 1e-8);
}

// Test for closest points on two 3D (non-homogeneous) lines
static void testLine3DClosestPoints()
{
  vcl_cout << "-------------------------------------------------- \n";
  vcl_cout << "Testing vgl_closest_points(3D lines, non-homog): \n";
  vcl_cout << "-------------------------------------------------- \n";

  // Test general case of non-parallel, non-intersecting lines
  {
    vgl_line_3d_2_points<double> l1(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(1,0,0));
    vgl_line_3d_2_points<double> l2(vgl_point_3d<double>(0,0,1), vgl_point_3d<double>(0,1,1));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(0,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,1) &&
                    u==true);
    TEST("Non-parallel, non-intersecting", success, true);
  }

  // Test common case of non-parallel, intersecting lines
  {
    vgl_line_3d_2_points<double> l1(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(1,0,0));
    vgl_line_3d_2_points<double> l2(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(0,1,0));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(0,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,0) &&
                    u==true);
    TEST("Non-parallel, intersecting", success, true);
  }

  // Test special case of parallel, non-collinear lines
  {
    vgl_line_3d_2_points<double> l1(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(1,0,0));
    vgl_line_3d_2_points<double> l2(vgl_point_3d<double>(0,0,1), vgl_point_3d<double>(1,0,1));
    bool u=true;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(0,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,1) &&
                    u==false);
    TEST("Parallel, non-collinear", success, true);
  }

  // Test special case of collinear lines
  {
    vgl_line_3d_2_points<double> l1(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(1,0,0));
    vgl_line_3d_2_points<double> l2(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(1,0,0));
    bool u=true;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(0,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,0) &&
                    u==false);
    TEST("Collinear", success, true);
  }
}

// Test for closest points on two 3D line segments
static void testLineSegment3DClosestPoints()
{
  vcl_cout << "-------------------------------------------------- \n";
  vcl_cout << "Testing vgl_closest_points(3D line segments): \n";
  vcl_cout << "-------------------------------------------------- \n";
  
  // Test general case of non-parallel, non-intersecting lines, with internal points closest
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(-1,0,0), vgl_point_3d<double>(1,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(0,-1,1), vgl_point_3d<double>(0,1,1));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(0,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,1) &&
                    u==true);
    TEST("Non-parallel, non-intersecting, internal points", success, true);
  }
  
  // Test general case of non-parallel, non-intersecting lines, with end points closest
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(1,0,0), vgl_point_3d<double>(2,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(0,1,1), vgl_point_3d<double>(0,2,1));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(1,0,0) && 
                    c.second==vgl_point_3d<double>(0,1,1) &&
                    u==true);
    TEST("Non-parallel, non-intersecting, end points", success, true);
  }

  // Test common case of non-parallel, non-intersecting lines, with 1 end point/ 1 internal point
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(1,0,0), vgl_point_3d<double>(2,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(0,-1,1), vgl_point_3d<double>(0,+1,1));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(1,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,1) &&
                    u==true);
    TEST("Non-parallel, non-intersecting, endpoint/internal", success, true);
  }

  // Test common case of non-parallel, intersecting lines
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(1,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(0,1,0));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(0,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,0) &&
                    u==true);
    TEST("Non-parallel, intersecting", success, true);
  }
  
  // Test special case of parallel, non-collinear lines with endpoints closest
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(1,0,0), vgl_point_3d<double>(2,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(-2,0,1), vgl_point_3d<double>(-1,0,1));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(1,0,0) && 
                    c.second==vgl_point_3d<double>(-1,0,1) &&
                    u==true);
    TEST("Parallel, non-collinear, endpoints", success, true);
  }
  
  // Test special case of parallel, non-collinear lines with non-unique internal points closest
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(2,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(-2,0,1), vgl_point_3d<double>(1,0,1));
    bool u=true;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(0,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,1) &&
                    u==false);
    TEST("Parallel, non-collinear, non-unique internal points", success, true);
  }

  // Test special case of collinear lines, non-overlapping
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(1,0,0), vgl_point_3d<double>(2,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(-2,0,0), vgl_point_3d<double>(-1,0,0));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(1,0,0) && 
                    c.second==vgl_point_3d<double>(-1,0,0) &&
                    u==true);
    TEST("Collinear, non-overlapping", success, true);
  }

  // Test special case of collinear lines, touching at endpoints
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(0,0,0), vgl_point_3d<double>(2,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(-2,0,0), vgl_point_3d<double>(0,0,0));
    bool u=false;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(0,0,0) && 
                    c.second==vgl_point_3d<double>(0,0,0) &&
                    u==true);
    TEST("Collinear, touching at endpoints", success, true);
  }
  
  // Test special case of collinear lines, overlapping
  {
    vgl_line_segment_3d<double> l1(vgl_point_3d<double>(-1,0,0), vgl_point_3d<double>(2,0,0));
    vgl_line_segment_3d<double> l2(vgl_point_3d<double>(-2,0,0), vgl_point_3d<double>(1,0,0));
    bool u=true;
    vcl_pair<vgl_point_3d<double>, vgl_point_3d<double> > c = vgl_closest_points(l1, l2, &u);
    bool success = (c.first==vgl_point_3d<double>(-1,0,0) && 
                    c.second==vgl_point_3d<double>(-1,0,0) &&
                    u==false);
    TEST("Collinear, overlapping", success, true);
  }
}

static void test_closest_point()
{
  testHomgLine2DClosestPoint();
  testHomgLine3DClosestPoints();
  testHomgPlane3DClosestPoint();
  testLine2DClosestPoint();
  testLine3DClosestPoint();
  test_line_3d_2_points_closest_point_t();
  testPlane3DClosestPoint();
  testPoly2DClosestPoint();
  testPoly3DClosestPoint();
  testLine3DClosestPoints();
  testLineSegment3DClosestPoints();
}

TESTMAIN(test_closest_point);
