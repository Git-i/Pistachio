#include "ptpch.h"
#include "ConsolePanel.h"
#include "imgui.h"
#include <streambuf>
#include <iostream>
#include <sstream>
#include "Pistachio/Core/Input.h"


namespace Pistachio
{
	ConsolePanel::ConsolePanel() : console("Debug Console")
	{
        // Log example information:
		

        ///////////////////////////////////////////////////////////////////////////
	}
	void ConsolePanel::OnImGuiRender()
	{
		if (activated)
		{
			console.Draw();
		}
	}
}