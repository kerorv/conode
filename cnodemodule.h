#pragma once

#include <list>
#include "cnode.h"

class CnodeModule
{
public:
	CnodeModule();
	~CnodeModule();

	bool Load(const char* name);
	void Close();

	Cnode* CreateCnode();
	void ReleaseCnode(Cnode* node);

private:
	void* lib_;
	void* fcc_;
	void* frc_;
	typedef std::list<Cnode*> CnodeList;
	CnodeList nodes_;
};

