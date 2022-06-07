#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct employee
{
 char employeeName[20];
 int employeeId;
 char joiningDate[20];
 struct employee* next;
};
struct employee* last = NULL;
//Function to insert the employee record from front
void insertatFront()
{
 char ename[20];
 int eid;
 char jdate[20];
 
 struct employee* current = (struct employee*)malloc(sizeof(struct employee));
 printf("Enter Employee Name:\n");
 scanf("%s",ename);
 printf("Enter Employee ID:");
 scanf("%d",&eid);
 printf("Enter Joining Date:");
 scanf("%s",jdate);
 //if the record is empty
  if (last == NULL) {
	strcpy(current->employeeName,ename);
        current->employeeId=eid;
        strcpy(current->joiningDate,jdate);
        current->next = current;
        last=current;
    }
    else {
        // Assigning the data.
        strcpy(current->employeeName,ename);
        current->employeeId=eid;
        strcpy(current->joiningDate,jdate);
        //current has reference of last
        current->next = last->next; 
        //last has reference of current
        last->next = current;

        /*//current has ref of last
        current->next = last->next; 
        //last has ref of current
        last->next = current;*/
    }
}

//Function to insert the employee record from end
void insertatEnd()
{
 char ename[20];
 int eid;
 char jdate[20];
 
 struct employee* current = (struct employee*)malloc(sizeof(struct employee));
// getchar();
 printf("Enter Employee Name:\n");
 scanf("%s",ename);
 printf("Enter Employee ID:");
 scanf("%d",&eid);
 printf("Enter Joining Date:");
 scanf("%s",jdate);
 //fgets(jdate, sizeof(jdate), stdin);
 //if the record is empty
  if (last == NULL) {
	strcpy(current->employeeName,ename);
        current->employeeId=eid;
        strcpy(current->joiningDate,jdate);
        current->next = current;
        last=current;
    }
    else {
        // Assigning the data.
        strcpy(current->employeeName,ename);
        current->employeeId=eid;
        strcpy(current->joiningDate,jdate);
	
	current->next=last->next;
	last->next=current;
	last=current;
    }
}
void deleteRecord()
{
struct employee* temp;
temp=last->next;
if(temp==0)
{
 printf("The Record is Empty");
}
else if(temp->next==temp)
{
 last=NULL;
 free(temp);
}
else
{
 last->next=temp->next;
 free(temp);
}

}

void employeeRecordDisplay()
{
    if (last == NULL)
        printf("\nThe Employee record is empty\n");
    else {
        struct employee* current;
        current=last->next;
        do {
	    printf("\n---------------------------------------------------------------------\n");
 	    printf("\nEmployee Name= %s",current->employeeName);
            printf("\nEmployee ID= %d", current->employeeId);
            printf("\nJoining Date= %s", current->joiningDate);
            printf("\n---------------------------------------------------------------------\n");
            current= current->next;
        } while (current != last->next);
    }
}

void employeeRecord()
{
 int option;
 do
 {
  printf("\n---------------------------------------------------------------------\n");
  printf("\nPress 1 for Employee Record Insertion\n");
  printf("\nPress 2 for Employee Record Display\n");
  printf("\nPress 3 for Employee Record Insert at End\n");
  printf("\nPress 4 for Delete the Record\n");
  printf("\nPress 0 for Exit");
  printf("\n---------------------------------------------------------------------\n");
  printf("\nEnter Your Option:");
  scanf("%d",&option);
  switch(option)
   {
	case 1:
           insertatFront();
           break;
        case 2:
           employeeRecordDisplay();
           break;
        case 3:
           insertatEnd();
           break;
        case 4:
           deleteRecord();
 	   break;
   }
  }while(option !=0);
}
int main()
{
 employeeRecord();
 return 0;
}
