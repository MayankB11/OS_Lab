#include "myfs.h"

// The file system
char* myfs_mem;

// Current working directory's inode
int cwd;

// Files in current working directory. mapped to inode no.
map < string,int > cwd_files;

// Semaphore to lock the functions
sem_t sem;
// Display errors
void error(string s)
{
	cout<<s<<endl;
	exit(1);
}

// Finds a free block, marks it as used and updates related data structures. Returns index of free block. Returns -1 if some error occurs.
int get_free_block()
{
	int i;
	superblock_t* sb = (superblock_t*)myfs_mem;
	for(i = 0; i < MAX_BLOCKS; i ++)
	{
		if(!sb->map_blocks[i])
		{	
			sb->act_blocks++;
			sb->map_blocks[i]=1;

			return i;
		}
	}
	return -1;
}

// Finds a free inode, marks it as used and updates related data structures.Returns index of free inode. Returns -1 if some error occurs.
int get_free_inode()
{
	int i;
	superblock_t* sb = (superblock_t*)myfs_mem;
	for(i = 0; i < MAX_INODES; i++)
	{
		if(!sb->map_inodes[i])
		{
			sb->map_inodes[i]=1;
			sb->act_inodes++;
			return i;
		}
	}
	return -1;
}

// Creates a file system, size in Mbytes.  Returns -1 if some error occurs.
int create_myfs(long size)
{
	sem_init(&sem,0,1);
	sem_wait(&sem);
	int i;
	int sz = 1<<20;
	size=size*sz;
	cout<<size<<endl;
	if(size < BLOCK_SIZE + size_blocks_superblock + size_blocks_inode_list)
	{
		sem_post(&sem);
		return -1;
		error("Size too small to create a file system");
	}
	else if(size > 256*256*256*64)
	{
		sem_post(&sem);
		return -1;
		error("Size too big");
	}

	myfs_mem = (char*) malloc(size);
	if(myfs_mem == NULL)
	{
		sem_post(&sem);
		return -1;
	}

	for(i = 0;i < size; i++)
	{
		myfs_mem[i]='\0';
	}

	// Initializing the data structure and the file system
	// A root directory will also be created

	// Initializing superblock
	superblock_t* sb = (superblock_t*)myfs_mem;

	sb->tot_size = size;
	sb->max_inodes = MAX_INODES;
	sb->act_inodes = 1; 
	sb->max_blocks = MAX_BLOCKS;
	sb->act_blocks = n_superblock + n_inode_list;


	// Initialize block bit map
	for(i = 0; i < sb->act_blocks; i++)
	{
		sb->map_blocks[i]=1;
	}
	for(i=sb->act_blocks;i<MAX_BLOCKS;i++)
	{
		sb->map_blocks[i]=0;
	}

	// Only 1 inode in inode list
	for(i = 0; i < MAX_INODES; i++)
	{
		sb->map_inodes[i]=0;
	}

	// For root directory
	sb->map_inodes[0]=1;

	for(i = 0; i < MAX_FD; i++)
		sb->fd_table[i].inode_no=-1;

	// Initiliazing inode list with root's inode

	inode_list_t * i_list = (inode_list_t*)(myfs_mem + size_blocks_superblock);

	i_list->inodes[0].file_type = true; // Directory --> true
	strcpy(i_list->inodes[0].filename,"/");
	i_list->inodes[0].file_size = 0;
	i_list->inodes[0].last_modified = time(NULL);
	i_list->inodes[0].last_read = time(NULL);
	i_list->inodes[0].mode = 0666;
	i_list->inodes[0].parent = -1;
	for(i = 0; i < 10; i++)
		i_list->inodes[0].pointers[i]=-1;

	cwd = 0; // 0 --> inode number for root directory
	sem_post(&sem);
	return 1;
}


// Returns pointer to location of the index of the block where data from (idx)*(BLOCK_SIZE) to (idx+1)*BLOCK_SIZE is stored
// Assumes that the the indirect and double indirect pointers have been assigned blocks/
int* get_inode_ptr(inode_t* inode,int idx)
{
	// 0 - 7
	if(idx < PTR_MAX)
	{

		return &inode->pointers[idx];
	}
	// 8 - (64 entries)
	idx-=PTR_MAX;
	if(idx < IPTR_MAX)
	{
		return (int*)(myfs_mem+inode->pointers[8]*BLOCK_SIZE+idx*4);
	}
	// (64*64 entries)
	idx-=IPTR_MAX;
	int d = idx / IPTR_MAX;
	int* ptr = (int*)(myfs_mem + inode->pointers[9]*BLOCK_SIZE+d*4);
	return (int*)(myfs_mem + (*ptr)*BLOCK_SIZE + (idx%IPTR_MAX)*4);

}

