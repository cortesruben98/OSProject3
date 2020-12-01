// this is our proj3 file
//#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int CWDFAT_OFFSET;
char * CWD_NAME[12];



typedef struct {
	int size;
	char **items;
} tokenlist;

typedef struct {
	int BPB_BytsPerSec;
	int BPB_SecPerClus;
 	int BPB_RsvdSecCnt;
	int BPB_NumFATs;
	int BPB_FATSz32;
	int BPB_RootClus;
	int BPB_TotSec32;
	FILE * a;

} fileinfo

void info();

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

int main()
{
	f32.a = fopen("fat32.img", "rb"); //idk if rb is right
	int i;
	fileinfo pt[4];

    fseek(in, 0x1BE, SEEK_SET); // go to partition table start
    fread(pt, sizeof(fileinfo), 4, in); // read all four entries

	struct fileinfo f32;
	f32.fileID = open("fat32.img", O_RDWR);
	int i;
	buffer = malloc(32);
	i = read(f32.fileID, buffer, 2, 11); //i = number of bytes read
	//flip it
    f32.BPB_BytesPerSector = buffer;

	i = read(f32.fileID, buffer, 1, 13); //i = number of bytes read
	//flip it
    f32.BPB_SecPerClus = buffer;

    i = read(f32.fileID, buffer, 2, 14); //i = number of bytes read
	//flip it
    f32.BPB_RsvdSecCnt = buffer;

    i = read(f32.fileID, buffer, 4, 32); //i = number of bytes read
	//flip it
    f32.BPB_TotSec32= buffer;

    i = read(f32.fileID, buffer, 4, 44); //i = number of bytes read
	//flip it
    f32.BPB_RootClus = buffer;



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
			if(tokens->items[0] == "exit"){
				break;
			}
			else if(tokens->items[0] == "info"){
				info(f32);
			}
		}



		free(input);
		free_tokens(tokens);
	}

	return 0;
}

void info(){
	printf("bytes per sector: %d\n", f32.BPB_BytsPerSec);
	printf("sectors per cluster: %d\n", f32.BPB_SecPerClus);;
	printf("reseverd sector count: %d\n", f32.BPB_RsvdSecCnt);
	printf("number of FATs: %d\n", f32.BPB_NumFATs);
	printf("total sectors: %d\n", f32.BPB_TotSec32);
	printf("FATsize: %d\n", f32.BPB_FATSz32);
	printf("root cluster: %d\n", f32.BPB_RootClus );
}

void FileSize(char * filename){
	//print error if filename not in cwd

// loop through CWD
	//if filename matches 
		//if it is a file 
			//print the size in bytes
		//else 
			//print " this isnt a file fool"
	//file not found 

	//print in bytes
}


void lsFunc(unsigned short cluster, char * dirname){
	int i, j, k;
	char dirname[12];
	DIR * cdir;	//current directory

	unsigned short Sector_offset = (cluster*4);
	unsigned short next_cluster;

	/*first sector we are working with would be where data sector starts + (cluster we are at - 2)
	* multiplied by Sectors per Cluster
	*/
	//unsigned short firstSector =

	//first we read the sector in each cluster
	for(i = 0; i < SectorPerCluster; i++){
		//and then every dir name in each sector
		for(j = 0; j < BytesPerSector/32; j++){

		}
	}
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

