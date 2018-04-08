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
    cout<<" $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ "<<endl;
    cout<<" TEST CASE 3 "<<endl;
    ret = restore_myfs("mydump-22.backup");	
    if(ret == -1)
        error("Error in restore");
    ret = ls_myfs();
    if(ret == -1)
        error("Error in ls");

    int* ptr = 0;
    char* buf = new char[sizeof(int)];
    int err = 1;
    vector <int> v;
    int fd = open_myfs("mytest.txt",'r');
    while(1)
    {
        err = read_myfs(fd,sizeof(int),buf);
        if(err == 0)
            break;
        if(err == -1)
            error("Error in read");
        ptr = (int *)buf;
        v.push_back(*ptr);
    }
    sort(v.begin(),v.end());
    fd = open_myfs("sorted.txt",'w');
    for(int i = 0; i < v.size(); i ++)
    {
        buf = (char*)&v[i];
        int ret = write_myfs(fd,sizeof(int),buf);
        if(ret == -1)
            error("Error in write");
        //cout<<v[i]<<" ";
    }
    close_myfs(fd);
//    showfile_myfs("sorted.txt");
    cout<<endl;
    fd = open_myfs("sorted.txt",'r');
    for(int i = 0; i < v.size(); i ++)
    {
        bzero(buf,sizeof(int));
        
        int ret = read_myfs(fd,sizeof(int),buf);
        if(ret == -1)
            error("Error in write");
        cout<<*(int *)buf<<" ";
    }
    close_myfs(fd);    
    cout<<endl;
 	return 0;
}