// Initialize the block with -1s.
void initialize_1(int block_idx)
{
	
	int* ptr = (int*)(myfs_mem + block_idx*BLOCK_SIZE);
	for(int j = 0; j < BLOCK_SIZE; j+=4,ptr++)
		*ptr = -1;
}

// Copy file source from pc to file dest in myfs.  Returns -1 if some error occurs.
int copy_pc2myfs(char* source, char* dest)
{
	// Check if dest already present in the cwd
	
	sem_wait(&sem);
	ls_myfs_helper(0);
	if(cwd_files.find(string(dest))!=cwd_files.end())
	{
		sem_post(&sem);
		return -1;
		error("File already present.");
	}
	// Superblock update
	superblock_t* sb = (superblock_t*)myfs_mem;	
		
	int i;
	FILE* fp = fopen(source,"r");
	char buf[256];

	// Create the file inode

	int inode_idx = get_free_inode();	
	if(inode_idx == -1)
	{
		sem_post(&sem);
		return -1;
		error("Memory full. Inode can't be added.");
	}
	inode_t* new_inode = &((inode_list_t * )(myfs_mem + size_blocks_superblock))->inodes[inode_idx];

	new_inode->file_type = false;
	strcpy(new_inode->filename , dest);

	fseek(fp,0,SEEK_END);	
	new_inode->file_size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	time(&new_inode->last_modified);
	time(&new_inode->last_read);

	for(int i = 0;i < 10; i++)
		new_inode->pointers[i]=-1;
	// Copy data from source to the data blocks of the inode, update the inode
	struct stat stat_temp;
	int err = stat(source,&stat_temp);
	if(err == -1)
	{
		sem_post(&sem);
		return -1;
		error("STAT ERROR");
	}
	new_inode->mode= stat_temp.st_mode;
	
	
	// If not present, find an empty space in the directory to store the file info
	int ret = add_directory(inode_idx,dest);
	;
	int ptrs = ceil((double)new_inode->file_size/BLOCK_SIZE);
	int idx = 0;
	if(ptrs>0)
		ptrs-=PTR_MAX;
	if(ptrs>0)
	{
		new_inode->pointers[8]=get_free_block();
		ptrs-=IPTR_MAX;
		initialize_1(new_inode->pointers[8]);
	}
	if(ptrs>0)
	{
		new_inode->pointers[9]=get_free_block();
		int* t_ptr = (int*)(myfs_mem+new_inode->pointers[9]*BLOCK_SIZE);
		initialize_1(new_inode->pointers[9]);
		while(ptrs>0)
		{
			*t_ptr = get_free_block();
			ptrs-=IPTR_MAX;
			initialize_1(*t_ptr);
			t_ptr++;
		}
	}
	int* ptr;
	while(!feof(fp))
	{
		bzero(buf,sizeof(buf));
		fread(buf,1,BLOCK_SIZE,fp);
		int block_idx = get_free_block();
		if(block_idx==-1)
		{
			sem_post(&sem);
			return -1;
			error("Memory full. No more Blocks.");
		}
		block_t* b = (block_t*)(myfs_mem + block_idx*BLOCK_SIZE);
		bcopy(buf,b,sizeof(buf));
		ptr = get_inode_ptr(new_inode,idx);
		*ptr = block_idx;
		idx++;

	}
	fclose(fp);
	sem_post(&sem);
	return 1;
}
// Initialize directory block with -1s
void init_directory(int block_idx)
{
	directory_t* d = (directory_t*)(myfs_mem + block_idx*BLOCK_SIZE);
	for(int j = 0; j < BLOCK_SIZE; j+=32,d++)
		d->ino=-1;
}

// Returns empty space in directory where new file info can be stored
directory_t* get_empty_directory(int block_idx)
{
	directory_t* d = (directory_t*)(myfs_mem+BLOCK_SIZE*block_idx);
	for(int j = 0; j < BLOCK_SIZE; j+=32,d++)
		if(d->ino==-1)
			return d;
	return NULL;
}

