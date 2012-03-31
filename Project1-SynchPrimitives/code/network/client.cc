#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"


void client(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data;
	char *ack;
	char buffer[MaxMailSize];
	bool success ;
	
	data = "alock1\0";
	outPktHdr.to = farAddr;
	outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    success = postOffice -> Send(outPktHdr, outMailHdr, data); 
	printf("\nSent Create lock request to server...\n");
	
    if ( !success ) 
	{
		  printf("\nThe postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		  interrupt->Halt();
    }
	//interrupt->Halt();
	// Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
	
    printf("\nGot \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
	
	data = "dcv1\0";
	outPktHdr.to = farAddr;
	outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    success = postOffice -> Send(outPktHdr, outMailHdr, data); 
	printf("\nSent Create cv request to server...\n");
	
    if ( !success ) 
	{
		  printf("\nThe postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		  interrupt->Halt();
    }
	//interrupt->Halt();
	// Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
	
    printf("\nGot \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
	
	data = "e1,1\0";
	outPktHdr.to = farAddr;
	outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    success = postOffice -> Send(outPktHdr, outMailHdr, data); 
	printf("\nSent Wait cv request to server...\n");
	
	
    fflush(stdout);
    interrupt->Halt();
}



/*******************************************************************************************/
/*
*	Client Program
*/