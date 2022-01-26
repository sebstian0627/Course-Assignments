//This is the implementation of basic MMU in C. It comprises implementation of Malloc and free commands.

//IMPORTS
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdint.h>
#include<sys/shm.h>

//DEFINES
#define NULL_PTR ((void*)0)
#define HEAP_SIZE sizeof(heap_type)
#define NODE_SIZE sizeof(free_node)
#define ALLOC_SIZE sizeof(alloc_node)

//Struct to store basic heap info.
typedef struct heap_tag{
	int currentSize;
	int freeMemory;
	int blockAllocated;
	struct list_node* firstNode;
}heap_type;

typedef struct alloc_node_tag{
	int size;
	int magic;
}alloc_node;

//struct to store free list nodes.
typedef struct list_node{
	int size;
	struct list_node* next;
	// struct list_node* prev;
}free_node;

//VARIABLES.
static heap_type* myHeap;


//this is a util function used to print the list
// void printList(){
// 	free_node* start=myHeap->firstNode;
// 	printf("__________________\n");
// 	while(start){
// 		printf("%d\n",start->size);
// 		printf("%p\n",start->next);
// 		printf("%p\n",start->prev);
// 		start=start->next;
// 	}
// 	printf("__________________\n");
// }


void printList(){
	free_node *tra =myHeap->firstNode;
	printf("=== List Info ================\n");

	while(tra != NULL){
		printf("%d\t", tra->size);
		tra = tra->next;
	}
	printf("\n");
	printf("==============================\n");
    printf("\n");
}

//This function is used to print the node
void printNode(void* x){
	printf("THIS IS PRINTING NODE\n");
	free_node* start=(free_node*)x;
	printf("%d\n",start->size);
	printf("%p\n",start->next);
	printf("%p\n",start->prev);
	printf("NODE PRINTING DONE\n");
}
// Implement these yourself
//this function requests 4KB page using mmap and initialised the Heap node and the header nodes in the free_list.
int my_init(){
	myHeap= (heap_type*)mmap ( NULL, 4096*sizeof(char),
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1, 0);
	if(myHeap == MAP_FAILED){
    printf("Mapping Failed\n");
    // return *((int*)MAP_FAILED);
	}

	myHeap->blockAllocated=0;
	myHeap->freeMemory=4096-HEAP_SIZE-NODE_SIZE;
	myHeap->currentSize=NODE_SIZE;
	myHeap->firstNode=(free_node *)((void *)myHeap+HEAP_SIZE);

	myHeap->firstNode->size=4096-HEAP_SIZE+ALLOC_SIZE-NODE_SIZE;
	myHeap->firstNode->next=NULL_PTR;
	// myHeap->firstNode->prev=NULL_PTR;

	return 0;
}

//This is the implementation of malloc command. Uses BestFit strategy. This allocates the node if the asked space is exactly the absolute free space of the node else it splits if we can split. If none of the above is possible, it returns the NULL pointer. if all the free space is allocated, then the pointer to the firstNode in the heap tag is set to NULL.
void* my_alloc(int count){
	if (count<=0 || count %8!=0){
		return NULL_PTR;
	}
	free_node* listIterator;
	free_node* bestListNode;
	int bestSize;

	listIterator=myHeap->firstNode;
	bestListNode=(free_node*) NULL_PTR;
	bestSize=5000;
	int remaining=0;
	while(listIterator){
		remaining=listIterator->size+NODE_SIZE-count-ALLOC_SIZE;
		if(remaining ==0){
			
			//delete this node.
		}

		if((listIterator->size>= count + ALLOC_SIZE)&& listIterator->size <= bestSize){
			bestListNode=listIterator;
			bestSize=listIterator->size;
		}
		listIterator=listIterator->next;
	}
	if(bestListNode!=NULL_PTR){
		// free_node* nodeAllocate;
		alloc_node* nodeToAllocate;

		bestListNode->size=bestListNode->size -count-ALLOC_SIZE;
		nodeToAllocate=(alloc_node*)(((void*)bestListNode)+NODE_SIZE+bestListNode->size);
		nodeToAllocate->size=count;
		nodeAllocate->magic=123456;
		myHeap->currentSize+=count+ALLOC_SIZE;
		myHeap->freeMemory-=count+ALLOC_SIZE;
		myHeap->blockAllocated++;
		if(myHeap->freeMemory==0){
			myHeap->firstNode=NULL_PTR;
		}
		// printNode(nodeAllocate);
		return (void *)(((void*) nodeToAllocate)+ALLOC_SIZE);
	}
	return NULL_PTR;
}

