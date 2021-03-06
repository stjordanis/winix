/**
 * 
 * Simple shell for WINIX.
 *
 * @author Bruce Tan
 * @email brucetansh@gmail.com
 * 
 * @author Paul Monigatti
 * @email paulmoni@waikato.ac.nz
 * 
 * @create date 2016-09-19
 * 
*/

#include "bash.h"

static char history_file[] = ".bash_history";

// Input buffer & tokeniser
static char buf[MAX_LINE];
static pid_t pgid;

// Prototypes
CMD_PROTOTYPE(slab);
CMD_PROTOTYPE(cmd_kill);
CMD_PROTOTYPE(print_pid);
CMD_PROTOTYPE(mem_info);
CMD_PROTOTYPE(do_trace_syscall);
CMD_PROTOTYPE(help);
CMD_PROTOTYPE(cmd_exit);
CMD_PROTOTYPE(printenv);
CMD_PROTOTYPE(cmd_bash);
CMD_PROTOTYPE(do_cd);
CMD_PROTOTYPE(do_cls);


// Command handling
struct cmd_internal builtin_commands[] = {
    { printenv, "env" },
    { do_trace_syscall, "trace"},
    { cmd_bash, "bash"},
    { slab, "slab"},
    { cmd_kill, "kill"},
    { print_pid, "pid"},
    { mem_info, "free"},
    { cmd_exit, "exit"},
    { help, "help"},
    { do_cd, "cd"},
    { do_cls, "clear"},
    { 0, NULL}
};

void init_shell(){
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    pgid = setsid();
    ioctl(STDIN_FILENO, TIOCSCTTY);
    ioctl(STDIN_FILENO, TIOCSPGRP, pgid);
}


int main() {
    int ret, len, newline_pos;
    char *c;
    int history_fd;

    init_shell();
    history_fd = open(history_file, O_CREAT | O_RDWR, 0x755);
    if(history_fd < 0){
        perror(history_file);
        return 1;
    }

    while(1) {
        printf("WINIX> ");
        ret = read(0, buf, MAX_LINE);

        if(ret == EOF){
            if(errno == EINTR){
                perror("stdin: ");
                printf("WINIX> ");
            }
            continue;
        }

        len = strlen(buf);
        newline_pos = len - 1;
        
        write(history_fd, buf, len);
        if(buf[newline_pos] == '\n'){
            buf[newline_pos] = '\0';
        }
        exec_cmd(buf, NULL);     
    }
    return 0;
}

int search_path(char* path, char* name){
    strcpy(path, "/bin/");
    strcat(path, name);
    return access(path, F_OK);
}

int _exec_cmd_pipe(struct cmdLine *cmd, int cmd1_idx, int cmd2_idx, int prev_fd, int next_fd){
    return 0;
}

#define PIPE_READ   (0)
#define PIPE_WRITE  (1)

/**
 * Handles any unknown command.
 **/
int _exec_cmd(char *line, struct cmdLine *cmd) {
    char buffer[128];
    int status;
    pid_t pid, second_pid;
    sigset_t sigmask = 0;
    int saved_stdin, saved_stdout;
    int sin = STDIN_FILENO, sout = STDOUT_FILENO;

    if(cmd->argc == 0)
        return -1;
    if(*line == '#')
        return 0;

    pid = vfork();
    
    if(!pid){
        pid_t child_pgid;
        int i, ret, cmd_start;
        int exit_code = 0;
        int pipe_fds[10];
        int *pipe_ptr, *prev_pipe_ptr;

        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        setpgid(0, 0);
        child_pgid = getpgid(0);
        ioctl(STDIN_FILENO, TIOCSPGRP, child_pgid);

        if(cmd->infile){ //if redirecting input
            // saved_stdin = dup(STDIN_FILENO); //backup stdin
            sin = open(cmd->infile, O_RDONLY);
            dup2(sin,STDIN_FILENO);
            close(sin);
        }

        for(i = 0; i < cmd->numCommands; i++){
            cmd_start = cmd->cmdStart[i];
            if(search_path(buffer, cmd->argv[cmd_start]) == 0){
                pipe_ptr = &pipe_fds[(i * 2)];
                
                if((i < cmd->numCommands - 1)){ // not the last command, create new pipe
                    ret = pipe(pipe_ptr);
                    if(ret){
                        perror("pipe");
                        exit(1);
                    }
                    fcntl(pipe_ptr[PIPE_READ], F_SETFL, O_NONBLOCK);
                    // printf("pipeptr %x ret %d %d\n", pipe_ptr, pipe_ptr[PIPE_READ], pipe_ptr[PIPE_WRITE]);

                }else if(cmd->outfile){ //if redirecting output and last command
                    int mode = O_WRONLY | O_CREAT;
                    if(cmd->append) //if append
                        mode |= O_APPEND;
                    else //else replace the original document
                        mode |= O_TRUNC;
                    sout = open(cmd->outfile, mode, 0x755);
                    dup2(sout,STDOUT_FILENO);
                    close(sout);
                }

                second_pid = vfork();

                if(second_pid == 0){ // child, actual command
                    if(cmd->numCommands > 1){
                        
                        if((i+1) < cmd->numCommands ){ // not the last command
                            pipe_ptr = &pipe_fds[(i * 2)];
                            dup2(pipe_ptr[PIPE_WRITE], STDOUT_FILENO);
                            close(pipe_ptr[PIPE_WRITE]);
                            // printf("cmd %d %s pipe  %d %d\n", i, buffer, pipe_ptr[PIPE_READ], pipe_ptr[PIPE_WRITE]);

                        }
                        // printf("cmd %d %s com %d, res %d\n", i, buffer,  cmd->numCommands, (i+1) < cmd->numCommands);

                        if(i > 0){ // not the first command, read previous pipe
                            prev_pipe_ptr = &pipe_fds[((i - 1) * 2)];
                            dup2(prev_pipe_ptr[PIPE_READ], STDIN_FILENO);
                            close(prev_pipe_ptr[PIPE_READ]);
                            // printf("cmd %d %s dup read %d\n", i, buffer, prev_pipe_ptr[PIPE_READ]);
                        }
                    }
                    cmd_start = cmd->cmdStart[i];
                    execv(buffer, &cmd->argv[cmd_start]);
                    exit(1);
                }else{
                    ret = wait(&status);
                    
                    // printf("parent awaken\n");
                }
            }else{
                cmd_start = cmd->cmdStart[i];
                fprintf(stderr, "Unknown command '%s'\n", cmd->argv[cmd_start]);
                exit_code = 1;
            }
        }

        // if(cmd->outfile){
        //     dup2(saved_stdout,STDOUT_FILENO);
        //     close(saved_stdout);
        // }

        // if(cmd->infile){
        //     dup2(saved_stdin,STDIN_FILENO);
        //     close(saved_stdin);
        // }
        exit(exit_code);
    }

    ioctl(STDIN_FILENO, TIOCENABLEECHO);
    ioctl(STDIN_FILENO, TIOCSPGRP, pgid);
    printf("\n");
    return 0;
}

