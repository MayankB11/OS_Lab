#include <bits/stdc++.h>
#include <fstream>

using namespace std;

double FCFS(int N,int arr[],int burst[])
{
	double atn;
	int f=burst[0];
	atn=f;

	for(int i=1;i<N;i++)
	{
		if(arr[i]>f)
			f=arr[i]+burst[i];
		else
			f+=burst[i];
		atn+=f-arr[i];
	}
	double n=N;
	atn=atn/n;
	cout<<"Average turnaround time for FCFS: "<<atn<<endl;
	return atn;

}
class mycomparision
{
public:
    int operator() (const pair<int , int> & p1, const pair<int, int> & p2)
    {
        return p1.second > p2.second;
    }
};
double psjf(int N,int arr[],int b[])
{
	
	int* burst = new int[N];
	int* completion = new int[N];
	for(int i=0;i<N;i++){
		cout<<arr[i]<<"\t"<<b[i]<<endl;
		burst[i]=b[i];
	}
	int cur_arr = 0;
	int prev_p = -1,count = 0,mini;
	while(1){
		mini = -1;
		for(int i=0;i<N;i++){
			if(arr[i]<=cur_arr && burst[i]!=0){
				if(mini == -1){
					mini = i;
				}
				else if(arr[i]<=cur_arr && burst[i]<burst[mini] && burst[i]!=0){
					mini=i;
				}
			}
		}
		if(mini == -1){
			int k;
			for(k=0;k<N;k++){
				if(burst[k]!=0){
					break;
				}
			}
			if(k == N){
				break;
			}
			else{
				cout<<"P None"<<endl;
			}
		}
		else{
			if(prev_p == -1){
				//cout<<"-1 "<<mini<<endl;
				prev_p = mini;
				count=0;
			}
			else if(prev_p == mini){
				count++;
			}
			else{
				cout<<"P"<<prev_p<<" :\t"<<arr[prev_p]<< "\t"<<cur_arr-count-1<<"\t"<<cur_arr<<"\t"<<count+1<<endl;
				completion[prev_p] = cur_arr;
				prev_p = mini;
				count = 0;
			}
			burst[mini]--;			
		}
		cur_arr++;
	}
	if(prev_p!=mini){
		cout<<"P"<<prev_p<<" :\t"<<arr[prev_p]<< "\t"<<cur_arr-count-1<<"\t"<<cur_arr<<"\t"<<count+1<<endl;
		completion[prev_p] = cur_arr;
	}
	float atn;
	for(int i=0;i<N;i++){
		atn+=completion[i]-arr[i];
	}
	atn/=N;
	cout<<"Average turnaround time for PSJF : "<<atn<<endl;
	return atn;
}
void printab(int arr[],int burst[],int N)

{
	ofstream fout;
	fout.open("table.txt");
	fout<<"Process  : Arrival time  Burst time"<<endl;
	for(int i=0;i<N;i++)
	{
		fout<<"P"<<i<<"\t :\t"<<arr[i]<<"\t\t"<<burst[i]<<endl;;
	}
	fout.close();
	for(int i=0;i<N;i++)
	{
		cout<<arr[i]<<" ";
	}
	cout<<endl;
	for(int i=0;i<N;i++)
	{
		cout<<burst[i]<<" ";
	}
	cout<<endl;
}

void printqueue(queue < int > q)
{

	while(!q.empty())
	{
		cout<<"P"<<q.front()<<" ";
		q.pop();
	}
	cout<<endl;
}
double rr(int N,int arr[],int b[], int l)
{
	int burst[N];
	for(int i=0;i<N;i++)
		burst[i]=b[i];
	int f=0;
	queue < int > q;
	q.push(0);
	int i=1;
	double atn=0;
	int temp[N];
	for(int i=0;i<N;i++)
		temp[i]=0;
	while(!q.empty())
	{
		printqueue(q);
		int frnt=q.front();
		int flag=0;
		q.pop();

//		cout<<"."<<endl;
		if(l<burst[frnt])
		{
			flag=1;
			f+=l;
			burst[frnt]-=l;
//			q.push(front)
		}
		else if(l>burst[frnt])
		{
			f+=burst[frnt];
			burst[frnt]=0;
			temp[frnt]+=f-arr[frnt];
			cout<<temp[frnt]<<" for i = "<<frnt<<endl;
			atn+=temp[frnt];
		}
		else
		{
			f+=l;

			burst[frnt]=0;
			temp[frnt]+=f-arr[frnt];
			cout<<temp[frnt]<<" for i = "<<frnt<<endl;	
			atn+=temp[frnt];
		}
		while(i<N&&arr[i]<=f)
		{	
			q.push(i);	
			i++;
		}

		if(flag)
		{
			q.push(frnt);
		}
		if(q.empty()&&i<N)
		{
			f=arr[i];
			q.push(i);
		}

		cout<<f<<endl;
	}

	double n=N;
	atn=atn/n;
	cout<<"Average turnaround time for RR with "<<l<<": "<<atn<<endl;
	return atn;
}
int main()
{

	int N;
	cin>>N;
	double lambda=0.01;
	//cin>>lambda;
	int* arr=new int[N];
	int* burst=new int[N];
	arr[0]=0;
	double r;
	cout<<arr[0]<<" ";
	//cout<<"Hi"<<endl;
	for(int i=1;i<N;i++)
	{
		r = ((double) rand() / (RAND_MAX));
		arr[i] = (int)fmod(((-1/lambda)*log(r)),10.0);
		cout<<arr[i]<<" ";
	}
	cout<<endl;
	for(int i=0;i<N;i++)
	{
		r = ((double) rand() / (RAND_MAX));
		burst[i] = (int)fmod(((-1/lambda)*log(r)),19.0)+1;
		cout<<burst[i]<<" ";
	}
	cout<<endl;
	for(int i=0;i<N;i++)
	{
		int min=arr[i];
		int pos=i;
		for(int j=i;j<N;j++)
		{
			if(arr[j]<min)
			{		
				min=arr[j];
				pos=j;
			}
		}
		int temp = arr[i];
		arr[i]=arr[pos];
		arr[pos]=temp;
//		cout<<arr[i]<<",";
		temp=burst[i];
		burst[i]=burst[pos];
		burst[pos]=temp;
	}
	cout<<endl;
	double a1,a2,a3,a4,a5;
	printab(arr,burst,N);
	a1=FCFS(N,arr,burst);
	a2=psjf(N,arr,burst);
	a3=rr(N,arr,burst,1);
	a4=rr(N,arr,burst,2);
	a5=rr(N,arr,burst,5);
	//cout<< " PSJF :\n";
	//psjf(N,arr,burst);
	cout<<"\n----------------------------------------------\n\n";

	cout<<"N = "<<N<<endl;
	cout<<"Process  : Arrival time  Burst time"<<endl;
	for(int i=0;i<N;i++)
	{
		cout<<"P"<<i<<"\t :\t"<<arr[i]<<"\t\t"<<burst[i]<<endl;;
	}
	cout<<"Average turnaround time for FCFS : "<<a1<<endl;	
	cout<<"Average turnaround time for PSJF : "<<a2<<endl;
	cout<<"Average turnaround time for RR 1 : "<<a3<<endl;
	cout<<"Average turnaround time for RR 2 : "<<a4<<endl;
	cout<<"Average turnaround time for RR 5 : "<<a5<<endl;

;	return 0;
}