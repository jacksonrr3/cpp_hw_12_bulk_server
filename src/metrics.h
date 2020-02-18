#pragma once
#include <string>
#include <iostream>
#include <mutex>
#include <vector>
#include <memory>

std::mutex console_m;

class Metric {
public:
	std::size_t _str_ch = 0;
	std::size_t _cmd_ch = 0;
	std::size_t _block_ch = 0;
	std::string _name;
	const bool _is_main;

	Metric(const std::string name, bool is_main = false) : _name(name), _is_main(is_main) {}

	~Metric() {
		console_m.lock();
		std::cout << _name << " thread - ";
		if (_is_main) {
			std::cout << _str_ch << " string, " << _cmd_ch << " command, " <<
				_block_ch << " blok." << std::endl;
		}
		else {

			std::cout << _block_ch << " blok, " << _cmd_ch << " command." << std::endl;
		}
		console_m.unlock();
	}
};

std::vector<std::shared_ptr<Metric>> v_m;


