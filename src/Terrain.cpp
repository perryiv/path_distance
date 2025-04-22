////////////////////////////////////////////////////////////////////////////////
//
//	The class that calculates the distance over the terrain.
//
//	References:
//	https://doc.cgal.org/latest/AABB_tree/index.html#Chapter_Fast_Intersection_and_Distance_Computation
//
////////////////////////////////////////////////////////////////////////////////

#include "Terrain.h"
#include "Tools.h"

#include "CGAL/AABB_tree.h"
#include "CGAL/AABB_traits.h"
#include "CGAL/AABB_triangle_primitive.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

#if 0
#ifdef _DEBUG
#define USE_FAKE_DATA
#endif
#endif


////////////////////////////////////////////////////////////////////////////////
//
//	Constants used below.
//
////////////////////////////////////////////////////////////////////////////////

const double HORIZONTAL_RESOLUTION = 30;
const double VERTICAL_RESOLUTION = 11;


////////////////////////////////////////////////////////////////////////////////
//
//	Constructor.
//
////////////////////////////////////////////////////////////////////////////////

Terrain::Terrain (
	unsigned int numX,
	unsigned int numY,
	unsigned int i1,
	unsigned int j1,
	unsigned int i2,
	unsigned int j2,
	const std::string &input
) :
	_numX ( numX ),
	_numY ( numY ),
	_i1 ( i1 ),
	_j1 ( j1 ),
	_i2 ( i2 ),
	_j2 ( j2 ),
	_heights(),
	_points(),
	_triangles(),
	_plane(),
	_lines(),
	_dist ( -1 )
{
#ifdef USE_FAKE_DATA

	_heights = {
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
		1, 1, 1, 1,
	};

	_numX = 4;
	_numY = 4;
	_i1 = 1;
	_j1 = 1;
	_i2 = 3;
	_j2 = 3;

#else // Use real data.

	// Check the size.
	if ( ( _numX < 2 ) || ( _numY < 2 ) )
	{
		throw std::invalid_argument ( "Number of pixels in the x and y directions must be at least 2" );
	}

	// We can't accept the same point.
	if ( ( _i1 == _i2 ) && ( _j1 == _j2 ) )
	{
		throw std::invalid_argument ( "Path start and end points are the same" );
	}

	// Make sure the indices are within range.
	if ( ( _i1 >= _numX ) || ( _i2 >= _numX ) )
	{
		throw std::out_of_range ( "Given indices are greater than the size" );
	}

	// Open the input file in binary.
	std::ifstream in ( input.c_str(), std::ios::binary );

	// Did it open?
	if ( !in.is_open() )
	{
		std::ostringstream out;
		out << "Could not open input file: " << input;
		throw std::runtime_error ( out.str() );
	}

	// Read the file into a vector of data.
	this->_readHeightData ( in );

#endif // Use real data.
}


////////////////////////////////////////////////////////////////////////////////
//
//	Get the start and end of the path.
//
////////////////////////////////////////////////////////////////////////////////

Terrain::Vec2ui Terrain::getPathStart() const
{
	return Vec2ui ( _i1, _j1 );
}
Terrain::Vec2ui Terrain::getPathEnd() const
{
	return Vec2ui ( _i2, _j2 );
}


////////////////////////////////////////////////////////////////////////////////
//
//	Find the distance.
//
////////////////////////////////////////////////////////////////////////////////

void Terrain::_findPath()
{
	// Make the ground points with real coordinates.
	this->_makeGroundPoints();

	// Make the triangles.
	this->_makeTriangles();

	// Make the plane.
	this->_makePlane();

	// Intersect the plane with the triangles.
	this->_intersect();
}


////////////////////////////////////////////////////////////////////////////////
//
//	Given an i and j position in the grid, return the index in the 1D array.
//
////////////////////////////////////////////////////////////////////////////////

