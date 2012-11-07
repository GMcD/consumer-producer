/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 690 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/

#include <ConProd2.h>


Message::Message(std::string s, int i) : msg(s), num(i) {

}

std::string Message::str() {
	return msg + boost::lexical_cast<std::string>(num) + std::string( " : run." );
}

void Message::operator()() {
	if ( num % 5 == 0) {
		// int j = 1 / (i % 5);
		throw boost::enable_current_exception(thread_error()) << thread_info("Whazzup?");
	} else {
		Logger::log(info, str());
	}
}