// adds inode_no, dest to current working directory.  Returns -1 if some error occurs.
int add_directory(short inode_no, char* dest)
{

	superblock_t * sb = (superblock_t*)myfs_mem;
	
	inode_t * inode = &((inode_list_t *)(myfs_mem + size_blocks_superblock))->inodes[cwd];
	time(&inode->last_modified);
	
	int i;
	int block_no;
	directory_t* d= NULL;
	inode->file_size++;

	// Traverse direct pointer
	for(i = 0; i < MAX_POINTERS - 2; i++)
	{	
		// Traverse block
		block_no = inode->pointers[i];
		if(block_no!=-1)
		{
			d = get_empty_directory(block_no);
			if(d!=NULL)
			{
				d->ino = inode_no;
				strcpy(d->name,dest);
				return 1;
			}
		}
		else
		{
			int block_idx = get_free_block();
			if(block_idx == -1)
			{
				return -1;
				error("Memory full. No more blocks");
			}
			inode->pointers[i] = block_idx;
			init_directory(block_idx);							
			d = get_empty_directory(block_idx);
			if(d!=NULL)
			{
				strcpy(d->name,dest);
				d->ino = inode_no;
			}
			return 1;
		}
	}

	// ^ Only 64 files can be added to a directory

	int i_b = inode->pointers[MAX_POINTERS-2];
	if(i_b!=-1)
	{
		int * ptr = (int *)(myfs_mem+i_b*BLOCK_SIZE);
		int j = 0;
		while(j<BLOCK_SIZE)
		{
			if(*ptr == -1)
			{
				*ptr = get_free_block();
				if(*ptr == -1)
				{
					return -1;
					error("Memory full. No more blocks");
				}
				init_directory(*ptr);
				directory_t* d = get_empty_directory(*ptr);
				if(d!=NULL)
				{
					strcpy(d->name,dest);
					d->ino = inode_no;
				}
				return 1;
			}
			else
			{
				directory_t* d = get_empty_directory(*ptr);
				if(d!=NULL)
				{
					d->ino = inode_no;
					strcpy(d->name,dest);
					return 1;
				}
			}	
			j+=4;
			ptr++;
		}
	}
	else
	{
		i_b = get_free_block();
		if(i_b==-1)
		{
			return -1;
			error("memory full. No block found");
		}
		inode->pointers[MAX_POINTERS-2]=i_b;
		
		int* ptr = (int*)(myfs_mem + BLOCK_SIZE*i_b);
		*ptr = get_free_block();
		if(*ptr==-1)
		{
			return -1;
			error("memory full. No block found");
		}
		init_directory(*ptr);
				directory_t* d = get_empty_directory(*ptr);
				if(d!=NULL)
				{
					d->ino = inode_no;
					strcpy(d->name,dest);
					//return 1;
				}
		ptr++;
		for(int j =4; j < BLOCK_SIZE; j ++,ptr++)
		{
			*ptr = -1;
		}
		return 1;

	}
/*	
	i_b = inode->pointers[MAX_POINTERS-1];
	if(i_b!=-1)
	{

	}
	else
	{

	}
	return -1;*/
	return -1;
	error("Directory full");

}
	
// To print permissions of a file
void print_permissions(mode_t mode)
{
    printf("File Permissions: \t");
    printf( (S_ISDIR(mode)) ? "d" : "-");
    printf( (mode & S_IRUSR) ? "r" : "-");
    printf( (mode & S_IWUSR) ? "w" : "-");
    printf( (mode & S_IXUSR) ? "x" : "-");
    printf( (mode & S_IRGRP) ? "r" : "-");
    printf( (mode & S_IWGRP) ? "w" : "-");
    printf( (mode & S_IXGRP) ? "x" : "-");
    printf( (mode & S_IROTH) ? "r" : "-");
    printf( (mode & S_IWOTH) ? "w" : "-");
    printf( (mode & S_IXOTH) ? "x" : "-");	
}
// Print file info of inode 
void print_file_info(int inode)
{
	inode_t* in = &((inode_list_t*)(myfs_mem + size_blocks_superblock))->inodes[inode];
	cout<<"\t"<<in->filename<<" : "<<endl;;
	print_permissions(in->mode);
	cout<<endl;
	cout<<asctime(localtime(&in->last_modified))<<""<<asctime(localtime(&in->last_read));
	cout<<in->file_size<<endl;

}
// Helper function Mprint file details of all the files in the directory block
void ls_block(directory_t* d,int flag)
{
	for(int j = 0; j < BLOCK_SIZE; j+=32,d++)
	{
		if(d->ino==-1)
			continue;
		if(flag)
			print_file_info(d->ino);
		cwd_files.insert(make_pair(string(d->name),d->ino));;
	}
}

// Helper function for ls for the indirect pointer in the directory inode, to print file details.
void ls_indirect_block(int* ptr,int flag)
{
	for(int i = 0; i < BLOCK_SIZE; i+=4,ptr++)
	{
		if(*ptr==-1)
			continue;
		directory_t* d = (directory_t*)(myfs_mem + BLOCK_SIZE*(*ptr));
		ls_block(d,flag);
	}
}

