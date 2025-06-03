#pragma once
class InputManager
{
public:
	InputManager();
	static void initialize();
	static InputManager* getInstance();
	void handleMainConsoleInput();
	void destroy();

private:
	static InputManager* inputManager;
};
