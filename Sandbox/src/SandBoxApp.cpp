#include "Pistachio.h"

class Sandbox : public Pistachio::Application
{
public:
	Sandbox(){}
	~Sandbox(){}
	void Run() {
		
	}
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}