// helper function for ls. updates cwd_files, lists directory according to flag.  Returns -1 if some error occurs.
int ls_myfs_helper(int flag)
{
	// List files with details for the current working directory
	// Develop inode list
	// Print directory name -- find using inode list and cwd
	// Print total number files 
	// Traverse directory

	inode_t* inode = &((inode_list_t *)(myfs_mem + size_blocks_superblock))->inodes[cwd];
	int i;
	directory_t* d = NULL;
	int block_no;
	cwd_files.clear();
	// Traverse direct pointers
	int k =0;
	for(i = 0; i < MAX_POINTERS - 2; i++)
	{	
		// Traverse block
		block_no = inode->pointers[i];
		if(block_no == -1)
			continue;
		d = (directory_t *)(myfs_mem + BLOCK_SIZE*block_no);
		ls_block(d,flag);
	}

	int indirect_block = inode->pointers[MAX_POINTERS-2]; 
	if(indirect_block != -1)
	{
		// Traverse indirect pointer
		int* indirect_b = (int *)(myfs_mem + indirect_block*BLOCK_SIZE);
		ls_indirect_block(indirect_b,flag);
	}
	
	// Traverse double indirect pointers
	int double_indirect = inode->pointers[MAX_POINTERS-1];
	if(double_indirect != -1)
	{
		int* double_indirect_b = (int* )(myfs_mem+double_indirect*BLOCK_SIZE);
		for(int k = 0; k < BLOCK_SIZE; k += 4,double_indirect_b++)
		{
			if(*double_indirect_b == -1)
				continue;
			int * indirect_b = (int *)(myfs_mem + (*double_indirect_b)*BLOCK_SIZE);
			ls_indirect_block(indirect_b,flag);				
		}
	}
	
	return 1;
}

// Function to list directory. Wrapper to ls_myfs_helper.  Returns -1 if some error occurs.
int ls_myfs()
{

	sem_wait(&sem);
	inode_t* inode = &((inode_list_t *)(myfs_mem + size_blocks_superblock))->inodes[cwd];
	cout<<"Listing files present in directory: "<<inode->filename<<endl;;
	int ret = ls_myfs_helper(1);
	cout<<"--------------------------------------------"<<endl;
	sem_post(&sem);
	return ret;
}

// Function to change current directory. ".." means changing to parent. Returns -1 if some error occurs.
int chdir_myfs(char* dirname)
{
	sem_wait(&sem);
	ls_myfs_helper(0);

	if(strcmp(dirname,"..")==0)
	{
		if(cwd == 0)
		{
			cout<<cwd<<endl;
			sem_post(&sem);
			return -1;
		}
		inode_t* inode = &((inode_list_t*)(myfs_mem + size_blocks_superblock))->inodes[cwd];
		cwd = inode->parent;
		sem_post(&sem);
		return 1;
	}

	if(cwd_files.find(string(dirname))==cwd_files.end())
	{
		sem_post(&sem);
		return -1;
		error("Directory not present.");
	}

	cwd = cwd_files.find(string(dirname))->second;
	
	ls_myfs_helper(0);
	sem_post(&sem);
	return 1;

}
// creates a directory named dirname and add it to current working directory.  Returns -1 if some error occurs.
int mkdir_myfs(char* dirname)
{
	// Add it to cwd_files
	sem_wait(&sem);
	ls_myfs_helper(0);
	if(cwd_files.find(string(dirname))!=cwd_files.end())
	{
		sem_post(&sem);
		return -1;
		error("File/Directory already present.");
	}
	// Superblock
	superblock_t* sb = (superblock_t*)myfs_mem;	
		
	// Create the directory inode
	int inode_idx = get_free_inode();	
	if(inode_idx == -1)
	{
		sem_post(&sem);
		return -1;
		error("Memory full. Inode can't be added.");
	}
	inode_t* new_inode = &((inode_list_t * )(myfs_mem + size_blocks_superblock))->inodes[inode_idx];

	new_inode->file_type = true;
	strcpy(new_inode->filename , dirname)	;

	new_inode->file_size = 0;;
	new_inode->parent = cwd;
	time(&new_inode->last_modified);
	time(&new_inode->last_read);
	new_inode->mode = 0666;

	for(int i = 0;i < 10; i++)
		new_inode->pointers[i]=-1;
	// Add to current directory.
	int ret = add_directory(inode_idx,dirname);

	sem_post(&sem);
	return ret;

}

// Free a block. Update superblock. Initialize with null. 
void remove_block(int block_idx)
{
	superblock_t* sb = (superblock_t*)myfs_mem;
	sb->act_blocks--;
	sb->map_blocks[block_idx]=0;
	char* t = (char*)(myfs_mem + block_idx*BLOCK_SIZE);
	for(int i = 0; i < BLOCK_SIZE; i++,t++)
		*t = '\0';

}

