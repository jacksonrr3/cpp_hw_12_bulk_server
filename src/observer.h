/**
\file
\brief Çàãîëîâî÷íûé ôàéë ñ îïèñàíèåì êëàññîâ
Äàííûé ôàéë ñîäåðæèò â ñåáå îïðåäåëåíèÿ êëàññîâ, èñïîëüçóåìûõ â ïðîãðàììå
*/

#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <thread>
#include <future>
#include <string>
#include <memory>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <tuple>
#include <functional>
#include "fibfac.h"
#include "metrics.h"

using commands = std::vector<std::string>;

class data_pack {
public:
	commands _command_pack;
	std::string _time;
};

void worker(std::function<void(const commands&, const std::string)> f, std::queue<data_pack>& q,
	std::condition_variable& cv, std::mutex& cv_m, std::atomic_bool& quit, std::shared_ptr<Metric> m) {
	while (true) {
		std::unique_lock<std::mutex> lk(cv_m);
		cv.wait(lk, [&]() {return !q.empty() || quit; });
		if (quit && q.empty()) { return; }
		if (!q.empty()) {
			commands a = q.front()._command_pack;
			std::string b = q.front()._time;
			m->_block_ch += 1;
			m->_cmd_ch += a.size();
			q.pop();
			lk.unlock();
			f(a, b);
		}
	}
}

void print_to_terminal(const commands& comm, const std::string&) {
	std::lock_guard<std::mutex> l_g(console_m);
	std::cout << "Bulk: ";
	bool first = true;
	for (auto& command : comm) {
		if (!first) { std::cout << ","; }
		//std::cout << fac(std::stoi(command));
		std::cout << command;
		first = false;
	}
	std::cout << std::endl;
}

void print_to_file(const commands& comm, const std::string& time) {
	static std::atomic_int file_id = 11;
	std::ofstream file;
	std::string path("bulk" + std::to_string(file_id) + time + ".log");
	++file_id;
	file.open(path);
	for (auto& command : comm) {
		//file << fib(std::stoi(command)) << "\n";
		file << command << "\n";
	}
	file.close();
}

/**
* @brief áàçîâûé êëàññ, äëÿ ðåàëèçàöèè êëàññîâ âûâîäà êîìàíä
*
*/

class Observer {
public:
	virtual void print(const commands&, const std::string&) = 0;
	virtual ~Observer() = default;

	std::condition_variable _cv;
	std::mutex _cv_m;
	std::atomic_bool _quit = false;
};


/**
* @brief êëàññ ðåàëèçóþùèé ôóíêöèþ âûâîäà êîìàíä â ôàéë
*
*/

class FileObserver : public Observer {
public:
	FileObserver() {

		_vtr.emplace_back(std::thread(worker, std::function<void(const commands&, const std::string&)>(print_to_file),
			std::ref(_data),
			std::ref(_cv),
			std::ref(_cv_m),
			std::ref(_quit),
			std::ref(_file1)
		));
		_vtr.emplace_back(std::thread(worker, std::function<void(const commands&, const std::string&)>(print_to_file),
			std::ref(_data),
			std::ref(_cv),
			std::ref(_cv_m),
			std::ref(_quit),
			std::ref(_file2)
		));

		v_m.emplace_back(_file1);
		v_m.emplace_back(_file2);
	}

	~FileObserver() {
		_quit = true;
		_cv.notify_all();
		for (auto& v : _vtr) {

			if (v.joinable())
				v.join();
		}
	}

	virtual void print(const commands& comm, const std::string& time) {
		{
			std::lock_guard<std::mutex> lk(_cv_m);
			_data.emplace(data_pack{ comm, time });
		}
		_cv.notify_one();
	}

	std::vector<std::thread> _vtr;
	std::queue<data_pack> _data;
	std::shared_ptr<Metric> _file1 = std::make_shared<Metric>("File1");
	std::shared_ptr<Metric> _file2 = std::make_shared<Metric>("File2");
};


/**
* @brief êëàññ ðåàëèçóþùèé ôóíêöèþ âûâîäà êîìàíä â êîíñîëü
*
*/

class TerminalObserver : public Observer {
public:
	TerminalObserver() {
		_vtr.emplace_back(std::thread(worker, std::function<void(const commands&, const std::string&)>(print_to_terminal),
			std::ref(_data),
			std::ref(_cv),
			std::ref(_cv_m),
			std::ref(_quit),
			std::ref(_log)));
		//console_m.lock();
		v_m.emplace_back(_log);
	}

	~TerminalObserver() {
		_quit = true;
		_cv.notify_all();
		//console_m.unlock();
		for (auto& v : _vtr) {
			if (v.joinable())
				v.join();
		}
	}

	virtual void print(const commands& comm, const std::string& time) {
		{
			std::lock_guard<std::mutex> lk(_cv_m);
			_data.emplace(data_pack{ comm, time });
		}
		_cv.notify_one();
	}

	std::queue<data_pack> _data;
	std::vector<std::thread> _vtr;
	std::shared_ptr<Metric> _log = std::make_shared<Metric>("log");
};

