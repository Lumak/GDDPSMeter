#include <windows.h>
#include "DispelBuffsWarning.h"


//=============================================================================
//=============================================================================
#define MAX_WARNING_DURATION 4000
#define FONT_SCALE_SIZE 4.0f

//=============================================================================
DispelBuffsWarning::DispelBuffsWarning()
{
    backgroundTransparency_ = 1.0f;
    startTime_ = 0;
    boldFont_ = NULL;
}

void DispelBuffsWarning::ShowWarning()
{
    startTime_ = (int)timeGetTime();
}

void DispelBuffsWarning::ShowWin(bool &showWindow)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;

    ImVec2 winSize = ImGui::GetWindowSize();
    ImGui::SetNextWindowPos(ImVec2(work_size.x * 0.5f - winSize.x*0.5f, work_size.y * 0.5f - 300.0f), ImGuiCond_FirstUseEver);

    int curTime = (int)timeGetTime();
    if (moveUI_)
    {
        if (curTime - startTime_ > MAX_WARNING_DURATION)
        {
            startTime_ = curTime;
        }
    }
    else
    {
        if (curTime - startTime_ > MAX_WARNING_DURATION)
        {
            return;
        }
    }

    Draw("DispelWarning", &showWindow);
}

void DispelBuffsWarning::Draw(const char* title, bool* p_open)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

    if (moveUI_)
    {
        window_flags &= ~(ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
    }

    if (!ImGui::Begin(title, p_open, window_flags))
    {
        ImGui::End();
        return;
    }
    
    if (boldFont_)
    {
        ImGui::PushFont(boldFont_);
    }

    ImGui::SetWindowFontScale(FONT_SCALE_SIZE);
    const ImVec4 REDISH(1.0f, 0.7f, 0.1f, backgroundTransparency_);
    ImGui::TextColored(REDISH, "Buffs Dispelled!");
    
    if (boldFont_)
    {
        ImGui::PopFont();
    }

    ImGui::End();
}

