#include "run_command.h"


//need to implement joblock() and jobunlock() to protect joblist
//void jobunlock();
//void joblock();


// built-in command: jobs
void bJobs() {
	printList();
	return;
}

void bKill(char** args, int argn) {
	int kill_flag = FALSE; //kill_flag is true when input has -9
	int dash_flag = 0;
	int is_jid[argn-2];
	int id[argn-2];
	Job* job[argn-2];
	int to_be_killed = 0;
	bzero(id,argn-2);
	bzero(is_jid,argn-2);
	if(argn == 1 || argn == 2) {
		printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
		return;
	}

	//parse the arguments and send proper respond
	for(int i = 1; i < argn; i++) {
		//if the input is invalid, we will print usage of kill and return
		if(dash_flag > 1) {
			printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
			return;
		}
		if(args[i][0] == '-'){
			dash_flag ++;
			if(strcmp(args[i], "-9") == 0) {
				kill_flag = TRUE; //probably we need to test whether kill_flag is false
			}
			else {
				printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
				return;
			}
		}
		else if(args[i][0] == '%') {
			is_jid[i-2] = TRUE;
			if(is_jid[i-2]) {

				if((id[i-2] = atoi(args[i]+1)) == 0) {
					printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
					return;
				}
				//sscanf(args[i],"%%%d", &id);
			}
		}
		else {
			if((id[i-2] = atoi(args[i])) == 0) {
				printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
				return;
			}
		}
	}

	//find the actual job
	for(int i = 0; i < argn-2; i++) {
		if(is_jid[i]) {
			job[i] = getJobJobId(id[i]);
		}
		else {
			job[i] = getJobPid(id[i]);
		}
		if(job[i] == NULL) {
			printf("invalid job number or process number\n");
			return;
		}
		//actually execute the kill
		if(is_jid[i]) {
			printf("kill by job %d\n", id[i]);
		}
		else {
			printf("kill by process %d\n", id[i]);
		}

		to_be_killed = (-1) * job[i]->pgid;
	    if(kill_flag) {
			if(kill(to_be_killed,SIGKILL) == -1)
				perror("Kill failed\n");
	    }
		else {
			if(kill(to_be_killed,SIGTERM) == -1) {
				perror("Kill failed\n");
			}
		}
	}
}
// void bKill(char** args, int argn) {
// 	int kill_flag = FALSE; //kill_flag is true when input has -9
// 	int dash_flag = 0;
// 	int percent_flag = 0;
// 	int is_jid = FALSE;
// 	int id;
// 	Job* job;
// 	int to_be_killed = 0;
// 	if(argn == 1) {
// 		printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
// 		return;
// 	}

// 	//parse the arguments and send proper respond
// 	for(int i = 1; i < argn; i++) {
// 		//if the input is invalid, we will print usage of kill and return
// 		if(dash_flag > 1 || percent_flag > 1) {
// 			printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
// 			return;
// 		}
// 		if(args[i][0] == '-'){
// 			dash_flag ++;
// 			if(strcmp(args[i], "-9") == 0) {
// 				kill_flag = TRUE; //probably we need to test whether kill_flag is false
// 			}
// 			else {
// 				printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
// 				return;
// 			}
// 		}
// 		else if(args[i][0] == '%') {
// 			percent_flag ++;
// 			is_jid = TRUE;
// 			if(is_jid) {
// 				sscanf(args[i],"%%%d", &id);
// 			}
// 			else {
// 				if((id = atoi(args[i])) == 0) {
// 					printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
// 					return;
// 				}
// 			}
// 		}
// 	}

// 	//find the actual job
// 	if(is_jid) {
// 		job = getJobJobId(id);
// 	}
// 	else
// 		job = getJobPid(id);
// 	if(job == NULL) {
// 		printf("invalid job number or process number\n");
// 		return;
// 	}
// 	//actually execute the kill
// 	if(is_jid) {
// 		printf("kill by job %d\n", id);
// 	}
// 	else
// 		printf("kill by process %d\n", id);

// 	to_be_killed = (-1) * job->pgid;
//     if(kill_flag){
// 		if(kill(to_be_killed,SIGKILL) == -1)
// 			perror("Kill failed\n");
//     }
// 	else
// 		if(kill(to_be_killed,SIGTERM) == -1)
// 			perror("Kill failed\n");

// }


