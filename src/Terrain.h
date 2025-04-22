////////////////////////////////////////////////////////////////////////////////
//
//	The class that calculates the distance over the terrain.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CGAL/Simple_cartesian.h"

#include "Eigen/Geometry"

#include <cstdint>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
//
//	The class that runs the Terrain.
//
////////////////////////////////////////////////////////////////////////////////

class Terrain
{
public:

	typedef CGAL::Simple_cartesian < double > Kernel;
	typedef CGAL::Segment_3 < Kernel > LineSegment;
	typedef Kernel::Plane_3 Plane;
	typedef Kernel::Point_3 Point;
	typedef Kernel::Triangle_3 Triangle;
	typedef Eigen::Vector2 < unsigned int > Vec2ui;

	typedef std::vector < std::uint8_t > Heights;
	typedef std::vector < Point > Points;
	typedef std::vector < Triangle > Triangles;
	typedef std::vector < LineSegment > LineSegments;

	// This is the only constructor we want.
	Terrain ( unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, const std::string & );

	// The default destructor is fine.
	~Terrain() = default;

	// Not copyable or movable.
	Terrain ( const Terrain & ) = delete;
	Terrain ( Terrain && ) = delete;
	Terrain & operator = ( const Terrain & ) = delete;
	Terrain & operator = ( Terrain && ) = delete;

	// Get the start and end of the path.
	Vec2ui getPathStart() const;
	Vec2ui getPathEnd() const;

	// Get the distance.
	const double &getDistance() const { return _dist; };
	double getDistance();

protected:

	void _addTriangleRow ( unsigned int rowA, unsigned int rowB, Triangles &triangles ) const;
	void _addTwoTriangles ( unsigned int rowA, unsigned int rowB, unsigned int colA, unsigned int colB, Triangles &triangles ) const;

	void _findPath();

	unsigned int _getIndex ( unsigned int i, unsigned int j ) const;
	double _getPathDistances() const;

	void _intersect();

	void _makeGroundPoints();
	void _makeTriangles();
	void _makePlane();

	void _readHeightData ( std::ifstream & );

private:

	unsigned int _numX;
	unsigned int _numY;
	unsigned int _i1;
	unsigned int _j1;
	unsigned int _i2;
	unsigned int _j2;
	Heights _heights;
	Points _points;
	Triangles _triangles;
	Plane _plane;
	LineSegments _lines;
	double _dist;
};
