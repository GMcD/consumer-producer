#include "SignalListener.hpp"

void Listener::logMessage(std::string words) {
	std::cout << words << std::endl;
}

void Listener::hearMe(std::string words)
{
	service.post(boost::bind(&Listener::logMessage, this, words));
}

void Listener::loop()
{
	service.run();
}

void Listener::start(){
	tp = new boost::thread(boost::bind(&Listener::loop, this));
}

void Listener::stop(){
	tp->join();
	delete tp;
}

Caller::Caller(Listener &l){
	m_signal.connect(boost::bind(&Listener::hearMe, boost::ref(l), _1));
}

void Caller::shout(const std::string &words){
	m_signal(words);
}
