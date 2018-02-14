#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <bits/stdc++.h>

using namespace std;
int isprime(int n)
{	
	if( n == 0 || n == 1)
		return 0;
	for(int i = 2; i <= sqrt(n); i++ )
	{
		if(n%i == 0)
			return 0;
	}
	return 1;
}
int main()
{
	struct timespec start, finish;
	double elapsed;
	int np,nc;
	cin>>np;
	cin>>nc;
	
	clock_gettime(CLOCK_MONOTONIC, &start);

/* ... */

	clock_gettime(CLOCK_MONOTONIC, &finish);
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

//	cout<<init<<endl;	
	int* pid=new int[np+nc];
	int shm_id;
	int * shm_ptr;
	int status;

	shm_id = shmget ( IPC_PRIVATE , 5 * sizeof(int) , IPC_CREAT | 0666 );
	if( shm_id < 0 )
	{
		printf("SHMGET Error\n");
		exit(1);
	}
	
	shm_ptr = (int * ) shmat( shm_id, NULL, 0 );
	if( * shm_ptr == -1 )
	{
		cout<<"SHMAT server error"<<endl;	
		exit(1);
	}

	shm_ptr[0]=-1;
	shm_ptr[1]=-1;
	shm_ptr[2]=-1;
	shm_ptr[3]=-1;
	shm_ptr[4]=-1;
	for(int i=0;i<np;i++)
	{
		int p=fork();
		pid[i]=p;
		if(p==0)
		{
			int k=1;
			while(1)
			{
				srand(k);
				k+=20-i;
				int n;
				while(1)
				{
					n = rand() % (i*500);	
					if(isprime(n))
						break;

				}
				int t = rand()%5;
		//		t=0;
				sleep(t);

				
				{
					int flag=0;
					for(int k=i%5;k<5;(k=k+1)%5)
					{
						if(shm_ptr[k]==-1)
						{
							flag=1;
							t=clock();
	//						cout<<t<<endl;	
							clock_gettime(CLOCK_MONOTONIC, &finish);
							elapsed = (finish.tv_sec - start.tv_sec);
							elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
							cout<<"time : "<<elapsed<<" seconds\t\t"<<"Producer "<<i<<" : "<<n<<"\t INDEX:"<<k<<endl;
							shm_ptr[k] = n;
							break;
						}
					}

				}

				// Producer processes
			}
			exit(0);
		}

	}

	for(int i=0;i<nc;i++)
	{
		int p=fork();
		pid[np+i]=p;
		if(p==0)
		{
			int k=1;
			while(1)
			{
				srand(k);
				// Consumer processes
				int t = rand()%5;
				k+=20-i;

				//t=0;
				srand(k-11);
				sleep(t);
				
				{
					int flag=0;
					for(int k = i%5; k < 5; (k=k+1)%5)
					{
						if(shm_ptr[k] != -1)
						{
							flag=1;
							t=clock();
	//						cout<<t<<endl;
							clock_gettime(CLOCK_MONOTONIC, &finish);
							elapsed = (finish.tv_sec - start.tv_sec);
							elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
							cout <<"time : "<<elapsed<<" seconds"<< "\t\t\t\t\t\t\tConsumer "<< i << " : "<<shm_ptr[k]<<"\t INDEX:"<<k<<endl;
							shm_ptr[k] = -1;
							break;
						}
					}
					
				}
			}
			exit(0);
//			cout << "consumer" << i << " : " << shm_ptr[i%5]<<endl;
		}
	}
	int wpid;
	sleep(20);
	for(int i=0; i < np + nc; i++ )
	{
		kill(pid[i],SIGTERM);
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	cout<<"time : "<<elapsed<<" seconds"<<endl;					
//	while ((wpid = wait(&status)) > 0);
	return 0;
}