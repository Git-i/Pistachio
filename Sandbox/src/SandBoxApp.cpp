#include "ptpch.h"
#include "Pistachio.h"
#pragma comment (linker, "/SUBSYSTEM:WINDOWS")
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