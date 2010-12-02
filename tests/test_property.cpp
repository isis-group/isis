#include <stdio.h>
#include <stdlib.h>
#include "CoreUtils/property.hpp"
#include "CoreUtils/propmap.hpp"
#include <iostream>
#include "CoreUtils/log.hpp"
#include "CoreUtils/vector.hpp"

using namespace isis::util; //needed for the log-levels

int main( int argc, char *argv[] )
{
	ENABLE_LOG( CoreLog, DefaultMsgPrint, info );
	PropertyValue a = std::string( "Hallo" );
	std::cout << a->toString( true ) << std::endl;
	PropertyValue b = a;
	std::cout << b->toString( true ) << std::endl;
	b = 5.2;
	std::cout << b->toString( true ) << std::endl;
	PropMap map1, map2, cont;
	map1["Test1"] = 6.4;
	map1["test2"] = 5;
	map2["Test1"] = 6.4;

	if ( map2["Test1"] == 6.4 ) {
		std::cout << map2["Test1"]->toString( true ) <<  " ist gleich " << Type<double>( 6.4 ).toString() << std::endl;
	}

	map2["Test2"] = 6;
	map2["Test3"] = std::string( "Hallo" );
	map1["Vector"] = fvector4( 1, 1, 1, 1 );
	cont["Prop"] = map2;
	std::cout << cont["Prop"]->toString() << std::endl;
	std::string x = map2["Test3"];
	std::cout << x << std::endl;
	//will get you int() ("0")
	int fail = map2["Test3"];
	//will raise bad_cast (Because "Hallo" cannot lexically be casted to "3"
	//  fail=map["Test3"]->as<int>();
	//will be ok ("3" can be lexically casted to int)
	map2["Test3"] = std::string( "3" );
	int ok = map2["Test3"]->as<int>();
	map1.print( std::cout, true );
	map2.print( std::cout, true );
	std::cout << map1 << std::endl;
	std::cout << map2 << std::endl;
	std::cout << map1.diff( map2 ) << std::endl;
	return EXIT_SUCCESS;
}
