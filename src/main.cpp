////////////////////////////////////////////////////////////////////////////////
//
//	Program for finding the path distance.
//
////////////////////////////////////////////////////////////////////////////////

#include "Program.h"
#include "Tools.h"

#include <iostream>
#include <stdexcept>


////////////////////////////////////////////////////////////////////////////////
//
//	Run the program.
//
////////////////////////////////////////////////////////////////////////////////

inline void run ( int argc, char **argv )
{
	// Get the number of "pixels" in the x and y directions.
	const unsigned int numX = Tools::getUint ( argv[1] );
	const unsigned int numY = Tools::getUint ( argv[2] );
	const unsigned int i1 = Tools::getUint ( argv[3] );
	const unsigned int j1 = Tools::getUint ( argv[4] );
	const unsigned int i2 = Tools::getUint ( argv[5] );
	const unsigned int j2 = Tools::getUint ( argv[6] );

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

	// Make the triangle indices.
	const Triangles triangles = getTriangles ( numX, numY, points );

	// Get the plane.
	const Vec4d plane = getPlane ( i1, j1, i2, j2, numX, numY, points );

	// Intersect the plane with the triangles.
	const Lines lines = intersect ( i1, j1, i2, j2, numX, numY, points, triangles, plane );

	// Get the distance along the path.
	const double dist = getDistance ( lines );

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
	const Lines lines = intersect ( i1, j1, i2, j2, numX, numY, points, triangles, plane );
	const double dist = getDistance ( lines );

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
