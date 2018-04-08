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
    cout<<" TEST CASE 1 "<<endl;
    cout<<"  -- Copying files  -- "<<endl;
    cout<<endl;
    for(int i = 0; i < 12; i ++)
    {
        string s = "test"+to_string(i)+".cpp";
        cout<<"  ---> "<<i<<" . Copying file myfs.h as "<<s<<endl;
        char* filename = new char[30];
        strcpy(filename,s.c_str());  
        ret = copy_pc2myfs("myfs.h",filename);
        if(ret == -1)
            error("Error in copying files");        
    }
    cout<<endl<<endl;
    cout<<"  -- Calling ls  -- "<<endl;
    ret = ls_myfs();
    if(ret == -1)
        error("Error in ls");
    cout<<endl<<endl;
    cout<<"  -- Enter filename to be deleted --"<<endl;
    char* filename = new char[30];
    cin>>filename;
    ret = rm_myfs(filename);
    if(ret == -1)
        error("Error in deletion");
    cout<<endl;
    cout<<"  -- File has been deleted. calling ls -- "<<endl;
    cout<<endl;
    ret = ls_myfs();
    if(ret == -1)
        error("Error in ls");
    cout<<endl;
    cout<<"  -- Enter filename to be deleted --"<<endl;
    cin>>filename;
    cout<<endl;
    ret = rm_myfs(filename);
    if(ret == -1)
        error("Error in deletion");
    cout<<"  -- File has been deleted. calling ls -- "<<endl;
    cout<<endl;
    ret = ls_myfs();
    if(ret == -1)
        error("Error in ls"); 
    cout<<endl;


    return 0;
}

