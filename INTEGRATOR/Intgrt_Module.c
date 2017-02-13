#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<time.h>
#include<fcntl.h>
#include<netinet/in.h>

#define SIZE 188*7


struct thread_global{

	char global_buf[SIZE];
	int size_var;
	int flag;	
};

struct ts_data{
	char addr_buf[32];
	int port_no;
	int soid;
	double bit_rate;
};

struct ts_send{
	char addr_buf_1[32];
	int port_no_1;
	int soid_1;
	double bit_rate_1;
};

struct thread_global info;
struct ts_data tdata;
struct ts_send tsend;

pthread_t tid_1, tid_2;
pthread_mutex_t m_1,m_2;



void *recv_data(void *p)
{
	int sock_id;
	sock_id = socket(AF_INET,SOCK_DGRAM, 0);
	if ( sock_id < 0 ){
		perror("socket");
		return;
	}
	struct sockaddr_in addr;
	char local_buf[SIZE];
	int len , clen;
	clen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(tdata.addr_buf);
	addr.sin_port = htons(tdata.port_no);
	
	struct ip_mreq mgroup;
	memset((char *) &mgroup, 0, sizeof(mgroup));
	mgroup.imr_multiaddr.s_addr = inet_addr(tdata.addr_buf);
	mgroup.imr_interface.s_addr = INADDR_ANY;
	printf("recv ip = %s;recv port = %d || send ip=%s;send port = %d \n",tdata.addr_buf,tdata.port_no,tsend.addr_buf_1,tsend.port_no_1);
	/////////////////
	int i;
	int pack_cnt = 0;
	int  reuse = 1;
	if (setsockopt(sock_id, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
		perror("setsockopt() SO_REUSEADDR: error ");
	}

	if (bind(sock_id, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind(): error");
		close(sock_id);
		return 0;
	}

	if (setsockopt(sock_id, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mgroup, sizeof(mgroup)) < 0) {
		perror("setsockopt() IPPROTO_IP: error ");
		close(sock_id);
		return 0;
	}


	while(1) {
		len = recvfrom(sock_id, local_buf, SIZE , 0, (struct sockaddr *) &addr,&clen);
//		printf("%d",len);
		if (len < 0) {
			perror("recvfrom(): error ");
			return ;
		} 
		else {	
			pthread_mutex_lock(&m_1);

			info.size_var = len;

			for ( i = 0 ; i < SIZE ; i+=188){

				if ( local_buf[i] == 0x47)
					pack_cnt++;
				else 
					fprintf(stderr,"PACK IS INVALID: pkt- %d\n",pack_cnt);
			}
			//if (pack_cnt == 7){
				printf("pack_cnt is %d\n",pack_cnt);				
				//pack_cnt = 0;
				memcpy(info.global_buf,local_buf,len);
				memset(local_buf, 0 ,len);
			



					pthread_mutex_unlock(&m_2);

		}
	}
}

void *send_data(void *p)
{
	int sock_id_1;
	sock_id_1 = socket(AF_INET,SOCK_DGRAM, 0);
	if ( sock_id_1 < 0 ){
		perror("socket");
		return;
	}
	struct sockaddr_in addr;
	char copy_buf[SIZE];
	int len;
	int clen;
	int sent;
	clen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(tsend.addr_buf_1);
	addr.sin_port = htons(tsend.port_no_1);

	while(1){
			pthread_mutex_lock(&m_2);
	//	printf("%s",info.global_buf);
		memcpy(copy_buf, info.global_buf, info.size_var);

		sent = sendto(sock_id_1, copy_buf, info.size_var, 0 ,(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
		if(sent < 0){
			perror("sendto");
			return;
		}
		else
			memset(copy_buf, 0 ,sent);

			pthread_mutex_unlock(&m_1);

	}
}



int main(int argc , char **argv)
{

	if(argc != 5){
		fprintf(stderr,"%s  <IP_ADDR_RECV> <PORT_NO_RECV> <IP_ADDR_SEND> <PORT_NO_SEND > <BIT_RATE>",argv[0]);
		printf("\n");
		return;
	}



	strncpy(tdata.addr_buf,argv[1],strlen(argv[1]));
	tdata.port_no = atoi(argv[2]); 

	strncpy(tsend.addr_buf_1,argv[3],strlen(argv[1]));
	tsend.port_no_1 = atoi(argv[4]); 

	pthread_create(&tid_1,NULL,recv_data,NULL);
	pthread_create(&tid_1,NULL,send_data,NULL);

	pthread_join(tid_1,NULL);
	pthread_join(tid_2,NULL);

	pthread_mutex_init(&m_1,NULL);
	pthread_mutex_init(&m_2,NULL);





} 
