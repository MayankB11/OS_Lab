#include <bits/stdc++.h>
#include <fstream>
#define LINE_NO cout<<"\t"<<lineno;
#define NEW_LINE cout<<endl;
#define RET_ZERO return 0;
#define RET_ONE  return 1;
using namespace std;

unsigned int pages[64];
int lru[64];

/*******DOESN'T USE lru ARRAY OR THE REFERENCE BITS********/
int random(deque<int> *fifo,int a,int b,int lineno)
{
	int i,j;
    j = rand()%fifo->size();
	int* arr = new int[j+1];
	for (i = 0; i <= j; i++)
	{
		arr[i] = fifo->front();
		fifo->pop_front();
	}
	int pageout = arr[j];
	for (i = j-1; i >= 0; i--)
	{
		fifo->push_front(arr[i]);
	}
	unsigned int temp = pages[pageout];
	temp = temp >> 1;
	if(temp%2 == 0)				//Condition to check if the page was modified or not.
	{
		//UNMAP
		pages[pageout] = 0;
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": IN "<<b<<" "<<temp<<endl;
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp<<endl;
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    fifo->push_back(b);
	    RET_ZERO
	}
	else
	{
		//UNMAP
		pages[pageout] = 0;
		//OUT
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": OUT "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": IN "<<b<<" "<<temp<<endl;
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp<<endl;
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    fifo->push_back(b);
	    RET_ONE
	}
}

/*******DOESN'T USE lru ARRAY OR THE REFERENCE BITS********/
int FIFO(deque<int> *fifo,int a,int b,int lineno)
{
	int pageout = fifo->front();
	fifo->pop_front();
	unsigned int temp = pages[pageout];
	temp = temp >> 1;
	if(temp%2 == 0)				//Condition to check if the page was modified or not.
	{
		//UNMAP
		pages[pageout] = 0;
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": IN "<<b<<" "<<temp<<endl;
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp<<endl;
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    fifo->push_back(b);
	    RET_ZERO
	}
	else
	{
		//UNMAP
		pages[pageout] = 0;
		//OUT
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": OUT "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": IN "<<b<<" "<<temp<<endl;
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp<<endl;
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    fifo->push_back(b);
	    RET_ONE
	}
}

/*******USES THE lru ARRAY BUT NOT THE REFERENCE BITS AND fifo ARRAY********/
int LRU(int a,int b,int lineno)
{
	int pageout,min,index,i;
    min = lineno+1;
	
	for(i=0;i<64;i++)
	{
		if(lru[i] < min && lru[i] != 0){
			min = lru[i];
			index = i;
		}
	}
	lru[index] = 0;
	pageout = index;
	unsigned int temp = pages[pageout];
	temp = temp >> 1;
	if(temp%2 == 0)				//Condition to check if the page was modified or not.
	{
		//UNMAP
		pages[pageout] = 0;
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp;
        NEW_LINE
		LINE_NO
		cout<<": IN "<<b<<" "<<temp;
        NEW_LINE
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp;
        NEW_LINE
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    lru[b] = lineno;
	    RET_ZERO
	}
	else
	{
		//UNMAP
		pages[pageout] = 0;
		//OUT
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp;
        NEW_LINE
		LINE_NO
		cout<<": OUT "<<pageout<<" "<<temp;
        NEW_LINE
		LINE_NO
		cout<<": IN "<<b<<" "<<temp;
        NEW_LINE
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp;
        NEW_LINE
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    lru[b] = lineno;
	    RET_ONE
	}
}

double getProb(){
    return ((double) rand() / (RAND_MAX));
}

