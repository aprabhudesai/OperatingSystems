#include "syscall.h"

int
main()
{
	int lockId, lockStatus, cvId, mvStatus, mvId, mvValue, lockId1, lockId2;

	WriteToConsole("\nCreating a Lock\n\0", -1, -1, -1);

	lockId = CreateLock("lock1",5);
	if(lockId == -1)
	{
		WriteToConsole("\nError Occurred while creating Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock Created, Lock Id returned by the server is : %d\n\0", lockId, -1, -1);
	}

	WriteToConsole("\nCreating a CV\n\0", -1, -1, -1);

	cvId = CreateCV("cv1",3);
	if(cvId == -1)
	{
		WriteToConsole("\nError Occurred while creating CV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nCV Created, CV Id returned by the server is : %d\n\0", cvId, -1, -1);
	}

	WriteToConsole("\nCreating a MV\n\0", -1, -1, -1);

	mvId = CreateMV("mv1",3, 5);
	if(mvId == -1)
	{
		WriteToConsole("\nError Occurred while creating MV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV Created, MV Id returned by the server is : %d\n\0", mvId, -1, -1);
	}


	WriteToConsole("\nSetting MV to 100\n", -1, -1, -1);
	mvStatus = SetMV(mvId, 1, 100);
	if(mvStatus == -1)
	{
		WriteToConsole("\nFailed to Set MV. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV set successfully.\n\0", -1, -1, -1);
	}

	WriteToConsole("\nDeleting MV\n", -1, -1, -1);
	mvStatus = DeleteMV(mvId);
	if(mvStatus == -1)
	{
		WriteToConsole("\nFailed to Delete MV. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV deleted successfully.\n\0", -1, -1, -1);
	}

	WriteToConsole("\nTrying to fetch value from MV which is deleted : %d\n\0",mvId,-1,-1);
	mvValue = GetMV(mvId+25, 1);
	if(mvValue == -1)
	{
		WriteToConsole("\nInvalid mv Id. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAbnormal Execution\n\0", -1, -1, -1);
	}

   Exit(0);
}
