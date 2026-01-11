#include "Globals.h"
#include "ModuleEditor.h"

#include "ImGuiPass.h"
#include "Application.h"
#include "D3D12Module.h"
#include "CameraModule.h"
#include "Model.h"
#include "ImGuizmo.h"

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
    
    if (app->getCamera()->getCameraEnabled() != camera)
        app->getCamera()->setCameraEnabled(camera);

    if (app->getCamera()->getFOV() != fov)
        app->getCamera()->setCameraFOV(fov);
}

void ModuleEditor::mainSettings()
{
	ImGui::Begin("Configuration");
	ImGui::SetNextWindowSize(ImVec2(480, 720), ImGuiCond_Always);
    char title[25];

    ImGui::Text("Options");

    if (ImGui::CollapsingHeader("Camera"))
    {
        ImGui::Checkbox("Camera Free Look", &camera);
        sprintf_s(title, "FOV");
        ImGui::SliderFloat(title, &fov, 0.5f, 2.0f);
        sprintf_s(title, "Reset FOV");
        if (ImGui::Button(title, ImVec2(96, 32)))
            fov = 1.0f;

        ImGui::Combo("Sampler", &samplerIndex, "Billinear Filtering Wrap\0Point Filtering Wrap\0Billinear Filtering Clamp\0Point Filtering Clamp", 4);
    }

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

        sprintf_s(title, "Framerate: %.1f", fpsHistory[lastIndex]);
        ImGui::PlotHistogram("##framerate", fpsHistory, HISTORY_SIZE, historyIndex, title, 0.0f, 3000.0f, ImVec2(310, 100));

        sprintf_s(title, "Milliseconds: %.1f", msHistory[lastIndex]);
        ImGui::PlotLines("##milliseconds", msHistory, HISTORY_SIZE, historyIndex, title, 0.0f, 30.0f, ImVec2(310, 100));
    }

    ImGui::End();
}

void ModuleEditor::modelOptions(Model& model)
{
    ImGuizmo::SetRect(0, 0, float(app->getD3D12()->getWindowWidth()), float(app->getD3D12()->getWindowHeight()));
    ImGui::Begin("Model Options");
    Matrix modelMatrix = model.getModelMatrix();

    static ImGuizmo::OPERATION currentOperation(ImGuizmo::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W))
        currentOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E))
        currentOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
        currentOperation = ImGuizmo::SCALE;

    if (ImGui::RadioButton("Translate", currentOperation == ImGuizmo::TRANSLATE))
        currentOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", currentOperation == ImGuizmo::ROTATE))
        currentOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", currentOperation == ImGuizmo::SCALE))
        currentOperation = ImGuizmo::SCALE;

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents((float*)&modelMatrix, matrixTranslation, matrixRotation, matrixScale);
    ImGui::InputFloat3("Translate", matrixTranslation);
    ImGui::InputFloat3("Rotate", matrixRotation);
    ImGui::InputFloat3("Scale", matrixScale);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&modelMatrix);

    if (ImGui::Button("Reset Transform", ImVec2(128, 32)))
    {
        matrixTranslation[0] = 0.0f;
        matrixTranslation[1] = 0.0f;
        matrixTranslation[2] = 0.0f;

        matrixRotation[0] = 0.0f;
        matrixRotation[1] = 0.0f;
        matrixRotation[2] = 0.0f;

        matrixScale[0] = 0.01f;
        matrixScale[1] = 0.01f;
        matrixScale[2] = 0.01f;

        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&modelMatrix);
    }

    const Matrix& viewMatrix = app->getCamera()->getView();
    Matrix projectionMatrix = app->getCamera()->getProjection();

    ImGuizmo::Manipulate((const float*)&viewMatrix, (const float*)&projectionMatrix, currentOperation, ImGuizmo::LOCAL, (float*)&modelMatrix);

    model.setModelMatrix(modelMatrix);

    ImGui::End();

}