int NRU(deque<int> *fifo,int a,int b,int lineno,int pagefault)
{
	int *arr,i,count,flag,size = fifo->size(),r,pageout;

    count = flag = 0;
	for (i = 0; i < size;)
	{
		if(pages[fifo->front()]%2 == 0)
		{
			pageout = fifo->front();
			fifo->pop_front();
			flag=1;
			break;
		}
        i++;
		fifo->push_back(fifo->front());
		fifo->pop_front();
	}
	if(!flag)
	{
		pageout = fifo->front();
		fifo->pop_front();
	}
	unsigned int temp = pages[pageout];
	temp = temp >> 1;
	if(temp%2 == 0)				//Condition to check if the page was modified or not.
	{
		//UNMAP
		pages[pageout] = 0;
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp;
        NEW_LINE
		LINE_NO
		cout<<": IN "<<b<<" "<<temp;
        NEW_LINE
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp;
        NEW_LINE
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    fifo->push_back(b);
	    RET_ZERO
	}
	else
	{
		//UNMAP
		pages[pageout] = 0;
		//OUT
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": OUT "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": IN "<<b<<" "<<temp<<endl;
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp<<endl;
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    fifo->push_back(b);
	    RET_ONE
	}
}

int secondchance(deque<int> *fifo,int a,int b,int lineno)
{
	int j,flag = 1;
	while(flag == 1)
	{
		j = fifo->front();
		if(pages[j]%2 != 0)
		{
			pages[j] = pages[j] >> 1;
			pages[j] = pages[j] << 1;
			fifo->pop_front();
			fifo->push_back(j);
		}
		else {
            flag = 0;
        }
	}
	int pageout = j;
	fifo->pop_front();
	unsigned int temp = pages[pageout];
	temp = temp >> 1;
	if(temp%2 == 0)				//Condition to check if the page was modified or not.
	{
		//UNMAP
		pages[pageout] = 0;
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": IN "<<b<<" "<<temp<<endl;
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp<<endl;
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    fifo->push_back(b);
	    RET_ZERO
	}
	else
	{
		//UNMAP
		pages[pageout] = 0;
		//OUT
		temp = temp >> 2;		//gives the 29-bit form no.
		LINE_NO
		cout<<": UNMAP "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": OUT "<<pageout<<" "<<temp<<endl;
		LINE_NO
		cout<<": IN "<<b<<" "<<temp<<endl;
		LINE_NO
		cout<<": MAP "<<b<<" "<<temp<<endl;
		//IN
		temp = temp << 3;		//left shift to make the first 29-bit as form no.
		if(a == 1)
	    	temp+=7;
	    else
	    	temp += 5;
	    //MAP
	    pages[b] = temp;
	    fifo->push_back(b);
	    RET_ONE
	}
}

int printMenu(){
    int choice;
    cout<<"Enter the choice of replacement algorithm:"<<endl;
	cout<<"1. FIFO"<<endl;
	cout<<"2. Random"<<endl;
	cout<<"3. LRU"<<endl;
	cout<<"4. NRU"<<endl;
	cout<<"5. Second Chance"<<endl;
	cin>>choice;
    return choice;
}

