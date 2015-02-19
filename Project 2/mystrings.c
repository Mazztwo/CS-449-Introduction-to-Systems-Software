#include <stdio.h> 		
#include <string.h>	
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if( argc < 2 )
	{
		printf("Please provide a file via command line to load into the program. Retry with a valid file name as an argument.\n");
		return 0;
	}
	else if( argc > 2 )
	{
		printf("Please only provide 1 argument file as input to the program. Please retry by only providing 1 command line argument.\n");
		return 0;
	}
	// Open the file specified by command line
	FILE *readFile = fopen(argv[1],"rb");
	
	// If file does not exist, exit program.
	if( readFile == NULL)
	{
		printf("Error opening the file or file does not exist. Please try again with a valid file.\n");
		return 0;
	}
	
	// When the nextChar variable dereferenced, you'll get the value of the character
	char nextChar;
	fread(&nextChar,sizeof(char),1,readFile);
		
	// the numChars will hold the number of characters in the string 
	int numCharsInString = 0;
	
	// Initializes the initial string with a default size of 10
	char *string = malloc( 10 * sizeof(char));
	
	// Continue to read chunks of 4 bytes from the file byte stream 
	// until the end of the file is reached
	while( feof(readFile) == 0 )
	{	
		// if the character is within the desired range, we should
		// append it to our string and move to the next character in the file
		if( nextChar > 31 && nextChar < 127 )
		{
			// To dynamically adjust the size of our string, we double its size if
			// the size of our string ever equals the number of characters in the string
			if( numCharsInString == strlen(string) && strlen(string) != 0 )
			{
				int newSize = strlen(string) * 2 * sizeof(char);
				string = (char *) realloc(string, newSize);
			}
			
				string[numCharsInString] = nextChar;
				numCharsInString = numCharsInString + 1;
	
		}
		else // if the character is not within our range, we print the string if it at least 4 chars long, then clear it 
		{
			if( numCharsInString >= 4)
			{
				printf("%s\n",string);
			}
			
			// Resets the string to have no characters and to have a size of 10 (20 bytes)
			numCharsInString = 0;
			string = (char *) malloc(20);
		}
		
		fread(&nextChar,sizeof(char),1,readFile);
	}
	
	return 0;
}