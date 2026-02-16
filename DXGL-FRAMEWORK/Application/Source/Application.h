
#ifndef APPLICATION_H
#define APPLICATION_H

#include "timer.h"
#include <string>
#include <iostream>

class Application
{
public:
	Application();
	~Application();
	void Init();
	void Run();
	void Exit();

	static constexpr float SCREEN_WIDTH = 1600.f;
	static constexpr float SCREEN_HEIGHT = 900.f;
	static constexpr float ASPECT_RATIO = SCREEN_WIDTH / SCREEN_HEIGHT;

	template <typename T>
	static void print(const T& info, unsigned newLineCount = 0) {
		std::string newlines = "";

		for (unsigned i = 0; i < newLineCount; i++)
			newlines += "\n";

		std::cout << info << newlines;
	}

private:

	//Declare a window object
	StopWatch m_timer;

	bool enablePointer = true;
	bool showPointer = true;

};

#endif