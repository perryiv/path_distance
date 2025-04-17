////////////////////////////////////////////////////////////////////////////////
//
//	The class that runs the program.
//
////////////////////////////////////////////////////////////////////////////////

#include "Program.h"
#include "Tools.h"

#include <fstream>
#include <sstream>
#include <stdexcept>


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

Program::Program ( int argc, char **argv ) :
	_numX ( Tools::getUint ( argv[1] ) ),
	_numY ( Tools::getUint ( argv[2] ) ),
	_i1   ( Tools::getUint ( argv[3] ) ),
	_j1   ( Tools::getUint ( argv[4] ) ),
	_i2   ( Tools::getUint ( argv[5] ) ),
	_j2   ( Tools::getUint ( argv[6] ) ),
	_heights(),
	_points(),
	_triangles(),
	_plane(),
	_lines(),
	_dist ( -1 )
{
#ifdef USE_FAKE_DATA

	_heights = {
		 0,  1,  2,  3,
		 4,  5,  6,  7,
		 8,  9, 10, 11,
		12, 13, 14, 15,
		16, 17, 18, 19,
	};

	_numX = 4;
	_numY = 5;
	_i1 = 1;
	_j1 = 1;
	_i2 = 4;
	_j2 = 2;

#else // Use real data.

	// Get the number of "pixels" in the x and y directions.
	_numX = Tools::getUint ( argv[1] );
	_numY = Tools::getUint ( argv[2] );
	_i1 = Tools::getUint ( argv[3] );
	_j1 = Tools::getUint ( argv[4] );
	_i2 = Tools::getUint ( argv[5] );
	_j2 = Tools::getUint ( argv[6] );

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

	// Make sure the indices are within tange.
	if ( ( _i1 >= _numX ) || ( _i2 >= _numX ) )
	{
		throw std::out_of_range ( "Given indices are greater than the size" );
	}

	// Open the input file in binary.
	const std::string name ( argv[7] );
	std::ifstream in ( name.c_str(), std::ios::binary );

	// Did it open?
	if ( !in.is_open() )
	{
		std::ostringstream out;
		out << "Could not open input file: " << name;
		throw std::runtime_error ( out.str() );
	}

	// Read the file into a vector of data.
	this->_readHeightData ( in );

#endif // Use real data.
}


////////////////////////////////////////////////////////////////////////////////
//
//	Run the program.
//
////////////////////////////////////////////////////////////////////////////////

void Program::_run()
{
	// Make the ground points with real coordinates.
	this->_makeGroundPoints();

	// Make the triangle indices.
	this->_makeTriangles();

	// Get the plane.
	this->_makePlane();

	// Intersect the plane with the triangles.
	this->_intersect();

	// Find the distance along the path.
	this->_findDistance();
}


////////////////////////////////////////////////////////////////////////////////
//
//	Given an i and j position in the grid, return the index in the 1D array.
//
////////////////////////////////////////////////////////////////////////////////

unsigned int Program::_getIndex ( unsigned int i, unsigned int j )
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

void Program::_readHeightData ( std::ifstream &in )
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

void Program::_makeGroundPoints()
{
	// Make sure the sizes match.
	if ( _heights.size() != ( _numX * _numY ) )
	{
		std::ostringstream out;
		out << "Heights size is " << _heights.size() << " but expected " << ( _numX * _numY );
		throw std::invalid_argument ( out.str() );
	}

	// Make the container of points and size it correctly.
	Points points;
	points.resize ( _numX * _numY );

	// Loop over the heights and make the points.
	for ( unsigned int i = 0; i < _numY; ++i )
	{
		for ( unsigned int j = 0; j < _numX; ++j )
		{
			const unsigned int index = this->_getIndex ( i, j );
			Vec3d &point = points.at ( index );
			point[0] = static_cast < double > ( j ) * HORIZONTAL_RESOLUTION;
			point[1] = static_cast < double > ( i ) * HORIZONTAL_RESOLUTION;
			point[2] = static_cast < double > ( _heights.at ( index ) ) * VERTICAL_RESOLUTION;
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

void Program::_makeTriangles()
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

void Program::_addTriangleRow ( unsigned int rowA, unsigned int rowB, Triangles &triangles )
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

void Program::_addTwoTriangles ( unsigned int rowA, unsigned int rowB, unsigned int colA, unsigned int colB, Triangles &triangles )
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

	// Add the triangles.
	triangles.push_back ( {
		this->_getIndex ( rowA, colA ),
		this->_getIndex ( rowB, colA ),
		this->_getIndex ( rowA, colB )
	} );
	triangles.push_back ( {
		this->_getIndex ( rowB, colB ),
		this->_getIndex ( rowA, colB ),
		this->_getIndex ( rowB, colA )
	} );
}


////////////////////////////////////////////////////////////////////////////////
//
//	Make the plane.
//	https://stackoverflow.com/questions/1243614/how-do-i-calculate-the-normal-vector-of-a-line-segment/1243676#1243676
//
////////////////////////////////////////////////////////////////////////////////

void Program::_makePlane()
{
	// Get the 3D points at the given indices.
	const Vec3d &p1 = _points.at ( this->_getIndex ( _i1, _j1 ) );
	const Vec3d &p2 = _points.at ( this->_getIndex ( _i2, _j2 ) );

	// Make sure they are not the same point.
	if ( p1 == p2 )
	{
		throw std::invalid_argument ( "Input points are equal when calculating plane" );
	}

	// Get the line from p1 to p2.
	const Vec3d line = p2 - p1;

	// Make a vertical line.
	const Vec3d vertical ( 0, 0, 1 );

	// Make the plane normal from the cross product.
	Vec3d n = ( line.cross ( vertical ) );

	// Check the length.
	if ( 0 == n.norm() ) // TODO: Should use a "close float" tolerance.
	{
		throw std::runtime_error ( "Points are the same, cannot calculate plane" );
	}

	// Normalize the normal vector.
	n.normalize();

	// Check the normal coefficients.
	if ( ( n[0] == 0.0 ) && ( n[1] == 0.0 ) && ( n[2] == 0.0 ) )
	{
		throw std::runtime_error ( "Normal vector is all zeros" );
	}

	// Get the plane coefficients from the point and normal.
	Vec4d plane ( n[0], n[1], n[2], -n.dot ( p1 ) );

	// Set the new plane.
	_plane = plane;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Intersect the plane with the triangles.
//
////////////////////////////////////////////////////////////////////////////////

void Program::_intersect()
{
	Lines lines;
	_lines = lines; // TODO
}


////////////////////////////////////////////////////////////////////////////////
//
//	Get the distance along the path.
//
////////////////////////////////////////////////////////////////////////////////

void Program::_findDistance()
{
	// Not sure yet if every two points is a new line segment, or if each new
	// point is (which is a line strip).
	_dist = 0; // TODO
}


////////////////////////////////////////////////////////////////////////////////
//
//	Get the distance along the path.
//
////////////////////////////////////////////////////////////////////////////////

double Program::getDistance()
{
	// Is this the first time?
	if ( _dist < 0 )
	{
		// Run through all the steps.
		this->_run();
	}

	// Return what we have.
	return _dist;
}
