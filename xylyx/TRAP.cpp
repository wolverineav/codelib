#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<time.h>

using namespace std;

#define DATA_SIZE 1
#define DISK_SIZE 512

class trap
	{
		public:
				time_t time_trap;
				char trap_data[DATA_SIZE];

				trap operator =(trap);
	};

trap trap::operator =(trap temp)
{
	time_trap=temp.time_trap;
	strcpy(trap_data,temp.trap_data);
}

class Data
{
 private:
		char Main_Data[DATA_SIZE];
		time_t Main_Data_Time;
		int Main_Data_Index;
		char curr_data[DATA_SIZE];
		trap trap_index[100];
		int index;

 public:
		Data()
		{
			index=0;
			Main_Data_Index=0;
		}

		void store_main_data(char [], time_t, int);
		void calculate_trap(char []);
		void store_trap(char []);

		void storage(int);
		void forward_recovery();
		void backward_recovery();

		int search_trap(char []);

		void take_main_data_backup();
//		void encryption();
};

void Data::store_main_data(char new_data[], time_t time1, int index)
{
	char s[25];

	Main_Data_Index=index;
	strcpy(Main_Data,new_data);
	Main_Data_Time=time1;
}

void Data::calculate_trap(char new_data[])
{	
	int i,j;
	bool binary_data_new[8],binary_data_old[8];
	bool binary_trap[8];

	void convert_binary(char, bool []);
	bool XOR(bool, bool);
	char convert_char(bool []);

	if(index==0)
		{
			strcpy(trap_index[index].trap_data,new_data);
			strcpy(curr_data,new_data);
		}
	else
		{
			for(i=0;i<DATA_SIZE;i++)
				{
					convert_binary(new_data[i],binary_data_new);
					convert_binary(curr_data[i],binary_data_old);

					for(j=0;j<8;j++)
						{
							binary_trap[j]=XOR(binary_data_new[j],binary_data_old[j]);
						}
					
					trap_index[index].trap_data[i]=convert_char(binary_trap);
				}
		}

	time(&trap_index[index].time_trap);
}

void Data::store_trap(char new_data[])
{
	calculate_trap(new_data);
	strcpy(curr_data,new_data);
}

int Data::search_trap(char time_reqd[])
{
	
	return -1;
}

void Data::storage(int flag)
{
	char new_data[DATA_SIZE+1];
	time_t sys_time;

	void input(char []);
	void output(trap, int);

	input(new_data);

	if(flag==0 || index==2)
		{
			time_t systime;
			char timet[25];

			time(&sys_time);
			
			store_main_data(new_data,sys_time,index);
		}
	
	store_trap(new_data);
	strcpy(curr_data,new_data);

	output(trap_index[index], index);
	index++;
}


void Data::forward_recovery()
{
	int i,j,k;
	bool binary_data_old[8], binary_data_new[8];
	bool binary_data_data[8];
	int index_new;
	char reqd_data[DATA_SIZE+1];

	void convert_binary(char, bool []);
	bool XOR(bool, bool);
	char convert_char(bool []);
	
	strcpy(reqd_data,Main_Data);
	reqd_data[DATA_SIZE]='\0';
	
	cout<<"\nEnter Index : ";
	cin>>index_new;

	if(index_new>index)
		cout<<"Enter Correct Index !\n";
	else
	 {
	
	for(i=Main_Data_Index+1;i<index_new;i++)
		{
			for(j=0;j<DATA_SIZE;j++)
				{
					convert_binary(reqd_data[j],binary_data_old);
					convert_binary(trap_index[i].trap_data[j],binary_data_new);

					for(k=0;k<8;k++)
						{
							binary_data_data[k]=XOR(binary_data_old[k],binary_data_new[k]);
						}
					
					reqd_data[j]=convert_char(binary_data_data);
				}

			reqd_data[DATA_SIZE]='\0';
		}

	cout<<endl<<reqd_data;

	}
}

void Data::backward_recovery()
{
	int i,j,k;
	bool binary_data_old[8], binary_data_new[8];
	bool binary_data_data[8];
	int index_new;
	char reqd_data[DATA_SIZE+1];

	void convert_binary(char, bool []);
	bool XOR(bool, bool);
	char convert_char(bool []);
	
	strcpy(reqd_data,Main_Data);
	reqd_data[DATA_SIZE]='\0';
	
	cout<<"\nEnter Index : ";
	cin>>index_new;

	if(index_new>Main_Data_Index)
		cout<<"Enter Correct Index !\n";
	else
	 {
	
	for(i=Main_Data_Index;i>=index_new;i--)
		{
			for(j=0;j<DATA_SIZE;j++)
				{
					convert_binary(reqd_data[j],binary_data_old);
					convert_binary(trap_index[i].trap_data[j],binary_data_new);

					for(k=0;k<8;k++)
						{
							binary_data_data[k]=XOR(binary_data_old[k],binary_data_new[k]);
						}
					reqd_data[j]=convert_char(binary_data_data);
				}

			reqd_data[DATA_SIZE]='\0';
		}

	cout<<endl<<reqd_data;

	}
}

void Data::take_main_data_backup()
{
	if(index==50)
		{
		
		}
}


void convert_binary(char c, bool b[])
{
	int n=int(c);
	int i;

	for(i=7;i>=0;i--)
        {
          b[i]=n%2;
          n/=2;
        }
}

bool XOR(bool a, bool b)
{
	if((a==0 && b==0) || (a==1 && b==1))
		return 0;
	
	return 1;
}

int get_int(bool b[])
{
	int i;
    float n=0;
    int k=7;
    
    for(i=0;i<8;i++,k--)
       {
         n+=b[k]*pow(float(2),i);
       }
    return (int(n));
}

char convert_char(bool binary_data[])
{
	int int_trap;

	int_trap=get_int(binary_data);

	return(char(int_trap));
}

void get_time(char exact_time[], char sys_time[])
{
	int i;
    for(i=0;i<8;i++)
		{
			exact_time[i]=sys_time[11+i];
		}
	
	exact_time[i]='\0';
}

void input(char new_data[])
{
	int i;
	printf("\nEnter Data : ");

	cin>>new_data;
	new_data[DATA_SIZE]='\0';
}

void output(trap t, int index)
{
	int i;

	for(i=0;i<DATA_SIZE;i++)
		cout<<int(t.trap_data[i]+'A');
		
}

int main()
{
	int choice;
	char b[9]="14:45:55";
	Data disk;

	do
	  {
		  cout<<"\nEnter\t1 ... Storage"
			  <<"\n\t2 ... Recovery"
			  <<"\n\t3 ... Stop"
			  <<"\nEnter Your Choice : ";
		  cin>>choice;

		  switch(choice)
		  {
		   case 1:{
					disk.storage(1);
					disk.search_trap(b);
					break;
				  }
		   case 2:{
					int c;
					cout<<"Enter\t1 ... Forward Recovery"
						<<endl<<"\t2 ... Backward Recovery"
						<<endl<<"Enter Your Choice : ";
					cin>>c;
					if(c==1)
						disk.forward_recovery();
					else
						if(c==2)
							disk.backward_recovery();
						else
							cout<<"\nEnter Correct Choice !";
					break;
				  }
		   default:cout<<"\nThank You";
		  }
		  
	  }while(choice!=3);
	
	return 1;
}
