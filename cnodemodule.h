#pragma once

#include "cnode.h"

class CnodeModule
{
public:
	CnodeModule();
	~CnodeModule();

	bool Load(const char* name);
	void Close();

	Cnode* CreateCnode(unsigned int id, const char* config);
	void ReleaseCnode(Cnode* node);

private:
	void* lib_;
	typedef Cnode* (*FuncCreateCnode)(unsigned int, MsgRouter*);
	typedef void (*FuncReleaseCnode)(Cnode*);
	FuncCreateCnode* fcc_;
	FuncReleaseCnode* frc_;
};

