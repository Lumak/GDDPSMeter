#pragma once

//=============================================================================
//=============================================================================
typedef void(__thiscall *VoidFn)();
typedef void (*VoidFnNorm)();

struct DetourFnData
{
  const char *dllName_;
  VoidFn realFn_;
  VoidFn detourFn_;
  const char *mangleName_;
};
struct DetourFnNormalData
{
    const char *dllName_;
    VoidFn realFn_;
    VoidFnNorm detourFn_;
    const char *mangleName_;
};

template <typename R, typename ...Args>
class RealFunc
{
public:
	typedef R(__thiscall *Fn)(Args...);

	RealFunc() : Fn_(NULL) {};

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

  void SetError(const char *err);

protected:
    int HookDetour(DetourFnData &detourData);
    int HookDetour(DetourFnNormalData &detourData);

};

