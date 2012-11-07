/*
* Copyright © Gary MacDonald 2011
* Portions © 2002 by Ted T. Yuan
* $Author: scapa $
* $Revision: 679 $
* $Date: 2011-09-21 12:03:57 +0100 (Wed, 21 Sep 2011) $
*/
#pragma once

// The Producer Consumer pattern produces and consumes 'Message' objects, which are defined in ConProd2.h 
typedef std::vector<Message>			Messages;
typedef Pool<Message>					MessagePool;
typedef Consumer<Message, MessagePool >	MessageConsumer;
typedef Producer<Message, MessagePool >	MessageProducer;

class Sink : public MessageConsumer {
	Message m_message;
public:
	Sink( MessagePool& slices, Latch& lh);
	std::string *shared_stuff;
	boost::exception_ptr error;

protected:
	virtual void consume(Message message_i);
	virtual bool cancel();
	virtual void started();
	virtual void done();
};