// Remove directory called dirname from current working directory.  Returns -1 if some error occurs.
int rmdir_myfs(char* dirname)
{
	sem_wait(&sem);

	// Update superblock
	// Check if dirname present
	ls_myfs_helper(0);
	if(cwd_files.find(string(dirname))==cwd_files.end())
	{
		sem_post(&sem);
		return -1;
		error("Directory not present.");
	}
	
	int inode_no = cwd_files.find(string(dirname))->second;
	// Check if dirname empty
	inode_t* inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[inode_no];
	if(inode->file_size!=0)
	{
		sem_post(&sem);
		return -1;
		error("Directory cannot be deleted");
	}

	// Remove inode for dirname, remove blocks assigned to dirname
	int i = 0;
	for(i = 0; i < 8; i ++)
	{
		if(inode->pointers[i]!=-1)
		{
			int block_idx = inode->pointers[i];
			remove_block(block_idx);
			inode->pointers[i]=-1;

		}
	}
	if(inode->pointers[8]!=-1)
	{
		int* ptr = (int*)(myfs_mem+inode->pointers[8]*BLOCK_SIZE);
		for(int i = 0; i < BLOCK_SIZE; i+=4,ptr++)
		{
			if(*ptr != -1)
			{
				remove_block(*ptr);
				*ptr = -1;
			}
		}
		remove_block(inode->pointers[8]);
		inode->pointers[8]=-1;
	}
	
	// Remove inode's details from cwd's inode
	superblock_t* sb = (superblock_t*)myfs_mem;
	inode_t* cwd_inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[cwd];
	cwd_inode->file_size--;
	time(&cwd_inode->last_modified);
	time(&cwd_inode->last_read);
	sb->act_inodes--;
	sb->map_inodes[inode_no]=0;

	for(int i = 0; i < 8; i ++)
	{
		if(cwd_inode->pointers[i]!=-1)
		{
			directory_t* d = (directory_t*)(myfs_mem + (cwd_inode->pointers[i])*BLOCK_SIZE);
			for(int j = 0; j < BLOCK_SIZE; j++,d++)
			{
				if(d->ino == inode_no)
				{
					d->ino = -1;
				}
			}
		}
	}
	if(cwd_inode->pointers[8]!=-1)
	{
		int * ptr = (int *)(myfs_mem + cwd_inode->pointers[8]*BLOCK_SIZE);
		for(int j = 0; j < BLOCK_SIZE; j+=4, ptr++)
		{
			if(*ptr != -1)
			{
				directory_t* d = (directory_t* )(myfs_mem+(*ptr)*BLOCK_SIZE);
				 
				for(int i = 0; i < BLOCK_SIZE; j += 32, d++)
				{
					if(d->ino == inode_no)
					{
						d->ino = -1;
					}
				}
			}
		}
	}
	sem_post(&sem);
	return 1;

}

// Remove file filename from directory and free its data blocks.  Returns -1 if some error occurs.
int rm_myfs(char* filename)
{
	sem_wait(&sem);

	// search current directory to see if file is present or not
	ls_myfs_helper(0);
	if(cwd_files.find(string(filename))==cwd_files.end())
	{
		sem_post(&sem);
		return -1;
		error("File not present.");
	}
	int c = 0;
	superblock_t* sb = (superblock_t*)myfs_mem;
	
	inode_t* inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[cwd];
	inode->file_size--;
	inode_list_t* i_list = (inode_list_t*)(myfs_mem+size_blocks_superblock);
	// If file is present, get the inode, and delete entry from directory
	int inode_no;
	int i;
	for(i=0;i<MAX_POINTERS-2;i++)
	{
		if(inode->pointers[i]!=-1)
		{
			directory_t* d = (directory_t*)(myfs_mem + (inode->pointers[i])*BLOCK_SIZE);
			int j = 0;
			for(j=0;j<BLOCK_SIZE;j+=32,d++)
			{
				if(d->ino!=-1&&strcmp(d->name,filename)==0)
				{
					inode_no = d->ino;
					d->ino=-1;
				}
			}
		}
	}
	if(inode->pointers[MAX_POINTERS-2]!=-1)
	{
		int* ptr = (int*)(myfs_mem + BLOCK_SIZE*inode->pointers[MAX_POINTERS-2]);
		int j;
		for(j=0;j<BLOCK_SIZE;j++,ptr++)
		{
			if(*ptr!=-1)
			{
				directory_t*d=(directory_t*)(myfs_mem+(*ptr)*BLOCK_SIZE);
				int k = 0;
				for(;k<BLOCK_SIZE;k+=32,d++)
				{
					if(d->ino!=-1&&strcmp(d->name,filename)==0)
					{
						inode_no = d->ino;
						d->ino = -1;
					}
				}
			}	
		}
	}

	// Update superblock
	// Get the inode, delete it, remove from inode list
	inode_t* file_inode = &i_list->inodes[inode_no];
	sb->act_inodes--;
	sb->map_inodes[inode_no]=0;
	int inode_ptr = 0;
	
	// Delete data related to inode
	while(1)
	{
		if(inode_ptr<8)
		{
			if(file_inode->pointers[inode_ptr]!=-1)
			{
				int block_idx = file_inode->pointers[inode_ptr];
				remove_block(block_idx);
				file_inode->pointers[inode_ptr]=-1;
				c++;
			}
			else
				break;
			inode_ptr++;
		}
		if(inode_ptr == 8)
		{
			if(file_inode->pointers[inode_ptr]!=-1)
			{
				int block_idx=file_inode->pointers[inode_ptr];
				c++;
				int* ptr = (int*)(myfs_mem + block_idx*BLOCK_SIZE);
				for(int i = 0; i < BLOCK_SIZE; i+=4,ptr++)
				{
					if(*ptr != -1)
					{
						remove_block(*ptr);
						*ptr = -1;
						c++;
					}
				}
				remove_block(block_idx);
				file_inode->pointers[inode_ptr]=-1;
			}
			else
				break;
			inode_ptr++;
		}
		if(inode_ptr==9)
		{
			if(file_inode->pointers[inode_ptr]!=-1)
			{
				int block_idx = file_inode->pointers[inode_ptr];
				c++;
				int* d_ptr = (int*)(myfs_mem + block_idx*BLOCK_SIZE);
				for(int i = 0;i < BLOCK_SIZE; i+=4,d_ptr++)
				{
					if(*d_ptr!=-1)
					{
						int * ptr = (int *)( myfs_mem+(*d_ptr)*BLOCK_SIZE);
						for(int j = 0; j < BLOCK_SIZE; j+=4,ptr++)
						{
							if(*ptr!=-1)
							{
								remove_block(*ptr);
								*ptr = -1;
								c++;
							}
						}
						remove_block(*d_ptr);
						*d_ptr = -1;
						c++;
					}
				}
				remove_block(block_idx);
				file_inode->pointers[inode_ptr]=-1;
			}
			break;
		}
	}
	sem_post(&sem);
	return 1;


}