unsigned int Terrain::_getIndex ( unsigned int i, unsigned int j ) const
{
	// Make sure the indices are in range.
	if ( ( i >= _numY ) || ( j >= _numX ) )
	{
		std::ostringstream out;
		out << "When getting 1D index, input indices i = " << i << " and j = " << j << " are out of range for numX = " << _numX << " and numY = " << _numY;
		throw std::out_of_range ( out.str() );
	}

	// Calculate the answer.
	const unsigned int answer = ( i * _numX + j );

	// Make sure it is in range.
	if ( answer >= ( _numX * _numY ) )
	{
		std::ostringstream out;
		out << "Calculated index " << answer << " is out of range for numX = " << _numX << " and numY = " << _numY;
		throw std::out_of_range ( out.str() );
	}

	// Return the answer.
	return answer;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Read the heights from the input file.
//
////////////////////////////////////////////////////////////////////////////////

void Terrain::_readHeightData ( std::ifstream &in )
{
	// Make the container of heights and size it correctly.
	Heights heights;
	heights.resize ( _numX * _numY );

	// The size of all the data in bytes.
	const std::size_t dataSize = heights.size() * ( sizeof ( Heights::value_type ) );

	// Read all the values into the container.
	in.read ( reinterpret_cast < char * > ( &heights[0] ), dataSize );

	// Make sure it all read correctly.
	if ( ( static_cast < std::streamsize > ( dataSize ) ) != in.gcount() )
	{
		std::ostringstream out;
		out << "Read " << in.gcount() << " bytes but expected " << dataSize;
		throw std::runtime_error ( out.str() );
	}

	// Set the heights. This should be a move-copy so not expensive.
	_heights = heights;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Make the ground points with real coordinates.
//	Note: CoPilot initially wrote this whole function. I changed very little.
//
////////////////////////////////////////////////////////////////////////////////

void Terrain::_makeGroundPoints()
{
	// Make sure the sizes match.
	if ( _heights.size() != ( _numX * _numY ) )
	{
		std::ostringstream out;
		out << "Heights size is " << _heights.size() << " but expected " << ( _numX * _numY );
		throw std::invalid_argument ( out.str() );
	}

	// Make the container of points and save space.
	Points points;
	points.reserve ( _numX * _numY );

	// Loop over the heights and make the points.
	for ( unsigned int i = 0; i < _numY; ++i )
	{
		for ( unsigned int j = 0; j < _numX; ++j )
		{
			const unsigned int index = this->_getIndex ( i, j );
			points.push_back ( Point (
				( static_cast < double > ( j ) * HORIZONTAL_RESOLUTION ),
				( static_cast < double > ( i ) * HORIZONTAL_RESOLUTION ),
				( static_cast < double > ( _heights.at ( index ) ) * VERTICAL_RESOLUTION )
			) );
		}
	}

	// Set the points. This should be a move-copy.
	_points = points;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Make the triangle mesh.
//
////////////////////////////////////////////////////////////////////////////////

void Terrain::_makeTriangles()
{
	// Make sure the sizes match.
	if ( _points.size() != ( _numX * _numY ) )
	{
		std::ostringstream out;
		out << "Points size is " << _points.size() << " but expected " << ( _numX * _numY );
		throw std::invalid_argument ( out.str() );
	}

	// Make the container of triangles and give it space.
	Triangles triangles;
	triangles.reserve ( ( _numX - 1 ) * ( _numY - 1 ) * 2 );

	// Make the first row.
	this->_addTriangleRow ( 0, 1, triangles );

	// Loop over the remaining rows.
	for ( unsigned int i = 1; i < ( _numY - 1 ); ++i )
	{
		this->_addTriangleRow ( i, i + 1, triangles );
	}

	// Set the triangles. This should be a move-copy.
	_triangles = triangles;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Add a row of triangles.
//
////////////////////////////////////////////////////////////////////////////////

void Terrain::_addTriangleRow ( unsigned int rowA, unsigned int rowB, Triangles &triangles ) const
{
	// Make sure the indices are within range.
	if ( ( rowA >= _numY ) || ( rowB >= _numY ) )
	{
		std::ostringstream out;
		out << "When adding a triangle row, input indices rowA = " << rowA << " and rowB = " << rowB << " are out of range for numY = " << _numY;
		throw std::out_of_range ( out.str() );
	}

	// Make the first column.
	this->_addTwoTriangles ( rowA, rowB, 0, 1, triangles );

	// Loop over the remaining columns.
	for ( unsigned int j = 1; j < ( _numX - 1 ); ++j )
	{
		this->_addTwoTriangles ( rowA, rowB, j, j + 1, triangles );
	}
}


////////////////////////////////////////////////////////////////////////////////
//
//	Add two triangles.
//
////////////////////////////////////////////////////////////////////////////////

void Terrain::_addTwoTriangles ( unsigned int rowA, unsigned int rowB, unsigned int colA, unsigned int colB, Triangles &triangles ) const
{
	// Make sure the indices are within range.
	if ( ( colB >= _numX ) || ( colA >= _numX ) )
	{
		std::ostringstream out;
		out << "When adding two triangles, input indices colA = " << colA << " and colB = " << colB << " are out of range for numX = " << _numX;
		throw std::out_of_range ( out.str() );
	}
	if ( ( rowA >= _numY ) || ( rowB >= _numY ) )
	{
		std::ostringstream out;
		out << "When adding two triangles, input indices rowA = " << rowA << " and rowB = " << rowB << " are out of range for numY = " << _numY;
		throw std::out_of_range ( out.str() );
	}

	// Get the 1D indices.
	const unsigned int itl = this->_getIndex ( rowA, colA ); // Index, top left.
	const unsigned int itr = this->_getIndex ( rowA, colB );
	const unsigned int ibl = this->_getIndex ( rowB, colA );
	const unsigned int ibr = this->_getIndex ( rowB, colB );

	// Get the points.
	const Point &tl = _points.at ( itl ); // Top left.
	const Point &tr = _points.at ( itr );
	const Point &bl = _points.at ( ibl );
	const Point &br = _points.at ( ibr );

	// Add the triangles.
	triangles.push_back ( Triangle ( tl, bl, tr ) );
	triangles.push_back ( Triangle ( br, tr, bl ) );
}


////////////////////////////////////////////////////////////////////////////////
//
//	Make the plane.
//	https://stackoverflow.com/questions/1243614/how-do-i-calculate-the-normal-vector-of-a-line-segment/1243676#1243676
//
////////////////////////////////////////////////////////////////////////////////

void Terrain::_makePlane()
{
	// Get the 3D points at the given indices.
	const Point &p1 = _points.at ( this->_getIndex ( _i1, _j1 ) );
	const Point &p2 = _points.at ( this->_getIndex ( _i2, _j2 ) );

	// Make sure they are not the same point.
	if ( p1 == p2 )
	{
		throw std::invalid_argument ( "Input points are equal when calculating plane" );
	}

	// Get the 2D line from p1 to p2.
	typedef Eigen::Vector3 < double > Vec3d;
	const Vec3d line ( ( p2[0] - p1[0] ), ( p2[1] - p1[1] ), 0 );

	// Make a vertical line.
	const Vec3d vertical ( 0, 0, 1 );

	// Make the plane normal from the cross product.
	Vec3d n = ( line.cross ( vertical ) );

	// Check the length.
	if ( 0 == n.norm() ) // TODO: Should use a "close float" tolerance.
	{
		throw std::runtime_error ( "Plane normal vector length is zero" );
	}

	// Normalize the normal vector.
	n.normalize();

	// Check the normal coefficients.
	if ( ( n[0] == 0.0 ) && ( n[1] == 0.0 ) && ( n[2] == 0.0 ) )
	{
		throw std::runtime_error ( "Plane normal vector is all zeros" );
	}

	// Get the plane from the point and normal.
	const Plane plane ( p1, Kernel::Vector_3 ( n[0], n[1], n[2] ) );

	// Set the new plane.
	_plane = plane;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Make the key to the map of the lines.
//
////////////////////////////////////////////////////////////////////////////////

namespace { namespace Details
{
	inline std::string makeMapKey ( const std::string &s0, const std::string &s1 )
	{
		return std::string ( "[" + s0 + "],[" + s1 + "]" );
	};
	inline std::string makeMapKey ( const Terrain::LineSegment &line )
	{
		// Format the points as a string.
		const std::string s0 = Tools::formatVec3 ( line[0] );
		const std::string s1 = Tools::formatVec3 ( line[1] );

		// Return the map key. If the first point (when represented as a string)
		// is greater than the second point, swap them.
		return ( ( s0 < s1 ) ?
			makeMapKey ( s0, s1 ) :
			makeMapKey ( s1, s0 )
		);
	};
} }


////////////////////////////////////////////////////////////////////////////////
//
//	Intersect the plane with the triangles.
//
////////////////////////////////////////////////////////////////////////////////

void Terrain::_intersect()
{
	// Types used below.
	typedef Triangles::const_iterator Itr;
	typedef CGAL::AABB_triangle_primitive < Kernel, Itr > Primitive;
	typedef CGAL::AABB_traits < Kernel, Primitive > Traits;
	typedef CGAL::AABB_tree < Traits > Tree;
	typedef Tree::Intersection_and_primitive_id < Plane >::Type IntersectionType;
	typedef boost::optional < IntersectionType > IntersectionData;
	typedef Kernel::Vector_3 Vector;

	// Make the AABB tree.
	Tree tree ( _triangles.begin(), _triangles.end() );

	// This is where the line-segments get added to.
	std::vector < IntersectionData > hits;

	// Intersect the triangles with the plane using the AABB tree.
	tree.all_intersections ( _plane, std::back_inserter ( hits ) );

	// Initialize.
	typedef std::map < std::string, LineSegment > LineSegmentMap;
	LineSegmentMap lsm;
	LineSegments lines;

	// Loop through the hits.
	for ( const auto &hit : hits )
	{
		// Make sure this is a valid hit.
		if ( !hit )
		{
			continue;
		}

		// Get the variant (union) from the hit.
		const auto &variant = hit.value().first;

		// We only care about line-segments, which are type number 1.
		if ( 1 != variant.which() )
		{
			continue;
		}

		// Get the line segment.
		const LineSegment &line = boost::get < LineSegment > ( variant );

		// This is our map key.
		const std::string key = Details::makeMapKey ( line );

		// Save the line segment in our map.
		lsm[key] = line;

		#if 0
		#ifdef _DEBUG
		std::cout << "Found line segment: [";
		std::cout << Tools::formatVec3 ( line[0] );
		std::cout << "], [";
		std::cout << Tools::formatVec3 ( line[1] );
		std::cout << ']' << std::endl;
		#endif
		#endif
	}

	// We need to clip the lines with two planes, one at each end of the path.
	// These are the two points at the start and end of the path.
	const Point &p1 = _points.at ( this->_getIndex ( _i1, _j1 ) );
	const Point &p2 = _points.at ( this->_getIndex ( _i2, _j2 ) );

	// The normal vectors.
	const Vector n1 = ( p1 - p2 );
	const Vector n2 = ( p2 - p1 );

	// Make the two planes.
	// Note: it does not matter that the normal vectors are not unit length.
	const Plane plane1 ( p1, n1 );
	const Plane plane2 ( p2, n2 );

	// Loop through the lines in the map.
	for ( const auto &item : lsm )
	{
		// Get the line segment.
		const LineSegment &line = item.second;

		// Get the midpoint.
		const Point mp = CGAL::midpoint ( line );

		// If the midpoint is on the negative side of both planes ...
		if ( plane1.has_on_negative_side ( mp ) && plane2.has_on_negative_side ( mp ) )
		{
			// Keep this line segment.
			lines.push_back ( line );
		}
	}

	// Set the line segments.
	_lines = lines;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Get the distance along the path.
//
////////////////////////////////////////////////////////////////////////////////

double Terrain::_getPathDistances() const
{
	// Initialize the distance.
	double dist = 0;

	// Loop through the lines in the container.
	for ( const auto &line : _lines )
	{
		#if 0
		#ifdef _DEBUG
		std::cout << "Final line segment: [";
		std::cout << Tools::formatVec3 ( line[0] );
		std::cout << "], [";
		std::cout << Tools::formatVec3 ( line[1] );
		std::cout << ']' << std::endl;
		#endif
		#endif

		// Add to the distance.
		dist += std::sqrt ( line.squared_length() );
	}

	// Return the total distance.
	return dist;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Get the distance along the path.
//
////////////////////////////////////////////////////////////////////////////////

double Terrain::getDistance()
{
	// Is this the first time?
	if ( _dist < 0 )
	{
		// Run through all the steps to find the path.
		this->_findPath();

		// Set the new distance.
		_dist = this->_getPathDistances();
	}

	// Return what we have.
	return _dist;
}