void put_job_in_foreground(Job* job, sigset_t child_mask, int flag_stop) {
	pid_t pid;
	int status;
	//put the job into foreground;
	//shell_terminal is a global file descriptor represented as shell
	tcsetpgrp(myShTerminal, job->pgid);

	//stroe the current shell status to its termios
	//tcgetattr(myShTerminal, &myShTmodes);

	//restore the terminal attributes when the job stopped last time -- if the job used to be stopped, we restroe, otherwise, we ignore
	if(flag_stop)
		tcsetattr(myShTerminal, TCSADRAIN, &job->j_Tmodes);

	//we have already changed the status of process
	//wait foreground job to exit
	while(job->status != JOBCOMP && job->status != JOBSTOP) {
		//pid = waitpid(WAIT_ANY, &status, WUNTRACED | WNOHANG);
		pid = waitpid(WAIT_ANY, &status, WUNTRACED);			// since it's in foreground, we shouldn't use WNOHANG
		printf("foreground job returned, pid is %d\n", pid);

		if(WIFSTOPPED(status))
			jobChangeStatus(job, JOBSTOP);
		else
			jobChangeStatus(job, JOBCOMP);

		printf("Job status is %d\n", job->status);
	}

	//printjob(job->id);
	//if the job complete, we exit the job
	if(job->status == JOBCOMP) {
		jobs_lock(child_mask); //need to implement, block all the possible access to job list
		jobRemoveJobId(job->jobId);
		jobs_unlock(child_mask);
	}

	if(job->status == JOBSTOP) {
		//if job has been suspended, we store its termios
		tcgetattr(myShTerminal, &job->j_Tmodes);
	}

    /* Put the shell back in the foreground.  */
    tcsetpgrp (myShTerminal, myShPGid);

    /* Restore the shell’s terminal modes.  */
    tcsetattr (myShTerminal, TCSADRAIN, &myShTmodes);

}

void bFg(char** args, int argn, sigset_t child_mask) {
	int jid; // get job id by args or use default setting
	Job* current_job = NULL;
	int is_suspended = 0;
	/*if(argn == 1)
		jid = num_of_job(); // implemented by jiaping, get the number of job in a job list
	else
		jid = atoi(args[1]); //convert the input job id to int
	current_job = getJobJobId(jid);
	if(current_job == NULL) {
		printf("job %d does not exist, we can find it\n", jid);
		return;
	}*/

	if (argn == 1) {
		current_job = getJLastBackgrounded();
	}
	else {
		for(int i = 1; i < argn; i++) {
			if(i >= 2) {
				continue;
			}
			if (args[i][0] == '%' && atoi(args[i]+1) != 0) {
				current_job = getJobJobId(atoi(args[i]+1));
			}
			//if called by command name
			else if(atoi(args[i]) == 0){
				//d
				current_job = getJobCommandName(args[i]);
			}
			else {
				current_job = getJobJobId(atoi(args[i]));
			}

		}
	}

	//update job status
	//first check whether the job used to be stopped
	if(current_job->field == JOBSTOP)
		is_suspended = 1;
	current_job->field = JOBFORE; //-- How do I know it is from stopped job or newly created job? how to keep track
	current_job->status = JOBRUN;
	pid_t current_pgid = -1 * current_job->pgid;
	if(kill(current_pgid, SIGCONT) < 0)
		perror("kill (SIGCONT)"); // send sigcont to all processes of that process group
	//we can directly send a signal to a process group

	//should add a flag to determine whether job used to be stopped
	put_job_in_foreground(current_job, child_mask, is_suspended);

}
// void bFg(char** args, int argn, sigset_t child_mask) {
// 	int jid; // get job id by args or use default setting
// 	Job* current_job = NULL;
// 	int is_suspended = 0;
// 	/*if(argn == 1)
// 		jid = num_of_job(); // implemented by jiaping, get the number of job in a job list
// 	else
// 		jid = atoi(args[1]); //convert the input job id to int

// 	current_job = getJobJobId(jid);
// 	if(current_job == NULL) {
// 		printf("job %d does not exist, we can find it\n", jid);
// 		return;
// 	}*/

// 	if (argn == 1) {
// 		current_job = getJLastBackgrounded();
// 	}
// 	else if (argn == 2){
// 		if (args[1][0] == '%' && atoi(args[1]+1) != 0) {
// 			current_job = getJobJobId(atoi(args[1]+1));
// 		}
// 		else {
// 			printf("Pleas type in %%[number]\n");
// 		}
// 	}
// 	else if(argn == 3) {
// 		if ((strcmp(args[1],"%") == 0) && atoi(args[2]) != 0) {
// 			current_job = getJobJobId(atoi(args[2]));
// 		}
// 		else {
// 			printf("Pleas type in %%[number]\n");
// 		}
// 	}
// 	else {
// 		printf("Pleas type in %%[number]\n");
// 	}

