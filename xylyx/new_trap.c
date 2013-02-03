/* Trap Program Includes*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<linux/types.h>
#include<asm/types.h>

/* Netlink Includes */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/genetlink.h>


/* Trap Program Macros*/

#define DATA_SIZE 512
#define DISK_SIZE 512


/* Netlink Macros */

#define GENLMSG_DATA(glh) ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))


// TRAP standards

typedef unsigned char u8;

typedef short int bool;

/*  Netlink User connection code */

int done = 0;
int nl_sd; /*the socket*/


static int create_nl_socket(int protocol, int groups)
{
        socklen_t addr_len;
        int fd;
        struct sockaddr_nl local;
        
        fd = socket(AF_NETLINK, SOCK_RAW, protocol);
        if (fd < 0){
		perror("socket");
                return -1;
        }

        memset(&local, 0, sizeof(local));
        local.nl_family = AF_NETLINK;
        local.nl_groups = groups;
        if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0)
                goto error;
        
        return fd;
 error:
        close(fd);
        return -1;
}



/*
 * Send netlink message to kernel
 */


int sendto_fd(int s, const char *buf, int bufLen)
{
        struct sockaddr_nl nladdr;
        int r;
        
        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;
        
        while ((r = sendto(s, buf, bufLen, 0, (struct sockaddr *) &nladdr,
                           sizeof(nladdr))) < bufLen) {
                if (r > 0) {
                        buf += r;
                        bufLen -= r;
                } else if (errno != EAGAIN)
                        return -1;
        }
        return 0;
}



/*
 * Probe the controller in genetlink to find the family id
 * for the CONTROL_EXMPL family
 */


int get_family_id(int sd)
{
	struct {
                struct nlmsghdr n;
                struct genlmsghdr g;
                char buf[256];
        } family_req;
        
        struct {
                struct nlmsghdr n;
                struct genlmsghdr g;
                char buf[256];
        } ans;
	
        int id;
        struct nlattr *na;
        int rep_len;

//         Get family name 
        family_req.n.nlmsg_type = GENL_ID_CTRL;
        family_req.n.nlmsg_flags = NLM_F_REQUEST;
        family_req.n.nlmsg_seq = 0;
        family_req.n.nlmsg_pid = getpid();
        family_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
        family_req.g.cmd = CTRL_CMD_GETFAMILY;
        family_req.g.version = 0x1;
        
        na = (struct nlattr *)GENLMSG_DATA(&family_req);

        na->nla_type = CTRL_ATTR_FAMILY_NAME;
        
	//------change here--------

        na->nla_len = strlen("CONTROL_EXMPL") + 1 + NLA_HDRLEN;
        strcpy(NLA_DATA(na), "CONTROL_EXMPL");
        
        family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

        if (sendto_fd(sd, (char *) &family_req, family_req.n.nlmsg_len) < 0)
		return -1;
    
	rep_len = recv(sd, &ans, sizeof(ans), 0);
        if (rep_len < 0){
		perror("recv");
		return -1;
	}

      //  Validate response message 
        if (!NLMSG_OK((&ans.n), rep_len)){
		fprintf(stderr, "invalid reply message\n");
		return -1;
	}

        if (ans.n.nlmsg_type == NLMSG_ERROR) { // error 
                fprintf(stderr, "received error\n");
                return -1;
        }

        na = (struct nlattr *)( GENLMSG_DATA(&ans));
        na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
        if (na->nla_type == CTRL_ATTR_FAMILY_ID) {
                id = *(__u16 *) NLA_DATA(na);
        }
        return id;
}


/*	End of Netlink */

/*  TRAP Program*/


void copy(u8 stg1[], u8 stg2[])
{
	int i;

	for(i=0;i<DATA_SIZE;i++)
	   {
		stg1[i]=stg2[i];
	   }
}


typedef
	struct 
		{
			time_t time_trap;
			u8 trap_data[DATA_SIZE];
		}trap;



void copy_trap(trap* src, trap* dest)
{
	int i;

	dest->time_trap=src->time_trap;
	for(i=0;i<DATA_SIZE;i++)
		{
			dest->trap_data[i]=src->trap_data[i];
		}
}


typedef
	struct block
		{
			u8 Main_Data[DATA_SIZE];
			time_t Main_Data_Time;
			int Main_Data_Index;
			u8 curr_data[DATA_SIZE];
			trap trap_index[100];
			int index;
		}block;


void store_main_data(u8 new_data[], time_t time1, int index, block* b)
{
	b->Main_Data_Index=index;
	copy(b->Main_Data,new_data);
	b->Main_Data_Time=time1;
}

