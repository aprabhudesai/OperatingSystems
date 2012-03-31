#include "ipt.h"

/*
*	Constructor for the IPT Entry
*/
IPTEntry::IPTEntry()
{
	processId = -1;
	virtualPage = -1;
	physicalPage = -1;
	valid = false;
	readOnly = false;
	use = false;
	dirty = false;
}

/*
*	Destructor for the IPT Entry
*/
IPTEntry::~IPTEntry()
{
}
