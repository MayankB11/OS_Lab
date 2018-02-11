#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<stdio.h>
#include <fcntl.h>

#include <sys/stat.h>
using namespace std;

void displayMenu()
{
	cout<<"-----------------------------Shell Menu---------------------------------------"<<endl;
	cout<<" A. Run an internal command."<<endl;
	cout<<" B. Run an external command."<<endl;
	cout<<" C. Run an external command by redirecting standard input from a file."<<endl;
	cout<<" D. Run an external command by redirecting standard output to a file."<<endl;
	cout<<" E. Run an external command in the background."<<endl;
	cout<<" F. Run several external commands in the pipe mode."<<endl;
	cout<<" G. Quit."<<endl;
	cout<<"------------------------------------------------------------------------------"<<endl;
	cout<<endl;
	cout<<"Enter your choice(A-G)/(a-g)."<<endl;
	return;
}

int main()
{
	do
	{
		displayMenu();
		char choice='b';	
		cin>>choice;
		char** tokens;
		int pid,ret,status;
		if(choice == 'a' || choice == 'A' || choice == 'b' || choice == 'B' || choice == 'c' || choice == 'C' || choice == 'd' || choice == 'D' || choice == 'e' || choice == 'E' || choice == 'f' || choice == 'F' )
		{
			vector<char*> v;
			v.clear();
			int i=-1;
			int count;
			//cout<<"Hi"<<endl;
			while(1)													// Store input command in vector of char strings
			{
				char* str=new char[30];
				cin>>str;
				v.push_back(str);
				i++;
				if(cin.peek()=='\n')
					break;
			} 															
			
			count=i+1;
			tokens=new char*[count+1]; 							// Store command in char ** argc
			for(i=0;i<count;i++)
			{
//				cout<<v[i]<<endl;
				tokens[i]=v[i];
			}
			tokens[count]=NULL;	
//			cout<<tokens[0]<<endl;			
		}


		if(choice == 'a' || choice == 'A')
		{
//			cout<<tokens[0]<<endl;
			if(strcmp(tokens[0],"cd")==0)
			{
				if(tokens[1]==NULL)
					cout<<"Error"<<endl;
				chdir(tokens[1]);
			}
			else if(strcmp(tokens[0],"mkdir")==0)
			{
				if(tokens[1]==NULL)
					cout<<"Error"<<endl;
				mkdir(tokens[1],0777);
			}	
			else if(strcmp(tokens[0],"rmdir")==0)
			{
				if(tokens[1]==NULL)
					cout<<"Error"<<endl;
				rmdir(tokens[1]);
			}					
		}


		else if(choice == 'b' || choice == 'B')
		{
			pid=fork();
			if(pid==0)
			{
				if(execvp(tokens[0],tokens)<0)
				{
					cout<<"Command not found"<<endl;
					exit(0);
				}

			}
			//int status,ret;
			if(ret = waitpid(pid,&status,0)==-1)
				printf("Command not Found!\n");

		}
		else if(choice == 'c' || choice == 'C')
		{
			int i=0;
			while(tokens[i])
			{
				if(strcmp(tokens[i],"<")==0)
					break;
				i++;
			}	
			if(tokens[i]==NULL)
			{
				cout<<"Wrong choice"<<endl;	
				continue;
			}
			tokens[i]=NULL;
			if(tokens[i+1]==NULL)
			{
				cout<<"File error"<<endl;
				continue;
			}
			int file_desc=open(tokens[i+1],O_RDONLY);
			close(0);
			dup(file_desc);
			pid=fork();
			if(pid==0)
			{
				if(execvp(tokens[0],tokens)<0)
				{
					cout<<"Command not found"<<endl;
					exit(0);
					exit(0);
				}				
			}
			if(ret = waitpid(pid,&status,0)==-1)
				printf("Command not Found!\n");
			close(0);	



		}
		else if(choice == 'd' || choice == 'D')
		{
			int i=0;
			if(!fork())
			{
				while(tokens[i])
				{
					if(strcmp(tokens[i],">")==0)
						break;
					i++;
				}	
				if(tokens[i]==NULL)
				{
					cout<<"Wrong choice"<<endl;	
					exit(1);
				}
				tokens[i]=NULL;
				if(tokens[i+1]==NULL)
				{
					cout<<"File error"<<endl;
					exit(1);
				}
				int file_desc=open(tokens[i+1],O_WRONLY | O_CREAT,0666);
				close(1);
				dup(file_desc);
;

					
				if(execvp(tokens[0],tokens)<0)
				{
						cout<<"Command not found"<<endl;
						exit(1);
				//		exit(1);
				}		
				close(1);
				exit(0);		
			}
			int ret,status;
			//continue;
			if(ret = waitpid(pid,&status,0)==-1)
				;			
			
		}
		else if(choice == 'e' || choice == 'E')
		{
			pid=fork();
			if(pid==0)
			{
				if(execvp(tokens[0],tokens)<0)
				{
					cout<<"Command not found"<<endl;
					exit(0);
				}

			}
			//int status,ret;			
		}
		else if(choice == 'f' || choice == 'F')
		{
			int i=0,flag=0;
			int c=0,j=1,prev_i=0;
			while(tokens[i]!=NULL){
				//cout<<"Here3"<<endl	;
				if(strcmp(tokens[i],"|")==0){
					c++;
				}
				i++;
			}
			char*** temp_tokens = new char**[c+1];
			int** pipes=new int*[c];
			for(int j=0;j<c;j++){
				pipes[j]=new int[2];
				pipe(pipes[j]);
			}
			c=0;
			//cout<<"Here2"<<endl	;
			i=0;
			while(1){
				if(tokens[i]==NULL||strcmp(tokens[i],"|")==0){
					c++;
					int temp_c = i-prev_i+1;
					temp_tokens[c-1]=new char*[temp_c];
					temp_tokens[c-1][temp_c-1]=NULL;
					for(int j=0;prev_i<i;prev_i++,j++){
						temp_tokens[c-1][j]=tokens[prev_i];
					}
					prev_i=i+1;
				}
				if(tokens[i]==NULL){
					break;
				}
				i++;
			}
			//cout<<c<<endl;
			int f=0;
			for(int j=0;j<c;j++){

				int pid=fork();
				if(pid==0){
					//cout<<"Here"<<endl;
					if(j!=0){
						dup2(pipes[j-1][0],0);
					}
					if(j!=c-1){
						dup2(pipes[j][1],1);
					}
						execvp(temp_tokens[j][0],temp_tokens[j]);
						exit(0);
				}
				else{
					waitpid(pid,NULL,0);
					if(j!=0){
						close(pipes[j-1][0]);
					}
					if(j!=c-1){
						close(pipes[j][1]);
					}
				}
				if(j>=1){

					f=pipes[j-1][0];
				}				
			}
		}
		else if(choice == 'g' || choice == 'G')
		{
			break;
		}
		else
		{	
			cout<<"Wrong choice!!"<<endl;
			continue;
		}
			
	}while(1);
	return 0;
}