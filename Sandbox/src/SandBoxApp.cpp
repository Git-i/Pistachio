#define PISTACHIO_RENDER_API_DX11
#include "Pistachio.h"


class Sandbox : public Pistachio::Application
{
public:
	Sandbox() {  }
	~Sandbox(){}
};

Pistachio::Application* Pistachio::CreateApplication()
{
	return new Sandbox;
}