// 	//update job status
// 	//first check whether the job used to be stopped
// 	if(current_job->field == JOBSTOP)
// 		is_suspended = 1;
// 	current_job->field = JOBFORE; //-- How do I know it is from stopped job or newly created job? how to keep track
// 	current_job->status = JOBRUN;
// 	pid_t current_pgid = -1 * current_job->pgid;
// 	if(kill(current_pgid, SIGCONT) < 0)
// 		perror("kill (SIGCONT)"); // send sigcont to all processes of that process group
// 	//we can directly send a signal to a process group
// 	/*
// 	while(current_job_process != NULL) {
// 		//need to implement kill()
// 		kill(current_job_process->pid, SIGCONT)； // send sigcont to each process in job
// 		current_job_process->status = PROCRUN; // set process to run status
// 		current_job_process = current_job_process->next;
// 	}
// 	*/

// 	//should add a flag to determine whether job used to be stopped
// 	put_job_in_foreground(current_job, child_mask, is_suspended);

// }

// void bBg(char** args, int argn) {
// 	Job* current_job = NULL;
// 	if (argn == 1) {
// 		current_job = getJLastSuspended();
// 	}
// 	for(int i = 0; i < argn - 1; i++) {
// 		if (args[i][0] == '%' && atoi(args[i]+1) != 0) {
// 			current_job = getJobJobId(atoi(args[1]+1));
// 		}
// 		else if (atoi(args[i]) != 0) {

// 		}
// 	}
// }


// void bBg(char** args, int argn) {
// 	Job* current_job = NULL;
// 	if (argn == 1) {
// 		current_job = getJLastSuspended();
// 	}
// 	for(int i = 0; i < argn - 1; i++) {
// 		if (args[i][0] == '%' && atoi(args[i]+1) != 0) {
// 			current_job = getJobJobId(atoi(args[1]+1));
// 		}
// 		else if (atoi(args[i]) != 0) {
// 			current_job = getJobJobId(atoi(args[1]));
// 		}
// 		else {
// 			continue;
// 		}
// 		if (current_job == NULL) {
// 			printf("job does not exist, we can find it\n");
// 			continue;
// 		}
// 		else {
// 			current_job->field = JOBBACK;
// 			current_job->status = JOBRUN;
// 			last_backgrounded = current_job->jobId;

// 		}

// 	}
// }



// could be "bg %        number" or "bg %number"
void bBg(char** args, int argn) {
	Job* current_job = NULL;
	/*if(argn == 1)
		jid = num_of_job(); // implemented by jiaping, get the number of job in a job list
	else
		jid = atoi(args[1]); //convert the input job id to int
	*/
	if (argn == 1) {
		current_job = getJLastSuspended();
	}
	else if (argn == 2){
		if (args[1][0] == '%' && atoi(args[1]+1) != 0) {
			current_job = getJobJobId(atoi(args[1]+1));
		}
		else {
			printf("Pleas type in %%[number]\n");
		}
	}
	else if(argn == 3) {
		if ((strcmp(args[1],"%") == 0) && atoi(args[2]) != 0) {
			current_job = getJobJobId(atoi(args[2]));
		}
		else {
			printf("Pleas type in %%[number]\n");
		}
	}
	else {
		printf("Pleas type in %%[number]\n" );
	}

	//current_job = getJobJobId(jid);
	if(current_job == NULL) {
		printf("job does not exist, we can find it\n");
		return;
	}

	//update job status
	current_job->field = JOBBACK;
	current_job->status = JOBRUN;
	last_backgrounded = current_job->jobId;
	pid_t current_pgid = -1 * current_job->pgid;
	if(kill(current_pgid, SIGCONT) < 0)
		perror("kill (SIGCONT)");


}
int check_built_in(Job* job) {
	char** args = job->processList->args;
	int argn = job->processList->argn;
	if(strcmp(args[0],"kill") == 0) {
		return TRUE;
	}
	else if (strcmp(args[0],"fg") == 0) {
		return TRUE;
	}
	else if (strcmp(args[0],"bg") == 0) {
		return TRUE;
	}
	else if (strcmp(args[0],"exit") == 0) {
		return TRUE;
	}
	else if (strcmp(args[0],"jobs") == 0) {
		return TRUE;
	}
	else
		return FALSE;
}

int exeBuiltIn(char** args, int argn, sigset_t child_mask) {
	if(strcmp(args[0],"kill") == 0) {
		bKill(args, argn);
	}
	else if (strcmp(args[0],"fg") == 0) {
        bFg(args, argn, child_mask);
	}
	else if (strcmp(args[0],"bg") == 0) {
        bBg(args, argn);
	}
	else if (strcmp(args[0],"exit") == 0) {
		bExit(); // todo
	}
	else if (strcmp(args[0],"jobs") == 0) {
		bJobs(); // todo
	}
    else {
		perror("invalid input, check_built_in wrong probably\n");
        return FALSE;
    }
    return TRUE;
}

