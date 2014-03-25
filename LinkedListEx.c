    /*
 ============================================================================
 Name        : LinkedListEx.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include<stdio.h>
#include<stdlib.h>


// The node in the linked list, contains an int value and a pointer to the next
struct node
{
    int val;
    struct node *next;
};

// Creating two nodes, head shouldn't change, curr will change as elements are added.
struct node *head = NULL;
struct node *curr = NULL;

// A function to create the list, allocates memory for the head node.
struct node* create_list(int val)
{
    printf("\n creating list with headnode as [%d]\n",val);
    struct node *ptr = (struct node*)malloc(sizeof(struct node));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->val = val;
    ptr->next = NULL;

    head = curr = ptr;
    return ptr;
}

// A function to add a node to the list.
struct node* add_to_list(int val)
{
	//First check to see if there is anything in the list.
    if(NULL == head)
    {
        return (create_list(val)); // list is created if it doesn't exist already.
    }
    printf("\n Adding node to the list containing [%d]\n",val);

    // Create a node for the new data.
    struct node *ptr = (struct node*)malloc(sizeof(struct node));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->val = val; // Add the data
    ptr->next = NULL;

    curr->next = ptr; // Link in the new node
    curr = ptr;			// Update the pointer

    return ptr;
}


// A function that traverses the list and prints the contents.
void print_list(void)
{
    struct node *ptr = head;

    printf("\n -------Printing list Start------- \n");
    while(ptr != NULL)
    {
        printf("\n [%d] \n",ptr->val);
        ptr = ptr->next;
    }
    printf("\n -------Printing list End------- \n");

    return;
}

int main(void)
// Some simple calls to print_list and add_to_list to show how they work.
{
    int i = 0;

    print_list();

    for(i = 5; i<10; i++)
        add_to_list(i);

    print_list();

    for(i = 4; i>0; i--)
        add_to_list(i);

    print_list();

    return 0;
}
