#pragma once
#include <src/Application/Application.h>
#include <src/Window.h>

#include "Windows/EditorInterface.h"
#include "Misc/GizmoDrawer.h"

#include "Commands/CommandBuffer.h"

using namespace NuakeEditor;

class EditorApplication : public Nuake::Application
{
private:
	CommandBuffer mCommandBuffer;

public:
	EditorApplication(const Nuake::ApplicationSpecification& specification)
		: Application(specification),
		m_Editor(nullptr)
	{
	}

	virtual void OnInit() override;

	// virtual void OnUpdate() override;
	virtual void OnShutdown() override;

private:
	Nuake::EditorInterface* m_Editor;
};