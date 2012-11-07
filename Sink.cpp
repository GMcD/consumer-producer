/*
 * Copyright © Gary MacDonald 2011
 * $Author: scapa $
 * $Revision: 690 $
 * $Date: 2011-09-22 12:53:51 +0100 (Thu, 22 Sep 2011) $
 */
#include "ConProd2.h"

Sink::Sink( MessagePool& slices, Latch& lh)
	: MessageConsumer(slices, lh) {
}

void Sink::consume(Message request) {

	try {
		m_message = request;
		m_message.operator()();

		Logger::log(info, m_message.str() + std::string(" consumed."));
	}
	catch(boost::exception &ex){
		error = boost::current_exception();
		std::string msg = diagnostic_information(ex);
		Logger::log(exception, msg);
	}
}

bool Sink::cancel() {
	return !channel_.channel_.size();
}

void Sink::started() {
	Logger::log(info, "Consumer started.");
}

void Sink::done() {
	Logger::log(info, "Consumer done.");
}