// not supporting pipe for now.
// executing the command without pipe. Example: emacs shell.c; or emacs shell.c &
void executing_command_without_pipe(Job *job, sigset_t child_mask) {
	pid_t pid;
	int status;
	//Job *childjob;
	char** args = job->processList->args;
	int argn = job->processList->argn;
	// check if the this job is built-in command, foreground, or background
	// TODO: check_built_in(), exeBuiltIn(), if this job is built-in command
	if (check_built_in(job)) {
		exeBuiltIn(args, argn, child_mask);
	}
	// If it is foreground job
	else if(job->field == JOBFORE) {
		pid = fork();
		if (pid == 0) {
			setpgrp(); // set the pgid of the child process
			job->pgid = getpgrp(); 
			//child process
			signal(SIGINT, SIG_DFL);
			signal(SIGTERM, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			//signal(SIGTTOU, SIG_DFL);
			signal(SIGTTIN, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);
			// execute the command
			//printf("%d %d %d \n",getpgid(getpid()), getpid(), getppid());
			tcsetpgrp(myShTerminal, getpid());
			signal(SIGTTOU, SIG_DFL);
			if((execvp(args[0], args)) == FAIL) {
				printf("Didn't execute the command: %s! Either don't know what it is, or it is unavailable. \n", args[0]);
      			exit(EXIT_FAILURE);
			}
		} else if (pid > 0) {
			// parent process
			setpgid(pid, 0); // set the pgid of the child process
			tcsetpgrp(myShTerminal, pid); //bring child to foreground, modified Tue 7:25 pm
			job->pgid = pid;
			waitpid(pid, &status, WUNTRACED);
			printf("Test!\n");
			// if the signal is termination (WIFSIGNALED) or normal exit, remove the job and free the memory.
			if (WIFSIGNALED(status) || WIFEXITED(status)) {
				jobs_lock(child_mask);
				jobRemovePid(pid);
				jobs_unlock(child_mask);
				tcsetpgrp (myShTerminal, myShPGid);
				tcsetattr (myShTerminal, TCSADRAIN, &myShTmodes);
				return;
				//freeJob(job);
			} else if (WIFSTOPPED(status)) {
			// else if the signal is stop, update the job's field, put it into background (save termios), 
				job->field = JOBBACK;
				tcgetattr(myShTerminal, &job->j_Tmodes);
				/* Put the shell back in the foreground.  */
    			tcsetpgrp (myShTerminal, myShPGid);

    			/* Restore the shell’s terminal modes.  */
    			tcsetattr (myShTerminal, TCSADRAIN, &myShTmodes);
			return;
			} else
				printf("Error in parent process");
		} else {
			perror("Forking error!");
			exit(EXIT_FAILURE);
		}
	} else if(job->field == JOBBACK) {
		// if this job is background job
		pid = fork();
		if (pid == 0) {
			//child process
			setpgrp(); // set the pgid of the child process
			job->pgid = getpgrp(); 
			//sigemptyset(&block_mask);
			signal(SIGINT, SIG_DFL);
			signal(SIGTERM, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			signal(SIGTTOU, SIG_DFL);
			signal(SIGTTIN, SIG_DFL);
			// execute the command
			if((execvp(args[0], args)) == FAIL) {
				printf("Didn't execute the command: %s! Either don't know what it is, or it is unavailable. \n", args[0]);
      			exit(EXIT_FAILURE);
			}
		} else if (pid > 0) {
			// parent process
			setpgid(pid, 0); // set the pgid of the child process
			job->pgid = pid;
			waitpid(pid, &status, WNOHANG);
			// if the signal is termination (WIFSIGNALED) or normal exit, remove the job and free the memory.
			if (WIFSIGNALED(status) || WIFEXITED(status)) {
				jobs_lock(child_mask);
				jobRemovePid(getpgid(pid));
				jobs_unlock(child_mask);
				//freeJob(job);
				return;
			} else
				printf("Error in parent process");
		} else {
			perror("Forking error!");
			exit(EXIT_FAILURE);
		}
	}
}

void bExit() {
	freeJobList();
	exit(EXIT_SUCCESS);
}

void jobs_lock(sigset_t child_mask) {
    sigprocmask(SIG_BLOCK, &child_mask, NULL);
}

void jobs_unlock(sigset_t child_mask) {
    sigprocmask(SIG_UNBLOCK, &child_mask, NULL);
}
