////////////////////////////////////////////////////////////////////////////////
//
//	Misc functions.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdexcept>


////////////////////////////////////////////////////////////////////////////////
//
//	Beginning of the namespace.
//
////////////////////////////////////////////////////////////////////////////////

namespace Tools {


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
//	End of the namespace.
//
////////////////////////////////////////////////////////////////////////////////

} // namespace Tools
