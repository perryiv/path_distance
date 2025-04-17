////////////////////////////////////////////////////////////////////////////////
//
//	The class that runs the program.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Eigen/Geometry"

#include <cstdint>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
//
//	The class that runs the program.
//
////////////////////////////////////////////////////////////////////////////////

class Program
{
public:

	typedef Eigen::Vector3 < std::uint32_t > Vec3ui;
	typedef Eigen::Vector3 < double > Vec3d;
	typedef Eigen::Vector4 < double > Vec4d;
	typedef std::vector < Vec3d > Points;
	typedef std::vector < std::uint8_t > Heights;
	typedef std::vector < Vec3ui > Triangles;
	typedef std::vector < Vec3d > Lines;

	// This is the only constructor we want.
	Program ( int argc, char **argv );

	// The default destructor is fine.
	~Program() = default;

	// Not copyable or movable.
	Program ( const Program & ) = delete;
	Program ( Program && ) = delete;
	Program & operator = ( const Program & ) = delete;
	Program & operator = ( Program && ) = delete;

	// Get the distance.
	double getDistance();

protected:

	void _addTriangleRow ( unsigned int rowA, unsigned int rowB, Triangles &triangles );
	void _addTwoTriangles ( unsigned int rowA, unsigned int rowB, unsigned int colA, unsigned int colB, Triangles &triangles );

	void _findDistance();

	unsigned int _getIndex ( unsigned int i, unsigned int j );

	void _intersect();

	void _makeGroundPoints();
	void _makeTriangles();
	void _makePlane();

	void _readHeightData ( std::ifstream & );
	void _run();

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
	Vec4d _plane;
	Lines _lines;
	double _dist;
};
