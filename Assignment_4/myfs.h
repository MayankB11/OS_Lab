#ifndef _myfs
#define _myfs

#include <bits/stdc++.h>
#include <time.h>
#include <math.h>
#include <bitset>
#include <sys/stat.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

using namespace std;

//  Constant definitions
#define BLOCK_SIZE 256
#define MAX_BLOCKS 1024
#define MAX_INODES 256
#define MAX_FILESIZE 30
#define MAX_POINTERS 10
#define MAX_FD 20
#define PTR_MAX 8
#define IPTR_MAX 64

// Data structure definitions

// File descripor table's individual entry
struct file_descriptor_t{
	// Stores inode number of file
	int inode_no; 
	// Stores byte offset of file. No of bytes read/written
	int byte_offset;	
	// 'r' for read, 'w' for write
	char mode; 
};

// Wrapper structure to store a block's data. (BLOCK_SIZE bytes)
struct block_t {
	// stores actual data
	char value[BLOCK_SIZE];

};

// Stores superblock's data. Data about the file system
struct superblock_t{
	// Total size of the file system.
	int tot_size;
	// Max number of inodes
	int max_inodes;
	// Actual inodes in use
	int act_inodes;
	// Max number of blocks
	int max_blocks;
	// Actual blocks in use
	int act_blocks;  
	// Bit map for inodes in use. 0 if free.
	bitset <MAX_INODES> map_inodes;
	// Bit map for blocks in use. 0 if free.
	bitset <MAX_BLOCKS> map_blocks;	
	// File descriptor table. can store 20 fds at the same time.
	file_descriptor_t fd_table[MAX_FD];
};

// Stores details of a file/directory.
struct inode_t{
	// True for directory, False for file
	bool file_type; 
	// Stores file/ directory name
	char filename[MAX_FILESIZE];
	// Stores file size/ no of files/directories in the directory.
	int file_size;
	time_t last_modified;
	time_t last_read;
	// Stores parent directory of this directory.
	int parent;
	// Permissions of a file
	mode_t mode; 
	// Direct, indirect, double indirect pointers.
	int pointers[MAX_POINTERS];
};

// List of inodes
struct inode_list_t{
	inode_t inodes[MAX_INODES];
};

// Helper structure that stores data for a file in a directory
struct directory_t{
	short ino;
	char name[30];
};


// Constant declarations
const int size_superblock = sizeof(superblock_t);
const int size_inode = sizeof(inode_t);
const int size_inode_list = sizeof(inode_list_t);

const int n_superblock = ceil((double)size_superblock/BLOCK_SIZE);
const int n_inode = ceil(size_inode/BLOCK_SIZE);
const int n_inode_list = ceil((double)size_inode_list/BLOCK_SIZE);

const int size_blocks_superblock = n_superblock*BLOCK_SIZE;
const int size_blocks_inode_list = n_inode_list*BLOCK_SIZE;
const int n_data_max = MAX_BLOCKS - n_superblock - n_inode_list;

// Declaration of myfs memory
extern char* myfs_mem;
// Current working directory's inode
extern int cwd;

// Function declarations

// Creates a file system, size in Mbytes.  Returns -1 if some error occurs.
int create_myfs(long size);

// Copy file source from pc to file dest in myfs.  Returns -1 if some error occurs.
int copy_pc2myfs(char* source, char* dest);

// Copy a file filename from myfs to pc.  Returns -1 if some error occurs.s
int copy_myfs2pc(char* source, char* dest);

// Remove file filename from directory and free its data blocks.  Returns -1 if some error occurs.
int rm_myfs(char* filename);

// Display contents of filename.  Returns -1 if some error occurs.
int showfile_myfs(char* filename);

// Function to list directory. Wrapper to ls_myfs_helper.  Returns -1 if some error occurs.
int ls_myfs();

// creates a directory named dirname and add it to current working directory.  Returns -1 if some error occurs.
int mkdir_myfs(char* dirname);

// Function to change current directory. ".." means changing to parent. Returns -1 if some error occurs.
int chdir_myfs(char* dirname);

// Remove directory called dirname from current working directory.  Returns -1 if some error occurs.
int rmdir_myfs(char* dirname);

// Open file filename in mode.Create entry in file table. Returns -1 if some error occurs. Else returns file descriptor for the file.
int open_myfs(char* filename, char mode);

// close file with file descriptor fd.  Returns -1 if some error occurs. Remove from fd table
int close_myfs(int fd);

// Read nbytes into buf, from file with file descripted fd. Returns no of bytes actually read. Returns -1 if some error occurs.
int read_myfs(int fd, int nbytes, char* buff);

// Write nbytes from buf into file with file descriptor fd.  Returns no of bytes actually written. Returns -1 if some error occurs.
int write_myfs(int fd, int nbytes, char* buff);

// Check if file with descriptor has reached its end. returns 1 if true. 0 if false. -1 if error.
int eof_myfs(int fd);

//  Dump file system to filename.  Returns -1 if some error occurs.
int dump_myfs(char* dumpfile);

// Restore file system from filename.  Returns -1 if some error occurs.
int restore_myfs(char* dumpfile);

// Prints status of the file system
int status_myfs();

// Change mode of filename in cwd.  Returns -1 if some error occurs.
int chmod_myfs(char* name, int mode);

// Helper functions

// Display errors
void error(string s);

// Finds a free block, marks it as used and updates related data structures. Returns index of free block. Returns -1 if some error occurs.
int get_free_block();

// Finds a free inode, marks it as used and updates related data structures.Returns index of free inode. Returns -1 if some error occurs.
int get_free_inode();

// Returns pointer to location of the index of the block where data from (idx)*(BLOCK_SIZE) to (idx+1)*BLOCK_SIZE is stored
// Assumes that the the indirect and double indirect pointers have been assigned blocks/
int* get_inode_ptr(inode_t* inode,int idx);

// Function to add file/directory to current directory
int add_directory(short inode, char* dest);

// helper function for ls. updats cwd_files. prints the directory contents based on flag.
int ls_myfs_helper(int flag);

// Initialize the block with -1s.
void initialize_1(int block_idx);

// Initialize directory block with -1s
void init_directory(int block_idx);

// Returns empty space in directory where new file info can be stored
directory_t* get_empty_directory(int block_idx);

// To print permissions of a file
void print_permissions(mode_t mode);

// Print file info of inode 
void print_file_info(int inode);

// Helper function Mprint file details of all the files in the directory block
void ls_block(directory_t* d,int flag);

// Helper function for ls for the indirect pointer in the directory inode, to print file details.
void ls_indirect_block(int* ptr,int flag);

// Free a block. Update superblock. Initialize with null. 
void remove_block(int block_idx);

#endif