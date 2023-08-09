#include <windows.h>
#include <cassert>

#include "DetourScreen.h"
#include "DetourMain.h"
#include "IPCMessage.h"
#include "DetourUtil.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
#define FADEOUT_DELAY_MS 1000

//=============================================================================
//=============================================================================
DetourFnData datEventManagerSend = { "engine.dll", NULL, (VoidFn)&DetourScreen::DTEventManagerSend, SYM_EVENTMANAGER_SEND };
DetourFnData datPieOmaticSetWedgeMode = { "engine.dll", NULL, (VoidFn)&DetourScreen::DTPieOmaticSetWedgeMode, SYM_PIEOMATIC_SETWEDGEMODE};
DetourFnData datPieOmaticFadeIn = { "engine.dll", NULL, (VoidFn)&DetourScreen::DTPieOmaticFadeIn, SYM_PIEOMATIC_FADEIN };
DetourFnData datPieOmaticFadeOut = { "engine.dll", NULL, (VoidFn)&DetourScreen::DTPieOmaticFadeOut, SYM_PIEOMATIC_FADEOUT };
DetourFnData datGfxCanvasRenderStyledRect = { "engine.dll", NULL, (VoidFn)&DetourScreen::DTGfxCanvasRenderStyledRect, SYM_GFXCANVAS_RENDERSYTLEDRECT };

RealFunc<void, void*, void*, unsigned int> DetourScreen::fnEventManagerSend_;
RealFunc<void, void*, bool> DetourScreen::fnPieOmaticSetWedgeMode_;
RealFunc<void, void*> DetourScreen::fnPieOmaticFadeIn_;
RealFunc<void, void*> DetourScreen::fnPieOmaticFadeOut_;
RealFunc<void, void*, unsigned int&, unsigned int&, unsigned int&> DetourScreen::fnGfxCanvasRenderStyledRect_;

DetourScreen *DetourScreen::sDetourScreen_ = NULL;
//=============================================================================
//=============================================================================
DetourScreen::DetourScreen()
{
    sDetourScreen_ = this;

    initialized_ = false;
    inWorld_ = false;
    showUI_ = 0;
    sDetourMain_ = NULL;
    fadeTime_ = 0;
}

bool DetourScreen::SetupDetour()
{
    int status = 0;

    status += HookDetour(datEventManagerSend);
    fnEventManagerSend_.SetFn(datEventManagerSend.realFn_);

    status += HookDetour(datPieOmaticSetWedgeMode);
    fnPieOmaticSetWedgeMode_.SetFn(datPieOmaticSetWedgeMode.realFn_);

    //status += HookDetour(datPieOmaticFadeIn);
    //fnPieOmaticFadeIn_ = (FnPieOmaticFadeIn)datPieOmaticFadeIn.realFn_;

    status += HookDetour(datPieOmaticFadeOut);
    fnPieOmaticFadeOut_.SetFn(datPieOmaticFadeOut.realFn_);

	status += HookDetour(datGfxCanvasRenderStyledRect);
	fnGfxCanvasRenderStyledRect_.SetFn(datGfxCanvasRenderStyledRect.realFn_);

    if (status != 0)
    {
        SetError("Error in DetourScreen::SetupDetour()");
    }

    return status == 0;
}

void DetourScreen::SetParent(DetourMain *parent)
{
    sDetourMain_ = parent;
}

void DetourScreen::Update(void *player, int idx)
{
    unsigned int curTime = (unsigned int)timeGetTime();

    if (showUI_ == 1 && curTime > fadeTime_)
    {
        showUI_ = 2;
        IPCMessage::SendShortData(DataMsgType::ShowUI, 1);
    }

}

void DetourScreen::EventMgrSend(void* This, void* pevent, unsigned int a)
{
    //not sure the meanings of the numbers but they seem to indicate:
    // 6 = world/zone -- player is in the game
    // 4 = world/zone -- player is no longer in the game
    if (a == 6)
    {
        inWorld_ = true;
        sDetourMain_->SetScreenEnable(true);
        IPCMessage::SendShortData(DataMsgType::ShowUI, 0);
        LOGF("EventMgrSend 6\n");
    }
    else if (a == 4)
    {
        LOGF("EventMgrSend 4: showui=%d\n", showUI_);

        sDetourMain_->SetScreenEnable(false);

        IPCMessage::ClearQueue();
        IPCMessage::SendShortData(DataMsgType::ShowUI, 0);
        IPCMessage::SendShortData(DataMsgType::ClearSkillBuffs, 0);

        showUI_ = 0;
        inWorld_ = false;
    }
}

void DetourScreen::PieOmaticSetWedgeMode(void*, bool set)
{
    showUI_ = 0;
    IPCMessage::SendShortData(DataMsgType::ShowUI, 0);
}

void DetourScreen::PieOmaticFadeIn(void*)
{
}

void DetourScreen::PieOmaticFadeOut(void*)
{
    fadeTime_ = (unsigned int)timeGetTime() + FADEOUT_DELAY_MS;

    if (playerPtr_ && !showUI_)
    {
        showUI_ = 1;
        IPCMessage::SendShortData(DataMsgType::ShowUI, 1);
    }
}

void DetourScreen::GfxCanvasRenderStyledRect(void* This, unsigned int& rectRef, unsigned int& nameRef, unsigned int& colorRef)
{
	struct Rect { float x, y, w, h; };
	Rect &rect = *(Rect*)&rectRef;

	//rectangle(0, 0, 1, 1) appears only in-game and never in character screen
	if (rect.x == 0.0f && rect.y == 0.0f && rect.w == 1.0f && rect.h == 1.0f)
	{
		sDetourMain_->SetInGameRendered(true);
	}
}


//==============================================================
// static fns
//==============================================================
void DetourScreen::DTEventManagerSend(void* This, void*, void* pevent, unsigned int a)
{
    sDetourScreen_->EventMgrSend(This, pevent, a);

    fnEventManagerSend_.Fn_(This, pevent, a);
}

void DetourScreen::DTPieOmaticSetWedgeMode(void* This, void*, bool set)
{
    sDetourScreen_->PieOmaticSetWedgeMode(This, set);
    fnPieOmaticSetWedgeMode_.Fn_(This, set);
}

void DetourScreen::DTPieOmaticFadeIn(void* This)
{
    sDetourScreen_->PieOmaticFadeIn(This);
    fnPieOmaticFadeIn_.Fn_(This);
}

void DetourScreen::DTPieOmaticFadeOut(void* This)
{
    sDetourScreen_->PieOmaticFadeOut(This);
    fnPieOmaticFadeOut_.Fn_(This);
}

void DetourScreen::DTGfxCanvasRenderStyledRect(void* This, void*, unsigned int& rectRef, unsigned int& nameRef, unsigned int& colorRef)
{
	sDetourScreen_->GfxCanvasRenderStyledRect(This, rectRef, nameRef, colorRef);
	fnGfxCanvasRenderStyledRect_.Fn_(This, rectRef, nameRef, colorRef);
}
