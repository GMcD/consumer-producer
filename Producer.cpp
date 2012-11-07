/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 690 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/

#include <ConProd2.h>

// This is the number of messages that the manager will produce
const int sizeN = 25;

// producer-consumer management
PoolManager::PoolManager(size_t np, size_t nc)
	: MessageManager (np, nc) {
}

PoolManager::~PoolManager(){
}

void PoolManager::pcModelCreated(Source &producer, Sink &consumer) {
	producer.count = sizeN;
	consumer.shared_stuff	= &producer.shared_stuff;
}

const int nProducers = 1;
const int nConsumers = 128;

const char *LevelNames[] = { "Debug", "Info", "Warn", "Error", "Exception", "Fatal" };
Listener s_callee;

void PoolManagerFactory (int argc, char **argv) {

	boost::xtime xt, xt2;
	boost::xtime_get(&xt, boost::TIME_UTC);

	Caller   caller(s_callee);
	s_callee.start();
	caller.shout("Welcome to the management thread!");

//	try {
	PoolManager manager(nProducers, sizeN > nConsumers ? nConsumers : sizeN);

	boost::thread thrd(manager);
	thrd.join();
//  }
//	catch (boost::exception &ex){
//		caller.shout( diagnostic_information(ex) );
//	}
	s_callee.stop();

	boost::xtime_get(&xt2, boost::TIME_UTC);
	long time = boost::lexical_cast<long>((xt2.sec*1000000000 + xt2.nsec - xt.sec*1000000000 - xt.nsec) / 1000000);
	printf("%d producer, max %d consumers: %ld msec\n", nProducers, nConsumers, time);

}
