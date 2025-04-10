////////////////////////////////////////////////////////////////////////////////
//
//	Program for finding the path distance.
//
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
//
//	Types used below.
//
////////////////////////////////////////////////////////////////////////////////

typedef std::vector < double > Heights;


////////////////////////////////////////////////////////////////////////////////
//
//	Safely return an unsigned int.
//
////////////////////////////////////////////////////////////////////////////////

unsigned int getUint ( const char *str )
{
	int answer = std::atoi ( str );

	if ( answer < 0 )
	{
		throw std::runtime_error ( "Invalid unsigned int" );
	}

	return static_cast < unsigned int > ( answer );
}


////////////////////////////////////////////////////////////////////////////////
//
//	Read the heights from the input file.
//
////////////////////////////////////////////////////////////////////////////////

Heights getHeights ( const  std::ifstream &in )
{
	Heights heights;
	return heights; // TODO
}

////////////////////////////////////////////////////////////////////////////////
//
//	Main function.
//
////////////////////////////////////////////////////////////////////////////////

int main ( int argc, char **argv )
{
	// Check input.
	if ( argc < 4 )
	{
		std::cerr << "Usage: " << argv[0] << " <width> <height> <input file>" << std::endl;
		return 1;
	}
	// Get the width and height.
	const unsigned int width  = getUint ( argv[1] );
	const unsigned int height = getUint ( argv[2] );

	// Check the width and height.
	if ( ( width <= 0 ) || ( height <= 0 ) )
	{
		std::cerr << "Width and height must be greater than zero" << std::endl;
		return 1;
	}

	// Open the input file in binary.
	std::ifstream in ( argv[1], std::ios::binary );

	// Did it open?
	if ( !in.is_open() )
	{
		std::cerr << "Could not open input file: " << argv[1] << std::endl;
		return 1;
	}

	// Read the file into a vector of heights.
	const Heights heights = getHeights ( width, height, in );

	// Get the distance along the path.
	const double dist = getDistance ( width, height, heights );


	std::cout << "Distance along the path is " << dist << " meters" << std::endl;
	return 0;
}
