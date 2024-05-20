#include "Engine/App.h"

#include <stdexcept>
#include <iostream>

int main() {
	Engine::App app;

	try
	{
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	std::cout << "\nApp Ended successfuly\n" << std::endl;

	return 0;
}