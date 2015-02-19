#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "mymalloc.h"

//To be used for the 1GB of memory assigned to the buddy malloc function
static void *base = NULL;

struct Node
{
	// 8 bits of info. 
	// First bits is if it is free/used. 
	// Other 7 bits represent the size of the piece.
	 char header;		

	// Holds address of the node before
	struct Node *previous;		

	// Holds address of the node after
	struct Node *next;				 
};

struct freeTable
{
	// The table has entries 0-25
	// These correspond from 32 bytes all the way up to 1GB
	// table[0] = 32 bytes
	// table[1] = 64 bytes 
	// etc..
	struct Node *table[26];

}freeTable;

void *my_buddy_malloc(int size)
{
	// Means the 1GB of memory has not been initialized yet, so we must initialize it
	if( base == NULL)
	{
		// Initialize base memory to point to 1GB and the freetable to have all null entries
		base = mmap(NULL,1<<30,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,0,0);
		int i;

		// Initialize all the entries in our pointer table to NULL
		for(i = 0; i < 26; i++)
		{
			freeTable.table[i] = NULL;
		}

		struct Node start;
		start.header = 0;

		// Make bit 1 equal to 0, meaning the memory is free, and store 30 into the 
		// remaning 7 bits, 30 being the exponent of 2^30 for the size. The eight bits
		// will look like this: 00011110. This is equal to 0x1e in hex, which is what
		// we store in the header of our node. 
		start.header = start.header | 30; 
		start.previous = NULL;
		start.next = NULL;

		// Last entry of our table, which represents 1GB, points to the node that
		// points to the 1GB space. We place the start node at the front of our 1GB chunk
		// and set freeTable[25] to point to that node.
		freeTable.table[25] = (struct Node*)base;
		*freeTable.table[25] = start;
	}

	// In order to get the correct index of the FreeTable we will be working in,
	// we will need to figure out the closest power of 2 to the user's allocation size.
	// To do this, we can divide the size by 2 until we get below 32. This will give us
	// the correct index in freeTable. 

	// For example, if the user wants to allocate a chunk of 80 bytes, the closest size we
	// support will be a 128 byte chunk. So, we can divide 80 by two twice before we get below 32.
	// The correct index in the free able is equal to the number of times we divided by two. 
	// So, freeTable.table[0] = 32 bytes, freeTable.table[1] = 64 bytes and freeTable.table[2] = 128 bytes. freetable.table[2] 
	// is what we want, so we know that this works. 
	int numTimesDividedByTwo = 0;
	double sizeCopy = (double) size;

	while( sizeCopy >= 32 )
	{
		sizeCopy = sizeCopy / 2;
		numTimesDividedByTwo = numTimesDividedByTwo + 1;
	}
	
	// The first thing we do is check the freeTable.table[numTimesDividedByTwo] to see if there are any nodes
	// there. If there are, then we don't have to do anything. We can just return a pointer to the memory
	// to the user.
	if(freeTable.table[numTimesDividedByTwo] != NULL)
	{
		// Grab the first node pointer on the freeTable
		struct Node *chunkToReturn = freeTable.table[numTimesDividedByTwo];

		// Sets the first byte of the header to a 1, signifying that it is no longer free 
		(*chunkToReturn).header = (*chunkToReturn).header | 128;

		// Remove the first node by making freeTable.table[numTimesDividedByTwo] point to the next node
		freeTable.table[numTimesDividedByTwo] = (*chunkToReturn).next;

		// Make the previous pointer of the new head point to NULL
		if( (*chunkToReturn).next != NULL)
		{
			(*freeTable.table[numTimesDividedByTwo]).previous = NULL;
		}

		// Increment the address of the chunk by 1 byte, moving past the header to the usable memory, and return
		// a pointer to that usable memory to the user.
		return ((char *)chunkToReturn + 1);
	}
	else
	{
		// The freeTable.table[numTimesDividedByTwo] is null if we reach this part of the code. We make a copy of numTimesDividedByTwo called index.
		// We will make a while loop that will check freeTable.table[index] to see if there is a chunk available.
		// For example, if size = 80 and we check freeTable.table[2] to give the user a pointer to a 128 byte chunk. If the freeTable.table[index] pointer 
		// points to nothing, that means that we must increment index and keep moving down the free table until we find a bigger chunk. When we find
		// this bigger chunk, we will perform our buddy algorithm and split the chunk evenly in two until we can satisfy the allocation size wanted by the user.
		int index = numTimesDividedByTwo;

		while(freeTable.table[index] == NULL)
		{
			index++;
		}

		// Now we've found the index in freeTable where there is a chunk big enough to start splitting.
		// We will keep splitting chunks until we've reached index == numTimesDividedByTwo.
		while(index != numTimesDividedByTwo)
		{
			// First we grab the chunk we need to split
			struct Node *chunk1 = freeTable.table[index];

			// Next, we remove the chunk from the freeTable 
			freeTable.table[index] = (*chunk1).next;

			// Make the previous pointer of the new head point to NULL
			if( (*chunk1).next != NULL)
			{
				(*freeTable.table[index]).previous = NULL;
			}
					
			// Splits the current chunk into two by making two different pointers
			// To do this properly, we must get the size of the chunk, and then divide it by 2 to get the size of the new chunks
			// We then add this as an offeset to chunk1 to get the correct address of chunk2
			char power = (*chunk1).header;
			int newChunkSize = (1<<power) / 2;

			// In order to get the halfway point of the block, i first need to convert the Node* to a char*. After that, I add the new
			// chunk size as an offset to the char pointer to indicate that I want to add that many bytes to the address. Then I convert 
			// that pointer back to a Node* and assign it to chunk2.
			//
			// We must do it in this fashion because simply adding newChunkSize to chunk1 will get us the newChunkSize element of the memory. 
			// We don't want that, we want to get to the newChunkSize byte after chunk1, which is why we cast chunk1 as a character pointer before 
			// adding the newChunkSize to it. 
			struct Node *chunk2 = (struct Node*) (((char *)chunk1) + newChunkSize);

			// We now make two new nodes for the two new chunks we'll be making
			struct Node node1;
			struct Node node2;

			// The free table starts at 32 bytes, which is freeTable.table[0]. If we want to get 32 bytes,
			// we add 5 to our index. So for example, freeTable.table[1] = 64 byte lists. 1 + 5 = 6 and 2^6 = 64.
			// Inside the header we store the power of two of our  new chunk, so we just take our index subtract 1 and add 5 and store
			// it in the header.
			node1.header = (index -1)  + 5;
			node2.header = (index -1)  + 5;

			// We now connect the two new nodes properly
			node1.next = chunk2;
			node1.previous = NULL;
			node2.next = NULL;
			node2.previous = chunk1;

			// We now place the nodes inside our new chunks of memory
			*chunk1 = node1;
			*chunk2 = node2;

			// We must now add these two new chunks to the free table at index - 1.
			// If the the pointer at index - 1 is null, then we just assign it to chunk1.
			if(freeTable.table[index -1] == NULL)
			{	
				// Set the pointer to point to chunk1
				freeTable.table[index - 1] = chunk1;
			}
			else
			{
				// We first set the next node after chunk 2 to be the current node at freeTabletable[index - 1];
				(*chunk2).next = &(*freeTable.table[index - 1]);

				// Then we set the current head of table[index - 1] previous pointer to point to chunk2
				(*freeTable.table[index - 1]).previous = &(*chunk2);

				// Finally, we make table[index -1] point to chunk 1 
				freeTable.table[index -1] = &(*chunk1);
			}	
			
			// The last thing we will do is decrement index. This will move us down the freeTable. We will keep repeating
			// the above code and splitting chunks until index == numTimesDividedByTwo. When this occurs we have available 
			// chunks to return to the user, so we will do so.
			index--;
		}

		// If we get to this point, it means that we have an available chunk at freeTable.table[numTimesDividedByTwo] 
		// to give to the user. So we will remove that chunk, update our freeTable, and return a pointer to it to the user.

		// Grab the first node pointer on the freeTable
		struct Node *chunkToReturn = freeTable.table[numTimesDividedByTwo];
		// Remove the first node by making freeTable.table[numTimesDividedByTwo] point to the next node
		freeTable.table[numTimesDividedByTwo] = (*chunkToReturn).next;

		// Make the previous pointer of the new head point to NULL
		if( (*chunkToReturn).next != NULL)
		{
			(*freeTable.table[numTimesDividedByTwo]).previous = NULL;
		}

		// Sets the first byte of the header to a 1, signifying that it is no longer free 
		(*chunkToReturn).header = (*chunkToReturn).header | 128;

		// Increment the address of the chunk by 1 byte, moving past the header to the usable memory, and return
		// a pointer to that usable memory to the user.
		return ((char *)chunkToReturn + 1);
	}		
	return NULL;
}



