#ifndef MYFILESYSTEM_H
#define MYFILESYSTEM_H
#include <sys/types.h>
#include <stdint.h>

/* 
Function update_hash_file updates the hash data file from the given index.
It takes three arguments int index (which block of the file in)
size_t length (the length of the file) and helper where all the file are stored.
It will modify the file do not return value
*/

void update_hash_file(size_t start, size_t length, void * helper);
 
/*
Function init_fs stores all the information in the struct.
It takes four arguments, f1, f2 and f3 are names of data file, directory file
and has data file. n_processors indicated the number of thread should be created
It will modify the struct do not return value
*/

void * init_fs(char * f1, char * f2, char * f3, int n_processors);

/* 
Function close_fs will free the struct created in helper
*/

void close_fs(void * helper);

/* 
Function created_file will write the file name and length in directory
creat a space in data file 
It takes three arguments the filename should be created and its length. 
The helper is the pointer points to three file names and n processors
the function will return 1 if the filename already exists. 
Return 2 if there is insufficient space in the virtual disk overall.


*/

int create_file(char * filename, size_t length, void * helper);

/* 
Function resize_file will resize the size of given file
It takes three arguments the filename should be resize and its length. 
The helper is the pointer points to three file names and n processors
the function will return 1 if the file does not exist. 
Return 2 if there is insufficient space in the virtual disk overall for the new file size.


*/

int resize_file(char * filename, size_t length, void * helper);

/* 
Function repack will repack the file data file
It takes one arguments helper which is the pointer points to three file names and n processors
the function will modify the file data file

*/

void repack(void * helper);

/*
Function delete_file will delete a file with the given filename.
Return 0 if the file is successfully deleted 
and 1 if an error occurs, such as the file not existing.
*/  

int delete_file(char * filename, void * helper);

/*
Function rename_file rename a file with filename oldname to newname. 
Return 0 if the file is successfully renamed 
and 1 if an error occurs, such as the file not existing.
*/

int rename_file(char * oldname, char * newname, void * helper);

/*
Function read_file will read count bytes from the file with given filename at offset 
from the start of the file. 
Store them into buf. 
Return 0 if successfully completed. 
If the file does not exist, return 1. 
If the provided offset makes it impossible to read count bytes given the file size, 
return 2.

*/

int read_file(char * filename, size_t offset, size_t count, void * buf, void * helper);

/*
Function write_file will write count bytes to the file with given filename, 
at the given offset from the start of the file, reading data from buf. 
If the current filesize is too small, it will repack the file data file.
Return 0 if successfully completed. 
If the file does not exist, return 1. 
If offset is greater than the current size of the file, return 2. 
If there is insufficient space in the virtual disk overall to fit the data to write, return 3.
*/

int write_file(char * filename, size_t offset, size_t count, void * buf, void * helper);

/*
Function file_size will return the file size of the file with given filename. 
If there is an error, such as the file not existing, it will return -1.
*/

ssize_t file_size(char * filename, void * helper);

/*
Function fletcher compute the fletcher has of give buff.
It will take three arguments the buf points to the content need to be hash.
Length the size of the content. Output data will be stored in output after it hashed.
*/

void fletcher(uint8_t * buf, size_t length, uint8_t * output);

/*
Function compute_hash_tree will compute the entire Merkle hash tree for file_data 
and store it in hash data file.
It will modify the hash data file.

*/
void compute_hash_tree(void * helper);

/*
Function compute_hash_block will compute the hash for the block at given block_offset
in file_data, and update all affected hashes in the Merkle hash tree, 
writing changes to hash_data.
It will modify the hash data file.
*/

void compute_hash_block(size_t block_offset, void * helper);

#endif

/* 
The struct file will store the name of file data file as filename, 
name of directiry file as directory and hash data file as hash.
It will also reocrd the number of rocessors as core
*/

typedef struct {
    char filename[BUFSIZ];
    char directory[BUFSIZ];
    char hash[BUFSIZ];
    int     core;
} file;


/* 
The struct file_content will store the name of file as filename, 
offset of file as file_offset and length of file as file_length.
It will also reocrd the content of the file as file_data and the offset of 
the file in the directory file as position
*/

typedef struct {
    char filename[64];
    size_t file_offset;
    size_t file_length;
    char file_data[BUFSIZ];
    int position;
} file_content;

