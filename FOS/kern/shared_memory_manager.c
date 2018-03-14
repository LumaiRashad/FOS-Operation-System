#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/environment_definitions.h>

#include <kern/shared_memory_manager.h>
#include <kern/memory_manager.h>
#include <kern/syscall.h>
#include <kern/kheap.h>

//2017

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// [1] Create "shares" array:
//===========================
//Dynamically allocate the array of shared objects
//initialize the array of shared objects by 0's and empty = 1
void create_shares_array(uint32 numOfElements)
{
	shares = kmalloc(numOfElements*sizeof(struct Share));
	if (shares == NULL)
	{
		panic("Kernel runs out of memory\nCan't create the array of shared objects.");
	}
	for (int i = 0; i < MAX_SHARES; ++i)
	{
		memset(&(shares[i]), 0, sizeof(struct Share));
		shares[i].empty = 1;
	}
}

//===========================
// [2] Allocate Share Object:
//===========================
//Allocates a new (empty) shared object from the "shares" array
//It dynamically creates the "framesStorage"
//Return:
//	a) if succeed:
//		1. allocatedObject (pointer to struct Share) passed by reference
//		2. sharedObjectID (its index in the array) as a return parameter
//	b) E_NO_SHARE if the the array of shares is full (i.e. reaches "MAX_SHARES")
int allocate_share_object(struct Share **allocatedObject)
{
	int32 sharedObjectID = -1 ;
	for (int i = 0; i < MAX_SHARES; ++i)
	{
		if (shares[i].empty)
		{
			sharedObjectID = i;
			break;
		}
	}

	if (sharedObjectID == -1)
	{
		//try to increase double the size of the "shares" array
		if (USE_KHEAP == 1)
		{
			shares = krealloc(shares, 2*MAX_SHARES);
			if (shares == NULL)
			{
				*allocatedObject = NULL;
				return E_NO_SHARE;
			}
			else
			{
				sharedObjectID = MAX_SHARES;
				MAX_SHARES *= 2;
			}
		}
		else
		{
			*allocatedObject = NULL;
			return E_NO_SHARE;
		}
	}

	*allocatedObject = &(shares[sharedObjectID]);
	shares[sharedObjectID].empty = 0;

	if (USE_KHEAP == 1)
	{
		shares[sharedObjectID].framesStorage = kmalloc(PAGE_SIZE);
		if (shares[sharedObjectID].framesStorage == NULL)
		{
			panic("Kernel runs out of memory\nCan't create the framesStorage.");
		}
		memset(shares[sharedObjectID].framesStorage, 0, PAGE_SIZE);
	}
	return sharedObjectID;
}

//=========================
// [3] Get Share Object ID:
//=========================
//Search for the given shared object in the "shares" array
//Return:
//	a) if found: SharedObjectID (index of the shared object in the array)
//	b) else: E_SHARED_MEM_NOT_EXISTS
int get_share_object_ID(int32 ownerID, char* name)
{
	int i=0;
	for(; i< MAX_SHARES; ++i)
	{
		if (shares[i].empty)
			continue;

		if(shares[i].ownerID == ownerID && strcmp(name, shares[i].name)==0)
		{
			return i;
		}
	}
	return E_SHARED_MEM_NOT_EXISTS;
}

