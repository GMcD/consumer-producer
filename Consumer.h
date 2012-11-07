/*
* Copyright © Gary MacDonald 2011
* Portions © 2002 by Ted T. Yuan
* $Author: scapa $
* $Revision: 690 $
* $Date: 2011-09-21 12:03:57 +0100 (Wed, 21 Sep 2011) $
*/
#pragma once

//
// Copyright (c) 2002 by Ted T. Yuan.
//
// Permission is granted to use this code without restriction as long as this copyright notice appears in all source files.
//

// in most cases, you do not want to define HAS_PUTTABLETAKABLE,
// it forces you to define Takable, Puttable in your code
#ifdef HAS_PUTTABLETAKABLE
#define _HAS_PUTTABLETAKABLE
#endif

/////////////////////////////////////////
// forward declarations...
class Latch;
class Gate;

// sync'd implementation of the corresponding std container classes
template < typename _Tp, typename _queueTp >
	class Channel; // intend to be a linear queue, FIFO
template < typename _Tp > struct Pool;
template < typename _Tp >
	struct Channels; // a pool of Channel objects

// access syntax api of the above implementation,
// _ChannelTp can be Channel or Channels
// TODO - _ChannelTp should be defined in a trait class...
template < typename _Tp, typename _ChannelTp > struct Takable;
template < typename _Tp, typename _ChannelTp > struct Puttable;

// producer-consumer idiom implemetation templates
template < typename _Tp, typename _ChannelTp > class Producer;
template < typename _Tp, typename _ChannelTp > class Consumer;

// Production has producers and consumers which are connected through queue
template < typename _Tp, typename _ChannelTp,
	typename _ProducerTp, typename _ConsumerTp >
	class Production;

// Consuming is a pool of consumers to consume a static queue
template < typename _Tp, typename _ChannelTp,
	typename _ConsumerTp > class Consuming;



class pc_exception : public std::runtime_error
{
public:
	explicit pc_exception(const std::string& msg)
		: std::runtime_error(msg) {}
};

/////////////////////////////////////////
// implementation


// _queueTp has to have Container traits
template < typename _Tp, typename _queueTp = std::deque<_Tp> >
class Channel : private _queueTp
{
private:
	size_t maxSize_;
	Mutex monitor_;
	Condition bufferNotFull_, bufferNotEmpty_;
	volatile bool bMayStop_;

public:
	explicit Channel(size_t limit = (size_t)-1) : maxSize_(limit < 1 ? 1 : limit), bMayStop_(false) {}

	Channel(_queueTp& queue)
		: _queueTp(queue), maxSize_(queue.max_size()) {}

	// for consumer thread...
	_Tp poll(long msecs = -1) // ignore msecs for now
	{
		Lock lk(monitor_);
        while (!bMayStop_ && 0 == ((_queueTp *)this)->size())
		{
            bufferNotEmpty_.wait(lk);
		}

		// outside caller intentionally calls for stop, last resort?
		if(bMayStop_ && 0 == ((_queueTp *)this)->size())
			throw pc_exception("consumer to end");

		// pop back
		_Tp item = pop();
		bufferNotFull_.notify_one();
		return item;
	}

	// for producer thread...
	bool offer(_Tp item, long msecs = -1) // ignore msecs for now
	{
		Lock lk(monitor_);
        while (maxSize_ == ((_queueTp *)this)->size())
		{
            bufferNotFull_.wait(lk);
		}

		// push front
		push(item);
		bufferNotEmpty_.notify_one();
		return true;
	}

	virtual void mayStop(bool bMayStop = true)
	{
		Lock lk(monitor_);
		bMayStop_ = bMayStop;
		if(bMayStop) // if outside says may stop, wake up the waiting threads...
		{
			bufferNotEmpty_.notify_all();
		}
	}

	// for outside callers only, calling this on Linux 7.2 within
	// the above poll() or offer() will deadlock
	typename _queueTp::size_type size()
	{
		Lock lk(monitor_);
		return ((_queueTp *)this)->size();
	}
protected:
	virtual _Tp pop() // retrieve from end
	{
		_Tp item = ((_queueTp *)this)->back();
		((_queueTp *)this)->pop_back();
		return item;
	}

	virtual void push(const _Tp item) // insert at head...
	{
		((_queueTp *)this)->insert(((_queueTp *)this)->begin(), item);
	}
};

