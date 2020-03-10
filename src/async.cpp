#include <map>
#include <algorithm>
#include <functional>
#include <mutex>
#include "async.h"
#include "command.h"

namespace async {
	
	std::mutex main_handler_mutex;
	std::mutex users_mutex;

	obs_vec_ptr observers = std::make_shared<std::vector<std::unique_ptr<Observer>>>();
	static std::shared_ptr<Command> _main_handler;

	class User {
		
		bool _main_mode = true;
	public:
		std::string _str = "";
		std::unique_ptr<Command> _command_handler;
				
		User(std::size_t n, std::size_t id){

			main_handler_mutex.lock();
			if (_main_handler == nullptr) {
				_main_handler = std::make_shared<Command>(observers, n, "main");
				_main_handler->subscribe(std::make_unique<FileObserver>());
				_main_handler->subscribe(std::make_unique<TerminalObserver>());
			}
			main_handler_mutex.unlock();
			_command_handler = std::make_unique<Command>(observers, n, std::string("user" + std::to_string(id)));
			_command_handler->set_mode(false);
		}

		bool is_main() const{
			return _main_mode;
		}

		void set_command_mode(bool b) {
			_main_mode = b;
		}

		void add_main(const std::string& s) {
			main_handler_mutex.lock();
			_main_mode = _main_handler->add_command(s, _main_mode);
			main_handler_mutex.unlock();
		}

		void add_personal(const std::string& s) {
			_main_mode = _command_handler->add_command(s, _main_mode);
		}
		
	};

	
	static std::size_t user_id;
	std::map<int, std::shared_ptr<User>> users;


	handle_t connect(std::size_t bulk) {
		std::lock_guard<std::mutex> lg(users_mutex);
		++user_id;
		users.emplace(std::make_pair(user_id, std::make_shared<User>(bulk, user_id)));
		return reinterpret_cast<handle_t>(user_id);
	}


	void receive(handle_t handle, const char *data, std::size_t size) {
		std::size_t id = reinterpret_cast<std::size_t>(handle);
		
	//	std::lock_guard<std::mutex> lg(users_mutex);
		users_mutex.lock();
		auto us = users.find(id);
		if (us == users.end()) {
			std::cout << "wrong handle!" << std::endl;
			users_mutex.unlock();
			return;
		}
		std::string user_str = us->second->_str;
		users_mutex.unlock();

		user_str.append(data, size);
		auto it = user_str.find('\n', 0);
		while (it != user_str.npos) {
			std::lock_guard<std::mutex> lg(users_mutex);
			if (users[id]->is_main()) {
				users[id]->add_main(user_str.substr(0, it));
			}
			else {
				users[id]->add_personal(user_str.substr(0, it));
			}
			user_str.erase(0, it + 1);
			it = user_str.find('\n', 0);
		}
		us->second->_str = user_str;
	
	}

	void disconnect(handle_t handle) {
		std::size_t id = reinterpret_cast<std::size_t>(handle);
		users_mutex.lock();
		//if (!users[id]->is_main()) {users[id]->_command_handler->notify();} // по условию, если блок команд ограниченный {} не закончен, то команды теряются.
		if (users.size() == 1) {					      // если наш пользователь был последним, 
			main_handler_mutex.lock();					//то выводим команды оставшиеся в основном обработчике.
			_main_handler->notify();
			main_handler_mutex.unlock();
			}
		users.erase(id);
		users_mutex.unlock();
	}

}



