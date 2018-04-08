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
    cout<<" TEST CASE 2 "<<endl;
    int fd = open_myfs("mytest.txt",'w');
    srand(1);
    for(int  i = 0; i < 100; i ++)
    {
        int n = rand()%1000 + 1;
        cout<<n<<" ";
        write_myfs(fd,sizeof(int),(char*)&n);
    }    
    cout<<endl;
    close_myfs(fd);
    int N;
    cout<<"  -- Enter N  -- "<<endl;
    cin>>N;
    char* buf = new char[BLOCK_SIZE];
    for(int i = 0; i < N; i ++)
    {

        string s = "mytest-"+to_string(i+1)+".txt";
        cout<<"  ---> "<<i<<" . Copying file mytest.txt as "<<s<<endl;
        char* filename = new char[30];
        strcpy(filename,s.c_str());  
        int fd1 = open_myfs(filename,'w');
//        cout<<fd1<<endl;
        if(fd1 == -1)
            error("Error in opening file");
        int fd2 = open_myfs("mytest.txt",'r');
//        cout<<fd2<<endl;
        if(fd2 == -1)
            error("Error in opening file");        
        
        int err = 1;
        bzero(buf,BLOCK_SIZE);
        while(err!=0)
        {
            err = read_myfs(fd2,BLOCK_SIZE,buf);
//            cout<<err<<endl;
            if(err == -1)
                error("Error in read");
            if(err == 0)
                break;
            int k = write_myfs(fd1,err,buf);
            if(k == -1)
                error("Error in write");
        }
//        ls_myfs();
//        showfile_myfs("mytest.txt");
        int k =close_myfs(fd1);
//        cout<<k<<endl;
        k = close_myfs(fd2);        
//        cout<<k<<endl;
//        cout<<"Hi"<<endl;
//        cout<<i<<endl;
    }

    ret = dump_myfs("mydump-22.backup");
    if(ret == -1)
        error("Error in dump");
	return 0;
}