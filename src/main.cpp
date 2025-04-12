////////////////////////////////////////////////////////////////////////////////
//
//	Program for finding the path distance.
//
////////////////////////////////////////////////////////////////////////////////

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

typedef std::vector < double > Heights;


////////////////////////////////////////////////////////////////////////////////
//
//	Constants used below.
//
////////////////////////////////////////////////////////////////////////////////

const double HORIZONTAL_RESOLUTION = 30;
const double VERTICAL_RESOLUTION = 11;


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
//	Read the heights from the input file.
//
////////////////////////////////////////////////////////////////////////////////

inline Heights getHeights ( unsigned int width, unsigned int height, std::ifstream &in )
{
	// Check input.
	if ( ( width <= 0 ) || ( height <= 0 ) )
	{
		throw std::invalid_argument ( "Width and height must be greater than zero" );
	}

	// Make the container of heights and size it correctly.
	typedef std::vector < std::uint8_t > RawHeights;
	RawHeights rh;;
	rh.resize ( width * height );

	// The size of all the data in bytes.
	const std::size_t dataSize = rh.size() * ( sizeof ( RawHeights::size_type ) );

	// Read all the values into the container.
	in.read ( reinterpret_cast < char * > ( &rh[0] ), dataSize );

	// Make sure it all read correctly.
	if ( ( static_cast < std::streamsize > ( dataSize ) ) != in.gcount() )
	{
		std::ostringstream out;
		out << "Read " << in.gcount() << " bytes but expected " << dataSize;
		throw std::runtime_error ( out.str() );
	}

	// Convert to the correct container type.
	Heights heights;
	heights.resize ( width * height );
	for ( std::size_t i = 0; i < rh.size(); ++i )
	{
		heights[i] = static_cast < double > ( rh[i] * ELEVATION_RESOLUTION );
	}

	// Return the heights. This should be a move-copy so not expensive.
	return heights;
}


////////////////////////////////////////////////////////////////////////////////
//
//	Run the program.
//
////////////////////////////////////////////////////////////////////////////////

void run ( int argc, char **argv )
{
	// Get the width and height.
	const unsigned int width  = getUint ( argv[1] );
	const unsigned int height = getUint ( argv[2] );

	// Check the width and height.
	if ( ( width <= 0 ) || ( height <= 0 ) )
	{
		throw std::invalid_argument ( "Width and height must be greater than zero" );
	}

	// Open the input file in binary.
	std::ifstream in ( argv[1], std::ios::binary );

	// Did it open?
	if ( !in.is_open() )
	{
		std::ostringstream out;
		out << "Could not open input file: " << argv[1];
		throw std::runtime_error ( out.str() );
	}

	// Read the file into a vector of heights.
	const Heights heights = getHeights ( width, height, in );

	// Get the distance along the path.
	const double dist = getDistance ( width, height, heights );


	std::cout << "Distance along the path is " << dist << " meters" << std::endl;
	return 0;
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