int exec_cmd(char *line, int tpipe[2]){
    struct cmdLine cmd;
    int ret;
    char *p;
    char* buf;
    struct cmd_internal *handler = NULL;

    ret = parse(line,&cmd);

    if(cmd.env && cmd.env_val){ // if a new environment variable is set
        buf = malloc(MAX_LINE);
        parse_quotes(cmd.env_val, buf);
        
        if(*buf){
            printf("setenv %s=%s\n", cmd.env, buf);
            setenv(cmd.env, buf, 1);
        }
        free(buf);
        return 0;
    }

    // Decode command
    handler = builtin_commands;
    while(handler && handler->name && strcmp(cmd.argv[0], handler->name)) {
        handler++;
    }
    // Run it
    if(handler->name){
        return handler->handle(cmd.argc, cmd.argv);
    }
    return _exec_cmd(line, &cmd);
}


int printenv(int argc, char **argv){
    const char **v;
    const char *p;
    v = environ;
    while((p = *v++)){
        printf("%s\n",p);
    }
    return 0;
}

int cmd_kill(int argc, char **argv){
    pid_t pid;
    int signum = SIGTERM;
    if(argc < 2){
        printf("kill <-signal> [pid]\n");
        return -1;
    }
        
    if(*argv[1] == '-'){
        signum = atoi(argv[1] + 1);
        pid = atoi(argv[2]);
    }else{
        pid = atoi(argv[1]);
    }

    if(kill(pid,signum))
        perror("kill ");
    return 0;
}

int do_cd(int argc, char** argv){
    int ret;
    if(argc < 2)
        return -1;
    ret = chdir(argv[1]);
    return ret;
}

int help(int argc, char** argv){
    struct cmd_internal* handler;
    handler = builtin_commands;
    printf("Available Commands\n");
    while(handler && handler->name != NULL) {
        printf(" * %s\n",handler->name);
        handler++;
    }
    return 0;
}

// byte stream for \e[1;1H\e[2J
char cls[] = {0x1b, 0x5b, 0x31, 0x3b, 0x31, 0x48, 0x1b, 0x5b, 0x32, 0x4a, 0};

int do_cls(int argc, char** argv){
    printf("%s", cls);
    return 0;
}

int do_trace_syscall(int argc, char** argv){
    return enable_syscall_tracing();
}

// Print the system wise memory info
int mem_info(int argc, char** argv){
    
    int ret = wramp_syscall(WINFO, WINFO_MEM);
    return 0;
}

// current pid
int print_pid(int argc, char **argv){
    printf("%d\n",getpid());
    return 0;
}

// list all the processes in the system
int slab(int argc, char **argv){
    return wramp_syscall(WINFO, WINFO_SLAB);
}

// start a new bash shell, parent shell is blocked until child shell exits
int cmd_bash(int argc, char **argv){
    pid_t child_pid;
    if(child_pid = fork()){
        if(child_pid == -1){
            perror("fork failed");
            return -1;
        }
        // printf("parent shell %d waiting for child shell %d\n",getpid(),child_pid);
        child_pid = wait(NULL);
        // printf("parent shell %d awakened by child shell %d\n",getpid(),child_pid);
    }else{
        printf("Child shell %d [parent %d] start:\n",getpid(),getppid());
    }
    return 0;
}

int cmd_exit(int argc, char **argv){
    int status = 0;
    if(argc > 1)
        status = atoi(argv[1]);
    printf("Bye!\n");
    // printf("Child %d [parent %d] exits\n",getpid(),getppid());
    exit(status);
    return -1;
}




