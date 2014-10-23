#include <dlfcn.h>
#include "cnodemodule.h"

typedef Cnode* (*FuncCreateCnode)();
typedef void (*FuncReleaseCnode)(Cnode*);

CnodeModule::CnodeModule()
	: lib_(nullptr)
	, fcc_(nullptr)
	, frc_(nullptr)
{
}

CnodeModule::~CnodeModule()
{
	Close();
}

bool CnodeModule::Load(const char* libname)
{
	void* lib = dlopen(libname, RTLD_LAZY);
	if (lib == nullptr)
	{
		return false;
	}

	fcc_ = dlsym(lib, "CreateCnode");
	frc_ = dlsym(lib, "ReleaseCnode");
	if (!fcc_ || !frc_)
	{
		Close();
		return false;
	}

	return true;
}

void CnodeModule::Close()
{
	if (lib_)
	{
		dlclose(lib_);
		lib_ = nullptr;
	}

	fcc_ = nullptr;
	frc_ = nullptr;
}

Cnode* CnodeModule::CreateCnode()
{
	if (fcc_ == nullptr)
		return nullptr;

	FuncCreateCnode fcc = (FuncCreateCnode)fcc_;
	return fcc();
}

void CnodeModule::ReleaseCnode(Cnode* node)
{
	if (frc_ == nullptr)
		return;

	FuncReleaseCnode frc = (FuncReleaseCnode)frc_;
	frc(node);
}