//=========================
// [4] Delete Share Object:
//=========================
//delete the given sharedObjectID from the "shares" array
//Return:
//	a) 0 if succeed
//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists
int free_share_object(uint32 sharedObjectID)
{
	if (sharedObjectID >= MAX_SHARES)
		return E_SHARED_MEM_NOT_EXISTS;

	//panic("deleteSharedObject: not implemented yet");
	clear_frames_storage(shares[sharedObjectID].framesStorage);
	if (USE_KHEAP == 1)
		kfree(shares[sharedObjectID].framesStorage);

	memset(&(shares[sharedObjectID]), 0, sizeof(struct Share));
	shares[sharedObjectID].empty = 1;

	return 0;
}
//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=========================
// [1] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable,
		void* virtual_address)
{
	//TODO: [PROJECT 2017 - [6] Shared Variables: Creation] createSharedObject() [Kernel Side]
	// your code is here, remove the panic and write your code
	//panic("createSharedObject() is not implemented yet...!!");

	struct Env* myenv = curenv; //The calling environment

	// This function should create the shared object at the given virtual address with the given size
	// and return the ShareObjectID
	// RETURN:
	//	a) ShareObjectID (its index in "shares" array) if success
	//	b) E_SHARED_MEM_EXISTS if the shared object already exists
	//	c) E_NO_SHARE if the number of shared objects reaches max "MAX_SHARES"

	// Steps:
	//	1) Allocate a new share object (use allocate_share_object())
	//	2) Allocate the required space in the physical memory on a PAGE boundary
	//	3) Map the allocated space on the given "virtual_address" on the current environment "myenv": object OWNER,
	//	   with writable permissions
	//	4) Initialize the share object with the following:
	//		a) Set the data members of the object with suitable values (ownerID, name, size, ...)
	//		b) Set references to 1 (as there's 1 user environment that use the object now - OWNER)
	//		c) Store the object's isWritable flag (0:ReadOnly, 1:Writable) for later use by getSharedObject()
	//		d) Add all allocated frames to "frames_storage" of this shared object to keep track of them for later use
	//		(use: add_frame_to_storage())
	// 	5) 	If succeed: return the ID of the shared object (i.e. its index in the "shares" array)
	//		Else, return suitable error

	//cprintf("------------------------Start Of Create!!!---------------------------\n");
	struct Share *ptr_Shared_obj=NULL;
	struct Frame_Info* ptr_frame_info=NULL;
	int obj_ID = 0;
	//size = ROUNDUP(size, PAGE_SIZE);
	size/=PAGE_SIZE;
	int Get_ID_Ret=get_share_object_ID(ownerID, shareName);
	//cprintf("Get ID: %d\n", Get_ID_Ret);
	if (Get_ID_Ret!=E_SHARED_MEM_NOT_EXISTS)
		return E_SHARED_MEM_EXISTS;
	else
	{
		obj_ID=allocate_share_object(&ptr_Shared_obj);
		//cprintf("The ID %d\n", obj_ID);
	    if (obj_ID == E_NO_SHARE)
	    	return E_NO_SHARE;
	    for (int i=0; i<size; i++)
	    {
	    	ptr_frame_info=NULL;
	    	uint32 ret= allocate_frame(&ptr_frame_info);
	    	map_frame(myenv->env_page_directory, ptr_frame_info, virtual_address, PERM_USER|PERM_WRITEABLE|PERM_PRESENT);
			add_frame_to_storage(shares[obj_ID].framesStorage,ptr_frame_info,i);
			//cprintf("Address Before: %x\n", virtual_address);
			virtual_address+=PAGE_SIZE;
			// cprintf("Address Before: %x\n", virtual_address);
	    }

	    shares[obj_ID].ownerID=ownerID;
	    shares[obj_ID].size=size;
	    shares[obj_ID].isWritable=isWritable;
	    shares[obj_ID].references=1;
	    strcpy(shares[obj_ID].name,shareName);
	    //cprintf("-------------------------End Of Create!!!-------------------------\n");
	    return  obj_ID;
	}
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//TODO: [PROJECT 2017 - [6] Shared Variables: GetSize] getSizeOfSharedObject()
	// your code is here, remove the panic and write your code
	//panic("getSizeOfSharedObject() is not implemented yet...!!");

	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	unsigned int size = 0;
	struct Share *ptr_Shared_obj=NULL;
	int obj_ID= get_share_object_ID(ownerID,shareName);//get ID by search shares array for the obj
	if (obj_ID==E_SHARED_MEM_NOT_EXISTS)
		return E_SHARED_MEM_NOT_EXISTS;
	else
	{
		size = shares[obj_ID].size*PAGE_SIZE;
		//cprintf("Size is: %d", size);
		return size;
	}
	//change this "return" according to your answer
	return 0;
}

//======================
// [3] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
	{
		//TODO: [PROJECT 2017 - [6] Shared Variables: Get] getSharedObject() [Kernel Side]
		// your code is here, remove the panic and write your code
		//panic("getSharedObject() is not implemented yet...!!");

		// 	This function should share the required object in the heap of the current environment
		//	starting from the given virtual_address with the specified permissions of the object: read_only/writable
			// 	and return the ShareObjectID
		// Steps:
			//	1) Get the shared object from the "shares" array (use get_share_object_ID())
			//	2) Get its physical frames from the frames_storage
			//		(use: get_frame_from_storage())
			//	3) Share these frames with the current environment "myenv" starting from the given "virtual_address"
			//  4) make sure that read-only object must be shared "read only", use the flag isWritable to make it either read-only or writable
			//	5) Update references
			// 	6) 	If succeed: return the ID of the shared object (i.e. its index in the "shares" array)
			//		Else, return suitable error
		struct Env* myenv = curenv;
		struct Share *ptr_Shared_obj= NULL;
		struct Frame_Info* ptr_frame_info= NULL;
		int obj_ID= get_share_object_ID(ownerID,shareName);
		if (obj_ID==E_SHARED_MEM_NOT_EXISTS)
			return E_SHARED_MEM_NOT_EXISTS;
		int size = shares[obj_ID].size;
		//size= ROUNDUP(size, PAGE_SIZE);
		//size = size/PAGE_SIZE;
		for (int i=0; i<size; i++)
		{
			ptr_frame_info= NULL;
			ptr_frame_info=get_frame_from_storage(shares[obj_ID].framesStorage,i);
			if (shares[obj_ID].isWritable)
				map_frame(myenv->env_page_directory, ptr_frame_info, virtual_address, PERM_USER | PERM_WRITEABLE | PERM_PRESENT);
			else
				map_frame(myenv->env_page_directory, ptr_frame_info, virtual_address, PERM_USER &~ PERM_WRITEABLE &~ PERM_PRESENT);
			virtual_address+=PAGE_SIZE;
		}
		shares[obj_ID].references++;
		return obj_ID;

		//change this "return" according to your answer
		return -1;
	}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===================
// Free Share Object:
//===================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	struct Env* myenv = curenv; //The calling environment

	//TODO: [PROJECT 2017 - BONUS4] Free Shared Variable [Kernel Side]
	// your code is here, remove the panic and write your code
	panic("freeSharedObject() is not implemented yet...!!");

	// This function should free (delete) the shared object from the User Heapof the current environment
	// If this is the last shared env, then the "frames_store" should be cleared and the shared object should be deleted
	// RETURN:
	//	a) 0 if success
	//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists


	// Steps:
	//	1) Get the shared object from the "shares" array (use get_share_object_ID())
	//	2) Unmap it from the current environment "myenv"
	//	3) If one or more table becomes empty, remove it
	//	4) Update references
	//	5) If this is the last share, delete the share object (use free_share_object())
	//	6) Flush the cache "tlbflush()"

	//change this "return" according to your answer
	return 0;
}