// Read nbytes into buf, from file with file descripted fd. Returns no of bytes actually read. Returns -1 if some error occurs.

int read_myfs(int fd, int nbytes, char* buf)
{

	sem_wait(&sem);
	// Check if fd is valid or not
	if(fd>=MAX_FD)
	{
		sem_post(&sem);
		return -1;
	}
	superblock_t* sb = (superblock_t*)myfs_mem;

	if(sb->fd_table[fd].inode_no == -1 || sb->fd_table[fd].mode!= 'r')
	{
		sem_post(&sem);

		return -1;
	}

	// Check if file pointer is at the end 
	inode_t * inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[sb->fd_table[fd].inode_no];
	int bytes_read = sb->fd_table[fd].byte_offset;
	int size = inode->file_size;
	if(bytes_read == size)
	{
		buf = NULL;
		sem_post(&sem);
		return 0;
	}
	// Read the nbytes
	int idx = 0;
	int ptrs;
	idx = bytes_read/BLOCK_SIZE;
	
	int* ptr;
	int k = 0;
	char * ans = new char[nbytes];

	while(nbytes > 0 && bytes_read < size)
	{
		ptr = get_inode_ptr(inode,idx);
		int block_idx = *ptr;
		char * temp = (char *)(myfs_mem + block_idx*BLOCK_SIZE);
		int j = 0;
		j = bytes_read%BLOCK_SIZE;
		while(nbytes > 0 && j  < BLOCK_SIZE && bytes_read < size)
		{
			buf[k] = temp[j];
			j++;
			nbytes--;
			k++;
			bytes_read ++;
		}
		idx++;
	}
	int ret = bytes_read - sb->fd_table[fd].byte_offset;
	sb->fd_table[fd].byte_offset = bytes_read;
	sem_post(&sem);
	return ret;


}

// Write nbytes from buf into file with file descriptor fd.  Returns no of bytes actually written. Returns -1 if some error occurs.
int write_myfs(int fd,int nbytes,char* buf)
{
	sem_wait(&sem);
	// Check if fd is valid or not
	if(fd>=MAX_FD)
	{
		sem_post(&sem);
		return -1;
	}
	superblock_t* sb = (superblock_t*)myfs_mem;

	if(sb->fd_table[fd].inode_no == -1 || sb->fd_table[fd].mode!= 'w')
	{
		sem_post(&sem);
		return -1;
	}

	inode_t * new_inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[sb->fd_table[fd].inode_no];
	int size = new_inode->file_size;
	int ptrs = ceil((double)(size + nbytes)/BLOCK_SIZE);
	int old_ptrs = ceil((double)size/BLOCK_SIZE);
	int idx = 0;

	if(ptrs > 0)
	{

		ptrs-= PTR_MAX;
		old_ptrs-=PTR_MAX;
	}
	if(ptrs>0)
	{
		if(old_ptrs<=0)
		{
			new_inode->pointers[8]=get_free_block();
			initialize_1(new_inode->pointers[8]);
		}
		old_ptrs-=IPTR_MAX;
		ptrs-=IPTR_MAX;
	}
	int * ptr;
	if(ptrs>0)
	{

		if(old_ptrs<=0)
		{
			new_inode->pointers[9]=get_free_block();
			initialize_1(new_inode->pointers[9]);

		}
		 ptr = (int*)(myfs_mem + new_inode->pointers[9]*BLOCK_SIZE);
		while(ptrs > 0)
		{
			if(old_ptrs <= 0)
			{
				*ptr = get_free_block();
				initialize_1(*ptr);

			}
			ptrs -= IPTR_MAX;
			old_ptrs-=IPTR_MAX;
			ptr++;
		}
	}
	idx = size/BLOCK_SIZE;
	char* t = NULL;
	int j;
	int k = 0;
	ptr = get_inode_ptr(new_inode,idx);
	j = size%BLOCK_SIZE;
	if(j!=0)
	{
		t = (char *)(myfs_mem + (*ptr)*BLOCK_SIZE);
		for(j = size%BLOCK_SIZE; j < BLOCK_SIZE && nbytes > 0; j++)
		{
			t[j]=buf[k];
			k++;
			nbytes--;
		}
		idx++;
	}	
	while(nbytes > 0)
	{
		j = 0;
		ptr = get_inode_ptr(new_inode,idx);
		*ptr = get_free_block();

		if(*ptr == -1)
		{
			sem_post(&sem);
			return -1;
		}
		t = (char*)(myfs_mem + (*ptr)*BLOCK_SIZE);
		while(nbytes > 0 && j < BLOCK_SIZE)
		{
			t[j]=buf[k];
			j++;
			nbytes--;
			k++;
		}
		idx++;

	}
	new_inode->file_size = size + k;

	sb->fd_table[fd].byte_offset = new_inode->file_size;
	sem_post(&sem);
	return k;

}

