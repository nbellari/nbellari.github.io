#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MYSH_RL_BUF_SIZE 1024 /* default buffer size to store the line */

int mysh_cd(char **args);
int mysh_pwd(char **args);
int mysh_exit(char **args);
int mysh_help(char **args);

/* The order of the strings here should match with that of mysh_builtin_fns */
char *mysh_builtin_strs[] = {
    "cd",
    "pwd",
    "help",
    "exit"
};

/* The order of the functions here should match with that of mysh_builtin_strs */
int (*mysh_builtin_fns[])(char **) = {
    &mysh_cd,
    &mysh_pwd,
    &mysh_help,
    &mysh_exit
};

int mysh_num_builtins()
{
    return sizeof(mysh_builtin_strs)/sizeof(char *);
}

int
mysh_cd(char **args)
{
    /* Need at least one argument, after argv[0] */
    if (args[1] == (char *)NULL) {
        fprintf(stderr, "cd: need at least one argument\n");
    } else {
        if (chdir(args[1]) < 0) {
            perror("Unable to change directory:");
        }
    }

    return 1;
}

#define PATH_BUF_SIZE 1024
int
mysh_pwd(char **args)
{
    char path_buf[PATH_BUF_SIZE];

    if (getcwd(path_buf, PATH_BUF_SIZE) == NULL) {
        perror("Failed to get current working directory: ");
    } else {
        printf("%s\n", path_buf);
    }

    return 1;
}

int
mysh_help(char **args)
{
    int i;

    printf("NagP's shell\n");
    printf("Type commands followed by arguments, Hit Enter..\n");
    printf("built-ins available are:\n");

    for (i=0; i<mysh_num_builtins(); i++) {
        printf(" %s\n", mysh_builtin_strs[i]);
    }

    return 1;
}

int
mysh_exit(char **args)
{
    return 0;
}

char *
mysh_get_line()
{
    int c; /* to store the current character */
    int position; /* current position in the buffer */
    char *buffer = malloc(sizeof(char) * MYSH_RL_BUF_SIZE);
    int bufsize = MYSH_RL_BUF_SIZE;

    if (buffer == NULL) {
     fprintf(stderr, "Failed to allocate memory!\n");
     exit(EXIT_FAILURE);
    }

    position = 0;
    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            /* Either the user has pressed Ctrl-D or Enter
               Mark the end of line and return
            */
            buffer[position] = '\0';
            return buffer;
        }

        /* If it is a normal character, append and continue */
        buffer[position++] = c;

        /* Check if the buffer needs expansion */
        if (position >= bufsize) {
            bufsize = bufsize + MYSH_RL_BUF_SIZE;
            buffer = realloc(buffer, bufsize);

            if (buffer == NULL) {
                fprintf(stderr, "Failed to allocate memory!\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define MYSH_MAX_TOKENS 64
#define MYSH_TOKEN_DELIMS " \t\r\n"

char **
mysh_split_line(char *line)
{
    int  num_max_tokens = MYSH_MAX_TOKENS;
    char **tokens = malloc(num_max_tokens * sizeof(char *));
    char *token;
    int position = 0;

    if (!tokens) {
        fprintf(stderr, "Memory Allocation Failure\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MYSH_TOKEN_DELIMS);
    while (token != NULL) {
        tokens[position++] = token;

        /* Extend the token stream, if needed */
        if (position >= num_max_tokens) {
            num_max_tokens += MYSH_MAX_TOKENS;
            tokens = realloc(tokens, num_max_tokens * sizeof(char *));

            if (!tokens) {
                fprintf(stderr, "Memory Allocation Failure\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, MYSH_TOKEN_DELIMS);
    }

    /* Delimit the token stream */
    tokens[position] = (char *)NULL;

    return tokens;
}

int
mysh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    /* Create a child process */
    pid = fork();

    if (pid == 0) {
        /* This is the child, exec the required process */
        if (execvp(args[0], args) < 0) {
            perror("Failed to exec: ");
            return 1; /* The shell should not quit here */
        }
    } else if (pid < 0) {
        perror("Failed to fork: ");
        exit(EXIT_FAILURE); /* A problem in fork, means, nothing can be done */
    } else {
        /* Shell process */
        /* Wait for the child to terminate, not just change the state */
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int
mysh_is_builtin(char *cmd, int *idx)
{
    int i;

    for (i=0; i<mysh_num_builtins(); i++) {
        if (strcmp(mysh_builtin_strs[i], cmd) == 0) {
            *idx = i;
            return 1;
        }
    }

    return 0;
}

int
mysh_execute(char **args)
{
    char *cmd = args[0];
    int builtin_fn_idx;

    if (args[0] == NULL) {
        return 1;
    }

    if (mysh_is_builtin(cmd, &builtin_fn_idx)) {
        return mysh_builtin_fns[builtin_fn_idx](args);
    } else {
        return mysh_launch(args);
    }
}

void
mysh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = mysh_get_line();
        args = mysh_split_line(line);
        status = mysh_execute(args);

        /* line and args are dynamically allocated */
        free(line);
        free(args);

    } while (status);
}

int
main(int argc, char **argv)
{
    /* Enter the loop */
    mysh_loop();

    return 0;
}