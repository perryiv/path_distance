////////////////////////////////////////////////////////////////////////////////
//
//	Program for finding the path distance.
//
////////////////////////////////////////////////////////////////////////////////

#include "Program.h"

#include <iostream>
#include <stdexcept>


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
		Program program ( argc, argv );
		const double dist = program.getDistance();
		std::cout << "Distance along the path is " << dist << " meters" << std::endl;
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
