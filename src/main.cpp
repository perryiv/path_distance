////////////////////////////////////////////////////////////////////////////////
//
//	Program for finding the path distance.
//
////////////////////////////////////////////////////////////////////////////////

#include "Eigen/Geometry"

#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
//
//	Types used below.
//
////////////////////////////////////////////////////////////////////////////////

typedef Eigen::Vector3 < std::uint32_t > Vec3ui;
typedef Eigen::Vector3 < double > Vec3d;
typedef Eigen::Vector4 < double > Vec4d;
typedef std::vector < Vec3d > Points;
typedef std::vector < std::uint8_t > Heights;
typedef std::vector < Vec3ui > Triangles;


////////////////////////////////////////////////////////////////////////////////
//
//	Constants used below.
//
////////////////////////////////////////////////////////////////////////////////

const double HORIZONTAL_RESOLUTION = 30;
const double VERTICAL_RESOLUTION = 11;


////////////////////////////////////////////////////////////////////////////////
//
//	Check the size of the grid.
//
////////////////////////////////////////////////////////////////////////////////

inline void checkSize ( unsigned int numX, unsigned int numY )
{
	if ( ( numX <= 1 ) || ( numY <= 1 ) )
	{
		throw std::invalid_argument ( "Number of pixels in the x and y directions must be greater than one" );
	}
}


////////////////////////////////////////////////////////////////////////////////
//
//	Safely return an unsigned int.
//
////////////////////////////////////////////////////////////////////////////////

inline unsigned int getUint ( const char *str )
{
	if ( !str )
	{
		throw std::runtime_error ( "Invalid string when converting to unsigned int" );
	}

	int answer = std::atoi ( str );

	if ( answer < 0 )
	{
		throw std::runtime_error ( "String is invalid unsigned int" );
	}

	return static_cast < unsigned int > ( answer );
}


////////////////////////////////////////////////////////////////////////////////
//
//	Given an i and j position in the grid, return the index in the 1D array.
//
////////////////////////////////////////////////////////////////////////////////

