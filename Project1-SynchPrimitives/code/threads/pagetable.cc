#include "pagetable.h"

/*
*	Constructor for the IPT Entry
*/
pageTableEntry::pageTableEntry()
{
	virtualPage = -1;
	physicalPage = -1;
	valid = false;
	readOnly = false;
	use = false;
	dirty = false;
	diskLocation = -1;
	byteOffset = 0;
}

/*
*	Destructor for the IPT Entry
*/
pageTableEntry::~pageTableEntry()
{
}
