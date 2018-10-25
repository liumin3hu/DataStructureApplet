
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>

#define PORT 1800
#define MAXLOG 20


struct MSG1
{
	int cmd;     
	char name[20];
	char passwd[20];
};

struct MSG2
{
	int cmd;     
	char name[20];
	char data[1024];
};

struct node
{
	int fd;
	char name[20];
	struct node *next;
};
struct node *List;

pthread_mutex_t lock;



void addList(int fd,struct MSG1 * buf)
{
	struct node *p = (struct node*)malloc(sizeof(struct node));
	p->fd = fd;
	strcpy(p->name,buf->name);
	pthread_mutex_lock(&lock);
	p->next = List->next;
	List->next = p;
	pthread_mutex_unlock(&lock);
}

void delList(int fd)
{
	pthread_mutex_lock(&lock);
	struct node *p=List->next;
	struct node *q=List->next;
	while(p!=NULL)
	{
		if(p->fd==fd)
		{
			q->next = p->next;
			free(p);
			return;
		}
		else
		{
			q = p;
			p=p->next;		
		}
	}
	pthread_mutex_unlock(&lock);
}

int findbyname(char *name)
{
	int fd=-1;
	pthread_mutex_lock(&lock);
	struct node *p=List->next;
	while(p!=NULL)
	{
		if(strcmp(p->name,name)==0)
		{
			fd=p->fd;
			break;
		}
		else
		{
			p=p->next;		
		}
	}
	pthread_mutex_unlock(&lock);
	return fd;
}

int ListSize()
{
	int size=0;
	pthread_mutex_lock(&lock);
	struct node *p = List->next;
	while(p!=NULL)
	{
		p=p->next;
		size++;
	}
	pthread_mutex_unlock(&lock);
	return size;
}

void getListName(char *data)
{
	int size=0;
	pthread_mutex_lock(&lock);
	struct node *p = List->next;
	while(p!=NULL)
	{
		sprintf(data+size,"%s ",p->name);
		size += strlen(p->name)+1;
		p=p->next;
	}
	pthread_mutex_unlock(&lock);
}


int regist(struct MSG1 *buf)
{
	FILE *fp = NULL;
	char name[20];
	char passwd[20];
	fp = fopen("user.txt","rt+");
	if(fp==NULL)
	{
		fp = fopen("user.txt","wt+");
		if(fp == NULL)
		{
			return 2;
		}
		fprintf(fp,"%s   %s\n",buf->name,buf->passwd);
		fclose(fp);
		return 0;
	}
	while(1)
	{
		fscanf(fp,"%s%s",name,passwd);
		if(feof(fp))
		{
			break;
		}
		if(strcmp(name,buf->name)==0)
		{
			fclose(fp);
			return 1;
		}
	}
	fprintf(fp,"%s   %s\n",buf->name,buf->passwd);
	fclose(fp);
	return 0;
}


int login(struct MSG1 *buf)
{
	FILE *fp = NULL;
	char name[20];
	char passwd[20];
	fp = fopen("user.txt","rt+");
	if(fp==NULL)
	{
		return 4;
	}
	while(1)
	{
		fscanf(fp,"%s%s",name,passwd);
		if(feof(fp))
		{
			break;
		}
		if(strcmp(name,buf->name)==0)
		{
			if(strcmp(passwd,buf->passwd)==0)
			{
				fclose(fp);
				if(findbyname(name)==-1)
				{
					return 3;
				}
				else
				{
					return 6;
				}
				
			}
			else
			{
				fclose(fp);
				return 5;
			}
		}
	}
	fclose(fp);
	return 4;
}

void writeAll(struct MSG2 *buf)
{
	struct node *q=NULL;
	pthread_mutex_lock(&lock);
	q = List->next;
	while(q!=NULL)
	{
		write(q->fd,buf,sizeof(struct MSG2));
		q=q->next;
	}
	pthread_mutex_unlock(&lock);
}

void *thread(void *p)
{
	int fd=*((int*)p);
	int tempfd;
	char buf[1500];
	int len;
	int res;
	char name[20];
	struct MSG1 *buf1;
	struct MSG2 *buf2;
	
	while(1)
	{
		len = read(fd,buf,sizeof(buf));
		if(len <=0)
		{
			delList(fd);
			return NULL;
		}
		
		switch(*((int*)buf))
		{
		case 1:
			res = regist((struct MSG1 *)buf);
			write(fd,&res,sizeof(int));
			break;
		case 2:
			res = login((struct MSG1 *)buf);
			if(res == 3)
			{
				writeAll((struct MSG2 *)buf);
				addList(fd,(struct MSG1 *)buf);
				strcpy(name,((struct MSG1 *)buf)->name);
			}
			write(fd,&res,sizeof(int));
			break;
		case 3:		
			buf2 = (struct MSG2 *)buf;
			strcpy(buf2->name,name);
			buf2->cmd = 1;
			writeAll(buf2);
			break;
		case 4:
			buf2 = (struct MSG2 *)buf;
			tempfd = findbyname(buf2->name);
			if(tempfd > 0)
			{
				strcpy(buf2->name,name);
				buf2->cmd =3;
				write(tempfd,buf2,sizeof(struct MSG2));
			}
			else
			{
				write(fd,buf2,sizeof(struct MSG2));
			}
			break;
		case 5:
			buf2 = (struct MSG2 *)buf;
			write(fd,buf,sizeof(int));
			delList(fd);
			buf2->cmd =6;
			strcpy(buf2->name,name);
			writeAll(buf2);
			break;
		case 6:
			buf2 = (struct MSG2 *)buf;
			buf2->cmd = 7;
			*((int*)buf2->name) = ListSize();
			getListName(buf2->data);
			write(fd,buf2,sizeof(struct MSG2));
			break;
		case 7:
			pthread_exit(NULL);
			break;
		default:
			break;
		}
	
	
	}

}




int main(int argc, char *argv[])
{
	struct sockaddr_in serv_sock;
	int sockfd;
	int fd;
	struct sockaddr_in client_addr;
	int addr_size = sizeof(struct sockaddr_in);
	short port = PORT;
	pthread_t pth;

	if(argc > 1)
	{
		port = atoi(argv[1]);
	}

	serv_sock.sin_family=AF_INET;
	serv_sock.sin_port=htons(port); 
	serv_sock.sin_addr.s_addr=INADDR_ANY; 
	memset(&(serv_sock.sin_zero),0,8);

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{ 
		perror("socket");
		exit(1);
	}

	if(bind(sockfd,(struct sockaddr*)&serv_sock,sizeof(serv_sock))==-1)
	{ 
		perror("bind");
		exit(1);
	}

	if(listen(sockfd, MAXLOG)==-1)
	{
		perror("listen");
		exit(1);
	}

	List = (struct node *)malloc(sizeof(struct node));
	List->next = NULL;
	
	pthread_mutex_init(&lock,PTHREAD_MUTEX_TIMED_NP);
	
	while(1)
	{
		fd = accept(sockfd,(struct sockaddr*)&client_addr,(socklen_t*)&addr_size);
		if (fd > 0)
		{
			pthread_create(&pth,NULL,thread,&fd);
		}	
	}

	close(sockfd);
	return 0;
}














