#include "BaseRender.h"
//=============================================================================
//=============================================================================
BaseRender::BaseRender()
{
  boldFont_ = NULL;
  moveUI_ = false;
}

void BaseRender::SetMoveUI(bool set)
{
  moveUI_ = set;
}

void BaseRender::SetBoldFont(ImFont *bold)
{
  boldFont_ = bold;
}

void BaseRender::ShowWin(bool &showWindow)
{

}

void BaseRender::Draw(const char* title, bool* p_open)
{
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

  if (moveUI_)
  {
    window_flags &= ~(ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
  }

}



