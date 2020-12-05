
// this is our proj3 file
//#include <iostream>
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <byteswap.h>
#include <time.h>
//using namespace std;


int CWD_CLUSTNUM;
unsigned int clust_list[500];
int clust_list_size= 0;
int clust_list_max_size = 500;
char CWD_NAME[11];
#define ATTR_READ_ONLY 0x01 
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04 
#define ATTR_VOLUME_ID 0x08 
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID) 

unsigned int rootcluster[500];


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

typedef struct {
	char * file_name;
	char * mode_spec;
	unsigned int clusternum;

} openfiledata;

fileinfo f32;
openfiledata openlist[40];
int openfilecount = 0; 
//test
void info();
void lsFunc(char * dirname);
void makingADir(char * dirname);
unsigned int * clusterlist(unsigned int clustnum);

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
void CD(char * directory);
void MV(char * from, char * to);
void createfile(char * filename);
void openfile(char * filename, char * mode);
void closefile(char * filename);


////////// MAIN LOOP W PARSER FROM PART 1 /////////

int main(int argc, char *argv[] )
{

	for(int i = 0; i < 500; i++){
		rootcluster[i] = 0;
	}
	
	if(argc ==2)
	{
		f32.fileID = open(argv[1], O_RDWR);
		if(f32.fileID==-1)
		{
			printf("ERROR: Opening %s failed.\n", argv[1]);
			return 0;
		}
	}
	else
	{
		printf("ERROR: Incorrect number of arguments. Expected 1.\n");
		return 0;
	}
	// f32.fileID = open("fat32.img", O_RDWR);
	printf("file ID: %d\n", f32.fileID);
	int i;
	struct DIR_Entry aDir;
	aDir.DIR_Attr = 0x10;
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
    rootcluster[0] = f32.BPB_RootClus;
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

		if(!strcmp(tokens->items[0], "exit"))
		{
			break;
		}
		else if(!strcmp(tokens->items[0], "info"))
		{
			info();
		}
		else if(!strcmp(tokens->items[0], "size"))
		{
			FileSize(tokens->items[1]);
		}
		else if(!strcmp(tokens->items[0], "ls"))
		{
			lsFunc(tokens->items[1]);
		}
		else if(!strcmp(tokens->items[0], "cd"))
		{
			CD(tokens->items[1]);
		}
		else if(!strcmp(tokens->items[0], "mv"))
		{
			MV(tokens->items[1], tokens->items[2]);
		}
		else if(!strcmp(tokens->items[0], "creat"))
		{
			createfile(tokens->items[1]);
		}
		else if(!strcmp(tokens->items[0], "mkdir"))
		{
			makingADir(tokens->items[1]);
		}
		else if(!strcmp(tokens->items[0],"open"))
		{
			openfile(tokens->items[1], tokens->items[2]);
		}
		else if(!strcmp(tokens->items[0],"close"))
		{
			closefile(tokens->items[1]);
		}
		else if(!strcmp(tokens->items[0],"lseek"))
		{
			printf("Not implemented");
		}
		else if(!strcmp(tokens->items[0],"read"))
		{
			printf("Not implemented");
		}
		else if(!strcmp(tokens->items[0],"write"))
		{
			printf("Not implemented");
		}
		else if(!strcmp(tokens->items[0],"rm"))
		{
			printf("Not implemented");
		}
		else if(!strcmp(tokens->items[0],"cp"))
		{
			printf("Not implemented");
		}
		else if(!strcmp(tokens->items[0],"rmdir"))
		{
			printf("Not implemented");
		}
		else
		{
			printf("not a valid command, please try again.");
		}



		free(input);
		free_tokens(tokens);
	}
	close(f32.fileID);
	return 0;
}


////////////// INFO FUNCTION ////////////////


void info(){
	printf("bytes per sector: %d\n", f32.BPB_BytesPerSec);
	printf("sectors per cluster: %d\n", f32.BPB_SecPerClus);;
	printf("reseverd sector count: %d\n", f32.BPB_RsvdSecCnt);
	printf("number of FATs: %d\n", f32.BPB_NumFATs);
	printf("total sectors: %d\n", f32.BPB_TotSec32);
	printf("FATsize: %u\n", f32.BPB_FATSz32);
	printf("root cluster: %d\n", f32.BPB_RootClus );
}


