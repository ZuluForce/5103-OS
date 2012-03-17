#include "utility/strConv.h"


string toBinary( unsigned long n ) {
	char     result[ (sizeof( unsigned long ) * 8) + 1 ];
	unsigned index  = sizeof( unsigned long ) * 8;
	result[ index ] = '\0';

	do result[ --index ] = '0' + (n & 1);
	while (n >>= 1);

	return string( result + index );
}
