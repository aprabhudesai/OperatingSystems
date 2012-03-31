#include "translate.h"

class pageTableEntry: public TranslationEntry
{
	public:
	int diskLocation;	// determines the location of the page : executable (1), not on disk (2), swap file (3)
	unsigned int byteOffset;	// Determines the byte offset in the executable
	unsigned int swapByteOffset;	// Determines the byte offset in the swap file
	pageTableEntry();	// Constructor
	~pageTableEntry();	// Destructor
};
