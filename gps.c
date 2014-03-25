/*
	Title: Trying to understand... linked lists
 */

#include<stdio.h>
#include<stdlib.h>

#define DEBUG

// The node in the linked list, contains an int value and a pointer to the next
struct node {
	int val;
	struct node *next;
};

// Creating two nodes, head shouldn't change, curr will change as elements are added.
struct node *head = NULL;
struct node *curr = NULL;

// A function to create the list, allocates memory for the head node.
struct node* create_list(val) {
	printf("Creating headnode with value of %d\n", val);
	struct node *ptr = (struct node*)malloc(sizeof(struct node)); 

	// Check that the pointer was created successfully
	if(ptr == NULL) {
		puts("Unable to allocate memory for the node");
	}
}

// A function to add a node to the list.
// This code notation means it returns a pointer to node
struct node* add_to_list(val) {
	//First check to see if there is anything in the list.
	// The first part of this function checks to see if there's anything
	// in the list at all. If there isn't, add SOMETHING. The *head pointer
	// is what shoud be checked first
	if(head == NULL) {
		return(create_list(val));
	}		
	// Create a node for the new data.
	// Not done yet!
}

// A function that traverses the list and prints the contents.
// [OK]
void print_list() {
	struct node *ptr;
	ptr = head; 
	while(ptr != NULL) {
		printf("We have node %d\n", ptr->val);
		ptr = head->next;
	}
}

// Some simple calls to print_list and add_to_list to show how they work.
void main(void) {
	print_list();
	for(i = 5; i < 10; i++) {
		add_to_list(i);
	}
	print_list();
}
