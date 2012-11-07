/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 690 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/
#pragma once

typedef Production<Message, MessagePool, Source, Sink > MessageManager;

// producer-consumer management
struct PoolManager : public MessageManager
{
	explicit PoolManager(size_t np, size_t nc);
	~PoolManager();

	virtual void pcModelCreated(Source &producer, Sink &consumer);

};

// This sparks a Producer Consumer Factory
void PoolManagerFactory (int argc, char **argv);
