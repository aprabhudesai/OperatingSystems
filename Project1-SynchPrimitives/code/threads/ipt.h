#include "translate.h"

class IPTEntry: public TranslationEntry
{
	public:
	int processId;
	IPTEntry();
	~IPTEntry();
};
