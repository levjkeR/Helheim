#include<iostream>
#include<Windows.h>
#include "application.h"

int main()
{
	Helheim::Application* app = Helheim::Application::getInstance();

	app->RenderLoop();

	return 0;
}