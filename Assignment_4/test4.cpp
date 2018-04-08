#include "myfs.h"
int main()
{
    int sz = 1<<20;
    sz=10;
    int ret = create_myfs(sz);
    cout<<" --  Creating MYFS  -- "<<endl; 
    if(ret == -1)
        error("Error in creating myfs");
    cout<<endl;
    ls_myfs();

    char* filename = new char[30];

    strcpy(filename,"mydocs");
    ret = mkdir_myfs(filename);
    if(ret == -1)
    	error("Error in mkdir");
    ls_myfs();

    strcpy(filename,"mydocs");
    ret = chdir_myfs(filename);
    if(ret == -1)
    	error("Error in chdir");    
//    cout<<"Hi"<<endl;
    ls_myfs();

    strcpy(filename,"mytext");
    ret = mkdir_myfs(filename);
    if(ret == -1)
    	error("Error in mkdir");
    strcpy(filename,"mypapers");
    ret = mkdir_myfs(filename);
    if(ret == -1)
    	error("Error in mkdir");
    ls_myfs();
    strcpy(filename,"..");
    ret = chdir_myfs(filename);
    if(ret == -1)
    	error("Error in chdir");
    ls_myfs();
    strcpy(filename,"mycode");
    ret = mkdir_myfs(filename);
    if(ret == -1)
    	error("Error in mkdir");
    ls_myfs();
    int pid = fork();
    if(pid == 0)
    {
		int fd = open_myfs("temp.txt",'w');
		if(fd == -1) 
			error("Error in open");
		char * t = new char[26];
		for(int i = 0; i < 26; i ++)
		{
			t[i]='A'+i;
		}				
		ret = write_myfs(fd,26,t);
		if(ret == -1)
			error("Error in write");
		//ret = showfile_myfs("temp.txt");
//		ret = ls_myfs();
		if(ret == -1)
			error("error in ls");											    		
    }
    else
    {
    	strcpy(filename,"mycode");
    	ret = chdir_myfs(filename);
    	if(ret == -1)
			error("error in chdir");	
    	ret = copy_pc2myfs("myfs.h","myfs.cpp");
    	if(ret == -1)
			error("error in copy");
//		ret = ls_myfs();
		if(ret == -1)
			error("error in ls");
		//ret = showfile_myfs("myfs.cpp");
		if(ret == -1)
			error("error in showfile");				

    }
    return 0;
}