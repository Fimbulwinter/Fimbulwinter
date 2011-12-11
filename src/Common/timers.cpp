#include <timers.hpp>

std::map<int, struct TimerManager::TimerData> TimerManager::timers_;
int TimerManager::timer_nid;
boost::asio::io_service *TimerManager::ios; 