// Display contents of filename.  Returns -1 if some error occurs.
int showfile_myfs(char* filename)
{	
	sem_wait(&sem);
	ls_myfs_helper(0);
	if(cwd_files.find(string(filename))==cwd_files.end())
	{
		sem_post(&sem);
		return -1;
		error("No such file in current Directory");
	}
	int inode_no = cwd_files.find(string(filename))->second;
	inode_t* inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[inode_no];
	int idx = 0;
	int size = inode->file_size;
	int n = ceil((double)inode->file_size/BLOCK_SIZE);
	int* ptr = NULL;
	char* buf=new char[256];
	block_t* b = NULL;

	while(size>0)
	{
		ptr = get_inode_ptr(inode,idx);
		idx++;
		 b = (block_t*)(myfs_mem + (*ptr)*BLOCK_SIZE);
		 buf = (char*)b; 
		if(size<BLOCK_SIZE)
		{
			for(int i = 0;i < size; i++)
				cout<<*(buf+i);
		}
		else
			for(int i = 0;i<BLOCK_SIZE;i++)
				cout<<*(buf+i);
		size-=BLOCK_SIZE;
	}
	sem_post(&sem);
	return 1;

}
// Copy a file filename from myfs to pc.  Returns -1 if some error occurs.s
int copy_myfs2pc(char* filename, char* dest)
{
	sem_wait(&sem);
	ls_myfs_helper(0);
	if(cwd_files.find(string(filename))==cwd_files.end())
	{
		sem_post(&sem);
		return -1;
		error("No such file in current Directory");
	}
	int inode_no = cwd_files.find(string(filename))->second;
	inode_t* inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[inode_no];
	int idx = 0;
	int size = inode->file_size;
	int n = ceil((double)inode->file_size/BLOCK_SIZE);
	int* ptr = NULL;
	char* buf2=new char[256];
	block_t* b = NULL;
	ofstream fout;
	fout.open(dest,ios::out);
	while(size>0)
	{
		ptr = get_inode_ptr(inode,idx);
		idx++;
		 b = (block_t*)(myfs_mem + (*ptr)*BLOCK_SIZE);

		buf2 = (char*)b;
		if(size<BLOCK_SIZE)
		{

			for(int i = 0;i < size; i++)
				fout<<*(buf2+i);
		}
		else
			for(int i = 0;i<BLOCK_SIZE;i++)
				fout<<*(buf2+i);
		size-=BLOCK_SIZE;
	}
	fout.close();
	sem_post(&sem);
	return 1;	
}

// Open file filename in mode.Create entry in file table. Returns -1 if some error occurs. Else returns file descriptor for the file.
int open_myfs(char* filename,char mode)
{
	sem_wait(&sem);
	superblock_t* sb = (superblock_t*)myfs_mem;
	int i;
	for(i = 0 ; i < MAX_FD; i++)
	{
		if(sb->fd_table[i].inode_no == -1)
			break;
	}
	if(i == MAX_FD)
	{
		sem_post(&sem);
		return -1;
	}
	ls_myfs_helper(0);
	int inode_idx;
	if(mode == 'r')
	{
		if(cwd_files.find(string(filename))==cwd_files.end())
		{
			sem_post(&sem);
			return -1;
		}
		inode_idx = cwd_files.find(string(filename))->second;

	}
	else if(mode == 'w')
	{
		if(cwd_files.find(string(filename))!=cwd_files.end())
			rm_myfs(filename);
		inode_idx = get_free_inode();	
		if(inode_idx == -1)
		{
			sem_post(&sem);
			return -1;
			error("Memory full. Inode can't be added.");
		}
		inode_t* new_inode = &((inode_list_t * )(myfs_mem + size_blocks_superblock))->inodes[inode_idx];

		new_inode->file_type = false;
		strcpy(new_inode->filename , filename);
			
		new_inode->file_size = 0;

		
		time(&new_inode->last_modified);
		time(&new_inode->last_read);

		for(int i = 0;i < 10; i++)
			new_inode->pointers[i]=-1;
		// Copy data from source to the data blocks of the inode, update the 

		new_inode->mode= 0666;
		
		int ret = add_directory(inode_idx,filename);
		ls_myfs_helper(0);

	}
	sb->fd_table[i].inode_no = inode_idx;
	sb->fd_table[i].byte_offset = 0;
	sb->fd_table[i].mode = mode;
	sem_post(&sem);
	return i;

}