/////////////// SIZE FUNCTION /////////////

 
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



/////////////// LS FUNCTION //////////////



void lsFunc(char * dirname){
	unsigned int lsclust_list[500];
	unsigned int lsclust_list_size = 0;
	unsigned int temp;
	unsigned char buffer[32];
	struct DIR_Entry temp_DIR;

	//current directory
	if(dirname == NULL){
		//working with cwd
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
			printf("%s\n", temp_DIR.DIR_Name);
		}
		return;

	}
	else if (dirname != NULL){
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
				printf("Error: invalid directory name\n");
				return;
			}
			if(temp_DIR.DIR_Name[0] == 0xE5) //empty
				continue;
			if((temp_DIR.DIR_Attr & ATTR_DIRECTORY) != ATTR_DIRECTORY){
				continue;
			}
			if((temp_DIR.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) //long file, ignore 
				continue;
			if(!strncmp(temp_DIR.DIR_Name, dirname, strlen(dirname)))
			{
				lsclust_list[0] = temp_DIR.DIR_FstClusHI << 8 | temp_DIR.DIR_FstClusLO;
				lsclust_list_size = 1;
				break;
			}
		}

		pread(f32.fileID, buffer, 4, GetFATOffset(lsclust_list[0])); // get the contents of the CWD fat cluster into buffer 			tempvar = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // endianness  into temp; 
		unsigned int tempvar = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // endianness  into temp; 
		int i = 1;
		while((tempvar < 0x0FFFFFF8 || tempvar > 0x0FFFFFFF) && tempvar != 0xFFFFFFFF) //check temp is good 
		{
			lsclust_list[i]=tempvar; //save clust num
			lsclust_list_size++;//update size 
			pread(f32.fileID, buffer, 4, GetFATOffset(lsclust_list[i])); //get next one 
			tempvar = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // transform into temp; 
			i++; //increase
		} //end building of CWD list
		int x = 0;
		unsigned int lsoffset_temp=GetDataOffset(lsclust_list[x]);
		//need to add second larger loop for the other clusters in list 
		while(lsoffset_temp <  GetDataOffset(lsclust_list[0]+1) )
		{
			//compare to filename parm
			pread(f32.fileID, &temp_DIR, 32, lsoffset_temp);
			lsoffset_temp += 32;
			x++;
			if (temp_DIR.DIR_Name[0] == 0x00) //last entry
			{
				printf("Error: invalid directory name\n");
				return;
			}
			if(temp_DIR.DIR_Name[0] == 0xE5) //empty
				continue;
			if((temp_DIR.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) //long file, ignore 
				continue;

			printf("%s\n", temp_DIR.DIR_Name);
		}
	}

}


//************* MKDIR FUNCTION ****************//