int main()
{
	long int n,lineno=0,time=0,pagefault = 0,paget=0;
	unsigned int temp;
	queue<int> forms;		//forms has the list of free forms.
	deque<int> fifo;		//fifo maintains the queue of loaded pages in order.
	int i,x,y,last_used = 0,working_set,choice;
  	double probx,proby;
  	cout<< "Enter the size of working set: ";
  	cin>>working_set;
	cout<<"Enter the number of frames: ";
	cin>>n;
	choice = printMenu();
	while(choice > 5){
		cout<<"Wrong Choice, Try Again :"<<endl;
		choice = printMenu();
	}
	for(i=0;i<n;forms.push(i),i++){

    }
		
	for(i=0;i<64;i++){
		pages[i] = 0;
		lru[i] = 0;
	}
	ofstream myfile;
  	myfile.open ("test.txt");
  	for (i = 0; i < 1000000; i++)
  	{
      probx = getProb();
      
      if(probx > 0.7)
      {
        x = 1;
      }
      else{
        x = 0;
      }

      proby = getProb();

      if(proby <= 0.97)
      {
        int r = rand()%working_set;
        y = (last_used - working_set/2 + r)%64;
        if(y < 0)
          y += 64;
        last_used = y;
      }
      else{
        y = rand()%64;
        last_used = y;
      }
  		myfile <<x<<" "<<y<<endl;
  	}
  	myfile.close();
	ifstream infile("test.txt");
	string line;

    int a,b;
	while (getline(infile, line))
	{
	    istringstream iss(line);
	    if(line[0] == '#')
	    	continue;
        if (!(iss >> a >> b)) { continue; }

	    cout<<"Instruction on line "<<++lineno<<": "<<a<<" "<<b;
        NEW_LINE
	    cout<<"Output generated:";
        NEW_LINE
	    temp = pages[b];
	    temp = temp >> 1;
        temp = temp >> 1;
	    if(temp%2 == 0)
	    {
	    	pagefault++;
	    	//Page is not in memory
	    	if(!forms.size())
	    	{
	    		//When a frame needs to be replaced.
	    		/*if(lineno%1000 == 0)
				{
					//Comment out if not using NRU algo
					for (i = 0; i < 64; i++)
					{
						pages[i] = pages[i] >> 1;
						pages[i] = pages[i] << 1;
					}
				}*/
				if(choice == 1)
				{
					if(FIFO(&fifo,a,b,lineno))
		    			{time = time + 6501;paget = paget + 2;}
		    		else
		    			{time = time + 3501;paget++;}	
				}
				else if(choice == 2){
					if(random(&fifo,a,b,lineno))
	    				{time = time + 6501;paget = paget + 2;}
	    			else
	    				{time = time + 3501;paget++;}
				}
				else if (choice == 3){
					if(LRU(a,b,lineno))
		    			{time = time + 6501;paget = paget + 2;}
		    		else
		    			{time = time + 3501;paget++;}	
				}
	    		else if(choice == 4){
	    			if(lineno%1000 == 0)
					{
						for (i = 0; i < 64; i++)
						{
							pages[i] = pages[i] >> 1;
							pages[i] = pages[i] << 1;
						}	
		    		}
		    		if(NRU(&fifo,a,b,lineno,pagefault))
		    			{time = time + 6501;paget = paget + 2;}
		    		else
		    			{time = time + 3501;paget++;}
		    	}
		    	else if(choice == 5){
		    		if(secondchance(&fifo,a,b,lineno))
		    			{time = time + 6501;paget = paget + 2;}
		    		else
		    			{time = time + 3501;paget++;}
		    	}
		    	else{
		    		cout<<"Wrong choice";
		    		RET_ZERO
		    	}
	    	}
	    	else
	    	{
	    		//When frames are available.
	    		fifo.push_back(b);
	    		int formno = forms.front();
	    		forms.pop();
	    		temp = formno;
	    		temp = temp << 1;
                temp = temp << 2;
	    		if(a == 1)
	    			temp+=7;
	    		else
	    			temp += 5;
	    		pages[b] = temp;
	    		lru[b] = lineno;
	    		time+=3000; 	//For Page-IN
	    		time+=250;		//For MAP
	    		time+=1;		//For read or write access
	    		paget++;
	    		LINE_NO
				cout<<": IN "<<b<<" "<<formno;
                NEW_LINE
	    		LINE_NO
				cout<<": MAP "<<b<<" "<<formno;
                NEW_LINE
	    	}
	    }
	    else
	    {
	    	//Page already in memory.
	    	time+=1;		//For read or write access.
	    	temp = temp>>1;
	    	temp = temp << 3;
	    	if(a==1)
	    	{
	    		temp+=7;
	    		pages[b]=temp;
	    	}
	    	else if(pages[b]%2 == 0)
	    		pages[b] += 1;
	    	lru[b] = lineno;
	    	cout<<"Page already in main memory"<<endl;
	    }
	    NEW_LINE
	}
	for (i = 0; i < 64; i++)
	{
		if(pages[i] == 0)
			continue;
		else
		{
			temp = pages[i];
			temp = temp >> 3;
			cout<<"Page "<<i<<" is at frame no.: "<<temp;
            NEW_LINE
		}
	}
	cout<<"Overall Execution time: "<<time;
    NEW_LINE
	cout<<"No. of pagefault: "<<pagefault;
    NEW_LINE
	cout<<"No. of page transfers: "<<paget;
    NEW_LINE
	RET_ZERO
}