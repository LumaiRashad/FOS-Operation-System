#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

//=================================================================================//
//============================ REQUIRED FUNCTION ==================================//
//=================================================================================//
/* pointer to first free VA in Heap*/
uint32 firstFreeVAInKHeap = KERNEL_HEAP_START;
uint32 start = KERNEL_HEAP_START;
int index=0;
struct Allocate_info
{
	uint32 start_allocate_address;
	int size;
	int is_kfree;
}allocate_info[1024];
void* kmalloc(unsigned int size)
{
	/*panic("kmalloc()   khcontalloc ");*/
	uint32 ptr_last_allocate = firstFreeVAInKHeap;
	struct Frame_Info *ptr_frame_info = NULL;
	int va2=0;
	size=ROUNDUP(size,PAGE_SIZE);

	if(isKHeapPlacementStrategyCONTALLOC())
	{
		if(((uint64)firstFreeVAInKHeap+size) < KERNEL_HEAP_MAX && size>0)
		{
			allocate_info[index].start_allocate_address = ptr_last_allocate;
		    allocate_info[index].size = ROUNDUP(size,PAGE_SIZE);
		    allocate_info[index].is_kfree = 0;
		    index++;

		    for(int i=0 ;i<size; i+=PAGE_SIZE)
		    {
			   ptr_frame_info = NULL;
			   int ret = allocate_frame(&ptr_frame_info);
			   if(ret!= E_NO_MEM)
			   {
		         map_frame(ptr_page_directory,ptr_frame_info,(void*)firstFreeVAInKHeap ,3);
				 firstFreeVAInKHeap+=PAGE_SIZE;
			   }
		    }
		    return (void*)ptr_last_allocate;
		}
		return NULL;

	}
	else if(isKHeapPlacementStrategyNEXTFIT())
	{
		if(((uint64)start+size) < KERNEL_HEAP_MAX && size>0)
				{
					allocate_info[index].start_allocate_address = ptr_last_allocate;
				    allocate_info[index].size = ROUNDUP(size,PAGE_SIZE);
				    allocate_info[index].is_kfree = 0;
				    index++;

				    for(int i=0 ;i<size; i+=PAGE_SIZE)
				    {
					   ptr_frame_info = NULL;
					   int ret = allocate_frame(&ptr_frame_info);
					   if(ret!= E_NO_MEM)
					   {
				         map_frame(ptr_page_directory,ptr_frame_info,(void*)start ,PERM_WRITEABLE|PERM_PRESENT);
						 start+=PAGE_SIZE;
					   }
				    }
				    return (void*)ptr_last_allocate;
				}
	    //size=ROUNDUP(size,PAGE_SIZE);
				uint32 sizeInPages = size/PAGE_SIZE;
				uint32 va;
				if(size>KERNEL_HEAP_MAX-KERNEL_HEAP_START)
				{
					return NULL;
				}
				int counter;
				for(uint32 i=KERNEL_HEAP_START;i<ptr_last_allocate;i+=PAGE_SIZE)
				{
					counter=0;
					va=i;
					for(int j=0;j<sizeInPages;j++)
					{
						if(KERNEL_HEAP_MAX-va<size)
						{
							return NULL;
						}
						uint32 *ptr_page_table1=NULL;
	                    struct Frame_Info *ptr_frame_info1 = NULL;
						ptr_frame_info1=get_frame_info(ptr_page_directory,(void*)va,&ptr_page_table1);
						if(ptr_frame_info1==NULL)
						{
							counter++;
							va+=PAGE_SIZE;
						}
						else
						{
							break;
						}
					}
					if(counter==sizeInPages)
					{
						va=i;
						//start=va;
						for(int k=0;k<sizeInPages;k++)
						{
							struct Frame_Info *ptr_frame_info2 = NULL;
						    int ret = allocate_frame(&ptr_frame_info2);
						    if(ret!= E_NO_MEM)
						    {
						     ret= map_frame(ptr_page_directory,ptr_frame_info2,(void*)va,PERM_WRITEABLE|PERM_PRESENT);
						     if(ret== E_NO_MEM)
						      {
						    	 free_frame(ptr_frame_info2);
						    	 return NULL;
						      }
						    }
						     va+=PAGE_SIZE;
						     //start=start+i-size
						}
						allocate_info[index].start_allocate_address = i;
						allocate_info[index].size = ROUNDUP(size,PAGE_SIZE);
						allocate_info[index].is_kfree = 0;
						  index++;
						return (void*)i;
					}
				}
				return NULL;
		}

		/*size=ROUNDUP(size,PAGE_SIZE);
		uint32 sizeInPages = size/PAGE_SIZE;
		uint32 va;
		if(size>KERNEL_HEAP_MAX-KERNEL_HEAP_START)
		{
			return NULL;
		}
		int counter;
		for(uint32 i=start;i<KERNEL_HEAP_MAX;i++)
		{
			counter=0;
			va=i;
			//for(int j=0;j<sizeInPages;j++)
			//{
		    if(KERNEL_HEAP_MAX-va<size)
			{
				return NULL;
			}
			uint32 *ptr_page_table1=NULL;
		    struct Frame_Info *ptr_frame_info1 = NULL;
			ptr_frame_info1=get_frame_info(ptr_page_directory,(void*)va,&ptr_page_table1);
			if(ptr_frame_info1==NULL)
			{
			  counter++;
			  //va+=PAGE_SIZE;
			}
			else
			{
			    counter=0;
			}
			//}
			if(counter>=size)
			{
				start=i;
				//va=i;
				va2=i+PAGE_SIZE-size;

				//start=va;
			for(int k=va2;k<va2;k++)
			{
				struct Frame_Info *ptr_frame_info2 = NULL;
				int ret = allocate_frame(&ptr_frame_info2);
				if(ret!= E_NO_MEM)
				{
					ret= map_frame(ptr_page_directory,ptr_frame_info2,(void*)k,PERM_WRITEABLE|PERM_PRESENT);
					if(ret== E_NO_MEM)
					{
						free_frame(ptr_frame_info2);
						return NULL;
					}
				}
			}
			allocate_info[index].start_allocate_address = va2;
			allocate_info[index].size = ROUNDUP(size,PAGE_SIZE);
			allocate_info[index].is_kfree = 0;
			index++;
			return (void*)va2;
		}
	 }
	 for(uint32 i=KERNEL_HEAP_START;i<va2;i+=PAGE_SIZE)
	 {
		counter=0;
		va=i;
		for(int j=0;j<sizeInPages;j++)
		{
			if(KERNEL_HEAP_MAX-va<size)
			{
			   return NULL;
			}
			uint32 *ptr_page_table1=NULL;
			struct Frame_Info *ptr_frame_info1 = NULL;
			ptr_frame_info1=get_frame_info(ptr_page_directory,(void*)va,&ptr_page_table1);
			if(ptr_frame_info1==NULL)
			{
				counter++;
											//va+=PAGE_SIZE;
			}
			else
			{
				break;
			}
		}
		if(counter==sizeInPages)
		{
			//va=i;
			va2=i+PAGE_SIZE-size;
			//start=va;
			for(int k=va2;k<va2;k++)
			{
				struct Frame_Info *ptr_frame_info2 = NULL;
				int ret = allocate_frame(&ptr_frame_info2);
				if(ret!= E_NO_MEM)
				{
					ret= map_frame(ptr_page_directory,ptr_frame_info2,(void*)k,PERM_WRITEABLE|PERM_PRESENT);
					if(ret== E_NO_MEM)
					{
						free_frame(ptr_frame_info2);
						return NULL;
					}
				}
			}
			allocate_info[index].start_allocate_address = va2;
			allocate_info[index].size = ROUNDUP(size,PAGE_SIZE);
			allocate_info[index].is_kfree = 0;
			index++;
			return (void*)va2;
		}
	}
	return NULL;*/

	else if(isKHeapPlacementStrategyFIRSTFIT())
	{
		    size=ROUNDUP(size,PAGE_SIZE);
			uint32 sizeInPages = size/PAGE_SIZE;
			uint32 va;
			if(size>KERNEL_HEAP_MAX-KERNEL_HEAP_START)
			{
				return NULL;
			}
			int counter;
			for(uint32 i=KERNEL_HEAP_START;i<KERNEL_HEAP_MAX;i+=PAGE_SIZE)
			{
				counter=0;
				va=i;
				for(int j=0;j<sizeInPages;j++)
				{
					if(KERNEL_HEAP_MAX-va<size)
					{
						return NULL;
					}
					uint32 *ptr_page_table1=NULL;
                    struct Frame_Info *ptr_frame_info1 = NULL;
					ptr_frame_info1=get_frame_info(ptr_page_directory,(void*)va,&ptr_page_table1);
					if(ptr_frame_info1==NULL)
					{
						counter++;
						va+=PAGE_SIZE;
					}
					else
					{
						break;
					}
				}
				if(counter==sizeInPages)
				{
					va=i;
					//start=va;
					for(int k=0;k<sizeInPages;k++)
					{
						struct Frame_Info *ptr_frame_info2 = NULL;
					    int ret = allocate_frame(&ptr_frame_info2);
					    if(ret!= E_NO_MEM)
					    {
					     ret= map_frame(ptr_page_directory,ptr_frame_info2,(void*)va,PERM_WRITEABLE|PERM_PRESENT);
					     if(ret== E_NO_MEM)
					      {
					    	 free_frame(ptr_frame_info2);
					    	 return NULL;
					      }
					    }
					     va+=PAGE_SIZE;
					     //start=start+i-size
					}
					allocate_info[index].start_allocate_address = i;
					allocate_info[index].size = ROUNDUP(size,PAGE_SIZE);
					allocate_info[index].is_kfree = 0;
					  index++;
					return (void*)i;
				}
			}
			return NULL;
	}
	return NULL;
}
int counter=0;
void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	/*panic("kfree()");*/
	int x=0;
	int found =0;
	while(1){
		if(allocate_info[x].start_allocate_address == (uint32)virtual_address)
			{found = 1;  break; }
			x++;
		}
	struct Frame_Info* ptr_frame_info = NULL;
	uint32* ptr_page_table = NULL;

	if(allocate_info[x].is_kfree!=1 && found==1){
	for(uint32 i = (uint32)virtual_address ;i<((uint32)virtual_address+allocate_info[x].size); i+=PAGE_SIZE)
	 {
		ptr_frame_info = get_frame_info(ptr_page_directory,(void*)i,&ptr_page_table);
		free_frame(ptr_frame_info);
		unmap_frame(ptr_page_directory , (void*)i);
	}
	}
	allocate_info[x].is_kfree = 1;
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	/*panic("kheap_virtual_address()");*/
	struct Frame_Info *ptr_frame_info1 = NULL;
	ptr_frame_info1 = to_frame_info(physical_address) ;

	struct Frame_Info *ptr_frame_info2 = NULL;
	uint32 *ptr_page_table2=NULL;

		ptr_frame_info1 = to_frame_info(physical_address);
		int i = 0;
		/*USING first start here TO AVOID searching in hole HEAP VIRUAL SPACE*/
		for(uint32 va=KERNEL_HEAP_START; va<firstFreeVAInKHeap ;va+=PAGE_SIZE)
		{i++;
			ptr_frame_info2 = NULL;
			ptr_frame_info2=get_frame_info(ptr_page_directory,(void*)va,&ptr_page_table2);
			if(ptr_frame_info2 == ptr_frame_info1)
				return va;
		}
	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2017 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	uint32 *ptr_page_table;
	get_page_table(ptr_page_directory,(void*)virtual_address,&ptr_page_table);
	if(ptr_page_table!=NULL){
		uint32 va = ptr_page_table[PTX(virtual_address)] & 0xFFFFF000;
		uint32 offset = virtual_address&0x00000FFF;
		va+=offset;
		return va;
	}
	//change this "return" according to your answer
	return 0;
}
//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2017 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
