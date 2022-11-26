#include "Pistachio.h"

class Sandbox : public Pistachio::Application
{
public:
	Sandbox(){}
	~Sandbox(){}
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}