/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 679 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/
#pragma once

class Source : public MessageProducer {
public:
	size_t count;
	std::string shared_stuff;

	// Producer-Consumer Interface
	Source(MessagePool &messages, Latch &lh);
	virtual Message produce();
	virtual bool cancel();
	virtual void started();
	virtual void done();
};
