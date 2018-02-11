#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

int main()
{
	int pid1,pid2;
	int pipeA[2],pipeB[2],pipeC[2];
	
	if(pipe(pipeA)==-1)
	{
		cout<<"Pipe failed"<<endl;
		exit(1);
	}
	
	if(pipe(pipeB)==-1)
	{
		cout<<"Pipe failed"<<endl;
		exit(1);
	}
	if(pipe(pipeC)==-1)	
	{
		cout<<"Pipe failed"<<endl;
		exit(1);
	}

	pid1=fork();
	pid2=fork();
	
	if(pid1!=0&&pid2!=0)
	{
//	cout<<"Hi!"<<endl;
		// Process A
		vector<int> v;
		int temp;
		srand(pid2);
		
		for(int i=0;i<100;i++)
		{	
			temp=rand()%200;
			v.push_back(temp);
		}	
		sort(v.begin(),v.end());
		for(int i=0;i<100;i++)
			write(pipeA[1],&v[i],sizeof(v[i]));
		int t=2002;
		write(pipeA[1],&t,sizeof(t));
	}

	else if(pid1!=0&&pid2==0)
	{
		// Process B
		vector<int> v;
		int temp;
		srand(pid1);
		for(int i=0;i<100;i++)
		{	
			temp=rand()%500;
			v.push_back(temp);
		}	
		sort(v.begin(),v.end());
		for(int i=0;i<100;i++)
			write(pipeB[1],&v[i],sizeof(v[i]));
		int t=2002;
		write(pipeB[1],&t,sizeof(t));
	}
	
	else if(pid1==0&&pid2!=0)
	{
		// Process C
		vector<int> v;
		int temp;
		srand(pid1);
		for(int i=0;i<100;i++)
		{	
			temp=rand()%1000;
			v.push_back(temp);
		}	
		sort(v.begin(),v.end());
		for(int i=0;i<100;i++)
			write(pipeC[1],&v[i],sizeof(v[i]));
		int t=2002;
		write(pipeC[1],&t,sizeof(t));
	}

	else
	{
		// Process D
		int a,b,c;
		for(int i=0;i<300;i++)
		{
			if(i==0)
			{
				read(pipeA[0],&a,sizeof(a));
				read(pipeB[0],&b,sizeof(b));
				//cout<<a<<endl;
				read(pipeC[0],&c,sizeof(c));
				//	cout<<a<<endl;
				i+=3;
			}
			else if(a<=b&&a<=c)
			{
				cout<<a<<" ";
				read(pipeA[0],&a,sizeof(a));

			}
			else if(b<=c&&b<=a)
			{
				cout<<b<<" ";
				read(pipeB[0],&b,sizeof(b));
			}
			else
			{
				cout<<c<<" ";
				read(pipeC[0],&c,sizeof(c));
			}

		}
		cout<<endl;

	}

	return 0;
}