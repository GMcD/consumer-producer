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

#define HAS_PUTTABLETAKABLE

// _Runnable is boost::function0<void> or its derivation, or any class that implements void operator()()
#define RunnablePtr _Runnable *

template < typename _Runnable >
struct RunPool : Pool<RunnablePtr>
{
	RunPool(size_t limit = (size_t)-1)
		: Pool<RunnablePtr>(limit) {}
};

template < typename _Runnable >
struct Runner;

#define RunnerBase Consumer<RunnablePtr, RunPool<_Runnable> >

template < typename _Runnable >
struct Runner : public RunnerBase
{
	typedef Takable<RunnablePtr, RunPool<_Runnable> > TakableRunPool;

	Runner(TakableRunPool& runners,
		Latch& lh)
		: RunnerBase(runners, lh) {}
	Runner(RunPool<_Runnable>& runners,
		Latch& lh)
		: RunnerBase(runners, lh) {}

protected:
	void consume(RunnablePtr runner)
	{
		try {
			runner->operator()();
		} catch (...) {
			Logger::log(exception, "Caught ... in ThreadPool::consume");
		}
	}
	bool cancel() {
		return !channel_.channel_.size();
	}
	void starting() {
	}
	void started() {
	}
	void done() {
	}
};

#define ThreadPoolBase Consuming<RunnablePtr, RunPool<_Runnable>, Runner<_Runnable> >

template < typename _Runnable >
struct ThreadPool : public ThreadPoolBase
{
	bool bStoppableThreadPool;	// in most server pool cases bStoppableThreadPool is default to false, meaning it runs forever...
								// however, in a finite pre-defined task queue case, one would set the flag to make sure proper consumer stop
	ThreadPool(RunPool<_Runnable>& channel, size_t nThreads = 1, bool bCanStop = false)
		: ThreadPoolBase (channel, nThreads, true, true),
		bStoppableThreadPool(bCanStop) {}

	void consumerModelCreated(Runner<_Runnable>& consumer)
	{
		consumer.mayStop(bStoppableThreadPool); //false); // see above comment
	}

	void execute(_Runnable*& runObj)
	{
		channel_.offer(runObj);
	}
};
