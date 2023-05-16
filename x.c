






















































//orphan

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	pid_t pid;
	pid = fork();

	switch(pid){
	case -1:
		perror("\nError calling fork");
		exit(1);
	case 0:
		printf("\nThis is the child running with PID: %d\n", getpid());
		sleep(3);
		execlp("ps", "ps", "-f", (char *)NULL);
		exit(1);
	default:
		int init_pid = getppid();
		printf("\ninit process PID: %d", init_pid);

		printf("\n\nThis is the parent running executing execlp with PID: %d\n", getpid());
		exit(0);
	}
}


//sig

#include <stdio.h>
#include <signal.h>

int main()
{
	sigset_t set;
	sigaddset(&set, SIGINT);	//blocks SIGINT signal i.e CTRL-C
	sigaddset(&set, SIGTSTP);	//blocks SIGTSTP Signal i.e CTRL-Z

	if(sigprocmask(SIG_SETMASK, &set, NULL) == -1)	// -1 means 	
		perror("sigprocmask");

	while(1){
		fprintf(stderr, "ZzzZZz...\t");
		sleep(2);
	}

	return 0;

}

#include <stdio.h>
#include <signal.h>

int main()
{
	int*ptr;
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGSEGV, &sa, NULL);

	*ptr = 1;

	return 0;

}


//l9

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main(){

	int fd1, fd2;
	pid_t pid;

	char sendbuf[BUFSIZ];
	char rcvbuf[BUFSIZ];

	memset(sendbuf, 0, BUFSIZ);
	memset(rcvbuf, 0, BUFSIZ);

	mkfifo("fifo1", 0777);
	mkfifo("fifo2", 0777);

	pid = fork();

	switch(pid){
	case -1:
		perror("fork");
		exit(1);
	case 0:
		fd1 = open("fifo1", O_WRONLY);

		fprintf(stderr, "\n\t*** CHild Talking ***\n"
				"\tEnter ur message...\n"
				">>> ");

		fgets(sendbuf, BUFSIZ, stdin);
		write(fd1, sendbuf, strlen(sendbuf));
		close(fd1);

		fd2 = open("fifo2", O_RDONLY);
		read(fd2, rcvbuf, BUFSIZ);
		fprintf(stderr, "C-Recvd<<< %s\n", rcvbuf);
		close(fd2);

		break;
	default:
		fd1 = open("fifo1", O_RDONLY);

		read(fd1, rcvbuf, BUFSIZ);
		fprintf(stderr, "P-Recvd<<< %s\n", rcvbuf);
		close(fd1);

		fprintf(stderr, "\n\t*** Parent Talking ***\n"
				"\tEnter ur message...\n"
				">>> ");

		fd2 = open("fifo2", O_WRONLY);
		fgets(sendbuf, BUFSIZ, stdin);
		write(fd2, sendbuf, strlen(sendbuf));
		close(fd2);

		wait(NULL);
		unlink("fifo1");
		unlink("fifo2");
	}

	return 0;
}




//l9


#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#define fifo "fifo_file"

// run this in a separate terminal together with sender
// reader

int main(){
	int fd;
	char buf[BUFSIZ];

	memset(buf, 0, BUFSIZ);

	fd = open(fifo, O_RDONLY);

	read(fd, buf, sizeof buf);

	fprintf(stderr, "Msg from writer : %s\n", buf);

	return 0;
}


#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#define fifo "fifo_file"

// run this in separate terminal with reciever
// writer

int main(){
	int fd;
	char buf[BUFSIZ];

	memset(buf, 0, BUFSIZ);

	mkfifo(fifo, 0777);

	fd = open(fifo, O_WRONLY);

	strcpy(buf, "Ubuntu 7.10");

	write(fd, buf, strlen(buf));

	return 0;
}



//l10 e1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *_buf;
};

int main()
{
	pid_t pid;
	int sem_id, ctr;
	union semun sem_un;
	struct sembuf sem_b;

	sem_id = semget(123, 1, 0666|IPC_CREAT);

	sem_un.val = 1;

	semctl(sem_id, 0, SETVAL, sem_un);

	pid = fork();

	for(ctr = 0; ctr < 3; ctr++)
	{
		sleep(rand()%3);

		sem_b.sem_num = 0;
		sem_b.sem_op = -1;
		sem_b.sem_flg = 0;
		semop(sem_id, &sem_b, 1);

		if(!pid)
			fprintf(stderr, "\tC-Entering\n");
		else
			fprintf(stderr, "\tP-Entering\n");

		sleep(rand()%3);

		if(!pid)
                        fprintf(stderr, "\tC-Exiting\n");
                else
                        fprintf(stderr, "\tP-Exiting\n");

		sem_b.sem_num = 0;
                sem_b.sem_op = 1;
                sem_b.sem_flg = 0;
                semop(sem_id, &sem_b, 1);

		sleep(rand()%3);
	}

	if(pid)
	{
		wait(NULL);
		semctl(sem_id, 0, IPC_RMID);
	}

	return 0;

}

//l11 e1 w

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

struct shm_st{
	char buf[50];
	int flag;
};

int main()
{
	int shm_id;
	struct shm_st *shm;

	if((shm_id = shmget(123, sizeof(struct shm_st), 0666|IPC_CREAT)) == -1)
	{
		perror("shmget");
		exit(1);
	}

	if((shm = shmat(shm_id, NULL, 0)) == (struct shm_st *) -1)
	{
		perror("shmat");
		exit(1);
	}

	strcpy(shm->buf, "Get busy living, or get busy dying...");

	/*** cleanup!! ***/

	while(1)
	{
		if(shm->flag)
		{
			if(shmdt(shm) == -1)
			{
				perror("shmdt");
				exit(1);
			}

			if(shmctl(shm_id, IPC_RMID, NULL) == -1)
			{
				perror("shmctl");
				exit(1);
			}
		}

		sleep(2);
	}

	return 0;

}

//l11 e1 r

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

struct shm_st{
	char buf[50];
	int flag;
};

int main()
{
	int shm_id;
	struct shm_st *shm;

	if((shm_id = shmget(123, sizeof(struct shm_st), 0444)) == -1)
	{
		perror("shmget");
		exit(1);
	}

	if((shm = shmat(shm_id, NULL, 0)) == (struct shm_st *) -1)
	{
		perror("shmat");
		exit(1);
	}

	fprintf(stderr, "\n\tText from the writer is as follows...\n");
	fprintf(stderr, "\t%s\n\n", shm->buf);

	shm->flag = 1;

	if(shmdt(shm) == -1)
	{
		perror("shmdt");
		exit(1);
	}

	return 0;

}





//l11 r e2
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

struct msg_st{
	long int flag;
	char buf[50];
};

int main()
{
	int msg_id, len;
	struct msg_st msg;

	msg_id = msgget(123, 0444);

	len = msgrcv(msg_id, &msg, 50, 0, 0);

	fprintf(stderr, "\n\tMsg. received is: %s\n", msg.buf);
	fprintf(stderr, "\tLength of msg. text is: %d\n\n", len);

	msgctl(msg_id, IPC_RMID, 0);

	return 0;
}

//l11 w e2

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

struct msg_st{
	long int flag;
	char buf[50];
};

int main()
{
	int msg_id;
	struct msg_st msg;
	msg.flag = 1;
	memset(msg.buf, 0, 50);

	msg_id = msgget(123, 0666|IPC_CREAT);

	strcpy(msg.buf, "The King is dead, Long live the King...");

	msgsnd(msg_id, &msg, 50, 0);

	return 0;
	

}	

}
