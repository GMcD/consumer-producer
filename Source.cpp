/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 678 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/

#include <ConProd2.h>

Source::Source(MessagePool& messages, Latch& lh) : MessageProducer(messages, lh) {
}

Message Source::produce() {
	if (count == 0)
		throw pc_exception("Producer done");
	std::string msg("Manager::produce()");
	Message message(msg, count--);
	return message;
}

// Cancel has to be signaled? somehow?
bool Source::cancel() {
	bool done = false;
	if (done){
		return true;
	}
	return done;
}

void Source::started() {
	Producer::started();
}

void Source::done() {
	Producer::done();
}
