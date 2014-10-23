#pragma once

#include "cnode.h"

class CnodeModule
{
public:
	CnodeModule();
	~CnodeModule();

	bool Load(const char* libname);
	void Close();

	Cnode* CreateCnode();
	void ReleaseCnode(Cnode* node);

private:
	void* lib_;
	void* fcc_;
	void* frc_;
};

