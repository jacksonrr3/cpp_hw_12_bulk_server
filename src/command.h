
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <mutex>
#include "observer.h"

std::mutex obs_mutex;
std::mutex metrick_mutex;

using obs_vec_ptr = std::shared_ptr<std::vector<std::unique_ptr<Observer>>>;

class Command {
	obs_vec_ptr _obs;
	commands _comm;
	std::size_t _block_size;
	std::size_t _comm_counter = 0;
	std::size_t _bracket_counter = 0;
	bool _is_reg = true;
	std::string _time;
	std::shared_ptr<Metric> _m_main;
public:
	Command(obs_vec_ptr obs, std::size_t N, const std::string name): _obs(obs), _block_size(N) {
		_m_main = std::make_shared<Metric>(name, true);
		std::lock_guard<std::mutex> lg(metrick_mutex);
		v_m.emplace_back(_m_main); 
	}

	~Command() {
		if (_is_reg) {
			if (_comm_counter) { 
				notify(); 
			}
		}
	}
	
	void subscribe(std::unique_ptr<Observer>&& obs) {
		obs_mutex.lock();
		_obs->emplace_back(std::move(obs));
		obs_mutex.unlock();
	}

	void setTime() {
		//std::time_t temp_time;
		//temp_time = std::time(0);
		//std::stringstream ss;
		//ss << temp_time;
		//ss >> _time;
		std::time_t temp_time = std::time(0);
		_time = std::to_string(temp_time);
	}

	void set_mode(bool b) {
		_is_reg = b;
		_bracket_counter = 1;
	}

	void notify() {
		obs_mutex.lock();
		for (auto& u : *_obs) {
			u->print(_comm, _time);
		}
		obs_mutex.unlock();
		_m_main->_block_ch += 1;
		_m_main->_cmd_ch += _comm.size();
		_comm.clear();
		_time.clear();
		_comm_counter = 0;
	}

	bool add_command(const std::string& s, bool u) {
		_m_main->_str_ch += 1;

		if (s[0] == '{') {
			if (_comm_counter) {
				notify();
				_comm_counter = 0;
					//_is_reg = false;
				
			}
			u = false;
			++_bracket_counter;
				//_is_reg = false;
		}
		else if (s[0] == '}') {
			--_bracket_counter;
			if (!_bracket_counter) {
				notify();
					//_is_reg = true;  
				u = true;
				_bracket_counter = 1;
			}

		}
		else {
			if (_comm.empty()) { setTime(); }
			_comm.push_back(s);
			if (_is_reg) {
				++_comm_counter;
				if (_comm_counter == _block_size) {
					notify();
					_comm_counter = 0;
				}
			}
		}
		return u;
	}



};

