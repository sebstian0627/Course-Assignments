#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <math.h>
#include<time.h>

//Global Variables.
#define PAGES (int)((pow(2,20)+0.5)) //max number of pages possible
#define PR_BIT_MASK 1<<10 //Present bit Mask
#define DR_BIT_MASK 1<<11 //Dirty bit mask.
int frames; //number of frames.
long int va; //complete virtual address.
int vpn; //VPN of the page
char* ptr; //useless.
int pr_bit=0; //present bit.
int verbose=0; //verbose bool.
int dr_bit=0; //dirty bits.

//For OPT, to make it efficient I implement a Array of Linked List to make the next access count operation to be O(1).
struct list_node{
	int access_count;
	struct list_node* next;
	struct list_node* prev;
}*List[1<<20];


int main(int argc, char* argv[]){
  srand(5635); //seeding to make deterministic.
	// freopen("output.txt","w",stdout);
  if(argc<3){
    printf("not enough params\n");
  }

//to print/not print every page replacement.
  if(argc>4){
    if(strcmp(argv[4],"-verbose")==0){
      verbose=1;
    }
  }

  int page_array[PAGES]; //Linear Page table.
	for( int i=0;i<PAGES;i++){
		page_array[i]=0;
		List[i]=NULL;
	}
  FILE* s=fopen(argv[1],"r");//file Handler

  if(s==NULL){
    printf("Please enter a valid trace file.\n");
    return 0;
  }

  sscanf(argv[2], "%d", &frames); //parsing number of frames.

  long long iter=0;
  int ta=1;


// to make OPT efficient, we are implementing an Array of linked list type structure. This stores access count. For making adding and deleting both order 1, I have implemented a partial DLL, only for the head of  the List, we set previous to the last node in the list, else all the prev pointers are null.
  if(strcmp(argv[3],"OPT")==0){
    FILE* ss=fopen(argv[1],"r");
    char sss[10]; //to store the complete address.
    char ssss[2]; //to store the access table.
    long vaa=0;
    while(fscanf(ss,"%s %s",sss,ssss)!=EOF){

      vaa = strtol(sss, &ptr, 0);
      struct list_node *temp;
      if(List[(int)( vaa>>12)]==NULL){
        temp=(struct list_node*)malloc(sizeof(struct list_node));
        temp->next=NULL;
        temp->access_count=ta;
        temp->prev=temp;
        List[(int) (vaa>>12)]=temp;
      }else{
        temp=List[(int) (vaa>>12)]->prev;
        temp->next=(struct list_node*)malloc(sizeof(struct list_node));
        temp=temp->next;
        temp->access_count=ta;
        temp->next=NULL;
				temp->prev=NULL;
        List[(int) (vaa>>12)]->prev=temp;
      }
      ta++;
    }
  }


  int frame_array[frames]; //inverted page table.
  int bitref_array[frames];//bitref_array for CLOCK.
  for(int i=0;i<frames;i++){
    bitref_array[i]=0;
		// LRU_arr[0]=-1;
  }
  for(int i=0;i<frames;i++){
    frame_array[i]=-10;
  }
  char virtual_address[10]; // to store the virtual address.
  char access_type[2]; // to store the access types.
  int curr_frames=0; //to store the current frames present.
  int no_access=0; //number of access
  int no_misses=0;//number of misses
  int start_clock=0;//this is to make a circular list implemented inverted page table.
  int no_writes=0;// number of writes
  int no_drops=0; //number of drops.
  int vpn=0; // VPN of the page
  int LRU_arr[frames]; //LRU array to find the least recently used frame.
  for(int lr=0;lr<frames;lr++){
    LRU_arr[lr]=-1;
  }

  int front_queue=0; // For FIFO, the start of the array is marked with this.

	//The main loop of the address reading.
  while(fscanf(s,"%s %s",virtual_address,access_type)!=EOF){

    no_access++;
    va = strtol(virtual_address, &ptr, 0); //parses the hex address to long int.
    vpn=(int) (va>>12); //obtains the vpn.
    pr_bit=(page_array[vpn]&PR_BIT_MASK)>>10; //obtains the VPN bit.
    if(pr_bit){

			//for OPT we update the corresponding list and remove the node in the list with current access count.
      if(strcmp(argv[3],"OPT")==0){

        struct list_node* ttemp;

        ttemp=List[vpn]->prev;
        if(ttemp!=List[vpn]||List[vpn]->next!=NULL){
          List[vpn]->next->prev=ttemp;
        }
        List[vpn]=List[vpn]->next;
      }
			//I update the recent access count in the LRU array.
      if(strcmp(argv[3],"LRU")==0){
        int PFN_MASK=1023;
        int PFN=(page_array[vpn])&PFN_MASK;
        LRU_arr[PFN]=no_access;
      }
			//CLOCK : update the reference bit array to give this frame second chance.
      if(strcmp(argv[3],"CLOCK")==0){
        int PFN_MASK=1023;
        int PFN=(page_array[vpn])&PFN_MASK;
        bitref_array[PFN]=1;
      }
			// if the access_type is W, we update the dirty bit.
      if(strcmp("W",access_type)==0){
        page_array[vpn]|=(1<<11);

      }
    }else{
      no_misses++;
			// if we can push more frames, we will do so and make different changes corresponding to the different techniques.
      if(curr_frames<frames){

        page_array[vpn]=curr_frames|PR_BIT_MASK; //updating the present bit.
        frame_array[curr_frames]=vpn; //inverted page table.
				//updating the dirty bit.
        if(strcmp("W",access_type)==0){
          page_array[vpn]|=DR_BIT_MASK;
        }
				// in case of LRU, we update the recent access in the LRU array.
        if(strcmp(argv[3],"LRU")==0){
          int PFN_MASK=1023;
          int PFN=(page_array[vpn])&PFN_MASK;
          LRU_arr[PFN]=no_access;
        }
				// in case of OPT, we remove the corresponding access count node from the list.
        if(strcmp(argv[3],"OPT")==0){
          struct list_node* ttemp;
          ttemp=List[vpn]->prev;
          if(ttemp!=List[vpn]){
            List[vpn]->next->prev=ttemp;
          }
          List[vpn]=List[vpn]->next;
        }
        curr_frames++;
      }else if(strcmp(argv[3],"FIFO")==0){
        dr_bit=page_array[frame_array[front_queue%frames]]&DR_BIT_MASK; // checking the dirty bit.
        dr_bit=dr_bit>>11;
        page_array[frame_array[front_queue%frames]]=0; //upodating the page table.
        if(dr_bit){
          if(verbose){
            printf("Page %#07x was read from disk, page %#07x was written to the disk.\n",vpn,frame_array[front_queue%frames]);
          }
          no_writes++;
        }else{
          if(verbose){

            printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n",vpn,frame_array[front_queue%frames]);
          }
          no_drops++;
					// printf("%#07x\n", frame_array[front_queue%frames]);
        }
				//updating the page table and inverted page table.
        page_array[vpn]=(front_queue%frames)|(PR_BIT_MASK);
        frame_array[front_queue%frames]=vpn;
				//updating the dirty bit.
        if(strcmp("W",access_type)==0){
          page_array[vpn]|=DR_BIT_MASK;
        }
        front_queue++;
      }else if(strcmp(argv[3],"RANDOM")==0){

        int page_to_evict=rand()%frames;

        dr_bit=page_array[frame_array[page_to_evict]]&DR_BIT_MASK;

        dr_bit=dr_bit>>11;

        page_array[frame_array[page_to_evict]]=0;
        if(dr_bit){
          if(verbose){
            printf("Page %#07x was read from disk, page %#07x was written to the disk.\n",vpn,frame_array[page_to_evict]);
          }
          no_writes++;
        }else{
          if(verbose){

            printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n",vpn,frame_array[page_to_evict]);
          }
          no_drops++;
        }
        page_array[vpn]=(page_to_evict)|(PR_BIT_MASK);
        frame_array[page_to_evict]=vpn;
        if(strcmp("W",access_type)==0){
          page_array[vpn]|=(1<<11);
        }
      }else if(strcmp(argv[3],"CLOCK")==0){

        int page_to_evict=-1;
				// rotating the array and setting refbits to 0 until a 0 ref bit is found.
        while(bitref_array[start_clock]){
          bitref_array[start_clock]=0;
          start_clock=(start_clock+1)%frames;
        }
        page_to_evict=start_clock;
				bitref_array[page_to_evict]=1;
        start_clock=(start_clock+1)%frames;




        dr_bit=page_array[frame_array[page_to_evict]]&DR_BIT_MASK;

        dr_bit=dr_bit>>11;

        page_array[frame_array[page_to_evict]]=0;
        if(dr_bit){
          if(verbose){
            printf("Page %#07x was read from disk, page %#07x was written to the disk.\n",vpn,frame_array[page_to_evict]);
          }
          no_writes++;
        }else{
          if(verbose){

            printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n",vpn,frame_array[page_to_evict]);
          }
          no_drops++;
        }
        page_array[vpn]=(page_to_evict)|(PR_BIT_MASK);
        frame_array[page_to_evict]=vpn;
        if(strcmp("W",access_type)==0){
          page_array[vpn]|=(1<<11);
        }

      }else if(strcmp(argv[3],"LRU")==0){
				// finding the least recently used page.
        int lru=0;
        for (int i=0;i<frames;i++){
          if(LRU_arr[lru]>LRU_arr[i]){
            lru=i;
          }
        }

        int page_to_evict=lru;
        LRU_arr[page_to_evict]=no_access; //updating the LRU array for the frame in the inverted page table.
        dr_bit=page_array[frame_array[page_to_evict]]&DR_BIT_MASK;

        dr_bit=dr_bit>>11;

        page_array[frame_array[page_to_evict]]=0;
        if(dr_bit){
          if(verbose){
            printf("Page %#07x was read from disk, page %#07x was written to the disk.\n",vpn,frame_array[page_to_evict]);
          }
          no_writes++;
        }else{
          if(verbose){

            printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n",vpn,frame_array[page_to_evict]);
          }
          no_drops++;
        }
        page_array[vpn]=(page_to_evict)|(PR_BIT_MASK);
        frame_array[page_to_evict]=vpn;
        if(strcmp("W",access_type)==0){
          page_array[vpn]|=(1<<11);
        }

      }else if(strcmp(argv[3],"OPT")==0){

        struct list_node* ttemp;
        ttemp=List[vpn]->prev;
        if(ttemp!=List[vpn]||List[vpn]->next!=NULL){
          List[vpn]->next->prev=ttemp;
        }

        List[vpn]=List[vpn]->next;

        int iterrr=0;
        // int not_accessed_anymore[frames];
        int page_to_evict=0;
        int temmmp=-1;
        for(iterrr;iterrr<frames;iterrr++){
          if(List[frame_array[iterrr]]==NULL){
            // not_accessed_anymore[iterrr]=-1;
            page_to_evict=iterrr;
            break;
          }else{
            if(List[frame_array[iterrr]]->access_count>temmmp){
              temmmp=List[frame_array[iterrr]]->access_count;
              page_to_evict=iterrr;
            }
          }
        }

        dr_bit=(page_array[frame_array[page_to_evict]]&DR_BIT_MASK);

        dr_bit=dr_bit>>11;

        page_array[frame_array[page_to_evict]]=0;
        if(dr_bit){
          if(verbose){
            printf("Page %#07x was read from disk, page %#07x was written to the disk.\n",vpn,frame_array[page_to_evict]);
          }
          no_writes++;
        }else{
          if(verbose){

            printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n",vpn,frame_array[page_to_evict]);
          }
          no_drops++;
        }
        page_array[vpn]=(page_to_evict)|(PR_BIT_MASK);
        frame_array[page_to_evict]=vpn;
        if(strcmp("W",access_type)==0){
          page_array[vpn]|=(1<<11);
        }




      }
    }
		// printf("_____________________________\n");
		// printf("(%#07x)\n",vpn);
		//
		// for (int i=0;i<frames;i++){
		// 	printf("(%#07x, %i)\n",frame_array[i],bitref_array[i]);
		// }
		// printf("_____________________________\n");
  }
  // printf("Number of memory accesses: %d\n",no_access);
  // printf("Number of misses: %d\n",no_misses);
  // printf("No of Writes: %d\n",no_writes);
  // printf("No of drops: %d\n",no_drops);
	printf("Number of memory accesses: %d\n", no_access);
	printf("Number of misses: %d\n", no_misses);
	printf("Number of writes: %d\n", no_writes);
	printf("Number of drops: %d\n", no_drops);
	// for (int i=0;i<frames;i++){
	// 	printf("(%#07x, %i)\n",frame_array[i],bitref_array[i]);
	// }
  return 0;
}
