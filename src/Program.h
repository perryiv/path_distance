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
	Program ( unsigned int numX, unsigned int numY, unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2 );

	// The default destructor is fine.
	~Program() = default;

	// Not copyable or movable.
	Program ( const Program & ) = delete;
	Program ( Program && ) = delete;
	Program & operator = ( const Program & ) = delete;
	Program & operator = ( Program && ) = delete;

	// Run the program.
	void run ( int argc, char **argv );

protected:

	void _addTriangleRow ( unsigned int rowA, unsigned int rowB, Triangles &triangles );
	void _addTwoTriangles ( unsigned int rowA, unsigned int rowB, unsigned int colA, unsigned int colB, Triangles &triangles );

	double _getDistance();
	unsigned int _getIndex ( unsigned int i, unsigned int j );

	void _intersect();

	void _makeGroundPoints();
	void _makeTriangles();
	void _makePlane();

	void _readHeightData ( std::ifstream & );

private:

	const unsigned int _numX;
	const unsigned int _numY;
	const unsigned int _i1;
	const unsigned int _j1;
	const unsigned int _i2;
	const unsigned int _j2;
	Heights _heights;
	Points _points;
	Triangles _triangles;
	Vec4d _plane;
	Lines _lines;
};
