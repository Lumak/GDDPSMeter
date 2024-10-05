#pragma once

//=============================================================================
//=============================================================================
#ifdef X64
#define VoidArg void *This
#else
#define VoidArg void *This, void*
#endif

typedef void(__thiscall *VoidFn)();

struct DetourFnData
{
  const char *dllName_;
  VoidFn realFn_;
  VoidFn detourFn_;
  const char *mangleName_;
};

template <typename R, typename ...Args>
class ThisFunc
{
public:
	typedef R(__thiscall *Fn)(Args...);

	ThisFunc() : Fn_(NULL) {};

	void SetFn(VoidFn rhs)
	{
		Fn_ = (Fn)rhs;
	}

	Fn Fn_;
};

template <typename R, typename ...Args>
class CdeclFunc
{
public:
	typedef R(__cdecl *Fn)(Args...);

	CdeclFunc() : Fn_(NULL) {};

	void SetFn(VoidFn rhs)
	{
		Fn_ = (Fn)rhs;
	}

	Fn Fn_;
};

//=============================================================================
//=============================================================================
class DetourBase
{
public:
  DetourBase();

  virtual bool SetupDetour() = 0;
  virtual void Update(void *player, int idx) = 0;
  virtual void SetPlayer(void* player) = 0;


protected:
  int HookDetour(DetourFnData &detourData);
  void SetError(const char *err);

};

