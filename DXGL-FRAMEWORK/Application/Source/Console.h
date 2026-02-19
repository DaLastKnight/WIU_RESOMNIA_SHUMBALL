#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <iostream>

template <typename T>
static void Print(const T& info, unsigned newLineCount = 0) {
	std::string newlines = "";

	for (unsigned i = 0; i < newLineCount; i++)
		newlines += "\n";

	std::cout << info << newlines;
}

template <typename T>
static void Error(const T& error) {
	std::cerr << error << "\n\n";
}

#endif