/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 690 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/
#pragma once

#define WITH_LOGGING

enum Levels { debug, info, warn, error, exception, fatal };
extern const char *LevelNames[];

/////////////////////////////////////////
// typedefs and convenient functions
typedef boost::mutex Mutex;
typedef boost::condition Condition;
typedef boost::mutex::scoped_lock Lock;

inline static void sleep(
	int secs, int msecs = 0, int usecs = 0, int nsecs = 0)
{
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);

	if(nsecs > 1000)
	{
		usecs += nsecs / 1000;
		nsecs = nsecs % 1000;
	}
	if(usecs > 1000)
	{
		msecs += usecs / 1000;
		usecs = usecs % 1000;
	}
	if(msecs > 1000)
	{
		secs += msecs / 1000;
		msecs = msecs % 1000;
	}

	xt.sec += secs;
	xt.nsec += (long)msecs * 1000000L + usecs * 1000L + nsecs;

	boost::thread::sleep(xt);
}

// if you want logging you need to define WITH_LOGGING,
// then call Logger::log(level, "any text"); once in your main thread before using the classes in this file. It assures
// Logger::instance() to run properly (static Mutex s_mu is created un-interruptedly).
#ifdef WITH_LOGGING
#define _WITH_LOGGING
#endif

// debug classes...
struct Logger
{
#ifdef _WITH_LOGGING
// call this once at start of application (main thread) to make sure s_mu is proper
inline static Mutex & instance()
{
	static Mutex s_mu;
	return(s_mu);
}
#endif

inline static void log(Levels level, const std::string &buf)
{
#ifdef _WITH_LOGGING
	// can not protect if an outside thread does not know the mutex and accesses std::cout directly
	Lock lock(instance());
//	std::cout << buf.c_str() << std::endl << std::flush;
	std::string msg = LevelNames[level] + std::string(" : ") + buf;
	Caller caller(s_callee);
	caller.shout(msg);
#endif
}

inline static void beeper(int interval, char * msg)
{
	if(msg) {
	   sleep(interval);
	   log(info, msg);
	}
}
};
