#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds.h"
#include "contact.h"

#define TREPORT(a1,a2) printf("Status %s: %s\n\n",a1,a2); fflush(stdout);

void process_line( char *test_case );

int main(int argc, char *argv[])
{
	printf("\nBUS TICKETING - PDS Application System\n\n");
	char test_case[50];
	int exiting = 1;
	while(exiting!= 0)
	{
		printf("MENU \n1. Create a new BUS database\n2. Open BUS database\n3. Store info\n4. Search according to Ticket Number\n5. Search according to other attributes\n6. Delete info\n7. Close the BUS database.\n8. EXIT THE SYSTEM.\nWrite corresponding number to the option you want to execute: ");
		int check;
		scanf("%d",&check );
		switch (check)
		{
		case 1:
		{
			strcpy(test_case , "CREATE newBUS 0");
			break;
		}
		case 2:
		{
			strcpy(test_case , "OPEN newBUS 0");
			break;
		}
		case 3:
		{
			printf("\nEnter Ticket Number to store : ");
			int id;
			scanf("%d" ,&id);
			sprintf(test_case, "STORE %d 0" , id);
			// strcpy(*test_case , ("STORE newBUS 0");
			break;
		}
		case 4:
		{
			printf("\nEnter Ticket Number to Search : ");
			int id;
			scanf("%d" ,&id);
			sprintf(test_case, "NDX_SEARCH %d 0" , id);
			break;
		}
		case 5:
		{
			printf("\nEnter Phone Number( FORMAT = [Phone-of-TicketNumber] ) to Search : ");
			char* name;
			scanf("%s" ,name);
			sprintf(test_case, "NON_NDX_SEARCH %s 0" , name);
			// strcpy(*test_case , ("STORE newBUS 0");
			break;
		}
		case 6:
		{
			printf("\nEnter Ticket Number to Delete : ");
			int id;
			scanf("%d" ,&id);
			sprintf(test_case, "NDX_DELETE %d 0" , id);
			// strcpy(*test_case , ("STORE newBUS 0");
			break;
		}
		case 7:
		{
			strcpy(test_case ,"CLOSE 0");
			break;
		}
		case 8:
		{
			exiting = 0;
			break;
		}

		}
		if(exiting!=0)
			process_line(test_case);
	}
}	

void process_line( char *test_case )
{
	
	char repo_name[30];
	char command[50], param1[50], param2[50], info[1000];
	char phone_num[10];
	int contact_id, status, rec_size, expected_status, expected_io, actual_io;
	char expected_name[50], expected_phone[50]; 
	struct Contact testContact;

	strcpy(testContact.contact_name, "dummy name");
	strcpy(testContact.phone, "dummy number");
	

	rec_size = sizeof(struct Contact);

	sscanf(test_case, "%s%s%s", command, param1, param2);
	printf("\nTest case: %s\n", test_case); fflush(stdout);
	if( !strcmp(command,"CREATE") ){
		strcpy(repo_name, param1);
		if( !strcmp(param2,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;
		
		status = pds_create( repo_name );
		sprintf(info,"pds_open returned status %d",status);
		
		if(status == CONTACT_SUCCESS)
		{
			TREPORT("SUCCESS", info);
		}
		else
		{
			TREPORT("FAILED", info);
		}
		
	}

	if( !strcmp(command,"OPEN") ){
		strcpy(repo_name, param1);
		if( !strcmp(param2,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		status = pds_open( repo_name, rec_size );
			sprintf(info,"pds_open returned status %d",status);
			if(status == CONTACT_SUCCESS)
		{
			TREPORT("SUCCESS", info);
		}
		else
		{
			TREPORT("FAILED", info);
		}
		
	}
	else if( !strcmp(command,"STORE") ){
		if( !strcmp(param2,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		sscanf(param1, "%d", &contact_id);
		testContact.contact_id = contact_id;
		sprintf(testContact.contact_name,"Name-of-%d",contact_id);
		sprintf(testContact.phone,"Phone-of-%d",contact_id);
		status = add_contact( &testContact );
		sprintf(info,"add_contact returned status %d",status);
		if(status == CONTACT_SUCCESS)
		{
			TREPORT("SUCCESS", info);
		}
		else
		{
			TREPORT("FAILED", info);
		}
		
	}
	else if( !strcmp(command,"NDX_SEARCH") ){
		if( strcmp(param2,"1") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		sscanf(param1, "%d", &contact_id);
		testContact.contact_id = -1;
		status = search_contact( contact_id, &testContact );
		sprintf(info,"search key: %d; Got status %d",contact_id, status);
		if(status == CONTACT_SUCCESS)
		{
			TREPORT("SUCCESS", info);
		}
		else
		{
			TREPORT("FAILED", info);
		}
		
		
	}
	else if( !strcmp(command,"NON_NDX_SEARCH") ){
		if( strcmp(param2,"-1") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		sscanf(param1, "%s", phone_num);
		sscanf(param2, "%d", &expected_io);
		testContact.contact_id = -1;
		int actual_io = 0;
		status = search_contact_by_phone( phone_num, &testContact, &actual_io );
		sprintf(info,"search key: %d; Got status %d",contact_id, status);
			if(status == CONTACT_SUCCESS)
		{
			TREPORT("SUCCESS", info);
		}
		else
		{
			TREPORT("FAILED", info);
		}
		
	}
	else if( !strcmp(command,"NDX_DELETE") ){
		if( strcmp(param2,"0") == 0 )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		sscanf(param1, "%d", &contact_id);
		testContact.contact_id = -1;
		status = delete_contact( contact_id );
		sprintf(info,"delete key: %d; Got status %d",contact_id, status);
			if(status == CONTACT_SUCCESS)
		{
			TREPORT("SUCCESS", info);
		}
		else
		{
			TREPORT("FAILED", info);
		}
		
	}
	else if( !strcmp(command,"CLOSE") ){
		if( !strcmp(param1,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		status = pds_close();
		sprintf(info,"pds_close returned status %d",status);
		if(status == CONTACT_SUCCESS)
		{
			TREPORT("SUCCESS", info);
		}
		else
		{
			TREPORT("FAILED", info);
		}
		
	}
}