void makingADir(char * dirname){
	unsigned char dotbuf[512] = {	 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                                 0x20, 0x10, 0x00, 0x64, 0x04, 0x8E, 0x78, 0x4E, 0x78, 0x4E,
                                 0x00, 0x00, 0x04, 0x8E, 0x78, 0x4E, 0xB3, 0x01, 0x00, 0x00,
                                 0x00, 0x00, 0x2E, 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                                 0x20, 0x20, 0x20, 0x10, 0x00, 0x64, 0x04, 0x8E, 0x78, 0x4E,
                                 0x78, 0x4E, 0x00, 0x00, 0x04, 0x8E, 0x78, 0x4E, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00};
	char buff[4];
	unsigned int currentdir;
	unsigned int count = 0;
	char buffer[32];
	for(int i = 0; i < 32; i++){
		buffer[i] = 0xFF;
	}
	lseek(f32.fileID, 0x4004, SEEK_SET);
	while(1){
		read(f32.fileID, buff, 4);
		unsigned int temp = (unsigned int)buff[3] << 24 | buff[2] << 16 | buff[1] << 8 | buff[0];
		count += 1;
		if(temp == 0x0000){
			break;
		}
	}
	lseek(f32.fileID, -4, SEEK_CUR);
	buff[0] = 0xF8;
	buff[1] = 0xFF;
	buff[2] = 0xFF;
	buff[3] = 0x0F;
	write(f32.fileID, buff, 4);
	int j=0;
	unsigned int offset_temp=GetDataOffset(clust_list[j]);
	struct DIR_Entry ourDIR;
	struct DIR_Entry newDir;
 
	for(int i = 0; i < 11; i++){
		newDir.DIR_Name[i] = dirname[i];
	}
	newDir.DIR_Attr = 0x10;
	newDir.DIR_FstClusHI = (count >> 16);
	newDir.DIR_FstClusLO = (count & 0xFFFF);
	//need to add second larger loop for the other clusters in list
	printf("NewDirHigh: %x\n", newDir.DIR_FstClusHI);
	printf("NewDirLo: %x\n", newDir.DIR_FstClusLO);
	while(offset_temp <  GetDataOffset(clust_list[0]+1) )
	{
		if(offset_temp == GetDataOffset(f32.BPB_RootClus)){
			dotbuf[20] = (count >> 16) & 0xFF;
			dotbuf[21] = (count >> 16) >> 8;
			dotbuf[26] = (count & 0xFFFF) & 0xFF;
			dotbuf[27] = (count & 0xFFFF) >> 8;
		}
		printf("Offset: %x\n", offset_temp);
		//compare to filename parm
		pread(f32.fileID, &ourDIR, 32, offset_temp);
		printf("Name: %s\n", ourDIR.DIR_Name);
		j++;
		if(ourDIR.DIR_Name[0] == '.' && ourDIR.DIR_Name[1] != '.'){
			currentdir = ((ourDIR.DIR_FstClusHI << 16) + ourDIR.DIR_FstClusLO);
			dotbuf[20] = (count >> 16) & 0xFF;
			dotbuf[21] = (count >> 16) >> 8;
			dotbuf[26] = (count & 0xFFFF) & 0xFF;
			dotbuf[27] = (count & 0xFFFF) >> 8;

			dotbuf[20+32] = (currentdir >> 16) & 0xFF;
			dotbuf[21+32] = (currentdir >> 16) >> 8;
			dotbuf[26+32] = (currentdir & 0xFFFF) & 0xFF;
			dotbuf[27+32] = (currentdir & 0xFFFF) >> 8;
		}
		if (ourDIR.DIR_Name[0] == 0x00) //last entry
		{
			pwrite(f32.fileID, buffer, 32, offset_temp);
			pwrite(f32.fileID, &newDir, 32, offset_temp+32);
			pwrite(f32.fileID, dotbuf, 64, GetDataOffset(count));
			for(int i = 0; i < 512; i++)
				dotbuf[i] = 0;
			pwrite(f32.fileID, dotbuf, 512-64, GetDataOffset(count+64));
			return;
		}

		if(ourDIR.DIR_Name[0] == 0xE5){ //empty
			//here too
			pwrite(f32.fileID, buffer, 32, offset_temp);
			pwrite(f32.fileID, &newDir, 32, offset_temp+32);
			pwrite(f32.fileID, dotbuf, 64, GetDataOffset(count));
			for(int i = 0; i < 512; i++)
				dotbuf[i] = 0;
			pwrite(f32.fileID, dotbuf, 512-64, GetDataOffset(count+64));
			return;
		}
		offset_temp += 32;


	}
	return;

}

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


//************ GET FAT REGION OFFSET ************//


unsigned int GetFATOffset(int N)
{
	return (FirstFATSector* f32.BPB_BytesPerSec+ N * 4);
}


//************ GET DATA REGION OFFSET ****************//


unsigned int GetDataOffset(int N) //returns sector offset in bytes 
{

	return (FirstDataSector + (N -2) * f32.BPB_SecPerClus)* f32.BPB_BytesPerSec;
}


//************ CD FUNCTION *************//


