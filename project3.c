
// this is our proj3 file
//#include <iostream>
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <byteswap.h>
//using namespace std;

/*void exit(){
	//safely exit program
	//free up resources
}*/

// main code for this project

// while(command != "exit"){
// 	print("$ ");
// 	read_from_stdin(command);
// 	if(command == "info"){
// 		//do stuff
// 	}
// 	else if(command == "ls"){
// 		//do stuff
// 	}
// 	else if(){

// 	}
// }

int CWD_CLUSTNUM;
unsigned int clust_list[500];
int clust_list_size= 0;
int clust_list_max_size = 500;
char * CWD_NAME[12];
#define ATTR_READ_ONLY 0x01 
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04 
#define ATTR_VOLUME_ID 0x08 
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID) 

typedef struct {
	int size;
	char **items;
} tokenlist;

typedef struct {
	unsigned int BPB_BytesPerSec;
	unsigned int BPB_SecPerClus;
 	unsigned int BPB_RsvdSecCnt;
	unsigned int BPB_NumFATs;
	unsigned int BPB_FATSz32;
	unsigned int BPB_RootClus;
	unsigned int BPB_TotSec32;
	int fileID;

} fileinfo;

struct DIR_Entry {
	unsigned char DIR_Name[11];
	unsigned char DIR_Attr; //1 byte
 	unsigned char DIR_NTRes;
	unsigned char DIR_CrtTimeTenth;
	unsigned short DIR_CrtTime; //2 byte
	unsigned short DIR_CrtDate;
	unsigned short DIR_LstAccDate;
	unsigned short DIR_FstClusHI;
	unsigned short DIR_WrtTime;
	unsigned short DIR_WrtDate;
	unsigned short DIR_FstClusLO;
	unsigned int DIR_FileSize; //4 byte 

}  __attribute__((packed));

fileinfo f32;

//test
void info();
void lsFunc(char * dirname);

char *get_input(void);
tokenlist *get_tokens(char *input);
unsigned int FirstFATSector;
unsigned int FirstDataSector;
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
int flipit(int origional);
void FileSize(char * filename);
unsigned int GetFATOffset(int N);
unsigned int GetDataOffset(int N);

int main()
{	

	f32.fileID = open("fat32.img", O_RDWR); 
	printf("file ID: %d\n", f32.fileID);
	int i; 

	unsigned char buffer[32];

	i = pread(f32.fileID, buffer, 2, 11); //i = number of bytes read 
	unsigned int temp = (unsigned int)buffer[1] << 8 | buffer[0];
    f32.BPB_BytesPerSec = temp;// saves 

	
	i = pread(f32.fileID, buffer, 1, 13); //i = number of bytes read 
	temp = (unsigned int)buffer[0];
	f32.BPB_SecPerClus = temp;


    i = pread(f32.fileID, buffer, 2, 14); //i = number of bytes read 
	temp = (unsigned int)buffer[1] << 8 | buffer[0];
 	f32.BPB_RsvdSecCnt = temp;

	i = pread(f32.fileID, buffer, 1, 16);
	temp = (unsigned int)buffer[0];
	f32.BPB_NumFATs = temp;
    
    i = pread(f32.fileID, buffer, 4, 32); //i = number of bytes read 
	temp = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0];
    f32.BPB_TotSec32= temp;

 	i = pread(f32.fileID, buffer, 4, 36);
	temp = (unsigned int)buffer[3] << 24 | (unsigned int)buffer[2] << 16 | (unsigned int)buffer[1] << 8 | (unsigned int)buffer[0];
	f32.BPB_FATSz32 = temp;
  
    i = pread(f32.fileID, buffer, 4, 44); //i = number of bytes read 
    temp = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0];
    f32.BPB_RootClus = temp;
    CWD_CLUSTNUM = f32.BPB_RootClus;

    FirstFATSector= f32.BPB_RsvdSecCnt;
    FirstDataSector=f32.BPB_RsvdSecCnt+ (f32.BPB_NumFATs* f32.BPB_FATSz32);

    //build CWD list 
	pread(f32.fileID, buffer, 4, GetFATOffset(CWD_CLUSTNUM)); // get the contents of the CWD fat cluster into buffer 
	temp = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // endianness  into temp; 

	
	i =1;
	clust_list[0]=CWD_CLUSTNUM; 
	clust_list_size = 1;
	while((temp < 0x0FFFFFF8 || temp > 0x0FFFFFFF) && temp != 0xFFFFFFFF) //check temp is good 
	{
		clust_list[i]=temp; //save clust num
		clust_list_size++;//update size 
		pread(f32.fileID, buffer, 4, GetFATOffset(clust_list[i])); //get next one 
		temp = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // transform into temp; 
		i++; //incease 
	} //end building of CWD list 

	while (1) {
		printf("$ ");

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);

		tokenlist *tokens = get_tokens(input);
		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		if(!strcmp(tokens->items[0], "exit")){

			break;
		}
		else if(!strcmp(tokens->items[0], "info")){

			info();
		}
		else if(!strcmp(tokens->items[0], "size")){
			FileSize(tokens->items[1]);
		}
		else if(!strcmp(tokens->items[0], "ls")){
			lsFunc(tokens->items[1]);
		}
		else if(!strcmp(tokens->items[0], "cd")){

		}
		else{
			printf("not a valid command, please try again.");}
	


		free(input);
		free_tokens(tokens);
	}
	close(f32.fileID);
	return 0;
}

