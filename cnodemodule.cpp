#include "cnodemodule.h"

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

bool CnodeModule::Load(const char* name)
{
	void* lib = dlopen(name.c_str(), RTLD_LAZY);
	if (lib == nullptr)
		return false;
	fcc_ = (FuncCreateCnode*)dlsym(lib, "CreateCnode");
	frc_ = (FuncReleaseCnode*)dlsym(lib, "ReleaseCnode");
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

Cnode* CnodeModule::CreateCnode(unsigned int id, const char* config)
{
	return fcc_(id, config);
}

void CnodeModule::ReleaseCnode(Cnode* node)
{
	frc_(node);
}