void calculate_trap(u8 new_data[], block* b)
{	
	int i,j;
	bool binary_data_new[8],binary_data_old[8];
	bool binary_trap[8];

	void convert_binary(u8, bool []);
	bool XOR(bool, bool);
	u8 convert_char(bool []);

	if(b->index==0)
		{
			copy(b->trap_index[b->index].trap_data,new_data);
			copy(b->curr_data,new_data);
		}
	else
		{
			for(i=0;i<DATA_SIZE;i++)
				{
					convert_binary(new_data[i],binary_data_new);
					convert_binary(b->curr_data[i],binary_data_old);

					for(j=0;j<8;j++)
						{
							binary_trap[j]=XOR(binary_data_new[j],binary_data_old[j]);
						}
					
					b->trap_index[b->index].trap_data[i]=convert_char(binary_trap);
				}
		}

	time(&b->trap_index[b->index].time_trap);
}

void store_trap(u8 new_data[], block* b)
{
	calculate_trap(new_data,b);
	copy(b->curr_data,new_data);
}

int search_trap(time_t time_reqd, block* b)
{
	double d,d1;
	int i;

	d=difftime(time_reqd,b->Main_Data_Time);

	if(d=0)
 	  return b->Main_Data_Index;
	else
	  {
		if(d<0)
		  {
			for(i=b->Main_Data_Index-1;i>=0;i++)
			   {
				d1=difftime(time_reqd,b->trap_index[i].time_trap);
				if(d1>=0)
				   return i;
			   }
		  }
		else
		  {
			for(i=b->Main_Data_Index+1;i<b->index;i++)
			   {
				d1=difftime(time_reqd,b->trap_index[i].time_trap);
				if(d1>=0)
				   return i;
			   }
		  }
	  }	
	return -1;
}

void storage(int flag, block* b, u8 new_data[])
{

    	time_t sys_time;
 
	if(flag==0 || b->index==3)
		{
			time_t systime;
			time(&sys_time);
			
			store_main_data(new_data,sys_time,b->index,b);
		}
	
	store_trap(new_data,b);
	copy(b->curr_data,new_data);

	(b->index)++;
}


void forward_recovery(block *b)
{
	int i,j,k;
	bool binary_data_old[8], binary_data_new[8];
	bool binary_data_data[8];
	int index_new;
	u8 reqd_data[DATA_SIZE];

	void convert_binary(u8, bool []);
	bool XOR(bool, bool);
	u8 convert_char(bool []);
	void get_time(time_t *);
	
	time_t t;

	copy(reqd_data,b->Main_Data);
	reqd_data[DATA_SIZE]='\0';
	
	get_time(&t);
	index_new=search_trap(t,b);
	
	for(i=b->Main_Data_Index+1;i<=index_new;i++)
		{
			for(j=0;j<DATA_SIZE;j++)
				{
					convert_binary(reqd_data[j],binary_data_old);
					convert_binary(b->trap_index[i].trap_data[j],binary_data_new);

					for(k=0;k<8;k++)
						{
							binary_data_data[k]=XOR(binary_data_old[k],binary_data_new[k]);
						}
					
					reqd_data[j]=convert_char(binary_data_data);
				}

			reqd_data[DATA_SIZE]='\0';
		}
}

void backward_recovery(block *b)
{
	int i,j,k;
	bool binary_data_old[8], binary_data_new[8];
	bool binary_data_data[8];
	int index_new;
	u8 reqd_data[DATA_SIZE];

	time_t reqd_time;

	void convert_binary(u8, bool []);
	bool XOR(bool, bool);
	u8 convert_char(bool []);
	void get_time(time_t *);
	
	copy(reqd_data,b->Main_Data);
	reqd_data[DATA_SIZE]='\0';
	
	get_time(&reqd_time);
	index_new=search_trap(reqd_time,b);
	
	for(i=b->Main_Data_Index;i>=index_new;i--)
		{
			for(j=0;j<DATA_SIZE;j++)
				{
					convert_binary(reqd_data[j],binary_data_old);
					convert_binary(b->trap_index[i].trap_data[j],binary_data_new);

					for(k=0;k<8;k++)
						{
							binary_data_data[k]=XOR(binary_data_old[k],binary_data_new[k]);
						}
					reqd_data[j]=convert_char(binary_data_data);
				}

			reqd_data[DATA_SIZE]='\0';
		}

}

void take_main_data_backup(block *b)
{
	if(b->index==50)
		{
	
		}
}




void convert_binary(u8 c, bool b[])
{
	int n=(int)c;
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

    int pw[8]={1,2,4,8,16,32,64,128};
    
    for(i=0;i<8;i++,k--)
       {
	 n+=(b[k])*pw[i];
       }
    return ((int)n);
}

