// 15CS10030 15CS30019 Group 22 Assignment 3b
// To compile : g++ Ass3b_22.cpp -pthread -std=c++11 -o o.out
// To run 	  : ./o.out

#include <bits/stdc++.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <iostream>
#include <chrono>
#include <thread>
 
using namespace std;

#define MAX_THREADS 50
#define TERMINATED 2
#define SUSPENDED 1
#define RUNNING 0
#define ERROR cout<<err<<endl;err++;
// Shared data
// Running status
int status[MAX_THREADS];
pthread_t threads[MAX_THREADS];
pthread_t scheduler, reporter;
int cur_thread;
int prev_thread;
int N;
// Function Declarations
// User defined signals

void sig_suspend(int id);
void sig_resume(int id);


// Worker Thread function
void * worker_thread_func(void* vararg);
// Scheduler Thread function
void * scheduler_thread_func(void * vararg);
// Reporter thread function
void * reporter_thread_func(void * vararg);


int main()
{
	int err = 0;
	int temp;
	int i;
	// Read N
	cout<< "Enter N: ";
	cin>>N;
	int* ind = new int[N];
	// Create signals
	signal(SIGUSR1,sig_suspend);
	signal(SIGUSR2,sig_resume);
	// Create N threads, Suspend them.
	for(i=0;i<N;i++)
	{
		status[i] = SUSPENDED;
		ind[i]=i;
		if(temp = pthread_create(&threads[i],NULL,worker_thread_func, &ind[i]))
		{
			cout<<"Error in creating thread"<<endl;
			exit(1);
		}
		pthread_kill(threads[i],SIGUSR1);
	}
	// Create scheduler and reporter
	if(temp = pthread_create(&scheduler,NULL,scheduler_thread_func,NULL))
	{
		cout<<"Error in creating scheduler thread"<<endl;;
		exit(1);
	}
	if(temp = pthread_create(&reporter,NULL,reporter_thread_func,NULL))
	{
		cout<<"Error in creating reporter thread"<<endl;;
		exit(1);
	}
	// Wait for all threads to join
	for(i=0;i<N;i++)
		pthread_join(threads[i],NULL);
	// Wait for scheduler to end, kill all.
	pthread_join(scheduler,NULL);
	pthread_join(reporter,NULL);
	return 0;
}


// Function definitions 

// User defined signals

void sig_suspend(int id)
{
	pause();
}

void sig_resume(int id)
{
	;
}

// Worker Thread function
void * worker_thread_func(void* vararg)
{
	// Generate 1000 random integers, sort them, wait for 1-10 sec before terminating
	int i = *(int *)vararg;
	srand(i*2+3);
	int arr[1000];
	for( int j = 0; j < 1000 ; j ++ )
	{
		arr[j] = rand()%99999;
	}
	int min, pos;
	for(int j = 0; j < 1000; j++ )
	{
		pos = j;
		min = arr[j];

		for(int k = j+1; k < 1000 ; k++)
		{
			if(arr[k] < min )
			{
				min = arr[k];
				pos = k;
			}
		}
		int temp = arr[pos];
		arr[pos]=arr[j];
		arr[j]=temp;
	}
	cout.flush();
	unsigned int t = rand()%10+1;
	while(t)
	{
		t = sleep(t);	
	}
	status[i] = TERMINATED;
}

// Scheduler Thread function
void * scheduler_thread_func(void * vararg)
{
	// Round robin, 1 sec quantam
	queue < int > q;
	cur_thread = -1;
	prev_thread = -1;
	int err = 1;
	for(int i = 0 ;i < N; i ++)
		q.push(i);

	while(q.size()!=1 && !q.empty())
	{	

		cout.flush();
		cur_thread = q.front();	
		q.pop();
		pthread_kill(threads[cur_thread],SIGUSR2);
		sleep(1);
		if(status[cur_thread]!=TERMINATED)
		{
			status[cur_thread]=SUSPENDED;
			q.push(cur_thread);
			pthread_kill(threads[cur_thread],SIGUSR1);
		}
		else
		{
		}
		prev_thread=cur_thread;
	}
	cur_thread  = -1;
	sleep(1);
	if(q.size())
	{

		prev_thread = q.front();
		cur_thread = prev_thread;
		pthread_kill(threads[prev_thread],SIGUSR2);
		while(status[prev_thread]!=TERMINATED);
	}
	cur_thread=-1;
}

// Reporter thread function
void * reporter_thread_func(void * vararg)
{
	// Monitor and print relevant info
	int count=0;
	while(count<N)
	{
		if(prev_thread!=-1 && prev_thread!=cur_thread)
		{

			if(status[prev_thread] == TERMINATED)
			{

				cout<<"Worker "<<prev_thread + 1<<" terminated"<<endl;
				count++;
			}
			else
			{
				cout<<"Worker "<<prev_thread + 1<<" suspended "<<cur_thread + 1<<" resumes"<<endl;
			}
			prev_thread=cur_thread;
		}
		usleep(400000);
	}
}