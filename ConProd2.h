/*
* Copyright © Gary MacDonald 2011
* $Author: scapa $
* $Revision: 690 $
* $Date: 2011-09-25 14:48:30 +0100 (Sun, 25 Sep 2011) $
*/
#pragma once

#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <deque>
#include <vector>

#include <boost/exception_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

typedef boost::error_info<struct tag_thread, std::string> thread_info;

class thread_error : public boost::exception { };

#include "SignalListener.hpp"
#include "Control.h"
#include "Message.h"
#include "Consumer.h"
#include "Sink.h"
#include "ThreadPool.h"
#include "Source.h"
#include "Producer.h"
