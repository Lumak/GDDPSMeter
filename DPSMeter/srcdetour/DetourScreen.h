#pragma once
#include "DetourBase.h"

//=============================================================================
//=============================================================================
class DetourMain;

//=============================================================================
//=============================================================================
#define SYM_EVENTMANAGER_SEND "?Send@EventManager@GAME@@QAEXPBUGameEvent@2@I@Z"
#define SYM_PIEOMATIC_SETWEDGEMODE "?SetWedgeMode@PieOmatic@GAME@@QAEX_N@Z" 
#define SYM_PIEOMATIC_FADEIN "?FadeIn@PieOmatic@GAME@@QAEXXZ"
#define SYM_PIEOMATIC_FADEOUT "?FadeOut@PieOmatic@GAME@@QAEXXZ"
#define SYM_GFXCANVAS_RENDERSYTLEDRECT "?RenderStyledRect@GraphicsCanvas@GAME@@QAEXABVRect@2@ABVName@2@ABVColor@2@@Z"

//=============================================================================
//=============================================================================
class DetourScreen : public DetourBase
{
public:
  DetourScreen();

  virtual bool SetupDetour();
  virtual void Update(void *player, int idx);
  virtual void SetPlayer(void* player) { playerPtr_ = player; };

  void EventMgrSend(void* This, void* pevent, unsigned int a);
  void PieOmaticSetWedgeMode(void*, bool);
  void PieOmaticFadeIn(void*);
  void PieOmaticFadeOut(void*);
  void SetParent(DetourMain *parent);
  void GfxCanvasRenderStyledRect(void*, unsigned int&, unsigned int&, unsigned int&);

  //static fns
  static void __fastcall DTEventManagerSend(void* This, void*, void* pevent, unsigned int a);
  static void __fastcall DTPieOmaticSetWedgeMode(void*, void*, bool);
  static void __fastcall DTPieOmaticFadeIn(void*);
  static void __fastcall DTPieOmaticFadeOut(void*);
  static void __fastcall DTGfxCanvasRenderStyledRect(void*, void*, unsigned int&, unsigned int&, unsigned int&);

private:
  bool initialized_;
  bool inWorld_;
  unsigned int showUI_;
  DetourMain *sDetourMain_;
  unsigned int fadeTime_;
  void *playerPtr_;

  //static vars
  static DetourScreen *sDetourScreen_;

  static RealFunc<void, void*, void*, unsigned int> fnEventManagerSend_;
  static RealFunc<void, void*, bool> fnPieOmaticSetWedgeMode_;
  static RealFunc<void, void*> fnPieOmaticFadeIn_;
  static RealFunc<void, void*> fnPieOmaticFadeOut_;
  static RealFunc<void, void*, unsigned int&, unsigned int&, unsigned int&> fnGfxCanvasRenderStyledRect_;
};