// Pool provides thread-safe access to std::vector<_Tp>
template < typename _Tp >
struct Pool : public Channel< _Tp, std::vector<_Tp> > {
	Pool(size_t limit = (size_t)-1) : Channel< _Tp, std::vector<_Tp> >(limit) {}

	Pool(std::vector<_Tp>& queue)
		: Channel< _Tp, std::vector<_Tp> >(queue) {}
};

// Channels is intended to be a thread-safe vector of thread-safe queues...
template < typename _Tp >
struct Channels : public Pool< Channel<_Tp> >
{
	Channels(size_t size = (size_t)-1) : Pool< Channel<_Tp> >(size) {}

	_Tp poll(long msecs = -1) // ignore msecs for now
	{
		Channel<_Tp> channel = ((Pool< Channel<_Tp> >*)this)->poll(msecs);
		return channel.poll(msecs);
	}
	bool offer(_Tp item, long msecs = -1) // ignore msecs for now
	{
		Channel<_Tp> channel = poll(msecs);
		return channel.offer(item, msecs);
	}
};

// taker API on top of thread-safe _ChannelTp
template < typename _Tp, typename _ChannelTp = Channel<_Tp> >
struct Takable // thread-safe storage iterator
{
	_ChannelTp& channel_;
	Takable(_ChannelTp& chan) : channel_(chan) {}

	virtual _Tp take(long msecs = -1)
	{
		try {
			return channel_.poll(msecs);
		} catch (pc_exception& pce) { 
			throw pce;
		} catch (boost::lock_error& err) { 
			Logger::log(exception, err.what()); 
			throw (int)err.what();
		} catch (std::exception& err) { 
			Logger::log(exception, err.what()); 
			throw (int)err.what();
		} catch (...) { 
			Logger::log(exception, "Caught ... in Takable"); 
			throw -1; 
		}
	}
};

// inserter API on top of thread-safe _ChannelTp
template < typename _Tp, typename _ChannelTp = Channel<_Tp> >
struct Puttable // thread-safe storage inserter
{
	_ChannelTp& channel_;
	Puttable(_ChannelTp& chan) : channel_(chan) {}

	virtual void put(_Tp item, long msecs = -1)
	{
		try {
			channel_.offer(item, msecs);
		} catch (boost::lock_error& err) { 
			Logger::log(exception, err.what()); 
			throw (int)err.what();
		} catch (std::exception& err) { 
			Logger::log(exception, err.what()); 
			throw (int)err.what();
		} catch (...) { 
			Logger::log(exception, "Caught ... in Puttable"); 
			throw -1; 
		}
	}
};

// Locking mechanisms
class Latch
{
protected:
	volatile bool state_;
#define latched_ state_
	Mutex monitor_;
	Condition cond_;
public:
	explicit Latch(bool state = false) : latched_(state) {}
	bool attempt(long msecs)
	{
		return false; // function holder, not implemented yet
	}
	void acquire()
	{
		Lock lk(monitor_);
		while (!latched_)
			cond_.wait(lk);
	}
	// enables all current and future acquires to pass
	void release(bool bRelease = true)
	{
		Lock lk(monitor_);
		latched_ = bRelease;
		cond_.notify_all();
	}
#undef latched_
};

// use object pointers of type _Tp to go around STL's value semantic...
template < typename _Tp, typename _ChannelTp = Channel<_Tp> >
class Producer
{
	Puttable<_Tp, _ChannelTp> * ptr_holder_;
	boost::shared_ptr< Puttable<_Tp, _ChannelTp> > holder_;
protected:
	Puttable<_Tp, _ChannelTp>& channel_;
	Latch& latch_;
	volatile bool bMayStop_;

public:
	Producer(Puttable<_Tp, _ChannelTp>& channel, Latch& lh)
		: channel_(channel), latch_(lh), bMayStop_(true) {}

	Producer(_ChannelTp& channel, Latch& lh)
		: ptr_holder_( new Puttable<_Tp, _ChannelTp>(channel) ),
		holder_(ptr_holder_), channel_(*holder_),
		latch_(lh), bMayStop_(true) {}

	void operator()()
	{
	  latch_.acquire();
	  started();

	  // bMayStop_ is a suggest to stop from outside, overwritable cancel() is internal condition...
      for(;!(bMayStop_ && cancel());)
	  {
		  try
		  {
			_Tp o = produce();
			channel_.put(o);
		  } catch (pc_exception) {
			break;
		  } catch (...) { // you threw
			Logger::log(exception, "Caught ... Producer::produce");
			break;
		  }

		boost::thread::yield(); // need breath...
      }

	  done();
	}

