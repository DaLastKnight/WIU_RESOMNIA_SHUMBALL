// JH
#ifndef EVENT_H
#define EVENT_H

#include <functional>
#include <utility>
#include <map>

/* how to use | Event:
* || declaration
* Event<return_type, parameter_1, parameter_2 ...> demoEvent;
* 
* || initiation
* demoEvent.Subscribe( lambda_or_function_with_same_parameters_declared ); // if you used lambda, the capture clause range is in this scope
* 
* || use
* demoEvent.Invoke(parameter_1, parameter_2 ...); // calls the lambda / function that you subscribed to with the parameters you pluged in
*/

/* how to use | EventPack:
* || declaration
* EventPack<key_type, return_type, parameter_1, parameter_2 ...> demoEventPack;
*
* || initiation
* demoEventPack.Subscribe(key_variable, lambda_or_function_with_same_parameters_declared ); // use different key to store different lambda / function
*
* || use
* demoEventPack.Invoke(key_variable, parameter_1, parameter_2 ...); // same as Event but with a key to find the lambda / function you subscribed
*/

template<typename Return, typename... Args>
class Event {
public:

	using Listener = std::function<Return(Args...)>;

	void Subscribe(Listener fn) {
		listener = std::move(fn);
	}

	Return Invoke(Args... args) {
		if (listener)
			return listener(std::forward<Args>(args)...);
		return Return{};
	}

private:
	Listener listener;

};
// void handle
template<typename... Args>
class Event<void, Args...> {
public:

	using Listener = std::function<void(Args...)>;

	void Subscribe(Listener fn) {
		listener = std::move(fn);
	}

	void Invoke(Args... args) {
		if (listener)
			listener(std::forward<Args>(args)...);
	}

private:
	Listener listener;

};


template<typename Key, typename Return, typename... Args>
class EventPack {
public:

	using Listener = std::function<Return(Args...)>;

	bool Subscribe(const Key& key, Listener fn) {
		auto it = listeners.find(key);
		if (it != listeners.end())
			return false;
		listeners[key] = std::move(fn);
		return true;
	}

	Return Invoke(const Key& key, Args... args) {
		auto it = listeners.find(key);
		if (it != listeners.end())
			return it->second(std::forward<Args>(args)...);
		return Return{};
	}

private:
	std::map<Key, Listener> listeners;
};
// void handle
template<typename Key, typename... Args>
class EventPack<Key, void, Args...> {
public:

	using Listener = std::function<void(Args...)>;

	bool Subscribe(const Key& key, Listener fn) {
		auto it = listeners.find(key);
		if (it != listeners.end())
			return false;
		listeners[key] = std::move(fn);
		return true;
	}

	void Invoke(const Key& key, Args... args) {
		auto it = listeners.find(key);
		if (it != listeners.end())
			it->second(std::forward<Args>(args)...);
	}

private:
	std::map<Key, Listener> listeners;
};

#endif