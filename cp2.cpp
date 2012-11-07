/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 679 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/

#include "ConProd2.h"

int main(int argc, char **argv){
  
	try {
		PoolManagerFactory (argc, argv);
	}
	catch (boost::exception &ex){
		std::cout << diagnostic_information(ex) << std::endl;
	}
	return 0;
}
