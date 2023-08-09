#pragma once
#include "BaseRender.h"

class DispelBuffsWarning : public BaseRender
{
public:
    DispelBuffsWarning();

    virtual void ShowWin(bool &showWindow);
    virtual void Draw(const char* title, bool* p_open = NULL);

    void ShowWarning();

private:
    int startTime_;
    float backgroundTransparency_;

};

