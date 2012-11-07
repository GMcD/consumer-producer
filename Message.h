/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 678 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/
#pragma once

class Message {
	std::string msg;
	size_t num;
public:
	Message(std::string s = "", int i = 0);
	std::string str();
	void operator()();
};
