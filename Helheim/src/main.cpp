#include<iostream>
#include<Windows.h>
#include "application.h"

void OnRender(bool& exit) 
{
	ImGui::SetWindowSize(ImVec2(300, 300), 0);
	ImGui::Begin("Hello", &exit);
	ImGui::Button("Button");
	ImGui::End();
	ImGui::ShowDemoWindow();
}

int main()
{
	Helheim::Application* app = Helheim::Application::getInstance();


	app->RenderLoop(OnRender);

	return 0;
}