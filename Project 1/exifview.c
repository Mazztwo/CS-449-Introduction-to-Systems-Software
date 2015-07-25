// CS449 ASSIGNMENT 1 Exif viewer
// T-TH 4-515
// Wonsun 


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct JPEG_header
{
	char header[20];
};

struct TIFF_tag
{
	char tag[12];
};


int main(int argc, char *argv[])
{
	//Open the file
	FILE *image_file;
	image_file = fopen(argv[1], "rb");

	//If file is bad, will abort program.
	if(image_file == NULL)
	{
		printf("\nYou've entered an invalid file name. Please try again.\n");
		return 0;
	}
	
	//Read first 20 bytes into JPEG_header struct
	struct JPEG_header JPEG_tag;
	fread(&JPEG_tag,sizeof(char),20,image_file);
	
	//Check to make sure the image is APP1
	char APP1 = 0xE1;
	if( JPEG_tag.header[3] != APP1)
	{
		printf("\nApp type is not APP1. Please try again with an APP1 image.\n");
		return 0;
	}
	
	//Make sure Exif string is in the right place
	char JPEG_exif[5];
	fseek(image_file,6,SEEK_SET);
	fread(&JPEG_exif,sizeof(char),5,image_file);
	char Exif[] = "Exif";
	if(strncmp(JPEG_exif,Exif,5) != 0)
	{
		printf("\nExif string is not in the right place. Please try again with another file.\n");
		return 0;
	}
	
	//Check to make sure file is little endian(II)
	char JPEG_endianness[2];
	fseek(image_file,12,SEEK_SET);
	fread(&JPEG_endianness,sizeof(char),2,image_file);
	char endianness[3] = "II";
	if(strncmp(JPEG_endianness,endianness,2) != 0)
	{
		printf("\nBig Endianness is not supported. Please try again with a different image.\n");
		return 0;
	}	
	
	//Get the first count variable at offset 20
	unsigned char count1;
	fseek(image_file,20,SEEK_SET);
	fread(&count1,sizeof(unsigned short),1,image_file);
	
	//Go through file count1 times, try to find important identifiers:
	//0x010F --> Manufacturer String
	//0x0110 --> Camera Model String
	//0x8769 --> Exif sub block address
	
	char manufacturer_string[2];
	manufacturer_string[0] = 0x01;
	manufacturer_string[1] = 0x0F;
	
	char camera_model_string[2];
	camera_model_string[0] = 0x01;
	camera_model_string[1] = 0x10;
	
	char exif_sub_block_address[2];
	exif_sub_block_address[0] = 0x87;
	exif_sub_block_address[1] = 0x69;
	
	struct TIFF_tag tifftag;
	
	//This is the starting offset for TIFF tags
	int file_index = 22;
	
	char TIFF_identifier[2];
	int offset;
	int bytes_in_string;
	
	//will store the manufacturer string
	char manufacturer[30];
	
	//will store the camera model string
	char camera_model[30];
	
	int i;
	for(i = 0; i < count1; i = i + 1)
	{
		fseek(image_file,file_index,SEEK_SET);
		fread(&tifftag,sizeof(char),12,image_file);
		TIFF_identifier[0] = tifftag.tag[0];
		TIFF_identifier[1] = tifftag.tag[1];
		
		//This checks to see if the identifier matches the manufacturer string.
		if(strncmp(manufacturer_string,TIFF_identifier,2) == 0)
		{
			//Go to number of data items in tag
			fseek(image_file,file_index + 4,SEEK_SET);
			
			//The size of the string in the bytes_in_string variable
			fread(&bytes_in_string,sizeof(int),1,image_file);
		
			//Go to the offset in the tag
			fseek(image_file,file_index + 8,SEEK_SET);
			
			//Store the offset int in offset variable
			fread(&offset,sizeof(int),1,image_file);
			
			//Go to 12 + offset location to grab the manufacturer string
			fseek(image_file,12 + offset,SEEK_SET);
			
			//Now we read each letter until we hit the NULL
			int k;
			for(k = 0; k < bytes_in_string; k = k + 1)
			{
				fread(&manufacturer[k],sizeof(char),1,image_file);
				fseek(image_file,1,SEEK_CUR);
			}
		}
		
		//This checks to see if the identifier matches the camera model string.
		if(strncmp(camera_model_string,TIFF_identifier,2) == 0)
		{
			//Go to number of data items in tag
			fseek(image_file,file_index + 4,SEEK_SET);
			
			//The size of the string in the bytes_in_string variable
			fread(&bytes_in_string,sizeof(int),1,image_file);
		
			//Go to the offset in the tag
			fseek(image_file,file_index + 8,SEEK_SET);
			
			//Store the offset int in offset variable
			fread(&offset,sizeof(int),1,image_file);
			
			//Go to 12 + offset location to grab the manufacturer string
			fseek(image_file,12 + offset,SEEK_SET);
			
			//Now we read each letter until we hit the NULL
			int k;
			for(k = 0; k < bytes_in_string; k = k + 1)
			{
				fread(&camera_model[k],sizeof(char),1,image_file);
				fseek(image_file,1,SEEK_CUR);
			}
		}
		
		//This checks to see if the identifier matches the exif sub block address
		if(strncmp(exif_sub_block_address,TIFF_identifier,2) == 0)
		{
			//Go to the offset in the tag
			fseek(image_file,file_index + 8,SEEK_SET);
			
			//Store the offset int in offset variable
			fread(&offset,sizeof(int),1,image_file);
			
			break;
		}
		//increment file index to next tag by adding an offset of 12 to the current position
		file_index = file_index + 12;
	}

	//seek to other exif sub block
	fseek(image_file,12 + offset,SEEK_SET);
	
	//grab the 2nd count
	unsigned short count2;
	
	fread(&count2,sizeof(unsigned short),1,image_file);
	
	file_index = 12 + offset + 2;
	
	//We must now loop through a second time and find important identifiers:
	// 0xA002 --> width in pixels
	// 0xA003 --> height in pixels
	// 0x8827 --> ISO speed
	// 0x829A --> Exposure speed
	// 0x829D --> F-stop
	// 0x920A --> Lens focal length
	// 0x9003 --> Date taken
	
	char width_string[2];
	width_string[0] = 0xA0;
	width_string[1] = 0x02;
	
	char height_string[2];
	height_string[0] = 0xA0;
	height_string[1] = 0x03;
	
	char iso_string[2];
	iso_string[0] = 0x88;
	iso_string[1] = 0x27;
	
	char exposure_string[2];
	exposure_string[0] = 0x82;
	exposure_string[1] = 0x9A;
	
	char fstop_string[2];
	fstop_string[0] = 0x82;
	fstop_string[1] = 0x9D;
	
	char lens_string[2];
	lens_string[0] = 0x92;
	lens_string[1] = 0x0A;
	
	char date_string[2];
	date_string[0] = 0x90;
	date_string[1] = 0x03;
	
	//Each of these variables below will store their namesake
	int width;
	int height;
	short iso_speed;
	unsigned int exposure_speed[2];
	unsigned int fstop[2];
	unsigned int focal_length[2];
	char date_taken [30];
	
	int n;
	for(n = 0; i < count2; n = n + 1)
	{
		fseek(image_file,file_index,SEEK_SET);
		fread(&tifftag,sizeof(char),12,image_file);
		TIFF_identifier[0] = tifftag.tag[0];
		TIFF_identifier[1] = tifftag.tag[1];
	
		//Checks to see if identifier matches width
		if(strncmp(width_string,TIFF_identifier,2) == 0)
		{
			//seek to integer and store it
			fseek(image_file,file_index + 8,SEEK_SET);
			fread(&width,sizeof(int),1,image_file);
		}
	
		//Checks to see if identifier matches height
		if(strncmp(height_string,TIFF_identifier,2) == 0)
		{
			//seek to integer and store it
			fseek(image_file,file_index + 8,SEEK_SET);
			fread(&height,sizeof(int),1,image_file);
		}
	
		//Checks to see if identifier matches iso speed
		if(strncmp(iso_string,TIFF_identifier,2) == 0)
		{
			//seek to short and store it
			fseek(image_file,file_index + 10,SEEK_SET);
			fread(&iso_speed,sizeof(short),1,image_file);
		}
	
		//Checks to see if identifier matches exposure speed
		if(strncmp(exposure_string,TIFF_identifier,2) == 0)
		{
			//Go to the offset in the tag
			fseek(image_file,file_index + 8,SEEK_SET);
			
			//Store the offset int in offset variable
			fread(&offset,sizeof(int),1,image_file);
			
			//Go to 12 + offset location to grab the exposure speed
			fseek(image_file,12 + offset,SEEK_SET);	
			fread(&exposure_speed[0],sizeof(unsigned int),1,image_file);
			fseek(image_file,12 + offset + 1,SEEK_SET);
			fread(&exposure_speed[1],sizeof(unsigned int),1,image_file);
		}
	
		//Checks to see if identifier matches fstop
		if(strncmp(fstop_string,TIFF_identifier,2) == 0)
		{
			//Go to the offset in the tag
			fseek(image_file,file_index + 8,SEEK_SET);
			
			//Store the offset int in offset variable
			fread(&offset,sizeof(int),1,image_file);
			
			//Go to 12 + offset location to grab the fstop speed
			fseek(image_file,12 + offset,SEEK_SET);	
			fread(&fstop[0],sizeof(unsigned int),1,image_file);
			fseek(image_file,12 + offset + 1,SEEK_SET);
			fread(&fstop[1],sizeof(unsigned int),1,image_file);
		}
	
		//Checks to see if identifier matches lens focal length
		if(strncmp(lens_string,TIFF_identifier,2) == 0)
		{
			//Go to the offset in the tag
			fseek(image_file,file_index + 8,SEEK_SET);
			
			//Store the offset int in offset variable
			fread(&offset,sizeof(int),1,image_file);
			
			//Go to 12 + offset location to grab the focal length
			fseek(image_file,12 + offset,SEEK_SET);	
			fread(&focal_length[0],sizeof(unsigned int),1,image_file);
			fseek(image_file,12 + offset + 1,SEEK_SET);
			fread(&focal_length[1],sizeof(unsigned int),1,image_file);
		}
		
		
		//This checks to see if the identifier matches date string
		if(strncmp(date_string,TIFF_identifier,2) == 0)
		{
			//Go to number of data items in tag
			fseek(image_file,file_index + 4,SEEK_SET);
			
			//The size of the string in the bytes_in_string variable
			fread(&bytes_in_string,sizeof(int),1,image_file);
		
			//Go to the offset in the tag
			fseek(image_file,file_index + 8,SEEK_SET);
			
			//Store the offset int in offset variable
			fread(&offset,sizeof(int),1,image_file);
			
			//Go to 12 + offset location to grab the date taken string
			fseek(image_file,12 + offset,SEEK_SET);
			
			//Now we read each character until we hit the NULL
			int m;
			for(m = 0; m < bytes_in_string; m = m + 1)
			{
				fread(&date_taken[m],sizeof(char),1,image_file);
				fseek(image_file,1,SEEK_CUR);
			}
		}
		
		//increment file index to next tag by adding an offset of 12 to the current position
		file_index = file_index + 12;
	}
	
	//Print out image statistics
	printf("\nManufacturer: %s",manufacturer);
	printf("\nModel: %s",camera_model);
	printf("\nExposure Time: %u / %u second",exposure_speed[0],exposure_speed[1]);
	printf("\nF-stop: %u / %u ",fstop[0],fstop[1]);
	printf("\nISO: %h",iso_speed);
	printf("\nDate Taken: %s",date_taken);
	printf("\nFocal Length: %u / %u mm",focal_length[0],focal_length[1]);
	printf("\nWidth: %i pixels",width);
	printf("\nHeight: %i pixels\n",height);
	
	
	return 0;
}
