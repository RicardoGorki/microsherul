#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

# define PIPE 1
# define S_COLLON 2

int		my_strlen(char *str);
void	error_fatal(void);
void	error_excve(char *path);
void	error_cd_path(char *path);
void	error_cd_arg(void);
void	exec_shell(char **argv, char **envp);
void	exec_to_pipe(char **argv, char **envp, int *prevpipe, int op);


int my_strlen(char *str)
{
	int i;

	i = 0;
	while(str[i])
		i++;
	return (i);
}

void	error_fatal(void)
{
	char *msg;

	msg = "error: fatal\n";
	write(2, msg, my_strlen(msg));
	exit(0);
}

void	error_excve(char *path)
{
	char *msg;

	msg = "error: cannot execute ";
	write(2, msg, my_strlen(msg));
	write(2, path, my_strlen(path));
	write(2, "\n", 1);
}

void	error_cd_path(char *path)
{
	char *msg;

	msg = "error: cd: cannot change directory to ";
	write(2, msg, my_strlen(msg));
	write(2, path, my_strlen(path));
	write(2, "\n", 1);
}
void	error_cd_arg(void)
{
	char *msg;

	msg = "error: cd: bad arguments\n";
	write(2, msg, my_strlen(msg));
}

void exec_to_pipe(char **argv, char **envp, int *prevpipe, int op)
{
	int i;
	int fd[2];
	int pid;

	i = 0;
	pid = 0;
	if (argv[i] && !strcmp(argv[i], "cd"))
	{
		if (argv[1] == NULL && chdir(argv[0]) == -1)
			error_cd_arg();
		else if (argv[1] != 0 && chdir(argv[1]) == -1)
			error_cd_path(argv[1]);
		return ;
	}

	if (pipe(fd) == -1)
		error_fatal();
	pid = fork();
	if (pid == -1)
		error_fatal();
	if (pid == 0)
	{
		if (op)
			if (close(fd[0]) == -1)
				error_fatal();
		if (op == PIPE)
			if (dup2(fd[1], 1) == -1)
				error_fatal();
		if (op == PIPE)
			if (close(fd[1]) == -1)
				error_fatal();
		if (dup2(*prevpipe, 0) == -1)
			error_fatal();
		if (*prevpipe != 0)
			if (close(*prevpipe) == -1)
				error_fatal();
		if (execve(argv[0], argv, envp) == -1)
			error_excve(argv[0]);
	}
	else
	{
		if (waitpid(pid, NULL, 0) == -1)
			error_fatal();
		if (op)
			if (close(fd[1]) == -1)
				error_fatal();
		if (*prevpipe != 0)
			if (close(*prevpipe) == -1)
				error_fatal();
		*prevpipe = fd[0];
	}
}

void exec_shell(char **argv, char **envp)
{
	int	i;
	int	op;
	int	start;
	int	prevpipe[1];

	i = 1;
	op = 0;
	*prevpipe = 0;
	while(argv[i] && (!strcmp(argv[i], "|") || !strcmp(argv[i], ";")))
		i++;
	start = i;
	while(argv[i])
	{
		while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
			i++;
		if (argv[i] && !strcmp(argv[i], "|"))
			op = PIPE;
		else if (argv[i] && !strcmp(argv[i], ";"))
			op = S_COLLON;
		else
			op = 0;
		while (argv[i] && (!strcmp(argv[i], "|") || !strcmp(argv[i], ";")))
			argv[i++] = NULL;
		exec_to_pipe(&argv[start], envp, prevpipe, op);
		start = i;
	}
}

int main(int argc, char **argv, char **envp)
{
	if (argc == 1)
		return (0);
	exec_shell(argv, envp);
	return (0);
}
