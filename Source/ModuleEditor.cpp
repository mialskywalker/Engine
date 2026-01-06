#include "Globals.h"
#include "ModuleEditor.h"

#include "ImGuiPass.h"
#include "Application.h"
#include "D3D12Module.h"

ModuleEditor::ModuleEditor() {}

ModuleEditor::~ModuleEditor()
{
	delete imGuiPass;
}

bool ModuleEditor::init()
{
	D3D12Module* d3d12 = app->getD3D12();
	imGuiPass = new ImGuiPass(d3d12->getDevice(), d3d12->getHwnd());

	return true;
}

void ModuleEditor::update()
{
    fpsHistory[historyIndex] = float(app->getFPS());
    msHistory[historyIndex] = float(app->getElapsedMilis());

    historyIndex = (historyIndex + 1) % HISTORY_SIZE;

    if (app->getD3D12()->getVsync() != vsync)
        app->getD3D12()->setVSync(vsync);
}

void ModuleEditor::fps()
{
	ImGui::Begin("Configuration");
	ImGui::SetNextWindowSize(ImVec2(480, 720), ImGuiCond_Always);

    ImGui::Text("Options");

    if (ImGui::CollapsingHeader("Rendering"))
    {
        ImGui::Checkbox("VSync", &vsync);
    }

    if (ImGui::CollapsingHeader("Application"))
    {
        ImGui::Text("App Name:");
        ImGui::SameLine();
        ImGui::Text("Engine");

        int lastIndex = (historyIndex - 1 + HISTORY_SIZE) % HISTORY_SIZE;

        char title[25];
        sprintf_s(title, "Framerate: %.1f", fpsHistory[lastIndex]);
        ImGui::PlotHistogram("##framerate", fpsHistory, HISTORY_SIZE, historyIndex, title, 0.0f, 3000.0f, ImVec2(310, 100));

        sprintf_s(title, "Milliseconds: %.1f", msHistory[lastIndex]);
        ImGui::PlotHistogram("##milliseconds", msHistory, HISTORY_SIZE, historyIndex, title, 0.0f, 30.0f, ImVec2(310, 100));
    }

    ImGui::End();
}