void CD(char * directory)
{
	if(directory == NULL)
	{
		printf("Error: empty directory parameter\n");
		return;
	}

	unsigned char buffer[32];
	struct DIR_Entry temp_DIR;
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
			printf("Error: invalid directory name\n");
			return;
		}
		if(temp_DIR.DIR_Name[0] == 0xE5) //empty
			continue;
		if((temp_DIR.DIR_Attr & ATTR_DIRECTORY) != ATTR_DIRECTORY){
			continue;
		}
		if((temp_DIR.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) //long file, ignore 
			continue;
		if(!strncmp(temp_DIR.DIR_Name, directory, strlen(directory)))
		{

			CWD_CLUSTNUM = temp_DIR.DIR_FstClusHI << 8 | temp_DIR.DIR_FstClusLO;
			if(!strcmp(directory, "..") && CWD_CLUSTNUM == 0x00)
			{
				CWD_CLUSTNUM = f32.BPB_RootClus;
			}
			clust_list[0] = CWD_CLUSTNUM;
			clust_list_size = 1;
			break;
		}
	}

	pread(f32.fileID, buffer, 4, GetFATOffset(CWD_CLUSTNUM)); // get the contents of the CWD fat cluster into buffer 
	unsigned int temp = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // endianness  into temp; 

	
	int i =1;
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

	strcpy(CWD_NAME, temp_DIR.DIR_Name);
}


//****************** MV FUNCTION *******************//


void MV(char * from, char * to)
{
	unsigned int lsclust_list[500];
	unsigned int lsclust_list_size = 0;
	unsigned int temp;
	unsigned char buffer[32];
	struct DIR_Entry temp_DIR;

	if(from == NULL)
	{
		printf("No file given\n");
	}
	else
	{
		//if to is file and from is file print ""The name is already being used by another file"
		//if to is file and from is directory, print "Cannot move directory: invalid destination argument"
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
				printf("File not found\n");
				break;
			}
			if(temp_DIR.DIR_Name[0] == 0xE5) //empty
				continue;
			if((temp_DIR.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) //long file, ignore 
				continue;

			//printf("%s\n", temp_DIR.DIR_Name);
			if(!strcmp(temp_DIR.DIR_Name, from))
			{
				printf("File found!\n");


				if(to == NULL)
				{
					//wite to file to change name 

				}
				else
				{
				}
			}

		}
		
		
		
	}

}


//****************** OPEN FUNCTION ******************//


