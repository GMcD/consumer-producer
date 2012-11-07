#
# Copyright © Gary MacDonald 2011
# $Author: scapa $
# $Revision: 690 $
# $Date: 2011-08-01 13:53:06 +0100 (Mon, 01 Aug 2011) $
#
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ConProd)

#
# Boost
#
FIND_PACKAGE ( Boost )

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}
       ${Boost_INCLUDE_DIRS}
)
LINK_DIRECTORIES(
	${Boost_LIBRARY_DIRS}
)

ADD_EXECUTABLE(cp2 cp2.cpp Control.h SignalListener.cpp SignalListener.hpp Message.cpp Message.h Sink.cpp Sink.h Source.cpp Source.h Producer.cpp Consumer.h Producer.h Threadpool.h ConProd2.h )