u8 convert_char(bool binary_data[])
{
	int int_trap;

	int_trap=get_int(binary_data);

	return((u8)(int_trap));
}


void input()
{
	int i;
	u8 new_data[DATA_SIZE];

	block b;

}

void output(trap t, int index)
{
	int i;

	printf("%s",ctime(&t.time_trap));

	for(i=0;i<DATA_SIZE;i++)
		printf("%d",(t.trap_data[i]+'A'));		
}

void get_time(time_t *t)
{
	time(t);
}


void init(block *b)
{

	b->Main_Data_Index=0;
	b->index=0;

}

int main()
{

	nl_sd = create_nl_socket(NETLINK_GENERIC,0);
        if(nl_sd < 0){
                printf("create failure\n");
                return 0;
        }
        int id = get_family_id(nl_sd);
	struct {
                struct nlmsghdr n;
                struct genlmsghdr g;
                char buf[256];
        } ans;

        struct {
                struct nlmsghdr n;
                struct genlmsghdr g;
                char buf[256];
        } req;
        struct nlattr *na;
      
        // Send command needed 
        req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
        req.n.nlmsg_type = id;
        req.n.nlmsg_flags = NLM_F_REQUEST;
        req.n.nlmsg_seq = 60;
        req.n.nlmsg_pid = getpid();
        req.g.cmd = 1;//DOC_EXMPL_C_ECHO;
        
        //compose message
        na = (struct nlattr *) GENLMSG_DATA(&req);
        na->nla_type = 1; //DOC_EXMPL_A_MSG
        char * message = "hello world!"; //message
        int mlength = 14;
        na->nla_len = mlength+NLA_HDRLEN; //message length
        memcpy(NLA_DATA(na), message, mlength);
        req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

        //send message
	struct sockaddr_nl nladdr;
        int r;
        
        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;
    
	r = sendto(nl_sd, (char *)&req, req.n.nlmsg_len, 0,  
			  (struct sockaddr *) &nladdr, sizeof(nladdr));
	int rep_len;

	//kernel says hello first
	rep_len = recv(nl_sd, &ans, sizeof(ans), 0);
     
	// Validate response message 
        if (ans.n.nlmsg_type == NLMSG_ERROR) { //	error 
                printf("error received NACK - leaving \n");
               	return -1;
        }
        if (rep_len < 0) {
               	printf("error receiving reply message via Netlink \n");
               	return -1;
        }
        if (!NLMSG_OK((&ans.n), rep_len)) {
               	printf("invalid reply message received via Netlink\n");
		return -1;
	}

        rep_len = GENLMSG_PAYLOAD(&ans.n);
        //parse reply message
        na = (struct nlattr *) GENLMSG_DATA(&ans);
        char * result2 = (char *)NLA_DATA(na);
        printf("kernel says: %s\n",result2);

	
	while(1)
	{
		/* recv sector no. */
		rep_len = recv(nl_sd, &ans, sizeof(ans), 0);
		//int rep_len = recv(nl_sd, &ans, sizeof(ans), 0);
		//int rep_len = recv(nl_sd, &ans, sizeof(ans), 0);
		
		// Validate response message 
		if (ans.n.nlmsg_type == NLMSG_ERROR) { //	error 
		        printf("error received NACK - leaving \n");
		       	return -1;
		}
		if (rep_len < 0) {
		       	printf("error receiving reply message via Netlink \n");
		       	return -1;
		}
		if (!NLMSG_OK((&ans.n), rep_len)) {
		       	printf("invalid reply message received via Netlink\n");
			return -1;
		}

		rep_len = GENLMSG_PAYLOAD(&ans.n);
		//parse reply message
		na = (struct nlattr *) GENLMSG_DATA(&ans);
		unsigned long *result = (unsigned long *)NLA_DATA(na);
		printf("Sect No: %lx\n",*result);

		/*recv no of sectors*/
		rep_len = recv(nl_sd, &ans, sizeof(ans), 0);
	
		// Validate response message 
		if (ans.n.nlmsg_type == NLMSG_ERROR) { //	error 
		        printf("error received NACK - leaving \n");
		       	return -1;
		}
		if (rep_len < 0) {
		       	printf("error receiving reply message via Netlink \n");
		       	return -1;
		}
		if (!NLMSG_OK((&ans.n), rep_len)) {
		       	printf("invalid reply message received via Netlink\n");
			return -1;
		}

		rep_len = GENLMSG_PAYLOAD(&ans.n);
		//parse reply message
		na = (struct nlattr *) GENLMSG_DATA(&ans);
		unsigned long *result1 = (unsigned long *)NLA_DATA(na);
		printf("No. of Sectors : %lx\n\n",*result1);

	}
        close(nl_sd);


	return 1;
}
