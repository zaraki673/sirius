#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main(int argc, char **argv){
	pid_t pid;
	int fds[2];
	
	pipe (fds);
	pid = fork();
	
	if(pid == 0){
		//Child Process
		/*This is the child process. Close our copy of the write end of the 
		file descriptor*/
		close (fds[1]);
		/*connect the read end of the pipe to standard input*/
		dup2(fds[0],STDIN_FILENO);
		/*execute a shell command*/
		execl("../src/online2bin/online2-wav-nnet2-latgen-faster",
					"--do-endpointing=false",
					"--online=true",
					"--config=nnet_a_gpu_online/conf/online_nnet2_decoding.conf",
					"--max-active=7000",
					"--beam=15.0",
					"--lattice-beam=6.0",
					"--acoustic-scale=0.1",
					"--word-symbol-table=graph/words.txt",
					"nnet_a_gpu_online/smbr_epoch2.mdl", 
					"graph/HCLG.fst",
					"\"ark:echo utterance-id1 utterance-id1|\"",
					"\"scp:echo utterance-id1 null|\"",
					"ark:/dev/null",(char*)NULL);		
					
	}		
	else if (pid <0){
		//failed
		perror("fork");
	}
	else {
		//Parent Process (pid>0)
		FILE * stream;
		//Close our copy of the read end of the file descriptor. */
		close (fds[0]);	
		
		stream = fdopen (fds[1],"w");
		fprintf (stream, "ENG_M.wav");	
		fflush(stream);
		close (fds[1]);
	}
	return 0;
}