// close file with file descriptor fd.  Returns -1 if some error occurs. Remove from fd table
int close_myfs(int fd)
{
	sem_wait(&sem);
	if(fd >= MAX_FD)
	{
		sem_post(&sem);
		return -1;
	}
	superblock_t* sb = (superblock_t*)myfs_mem;
	if(sb->fd_table[fd].inode_no == -1)
	{
		sem_post(&sem);
		return -1;
	}
	sb->fd_table[fd].inode_no = -1;
	sem_post(&sem);
	return 1;

}

// Check if file with descriptor has reached its end. returns 1 if true. 0 if false. -1 if error.
int eof_myfs(int fd)
{
	sem_wait(&sem);	
	if(fd >= MAX_FD)
	{
		sem_post(&sem);
		return -1;
	}
	superblock_t* sb = (superblock_t*)myfs_mem;
	if(sb->fd_table[fd].inode_no == -1)
	{
		sem_post(&sem);
		return -1;
	}
	inode_t* inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[sb->fd_table[fd].inode_no];
	if(sb->fd_table[fd].byte_offset == inode->file_size)
	{
		sem_post(&sem);
		return 1;
	}
	sem_post(&sem);
	return 0;
}
//  Dump file system to filename.  Returns -1 if some error occurs.
int dump_myfs(char* filename)
{
	sem_wait(&sem);	
	FILE* out = fopen(filename,"wb");
	superblock_t* sb = (superblock_t*)myfs_mem;
	int sz=0;
	if(out == NULL)
	{
		sem_post(&sem);
		return -1;
	}

	while(sz < sb->tot_size)
	{
		int wr = fwrite(myfs_mem+sz,1,BLOCK_SIZE,out);
		if(wr == 0)
		{
			fclose(out);
			sem_post(&sem);
			return -1;
		}
		sz+=BLOCK_SIZE;
	}
	fclose(out);
	cwd = 0;
	sem_post(&sem);
	return 1;
}

// Restore file system from filename.  Returns -1 if some error occurs.
int restore_myfs(char* filename)
{
	sem_wait(&sem);
	FILE* in = fopen(filename,"rb");
	superblock_t* sb = (superblock_t*)myfs_mem;
	int sz=0;
	if(in == NULL)
	{
		sem_post(&sem);
		return -1;
	}

	while(sz < sb->tot_size)
	{
		int wr = fread(myfs_mem+sz,1,BLOCK_SIZE,in);
		if(wr == 0)
		{
			fclose(in);
			sem_post(&sem);
			return -1;
		}
		sz+=BLOCK_SIZE;
	}
	fclose(in);
	cwd = 0;
	sem_post(&sem);
	return 1;
}

// Prints status of the file system
int status_myfs()
{
	sem_wait(&sem);
	superblock_t* sb = (superblock_t*)myfs_mem;
	cout<<" --------------------------- FILE SYSTEM STATUS -------------------------------- "<<endl;
	cout<<"Total size: "<<sb->tot_size<<endl;
	cout<<"Total Blocks: "<<sb->max_blocks<<endl;
	cout<<"Actual Blocks: "<<sb->act_blocks<<endl;
	cout<<"Total Inodes: "<<sb->max_inodes<<endl;
	cout<<"Actual Inodes: "<<sb->act_inodes<<endl;
	cout<<" ------------------------------------------------------------------------------- "<<endl;
	sem_post(&sem);
	return 1;
}

// Change mode of filename in cwd.  Returns -1 if some error occurs.
int chmod_myfs(char* filename,int mode)
{	
	sem_wait(&sem);
	ls_myfs_helper(0);
	if(cwd_files.find(string(filename))==cwd_files.end())
	{
		sem_post(&sem);
		return -1;
	}
	int inode_no = cwd_files.find(string(filename))->second;
	inode_t* inode = &((inode_list_t*)(myfs_mem+size_blocks_superblock))->inodes[inode_no];
	inode->mode = mode;
	sem_post(&sem);
	return 1;

}
