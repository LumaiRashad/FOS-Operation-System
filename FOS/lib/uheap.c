#include <inc/lib.h>

// malloc()
// This function use FIRST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

// To do this, we need to switch to the kernel, allocate the required space
// in Page File then switch back to the user again.
//
// We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//  switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//  "memory_manager.c", then switch back to the user mode here
// the allocateMem function is empty, make sure to implement it.


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
struct AllocationInfo
{
 uint32 va_start;
 uint32 allocation_size;
 int isfree;
}allocationInfo[100000];int length=0;
uint32 numOFpagesInheap = (USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE;
uint32 pageindex[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE] = {0};

void* malloc(uint32 size)
{
 if (sys_isUHeapPlacementStrategyFIRSTFIT())
 {
 uint32 freesize=0;
 size = ROUNDUP(size,PAGE_SIZE);
 	 if(size==0 || size > numOFpagesInheap * PAGE_SIZE)
	 return NULL;
 uint32 sizeInPages = size/PAGE_SIZE;
 uint32 startindex = 0; int found=0;
 int counter1 =0, counter2 = 0;
 int firstOne=1;
 /*Searching for space*/
 for(uint32 i=0 ;i<numOFpagesInheap;i++)
 {
  if(pageindex[i]==1)
  {
	  firstOne = 1;
	  freesize=0;
   }
  else
  {
	  freesize++;
	    if(firstOne)
	    {
	     startindex=i;
	     firstOne = 0;
	     }
  }

  if(freesize == sizeInPages)
  {
   found=1;
   break;
  }
 }
   /*first fit allocation*/
   if(found){
   uint32 startAllocation = (USER_HEAP_START + (startindex*PAGE_SIZE));
   allocationInfo[length].va_start = startAllocation;
   allocationInfo[length].allocation_size = size;
   allocationInfo[length].isfree = 0;
   length++;

   for(int k=startindex; k<(startindex+freesize);k++)
    pageindex[k] = 1;


    sys_allocateMem(startAllocation,size);
    //cprintf("Address is %d \n", startAllocation);
    return (void*)startAllocation;
 }

 //This function should find the space of the required range
 // ******** ON 4KB BOUNDARY ******************* //

  //to check the current strategy

 //change this "return" according to your answer
 }
   return NULL;

}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
 //TODO: [PROJECT 2017 - [6] Shared Variables: Creation] smalloc() [User Side]
 // Write your code here, remove the panic and write your code
 //panic("smalloc() is not implemented yet...!!");

 // Steps:
 // 1) Implement FIRST FIT strategy to search the heap for suitable space
 //  to the required allocation size (space should be on 4 KB BOUNDARY)
 // 2) if no suitable space found, return NULL
 //  Else,
 // 3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
 //  sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
 // 4) If the Kernel successfully creates the shared variable, return its virtual address
 //    Else, return NULL

 //This function should find the space of the required range
 // ******** ON 4KB BOUNDARY ******************* //
 if (sys_isUHeapPlacementStrategyFIRSTFIT())
 {
 uint32 freesize=0;
  size = ROUNDUP(size,PAGE_SIZE);
  if(size==0)
 	 return NULL;
  uint32 sizeInPages = size/PAGE_SIZE;
  uint32 startindex = 0; int found=0;
  int counter1 =0, counter2 = 0;
  int firstOne=1;
  /*Searching for space*/
  for(uint32 i=0 ;i<numOFpagesInheap;i++)
  {
   if(pageindex[i]==0) {
    freesize++;
    if(firstOne){
     startindex=i;
     firstOne = 0;
     }
    }
   else{
    firstOne = 1;
    freesize=0;
   }

   if(freesize == sizeInPages){
    found=1;
    break;
   }
  }
    /*first fit allocation*/
  if(found)
  {

   uint32 startAllocation = (USER_HEAP_START + (startindex*PAGE_SIZE));
   int ID = sys_createSharedObject(sharedVarName, size, isWritable, (void*)startAllocation);
   if (ID<0)
    return NULL;
   allocationInfo[length].va_start = startAllocation;
   allocationInfo[length].allocation_size = size;
   allocationInfo[length].isfree = 0;
   length++;

   for(int k=startindex; k<(startindex+freesize);k++)
    pageindex[k] = 1;
   return (void*)startAllocation;
  }
 }

 //Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

 //change this "return" according to your answer
 return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
 //TODO: [PROJECT 2017 - [6] Shared Variables: Get] sget() [User Side]
 // Write your code here, remove the panic and write your code
 //panic("sget() is not implemented yet...!!");

 // Steps:
 // 1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
 // 2) If not exists, return NULL
 // 3) Implement FIRST FIT strategy to search the heap for suitable space
 //  to share the variable (should be on 4 KB BOUNDARY)
 // 4) if no suitable space found, return NULL
 //  Else,
 // 5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
 //  sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
 // 6) If the Kernel successfully share the variable, return its virtual address
 //    Else, return NULL
 //

 //This function should find the space for sharing the variable
 // ******** ON 4KB BOUNDARY ******************* //

 //Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy
 if (sys_isUHeapPlacementStrategyFIRSTFIT())
 {
 uint32 virsual_address = 0;
 int shared_obj_ID = 0;

 int shared_obj_size = sys_getSizeOfSharedObject(ownerEnvID, sharedVarName);

 if (shared_obj_size == E_SHARED_MEM_NOT_EXISTS)
   return NULL;
 else
 {
  //virsual_address=First_Fit_function(shared_obj_size);
  uint32 freesize=0;
  shared_obj_size = ROUNDUP(shared_obj_size,PAGE_SIZE);
  uint32 sizeInPages = shared_obj_size/PAGE_SIZE;
  uint32 startindex = 0; int found=0;
  int counter1 =0, counter2 = 0;
  int firstOne=1;
  /*Searching for space*/
  for(uint32 i=0 ;i<numOFpagesInheap;i++)
  {
   if(pageindex[i]==0)
   {
    freesize++;
    if(firstOne)
    {
     startindex=i;
     firstOne = 0;
    }
   }
   else
   {
    firstOne = 1;
    freesize=0;
   }
   if(freesize == sizeInPages)
   {
    found=1;
    break;
   }
  }
   /*first fit allocation*/
  if(found)
  {
   uint32 startAllocation = (USER_HEAP_START + (startindex*PAGE_SIZE));
   virsual_address=startAllocation;
   shared_obj_ID=sys_getSharedObject(ownerEnvID, sharedVarName, (void*)virsual_address);
   if(/*shared_obj_ID==-1 || shared_obj_ID==E_SHARED_MEM_NOT_EXISTS*/ shared_obj_ID<0)
       return NULL;
   allocationInfo[length].va_start = startAllocation;
   allocationInfo[length].allocation_size = shared_obj_size;
   allocationInfo[length].isfree = 0;
   length++;

   for(int k=startindex; k<(startindex+freesize);k++)
    pageindex[k] = 1;


    return (void *)virsual_address;
  }
  return NULL;
 }
}
 //change this "return" according to your answer
 return 0;
}