void openfile(char * filename, char * mode)
{
	unsigned int lsclust_list[500];
	unsigned int lsclust_list_size = 0;
	unsigned int temp;
	unsigned char buffer[32];
	struct DIR_Entry temp_DIR;

	//working with cwd
	int j=0;
	unsigned int offset_temp=GetDataOffset(clust_list[j]);
	//need to add second larger loop for the other clusters in list 
	if(strcmp(mode,"r") && strcmp(mode,"w") && strcmp(mode,"rw") && strcmp(mode,"wr"))
	{
		printf("Invalid mode\n");
		return;
	}
	while(offset_temp <  GetDataOffset(clust_list[0]+1) )
	{
		//compare to filename parm
		pread(f32.fileID, &temp_DIR, 32, offset_temp);
		offset_temp += 32;
		j++;
		if (temp_DIR.DIR_Name[0] == 0x00) //last entry
		{
			printf("File not found");
			break;
		}
		if(temp_DIR.DIR_Name[0] == 0xE5) //empty
			continue;
		if((temp_DIR.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) //long file, ignore 
			continue;

		//printf("%s\n", temp_DIR.DIR_Name);
		if(!strncmp(temp_DIR.DIR_Name, filename, strlen(filename)))
		{
			//printf("found\n");
			if(temp_DIR.DIR_Attr == 0x10)
			{
				printf("That is a Directory\n");
				return;
			}
			int i;
			for(i = 0; i < openfilecount; i++)
			{
				if(openlist[i].file_name == filename)
				{
					printf("File Already Open\n");
					return;
				}
			}
			openlist[openfilecount].file_name = filename;
			openlist[openfilecount].mode_spec = mode;
			openlist[openfilecount].clusternum = temp_DIR.DIR_FstClusHI << 8 | temp_DIR.DIR_FstClusLO;
			//need to write to file still *********************

			return;
		}
	}


}


//***************** CLOSE FUNCTION ****************//


void closefile(char * filename)
{

	int i;
	for(i = 0; i <= openfilecount; i++)
	{
		if(openlist[i].file_name == filename)
		{
			openfilecount--;
			return;
		}
	}
	printf("File is not open\n");
}


//***************** CREATE FUNCTION *******************//


void createfile(char * filename)
{
	//loop through fat and find a spot with 0x00 in it
	unsigned char buffer[32];
	struct DIR_Entry temp_DIR ;
	struct DIR_Entry new_DIR ;
	unsigned int temp;
	unsigned int tempoffset = 2;
	while(tempoffset < FirstDataSector) 
	{
		pread(f32.fileID, buffer, 4, GetFATOffset(++tempoffset));
		temp = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0];
		if( temp == 0x00)
		{
			//its empty lets claim it 
			unsigned int EOC = 0x0FFFFFF8;
			pwrite(f32.fileID, &EOC, 4, GetFATOffset(tempoffset));
			break;
		}
	}

	//go to data cluster and WIPE IT CLEAN 
	unsigned int dataoffset= GetDataOffset(tempoffset);
	pwrite(f32.fileID, 0x00, 512, dataoffset); // set it all to 0;
	unsigned short clustnum =GetFATOffset(tempoffset);

	strcpy(new_DIR.DIR_Name, filename); //name
	new_DIR.DIR_Attr = 0x02;
	new_DIR.DIR_NTRes = 0x00;
	new_DIR.DIR_CrtTimeTenth = 0x00;
	new_DIR.DIR_CrtTime = 0x00;
	new_DIR.DIR_CrtDate =0x00;
	new_DIR.DIR_LstAccDate =0x00;
	new_DIR.DIR_FstClusHI = clustnum >>8;
	new_DIR.DIR_WrtTime =0x00;
	new_DIR.DIR_WrtDate =0x00;
	new_DIR.DIR_FstClusLO = clustnum & 0x00FF;
	new_DIR.DIR_FileSize =0x00;

	//add to CWD 
	int j=0;
	unsigned int offset_temp=GetDataOffset(clust_list[j]);
	unsigned char zeros[32] = {0};
	int count =0;
	// unsigned char test[4] = {0xFF};
	while(offset_temp <  GetDataOffset(clust_list[0]+1) )
	{
		
		//compare to filename parm
		pread(f32.fileID, &temp_DIR, 32, offset_temp);
		offset_temp += 32;
		j++;
		if (temp_DIR.DIR_Name[0] == 0x00) //last entry
		{
			// printf("here!\n");
			printf("%X\n",offset_temp );
			count = pwrite(f32.fileID, &new_DIR, 32, offset_temp-32);
			// printf("%s\n",temp_DIR.DIR_Name );
			pwrite(f32.fileID, zeros, 32, offset_temp);
			return;
		}
		if(temp_DIR.DIR_Name[0] == 0xE5) //empty
		{
			printf("%X\n",offset_temp );
			count = pwrite(f32.fileID, &new_DIR, 32, offset_temp-32);
			return;
		}

		
		if((temp_DIR.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) //long file, ignore 
		{
			printf("%X\n",offset_temp );
			count = pwrite(f32.fileID, &new_DIR, 32, offset_temp-32);
			return;
		}

	}
	printf("Error: Out of space\n");
}


//************ BUILD LIST OF CWD CLUSTER FUNCTION **************//


unsigned int * clusterlist(unsigned int clustnum){
	static unsigned int cluster_list[500];
	char buffer[32];
	pread(f32.fileID, buffer, 4, GetFATOffset(CWD_CLUSTNUM)); // get the contents of the CWD fat cluster into buffer 
	unsigned int tempnum = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // endianness  into temp; 


	int i =1;
	cluster_list[0]=CWD_CLUSTNUM; 
	int cluster_list_size = 1;
	while((tempnum < 0x0FFFFFF8 || tempnum > 0x0FFFFFFF) && tempnum != 0xFFFFFFFF) //check temp is good 
	{
		cluster_list[i]=tempnum; //save clust num
		cluster_list_size++;//update size 
		pread(f32.fileID, buffer, 4, GetFATOffset(cluster_list[i])); //get next one 
		tempnum = (unsigned int)buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0]; // transform into temp; 
		i++; //incease 
	} //end building of CWD list 
	return cluster_list;

}