	virtual void mayStop(bool bMayStop = true)
		{ channel_.channel_.mayStop(bMayStop_ = bMayStop); }

protected: // overwritable by derived classes...
	virtual _Tp produce() { // value semantic, _Tp may need to have a copy constructor...
		Logger::beeper(1, "producer not implemented");
		throw -1 ;
	}
	virtual bool cancel() { return false; } // defualt implementation is never to cancel
	virtual void started() {
		Logger::log(info, "producer started");
	}
	virtual void done() {
		Logger::log(info, "producer done");
	}
};

template < typename _Tp, typename _ChannelTp = Channel<_Tp> >
class Consumer
{
	Takable<_Tp, _ChannelTp> * ptr_holder_;
	boost::shared_ptr< Takable<_Tp, _ChannelTp> > holder_;
protected:
	Takable<_Tp, _ChannelTp>& channel_;
	Latch& latch_;
	long checkEmptyWithinMSec_;
	volatile bool bMayStop_;

public:
	Consumer(Takable<_Tp, _ChannelTp>& channel, Latch& lh)
		: channel_(channel), checkEmptyWithinMSec_(-1),
		latch_(lh), bMayStop_(true) {}

	Consumer(_ChannelTp& channel, Latch& lh)
		: ptr_holder_( new Takable<_Tp, _ChannelTp>(channel) ),
		holder_(ptr_holder_), channel_(*holder_),
		checkEmptyWithinMSec_(-1), latch_(lh), bMayStop_(true) {}

	void operator()()
	{
	  latch_.acquire();
	  started();

	  // bMayStop_ is a suggest to stop from outside, overwritable cancel() is internal condition...
      for(;!(bMayStop_ && cancel());)
	  {
		try {
			// wait for within_msec milli-seconds to make sure no new tasks coming in...
			_Tp item = channel_.take(checkEmptyWithinMSec_ < 0 ? -1 : checkEmptyWithinMSec_);
			consume(item);
		} catch (pc_exception) { // thrown by mayStop() in _ChannelTp
			break;
		} catch (...) { // you threw
			Logger::log(exception, "Caught ... in Consumer::consume");
			break;
		}

		boost::thread::yield(); // need breath...
      }

	  done();
	}

	virtual void mayStop(bool bMayStop = true)
		{ channel_.channel_.mayStop(bMayStop_ = bMayStop); }

protected: // overwritable by derived classes...
	virtual void consume(_Tp x) {
	   Logger::log(warn, "Consumer not implemented");
	}
	virtual bool cancel() { return false; } // default implementation is never to cancel
	virtual void started() { // default implementation does nothing
		Logger::log(info, "consumer started");
	}
	virtual void done() { // defualt implementation does nothing
		Logger::log(info, "consumer done");
	}
};

// thread-safe, sustainable production-line logic that chains producers with consumers
template < typename _Tp,
	typename _ChannelTp = Channel<_Tp>,
	typename _ProducerTp = Producer<_Tp, _ChannelTp>,
	typename _ConsumerTp = Consumer<_Tp, _ChannelTp> >
class Production
{
private:
	size_t queueLen_, nProducers_, nConsumers_;
	bool syncStart_, bHidePuttableTakable_;
public:
	// bHideTakable indicates derived classes of Producer and Consumer
	// do not have custom implementation of Puttable and Takable interfaces
	explicit Production(size_t np = 1, size_t nc = 1,
		bool sc = true, size_t ql = (size_t)-1, bool bHideTakable = false)
		: nProducers_(np), nConsumers_(nc), syncStart_(sc),
		queueLen_(ql), bHidePuttableTakable_(bHideTakable) {}