void info(){
	printf("bytes per sector: %d\n", f32.BPB_BytesPerSec);
	printf("sectors per cluster: %d\n", f32.BPB_SecPerClus);;
	printf("reseverd sector count: %d\n", f32.BPB_RsvdSecCnt);
	printf("number of FATs: %d\n", f32.BPB_NumFATs);
	printf("total sectors: %d\n", f32.BPB_TotSec32);
	printf("FATsize: %u\n", f32.BPB_FATSz32);
	printf("root cluster: %d\n", f32.BPB_RootClus );
}

int flipit(int origional); 
 
void FileSize(char * filename){
	unsigned char buffer[32];
	struct DIR_Entry temp_DIR;
	//print error if filename not in cwd
	if(filename == NULL)
	{
		printf("Error no file given\n");
		return;
	}

// loop through CWD 32 bytes each directory content entry start at root dir

	//go to first data cluster of CWD
	int j=0;
	unsigned int offset_temp=GetDataOffset(clust_list[j]);
	//need to add second larger loop for the other clusters in list 
	while(offset_temp <  GetDataOffset(clust_list[0]+1) )
	{
		
		//compare to filename parm 
		pread(f32.fileID, &temp_DIR, 32, offset_temp);
		offset_temp += 32;
		j++;
		if (temp_DIR.DIR_Name[0] == 0x00) //last entry 
		{
			break;

		}
		if(temp_DIR.DIR_Name[0] == 0xE5) //empty 
			continue;
		if((temp_DIR.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) //long file, ignore 
			continue;
		
		if(!strncmp(temp_DIR.DIR_Name, filename, strlen(filename)))
		{
			printf("Size of %s is: %u\n", filename, temp_DIR.DIR_FileSize);
			return;
		}
	}
	printf("Error: File not found.\n");
	return;
	//we didnt find it :/
	//print error and return VOID 



}


void lsFunc(char * dirname){

	unsigned int temp;
	unsigned char buffer[32];
	struct DIR_Entry temp_DIR;

	//current directory
	if(dirname == NULL){
		//working with cwd
		//pread(f32.fileID, buffer, 4, GetFATOffset(CWD_CLUSTNUM)); // get the contents of the CWD fat cluster into buffer 
		//temp = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // endianness  into temp; 
		int j=0;
		unsigned int offset_temp=GetDataOffset(clust_list[j]);
		//need to add second larger loop for the other clusters in list 
		while(offset_temp <  GetDataOffset(clust_list[0]+1) )
		{
			//compare to filename parm
			pread(f32.fileID, &temp_DIR, 32, offset_temp);
			offset_temp += 32;
			j++;
			if (temp_DIR.DIR_Name[0] == 0x00) //last entry
			{
				break;

			}
			if(temp_DIR.DIR_Name[0] == 0xE5) //empty
				continue;
			if((temp_DIR.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) //long file, ignore 
				continue;
			if(!strncmp(temp_DIR.DIR_Name, temp_DIR.DIR_Name, strlen(temp_DIR.DIR_Name)))
			{
				printf("%s\n", temp_DIR.DIR_Name);
				return;
			}
		}
		printf("Error\n");
		return;

	}

	printf("Dirname not equal to null\n");


}


// DIR_entry cd(char * dirname){
// 	//return 0 if error otherwise change cwd
// 	if (dirname == NULL){
// 		printf("Error: no matching directory");
// 	}

// 	for(int i = 0; i<11;i++){
		
// 	}
	
// }

tokenlist *new_tokenlist(void)
{
	tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **) malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
	int i = tokens->size;

	tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *) malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

char *get_input(void)
{
	char *buffer = NULL;
	int bufsize = 0;

	char line[5];
	while (fgets(line, 5, stdin) != NULL) {
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;

		buffer = (char *) realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;

		if (newln != NULL)
			break;
	}

	buffer = (char *) realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;

	return buffer;
}

tokenlist *get_tokens(char *input)
{
	char *buf = (char *) malloc(strlen(input) + 1);
	strcpy(buf, input);

	tokenlist *tokens = new_tokenlist();

	char *tok = strtok(buf, " ");
	while (tok != NULL) {
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}

	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens)
{
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);

	free(tokens);
}

/////////read function/////////
//ssize_t read(int fildes, void *buf, size_t nbyte, off_t offset){

unsigned int GetFATOffset(int N)
{
	return (FirstFATSector* f32.BPB_BytesPerSec+ N * 4);
}

unsigned int GetDataOffset(int N) //returns sector offset in bytes 
{

	return (FirstDataSector + (N -2) * f32.BPB_SecPerClus)* f32.BPB_BytesPerSec;
}

