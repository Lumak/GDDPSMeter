#include "CombatLog.h"

//=============================================================================
//=============================================================================
//#define SHOW_DBG_ADDSAMPLE

//=============================================================================
//=============================================================================
CombatLog::CombatLog()
  : maxLines_(0)
{
  AutoScroll = true;
  Clear();
}

CombatLog::CombatLog(int lines)
  : maxLines_(lines)
{
  AutoScroll = true;
  Clear();
}

void CombatLog::Clear()
{
  Buf.clear();
  LineOffsets.clear();
  LineOffsets.push_back(0);
}

void CombatLog::SetTotalLinesToShow(int lines)
{
  maxLines_ = lines;
}
void CombatLog::ShowWin(bool &showWindow)
{
  // For the demo: add a debug button _BEFORE_ the normal log window contents
  // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
  // Most of the contents of the window will be added by the log.Draw() call.
  ImGui::SetNextWindowPos(ImVec2(35, 350), ImGuiCond_FirstUseEver, ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(350, 450), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowBgAlpha(0.7f); // Transparent background

#ifdef SHOW_DBG_ADDSAMPLE
  ImGui::Begin("Combat Log");
  //IMGUI_DEMO_MARKER("Examples/Log");
  if (ImGui::SmallButton("[Debug] Add 5 entries"))
  {
    static int counter = 0;
    const char* categories[3] = { "info", "warn", "error" };
    const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
    for (int n = 0; n < 5; n++)
    {
      const char* category = categories[counter % IM_ARRAYSIZE(categories)];
      const char* word = words[counter % IM_ARRAYSIZE(words)];
      log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
        ImGui::GetFrameCount(), category, ImGui::GetTime(), word);
      counter++;
    }
  }
  ImGui::End();
#endif

  // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
  Draw("Combat log", &showWindow);
}

void CombatLog::AddLog(const char* fmt, ...)
{
  int old_size = Buf.size();
  va_list args;
  va_start(args, fmt);
  Buf.appendfv(fmt, args);
  va_end(args);

  for (int new_size = Buf.size(); old_size < new_size; old_size++)
  {
    if (Buf[old_size] == '\n')
    {
      LineOffsets.push_back(old_size + 1);
    }
  }

  //the LineOffsets has one empty buffer at initialization so skip 1 buffer count
  if (maxLines_ > 0 && LineOffsets.size() > (maxLines_ + 1))
  {
    while (LineOffsets.size() - (maxLines_ + 1))
    {
      LineOffsets.erase(LineOffsets.begin());
    }

    int startBuff = LineOffsets[0];
    memmove(Buf.Buf.Data, &Buf.Buf.Data[startBuff], Buf.Buf.Size - startBuff);
    for (int i = 0; i < LineOffsets.size(); ++i)
    {
      LineOffsets[i] = LineOffsets[i] - startBuff;
    }
    Buf.Buf.Size -= startBuff;
  }

}

void CombatLog::Draw(const char* title, bool* p_open)
{
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

  if (moveUI_)
  {
    window_flags &= ~(ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
    ImGui::SetNextWindowPos(ImVec2(100, 50), ImGuiCond_Always);
  }

  if (!ImGui::Begin(title, p_open, window_flags))
  {
    ImGui::End();
    return;
  }

  // Options menu
  //if (ImGui::BeginPopup("Options"))
  //{
  //  ImGui::Checkbox("Auto-scroll", &AutoScroll);
  //  ImGui::EndPopup();
  //}

  // Main window
  //if (ImGui::Button("Options"))
  //  ImGui::OpenPopup("Options");
  //ImGui::SameLine();
  //bool clear = ImGui::Button("Clear");
  //ImGui::SameLine();
  //bool copy = ImGui::Button("Copy");
  //ImGui::SameLine();
  //Filter.Draw("Filter", -100.0f);

  //ImGui::Separator();
  ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

  //if (clear)
  //  Clear();
  //if (copy)
  //  ImGui::LogToClipboard();

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
  const char* buf = Buf.begin();
  const char* buf_end = Buf.end();
#if 0
  if (Filter.IsActive())
  {
    // In this example we don't use the clipper when Filter is enabled.
    // This is because we don't have a random access on the result on our filter.
    // A real application processing logs with ten of thousands of entries may want to store the result of
    // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
    for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
    {
      const char* line_start = buf + LineOffsets[line_no];
      const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
      if (Filter.PassFilter(line_start, line_end))
        ImGui::TextUnformatted(line_start, line_end);
    }
  }
  else
#endif
  {
    // The simplest and easy way to display the entire buffer:
    //   ImGui::TextUnformatted(buf_begin, buf_end);
    // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
    // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
    // within the visible area.
    // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
    // on your side is recommended. Using ImGuiListClipper requires
    // - A) random access into your data
    // - B) items all being the  same height,
    // both of which we can handle since we an array pointing to the beginning of each line of text.
    // When using the filter (in the block of code above) we don't have random access into the data to display
    // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
    // it possible (and would be recommended if you want to search through tens of thousands of entries).
#if 1

    ImGuiListClipper clipper;
    clipper.Begin(LineOffsets.Size);

    ImVec2 window_size = ImGui::GetWindowSize();
    //ImGuiWindow win;
    //ImGuiWindow* window = ImGui::GetCurrentWindow();
    //window->AutoFitFramesX = (size.x <= 0.0f) ? 2 : 0;
    //window->AutoFitFramesY = (size.y <= 0.0f) ? 2 : 0;
#if 0 // this doesn't change anything
    if (window_size.x < 100.0f || window_size.y < 100.0f)
    {

      window_size.x = window_size.x < 100.0f ? 100.0f : window_size.x < 100.0f;
      window_size.y = window_size.y < 100.0f ? 100.0f : window_size.y < 100.0f;
      ImGui::SetWindowSize(window_size);
    }
#endif
    ImGui::PushTextWrapPos(window_size.x > 100.0f ? window_size.x : 100.0f);

    while (clipper.Step())
    {
      for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
      {
        const char* line_start = buf + LineOffsets[line_no];
        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
        ImGui::TextUnformatted(line_start, line_end);
      }
    }
    ImGui::PopTextWrapPos();

    clipper.End();
#endif
  }
  ImGui::PopStyleVar();

  if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    ImGui::SetScrollHereY(1.0f);

  ImGui::EndChild();
  ImGui::End();
}