	void operator()()
	{
		_ChannelTp chan(queueLen_);

		Latch theLatch, noLatch(true);

#if defined(_HAS_PUTTABLETAKABLE)
		Puttable<_Tp, _ChannelTp> puttable(chan);
		Takable<_Tp, _ChannelTp>  takable(chan);

		_ProducerTp* producer = bHidePuttableTakable_
			? new _ProducerTp(chan, noLatch)
			: new _ProducerTp(puttable, noLatch) ;
		_ConsumerTp* consumer = bHidePuttableTakable_
			? new _ConsumerTp(chan, syncStart_ ? theLatch : noLatch)
			: new _ConsumerTp(takable, syncStart_ ? theLatch : noLatch);
#else
		_ProducerTp* producer =
			new _ProducerTp(chan, noLatch);
		_ConsumerTp* consumer =
			new _ConsumerTp(chan, syncStart_ ? theLatch : noLatch);
#endif
		std::auto_ptr<_ProducerTp> prodClean(producer);
		std::auto_ptr<_ConsumerTp> consClean(consumer);
		consumer->mayStop(false);

		pcModelCreated(*producer, *consumer);

		try {
			boost::thread_group pthreads, cthreads;
			size_t i;
			for (i = 0; i < nProducers_; ++i)
				pthreads.create_thread(*producer);

			for (i = 0; i < nConsumers_; ++i)
				cthreads.create_thread(*consumer);

			theLatch.release();

			beforeJoin();
			pthreads.join_all();
			// wait for tasks to be processed by consumers, then tell them to stop...not really needed?
			while(chan.size() > 0) sleep(0, 10);
			consumer->mayStop(true); // producers are done, consumers *may* stop...

			cthreads.join_all();
			afterJoin();
		} catch (boost::lock_error& err) {
			Logger::log(warn, err.what());
		} catch (std::exception& err) {
			Logger::log(exception, err.what());
		} catch (...) {
			Logger::log(exception, "Caught ... in Production.operator()()");
		}
	}
	// sub-classes overwritables ...
	// monitoring...
	virtual void pcModelCreated(_ProducerTp& producer, _ConsumerTp& consumer) {}
	virtual void beforeJoin() {
		Logger::log(debug, "production before join");
	}
	virtual void afterJoin() {
		Logger::log(debug, "production after join");
	}
};

template < typename _Tp,
	typename _ChannelTp = Channel<_Tp>,
	typename _ConsumerTp = Consumer<_Tp, _ChannelTp> >
class Consuming
{
protected:
	size_t nConsumers_;
	bool syncStart_, bHidePuttableTakable_;
	_ChannelTp& channel_;
public:

	// bHideTakable indicates derived classes of Consumer
	// do not have custom implementation of Takable interfaces
	Consuming(_ChannelTp& channel, size_t nc = 1,
		bool sc = true, bool bHideTakable = false) //, Gate * gate = NULL)
		: channel_(channel), nConsumers_(nc), syncStart_(sc),
		bHidePuttableTakable_(bHideTakable) //, channelReadGate(gate)
	{}

	void operator()()
	{
		Latch theLatch, noLatch(true);

#if defined(_HAS_PUTTABLETAKABLE)
		Takable<_Tp, _ChannelTp>  takable(channel_); // valid in this function scope

		// consumer is created here, and it is shared by all consumer threads...
		// in a sense the single consumer is the model for a group of threaded live consumers
		_ConsumerTp* consumer = bHidePuttableTakable_
			? new _ConsumerTp(channel_, syncStart_ ? theLatch : noLatch)
			: new _ConsumerTp(takable, syncStart_ ? theLatch : noLatch);
#else
		_ConsumerTp* consumer = new _ConsumerTp(channel_, syncStart_ ? theLatch : noLatch);
#endif
		std::auto_ptr<_ConsumerTp> consClean(consumer);
		consumer->mayStop(true);
		consumerModelCreated(*consumer);

		boost::thread_group threads;
		try {
			for (size_t i = 0; i < nConsumers_; ++i)
				threads.create_thread(*consumer);
		} catch (boost::lock_error& err) {
			Logger::log(warn, err.what());
		} catch (std::exception& err) {
			Logger::log(exception, err.what());
		} catch (...) {
			Logger::log(exception, "Caught ... in Consuming.operator()()");
		}

		theLatch.release();

		beforeJoin();
		threads.join_all();
		afterJoin();
	}

	// sub-classes overwritables ...
	// monitoring...
	virtual void consumerModelCreated(_ConsumerTp& consumer) {}
	virtual void beforeJoin() {
		Logger::log(info, "Consuming before join");
	}
	virtual void afterJoin() {
		Logger::log(info, "Consuming after join");
	}
	// if you set consumer->mayStop(false) in consumerModelCreated, like in ThreadPool
	// this is the only way to stop the pool later by an application...
	virtual void mayStop(bool bMayStop = true)
		{ channel_.mayStop(bMayStop); }
};