// free():
// This function frees the allocation of the given virtual_address
// To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
// from page file and main memory then switch back to the user again.
//
// We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//  switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//  "memory_manager.c", then switch back to the user mode here
// the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
 //TODO: [PROJECT 2017 - [5] User Heap] free() [User Side]
 // Write your code here, remove the panic and write your code
 //panic("free() is not implemented yet...!!");

	 uint32 size;
	 uint32 startindex;
	 uint32 numOfpagesTofree;
	 int Index = 0;
	 uint32 va , Size , isfree;
	 for(uint32 i=0 ;i<=length; i++)
	 {
	  if((uint32)virtual_address == allocationInfo[i].va_start && allocationInfo[i].isfree != 1)
	  {
	   allocationInfo[i].isfree = 1;
	   size = allocationInfo[i].allocation_size; // size to free
	   numOfpagesTofree = (allocationInfo[i].allocation_size / PAGE_SIZE);
	   startindex = (allocationInfo[i].va_start - USER_HEAP_START) / PAGE_SIZE;
	   Index = i;
	   break;
	  }
	 }
	   for(uint32 k= startindex ;k<(startindex+numOfpagesTofree);k++)
	   {
	    pageindex[k] = 0;
	   }

	   allocationInfo[Index].allocation_size = allocationInfo[length].allocation_size;
	   allocationInfo[Index].va_start = allocationInfo[length].va_start;
	   allocationInfo[Index].isfree = allocationInfo[length].isfree;
	 length--;
	 sys_freeMem((uint32)virtual_address,size);

 //you should get the size of the given allocation using its address
 //you need to call sys_freeMem()
 //refer to the project presentation and documentation for details
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=============
// [1] sfree():
//=============
// This function frees the shared variable at the given virtual_address
// To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
// from main memory then switch back to the user again.
//
// use sys_freeSharedObject(...); which switches to the kernel mode,
// calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
// the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
 //TODO: [PROJECT 2017 - BONUS4] Free Shared Variable [User Side]
 // Write your code here, remove the panic and write your code
 panic("sfree() is not implemented yet...!!");

 // 1) you should find the ID of the shared variable at the given address
 // 2) you need to call sys_freeSharedObject()

}


//===============
// [2] realloc():
//===============

// Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
// possibly moving it in the heap.
// If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
// On failure, returns a null pointer, and the old virtual_address remains valid.

// A call with virtual_address = null is equivalent to malloc().
// A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//  which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//  in "memory_manager.c", then switch back to the user mode here
// the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
 //TODO: [PROJECT 2017 - BONUS3] User Heap Realloc [User Side]
 // Write your code here, remove the panic and write your code
 panic("realloc() is not implemented yet...!!");

}
