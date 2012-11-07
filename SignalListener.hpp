#include <iostream>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/signals2.hpp>
#include <boost/asio/io_service.hpp>

struct Listener
{
	boost::thread *tp;
	boost::asio::io_service service;
	void logMessage(std::string words);

	void hearMe(std::string words);

	void loop();
	void start();
	void stop();
};

struct Caller {
	boost::signals2::signal<void (std::string)> m_signal;
	Caller(Listener &l);
	void shout(const std::string &words);
};

extern Listener s_callee;
