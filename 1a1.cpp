#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;

const int SIZE_OF_ARR = 5;
int main()
{

	int temp=0,pid1,pid2, pip[2];
	char instring[20];
	int* number=new int;
	*number =0;
	int mini = 2147483647;
	int res = pipe(pip); 
	pid1 = fork();
	pid2 = fork();

	if(res==-1){
		perror("Pipe");
		exit(-1);
	}

	if (pid1 == 0 && pid2==0)           
	{
		/* read from the pipe */
		
		vector<int> v;
		for(int j=3*SIZE_OF_ARR;j>0;j--){
			read(pip[0], &temp, sizeof(temp));  
			v.push_back(temp);
		}
		sort(v.begin(),v.end());
		
		cout<<"\nSorted :";
		for(int i=0;i<v.size();i++){
			cout<<v[i]<<"\t";
		}
		cout<<endl;
	}
	else			
	{	
		int mod;
		if(pid1 == 0){
			cout<<"\nA : ";
			mod = rand() % 100;
		}
		else if(pid2 == 0){
			cout<<"\nB : ";
			mod = rand() % 150;
		}
		else{
			cout<<"\nC : ";
			mod = rand() % 200;
		}
		for(int j=0;j<SIZE_OF_ARR;j++){
			temp = rand()%mod;
			cout<<temp<<"\t";
			write(pip[1],&temp,sizeof(temp));
		}
		cout<<endl;
	}
}