inline unsigned int getIndex ( unsigned int i, unsigned int j, unsigned int numX, unsigned int numY )
{
	// Check the sizes.
	checkSize ( numX, numY );

	// Make sure the indices are in range.
	if ( ( i >= numY ) || ( j >= numX ) )
	{
		std::ostringstream out;
		out << "When getting 1D index, input indices i = " << i << " and j = " << j << " are out of range for numX = " << numX << " and numY = " << numY;
		throw std::out_of_range ( out.str() );
	}

	// Calculate the answer.
	const unsigned int answer = ( i * numX + j );

	// Make sure it is in range.
	if ( answer >= ( numX * numY ) )
	{
		std::ostringstream out;
		out << "Calculated index " << answer << " is out of range for numX = " << numX << " and numY = " << numY;
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

inline Heights getHeightData ( unsigned int numX, unsigned int numY, std::ifstream &in )
{
	// Check the sizes.
	checkSize ( numX, numY );

	// Make the container of heights and size it correctly.
	Heights heights;
	heights.resize ( numX * numY );

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

	// Return the heights. This should be a move-copy so not expensive.
	return heights;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Make the ground points with real coordinates.
//	Note: CoPilot initially wrote this whole function. I changed very little.
//
////////////////////////////////////////////////////////////////////////////////

inline Points getGroundPoints ( unsigned int numX, unsigned int numY, const Heights &heights )
{
	// Check the sizes.
	checkSize ( numX, numY );

	// Make sure the sizes match.
	if ( heights.size() != ( numX * numY ) )
	{
		std::ostringstream out;
		out << "Heights size is " << heights.size() << " but expected " << ( numX * numY );
		throw std::invalid_argument ( out.str() );
	}

	// Make the container of points and size it correctly.
	Points points;
	points.resize ( numX * numY );

	// Loop over the heights and make the points.
	for ( unsigned int i = 0; i < numY; ++i )
	{
		for ( unsigned int j = 0; j < numX; ++j )
		{
			const unsigned int index = getIndex ( i, j, numX, numY );
			Vec3d &point = points.at ( index );
			point[0] = static_cast < double > ( j ) * HORIZONTAL_RESOLUTION;
			point[1] = static_cast < double > ( i ) * HORIZONTAL_RESOLUTION;
			point[2] = static_cast < double > ( heights.at ( index ) ) * VERTICAL_RESOLUTION;
		}
	}

	// Return the container of points. This should be a move-copy.
	return points;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Add two triangles.
//
////////////////////////////////////////////////////////////////////////////////

inline void addTwoTriangles ( unsigned int numX, unsigned int numY, unsigned int rowA, unsigned int rowB, unsigned int colA, unsigned int colB, const Points &points, Triangles &triangles )
{
	// Check the sizes.
	checkSize ( numX, numY );

	// Make sure the indices are within range.
	if ( ( colB >= numX ) || ( colA >= numX ) )
	{
		std::ostringstream out;
		out << "When adding two triangles, input indices colA = " << colA << " and colB = " << colB << " are out of range for numX = " << numX;
		throw std::out_of_range ( out.str() );
	}
	if ( ( rowA >= numY ) || ( rowB >= numY ) )
	{
		std::ostringstream out;
		out << "When adding two triangles, input indices rowA = " << rowA << " and rowB = " << rowB << " are out of range for numY = " << numY;
		throw std::out_of_range ( out.str() );
	}

	// Add the triangles.
	triangles.push_back ( {
		getIndex ( rowA, colA, numX, numY ),
		getIndex ( rowB, colA, numX, numY ),
		getIndex ( rowA, colB, numX, numY )
	} );
	triangles.push_back ( {
		getIndex ( rowB, colB, numX, numY ),
		getIndex ( rowA, colB, numX, numY ),
		getIndex ( rowB, colA, numX, numY )
	} );
}


////////////////////////////////////////////////////////////////////////////////
//
//	Add a row of triangles.
//
////////////////////////////////////////////////////////////////////////////////

inline void addTriangleRow ( unsigned int numX, unsigned int numY, unsigned int rowA, unsigned int rowB, const Points &points, Triangles &triangles )
{
	// Check the sizes.
	checkSize ( numX, numY );

	// Make sure the indices are within range.
	if ( ( rowA >= numY ) || ( rowB >= numY ) )
	{
		std::ostringstream out;
		out << "When adding a triangle row, input indices rowA = " << rowA << " and rowB = " << rowB << " are out of range for numY = " << numY;
		throw std::out_of_range ( out.str() );
	}

	// Make the first column.
	addTwoTriangles ( numX, numY, rowA, rowB, 0, 1, points, triangles );

	// Loop over the remaining columns.
	for ( unsigned int j = 1; j < ( numX - 1 ); ++j )
	{
		addTwoTriangles ( numX, numY, rowA, rowB, j, j + 1, points, triangles );
	}
}


////////////////////////////////////////////////////////////////////////////////
//
//	Make a triangle mesh.
//
////////////////////////////////////////////////////////////////////////////////

inline Triangles getTriangles ( unsigned int numX, unsigned int numY, const Points &points )
{
	// Check the sizes.
	checkSize ( numX, numY );

	// Make the container of triangles and give it space.
	Triangles triangles;
	triangles.reserve ( ( numX - 1 ) * ( numY - 1 ) * 2 );

	// Make the first row.
	addTriangleRow ( numX, numY, 0, 1, points, triangles );

	// Loop over the remaining rows.
	for ( unsigned int i = 1; i < ( numY - 1 ); ++i )
	{
		addTriangleRow ( numX, numY, i, i + 1, points, triangles );
	}

	// Return the triangles. This should be a move-copy.
	return triangles;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Get the plane.
//	https://stackoverflow.com/questions/1243614/how-do-i-calculate-the-normal-vector-of-a-line-segment/1243676#1243676
//
////////////////////////////////////////////////////////////////////////////////

Vec4d getPlane ( unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2, unsigned int numX, unsigned int numY, const Points &points )
{
	// Get the 3D points at the given indices.
	const Vec3d &p1 = points.at ( getIndex ( i1, j1, numX, numY ) );
	const Vec3d &p2 = points.at ( getIndex ( i2, j2, numX, numY ) );

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

	// Return the new plane.
	return plane;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Get the distance along the path.
//
////////////////////////////////////////////////////////////////////////////////

const double getDistance ( unsigned int numX, unsigned int numY, unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2, const Points &points, const Triangles &triangles, const Vec4d &plane )
{
	return 0; // TODO
}


////////////////////////////////////////////////////////////////////////////////
//
//	Run the program.
//
////////////////////////////////////////////////////////////////////////////////

inline void run ( int argc, char **argv )
{
	// Get the number of "pixels" in the x and y directions.
	const unsigned int numX = getUint ( argv[1] );
	const unsigned int numY = getUint ( argv[2] );
	const unsigned int i1 = getUint ( argv[3] );
	const unsigned int j1 = getUint ( argv[4] );
	const unsigned int i2 = getUint ( argv[5] );
	const unsigned int j2 = getUint ( argv[6] );

	// We can't accept the same point.
	if ( ( i1 == i2 ) && ( j1 == j2 ) )
	{
		throw std::invalid_argument ( "Path start and end points are the same" );
	}

	// Check the sizes.
	checkSize ( numX, numY );

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
	const Heights heights = getHeightData ( numX, numY, in );

	// Make the ground points with real coordinates.
	const Points points = getGroundPoints ( numX, numY, heights );

	// Make a triangle indices.
	const Triangles triangles = getTriangles ( numX, numY, points );

	// Get the plane.
	const Vec4d plane = getPlane ( i1, j1, i2, j2, numX, numY, points );

	// Get the distance along the path.
	const double dist = getDistance ( numX, numY, i1, j1, i2, j2, points, triangles, plane );

	std::cout << "Distance along the path is " << dist << " meters" << std::endl;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Run the test.
//
////////////////////////////////////////////////////////////////////////////////

inline void runTest()
{
	const Heights heights = {
		 0,  1,  2,  3,
		 4,  5,  6,  7,
		 8,  9, 10, 11,
		12, 13, 14, 15,
		16, 17, 18, 19,
	};

	const unsigned int numX = 4;
	const unsigned int numY = 5;
	const unsigned int i1 = 1;
	const unsigned int j1 = 1;
	const unsigned int i2 = 4;
	const unsigned int j2 = 2;

	const Points points = getGroundPoints ( numX, numY, heights );
	const Triangles triangles = getTriangles ( numX, numY, points );
	const Vec4d plane = getPlane ( i1, j1, i2, j2, numX, numY, points );
	const double dist = getDistance ( numX, numY, i1, j1, i2, j2, points, triangles, plane );

	std::cout << "Distance along the path is " << dist << " meters" << std::endl;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Main function.
//
////////////////////////////////////////////////////////////////////////////////

int main ( int argc, char **argv )
{
	// Check input.
	if ( argc < 6 )
	{
		std::cerr << "Usage: " << argv[0] << " <num x> <num y> <x1> <y1> <x2> <y2> <input file>" << std::endl;
		return 1;
	}

	// Safely run the program.
	try
	{
		runTest();
		// run ( argc, argv );
	}

	// Catch standard exceptions.
	catch ( const std::exception &e )
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return 1;
	}

	// Catch all other exceptions.
	catch ( ... )
	{
		std::cerr << "Unknown exception caught" << std::endl;
		return 1;
	}

	// If we get to here then it worked.
	return 0;
}