void my_free(void *ptr)
{
	char *pointer = ptr;

	// The pointer given to us points to the byte after the header, so we must decrement 
	// our pointer 1 byte to have the correct pointer
	pointer  = pointer - 1;

	// We first want to turn off the first bit;
	pointer[0] = pointer[0] & 127;

	// First we grab the first byte in the memory that was given to us by the user.
	// This will contain the header information that we need for the merging algorithm.
	char blockSize = pointer[0];

	// We then convert the size we extract from the header into an index for our freeTable.
	// We do this by subtracting 5 from our size. We subtract five since we start at 2^5.
	int index = ((int)blockSize) - 5;

	// We must now perform the buddy algorithm to see if the buddies are available to be merged.
	// While the buddy is free to be merged, we merge the memory. At the end we will add the memory
	// back to the freeTable. 

	// Get the offset of our pointer so we can treat our first address as 0
	int addr = (pointer - (char *)base);

	// We now get the size of our block
	int size = 1<<blockSize;

	// Now we calculate the address offset of our buddy
	int buddyOffset = (addr ^ size);

	// We'll add this offset to base to get the location of our buddy
	char *buddy = ((char *)base) + buddyOffset; 

	// Now that we have the address of our buddy, we have to check the header bit to see if it is
	// a 0 or 1. If it is a 1, then the buddy is not free, so we just add our chunk back to the free table
	// If it is a 0, then this means that our buddy is free, and we can merge our chunks and check for
	// another open buddy.
	char buddyHeader = buddy[0];
	int bit7 = buddyHeader>>7 & 1;

	struct Node *block = (struct Node*)pointer;
	struct Node front;
	front.header = blockSize;
	front.next = NULL;
	front.previous = NULL;
	*block = front;


	// This is where we will merge buddies until the occupancy bit is a 1
	while(bit7 == 0)
	{
		// First make a new node to be the head of the new chunk
		struct Node head;
		head.header = blockSize + 1;
		head.previous = NULL;
		head.next = NULL;

		// We then add our node to the beginning of the memory
		*block = head;

		// We know where our buddy is and we know it is free. We now check for two things.
		// If the buddy.previous is NULL, it means that it is the first chunk in the free list,
		// so we should make freeList at that index point to buddy.next. If buddy.previous is
		// not NULL, then this means that it is in the middle of our freelist somwhere, and we should
		// make the node before the buddy point to the node after the buddy.
		struct Node *node_buddy = (struct Node*)buddy;

		if( (*node_buddy).previous == NULL ) 
		{
			freeTable.table[index] = (*node_buddy).next;
		}
		else
		{
			(*(*node_buddy).previous).next = (*node_buddy).next;
		}	

		// If the address of the buddy is less than the address of the other buddy,
		// we update our pointer to point to the first buddy
		if(buddy < pointer)
		{
			pointer = buddy;
			block = (struct Node*)pointer;
			*block = head;
		}

		// Now we've found and removed the buddy from our free table, thereby merging the buddies together.
		// We must now increment blockSize by 1 and start the process of finding the 7th bit of the buddy
		// header again.
		blockSize = blockSize + 1;
		index = ((int)blockSize) - 5;

		// if we reach the top of the table, we will be done
		if(index == 25)
		{
			break;
		}

		size = 1<<blockSize;
		buddyOffset = (addr ^ size);
		buddy = ((char *)base) + buddyOffset; 
		buddyHeader = buddy[0];
		bit7 = buddyHeader>>7 & 1;
		
		// This will keep going until the occupancy bit is 1, which means the buddy isn't free. At that point
		// we will add our chunk to the correct position in the free table
	}

	// Add block to correct place in the freeTable and we are done
	int indexToPlace = ((int)pointer[0]) - 5;

	if( freeTable.table[indexToPlace] != NULL )
	{
		(*freeTable.table[indexToPlace]).previous = block;
		freeTable.table[indexToPlace] = block;
	}
	else
	{
		freeTable.table[indexToPlace] = block;
	}
}