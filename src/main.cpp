////////////////////////////////////////////////////////////////////////////////
//
//	Program for finding the path distance.
//
////////////////////////////////////////////////////////////////////////////////

#include "Terrain.h"
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
	const unsigned int numX = Tools::getUint ( argv[1] );
	const unsigned int numY = Tools::getUint ( argv[2] );
	const unsigned int i1   = Tools::getUint ( argv[3] );
	const unsigned int j1   = Tools::getUint ( argv[4] );
	const unsigned int i2   = Tools::getUint ( argv[5] );
	const unsigned int j2   = Tools::getUint ( argv[6] );

	const std::string input1 = argv[7];
	const std::string input2 = argv[8];

	std::cout << "Processing input file: " << input1 << std::endl;
	Terrain t1 ( numX, numY, i1, j1, i2, j2, input1 );
	const double d1 = t1.getDistance();
	std::cout << "Path distance: " << d1 << " m" << std::endl;

	std::cout << "Processing input file: " << input2 << std::endl;
	Terrain t2 ( numX, numY, i1, j1, i2, j2, input2 );
	const double d2 = t2.getDistance();
	std::cout << "Path distance: " << d2 << " m" << std::endl;

	const double dd = std::fabs ( d1 - d2 );
	std::cout << "Change in distance: " << dd << " m" << std::endl;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Main function.
//
////////////////////////////////////////////////////////////////////////////////

int main ( int argc, char **argv )
{
	// Check input.
	if ( argc < 9 )
	{
		std::cerr << "Usage: " << argv[0] << " <num x> <num y> <x1> <y1> <x2> <y2> <input file before> <input file after>" << std::endl;
		return 1;
	}

	// Safely run the program.
	try
	{
		run ( argc, argv );
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