//this is the implementation free command. If the given pointer is null or if the node pointer from the given pointer is null or if the node pointer is already in the free list, the function returns doing nothing. Else to add the node in the free_list, it traverse the list and find nodes which can be previous and next node for the node to be added. then all the VARIABLES are updated. All the possibilities of merging(coalescing) are covered. If the Freelist is null i.e. all the blocks are identified, the first block to be freed, after that, is set the first node and the firstNode pointer is shifted left, if possible in the subsequent free calls.
void my_free(void *ptr){
	if(ptr==NULL_PTR){
		return;
	}
	alloc_node* nodeToFree=(alloc_node*)((void*)ptr - ALLOC_SIZE);
	if(nodeToFree==NULL_PTR){
		return;
	}
	// if((nodeToFree->next!=NULL_PTR)||(nodeToFree->prev!=NULL_PTR)){
	// 	return;
	// }

	nodeToFree->magic=98765;

	// if(myHeap->firstNode==NULL_PTR){
	// 	myHeap->freeMemory+=nodeToFree->size;
	// 	myHeap->currentSize-=nodeToFree->size;
	// 	myHeap->blockAllocated--;
	// 	myHeap->firstNode=nodeToFree;
	// 	return;
	// }


	free_node* prevNode;
  free_node* nextNode;

	prevNode=NULL_PTR;
	nextNode=myHeap->firstNode;
	while ((nextNode != NULL_PTR) && (nextNode < nodeToFree))
  {
    prevNode = nextNode;
    nextNode = nextNode->next;
  }
	// printNode(prevNode);
	// printNode(nextNode);
	nodeToFree->next=nextNode;
	nodeToFree->prev=prevNode;
	if(nextNode!=NULL_PTR){
		nextNode->prev=nodeToFree;
	}
	if (prevNode != NULL_PTR)
  {

    prevNode->next = nodeToFree;
  }
	// printf("here\n");
	myHeap->freeMemory+=nodeToFree->size;
	myHeap->currentSize-=nodeToFree->size;
	myHeap->blockAllocated--;
	// printf("%d freeMemory, %d currentSizeeeeeeeeeeeeeeeeeeeeeee\n",nodeToFree->size,myHeap->currentSize);

	if ( (nodeToFree->next != NULL_PTR) &&
			 ( ((void*)nodeToFree + nodeToFree->size + NODE_SIZE) == (void*)nodeToFree->next) )
	{
		myHeap->freeMemory+=NODE_SIZE;
		myHeap->currentSize-=NODE_SIZE;
		nodeToFree->size += nodeToFree->next->size + NODE_SIZE;
		nodeToFree->next = nodeToFree->next->next;
		if(nodeToFree->next!=NULL_PTR){
			nodeToFree->next->prev=nodeToFree;
		}
	}
	if ( (nodeToFree->prev != NULL_PTR) &&
     ( ((void*)nodeToFree->prev) +nodeToFree->prev->size + NODE_SIZE) == nodeToFree)
  {
		myHeap->freeMemory+=NODE_SIZE;
		myHeap->currentSize-=NODE_SIZE;
    nodeToFree->prev->size += nodeToFree->size + NODE_SIZE;
    nodeToFree->prev->next = nodeToFree->next;
		if(nodeToFree->prev->next!=NULL_PTR){
			nodeToFree->prev->next->prev=nodeToFree->prev;
		}
  }

	//if the freeList
	free_node* startHandler;
	startHandler=myHeap->firstNode;
	while(startHandler){
		// printf("hi\n");
		myHeap->firstNode=startHandler;
		startHandler=startHandler->prev;
	}
	// printf("HiHIHI\n");
	ptr=NULL_PTR;
	return;
}

//unmaps the memory requested from mmap.
void my_clean(){
	munmap(myHeap,4096*sizeof(char));

	return;
}

//prints the required info about the heap.
void my_heapinfo(){
	int a, b, c, d, e, f;
	e=5000;
	f=0;

	free_node *freeIterator=myHeap->firstNode;

	while(freeIterator){
		if(freeIterator->size>=f){
			f=freeIterator->size;
		}
			if(freeIterator->size<=e){
				e=freeIterator->size;
			}
			freeIterator=freeIterator->next;
	}
	if(e==5000){
		e=0;
	}

	// Do not edit below output format
	printf("=== Heap Info ================\n");
	printf("Max Size: %ld\n", 4096-HEAP_SIZE);
	printf("Current Size: %d\n", myHeap->currentSize);
	printf("Free Memory: %d\n", myHeap->freeMemory);
	printf("Blocks allocated: %d\n", myHeap->blockAllocated);
	printf("Smallest available chunk: %d\n", e);
	printf("Largest available chunk: %d\n", f);
	printf("==============================\n");
	// Do not edit above output format
	return;
}


//testcase
int main(int argc, char *argv[]){
	my_init();
	printf("Begining :-\n");
	printList();

	int *test = my_alloc(sizeof(int)*4);
	*test = 24;
	printf("after allocating test:\n");
	printList();
	my_heapinfo();


	char *test2 = my_alloc(8);
	*(test2+0) = 'a';
	*(test2+1) = 'b';
	*(test2+2) = 'c';
	*(test2+3) = 'd';
	printf("allocated test2:\n");
	printList();
	my_heapinfo();

	int *test3 = my_alloc(sizeof(int)*8);
	if(test3 != NULL){
		*(test3+0) = 1;
		*(test3+1) = 2;
		*(test3+2) = 3;
		*(test3+3) = 500;
		printf("allocated test3:\n");
	}

	printList();
	my_heapinfo();



	my_free(test2);
	printf("after freeing test2 :-\n");
	printList();
	my_heapinfo();


	my_free(test);
	printf("after freeing test :-\n");
	printList();
	my_heapinfo();

	printf("1 :- \n");
	void* ddd= my_alloc(40*sizeof(int));
	printList();

	printf("2 :- \n");
	void* d= my_alloc(48);
	printList();

	printf("3 :- \n");
	void* dd= my_alloc(48);
	printList();

	printf("4 :- \n");
	void* dddd= my_alloc(48);
	printList();

	printf("5 :- \n");
	void* dddd1= my_alloc(3000);
	if(dddd1 == NULL){
		printf("WOW\n");
	}

	printList();
	my_free(d);

	printf("6 :- \n");
	printList();

	printf("7 :- \n");
	my_free(ddd);
	my_free(dddd1);
	printList();
	my_heapinfo();

	my_clean();
	return 0;
}
