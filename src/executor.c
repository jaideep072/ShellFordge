#include "executor.h"
#include "builtin.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int count_commands(Command *commands)
{
    int count = 0;

    while (commands != NULL) {
        count++;
        commands = commands->next;
    }

    return count;
}

static int setup_input_redirection(Command *command)
{
    if (command->input_file == NULL) {
        return 0;
    }

    int fd = open(command->input_file, O_RDONLY);

    if (fd < 0) {
        perror("shellforge: input");
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) < 0) {
        perror("shellforge: dup2");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

static int setup_output_redirection(Command *command)
{
    if (command->output_file != NULL) {

        int fd = open(command->output_file,
                      O_WRONLY | O_CREAT | O_TRUNC,
                      0644);

        if (fd < 0) {
            perror("shellforge: output");
            return -1;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("shellforge: dup2");
            close(fd);
            return -1;
        }

        close(fd);
    }

    if (command->append_file != NULL) {

        int fd = open(command->append_file,
                      O_WRONLY | O_CREAT | O_APPEND,
                      0644);

        if (fd < 0) {
            perror("shellforge: append");
            return -1;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("shellforge: dup2");
            close(fd);
            return -1;
        }

        close(fd);
    }

    return 0;
}

static void execute_child(Command *command)
{
    if (setup_input_redirection(command) < 0) {
        exit(EXIT_FAILURE);
    }

    if (setup_output_redirection(command) < 0) {
        exit(EXIT_FAILURE);
    }

    execvp(command->argv[0], command->argv);

    perror("shellforge");

    exit(EXIT_FAILURE);
}

int executor_execute(Command *commands)
{
    if (commands == NULL) {
        return 0;
    }

    int command_count = count_commands(commands);

    if (command_count == 1 &&
        commands->argc > 0 &&
        builtin_is_command(commands->argv[0]) &&
        !commands->background) {

        return builtin_execute(commands);
    }

    int previous_pipe_read = -1;

    pid_t *child_pids = malloc(sizeof(pid_t) * command_count);

    if (child_pids == NULL) {
        perror("malloc");
        return 1;
    }

    int child_index = 0;

    Command *current = commands;

    while (current != NULL) {

        int pipe_fd[2] = {-1, -1};

        if (current->next != NULL) {

            if (pipe(pipe_fd) < 0) {
                perror("shellforge: pipe");
                free(child_pids);

                if (previous_pipe_read != -1) {
                    close(previous_pipe_read);
                }

                return 1;
            }
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("shellforge: fork");

            if (pipe_fd[0] != -1) {
                close(pipe_fd[0]);
            }

            if (pipe_fd[1] != -1) {
                close(pipe_fd[1]);
            }

            if (previous_pipe_read != -1) {
                close(previous_pipe_read);
            }

            free(child_pids);

            return 1;
        }

        if (pid == 0) {

            /*
             * If this command receives input from
             * the previous command in the pipeline.
             */
            if (previous_pipe_read != -1) {

                if (dup2(previous_pipe_read, STDIN_FILENO) < 0) {
                    perror("shellforge: dup2");
                    exit(EXIT_FAILURE);
                }
            }

            /*
             * If this command sends output to
             * the next command in the pipeline.
             */
            if (pipe_fd[1] != -1) {

                if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
                    perror("shellforge: dup2");
                    exit(EXIT_FAILURE);
                }
            }

            /*
             * Close inherited pipe descriptors.
             */
            if (previous_pipe_read != -1) {
                close(previous_pipe_read);
            }

            if (pipe_fd[0] != -1) {
                close(pipe_fd[0]);
            }

            if (pipe_fd[1] != -1) {
                close(pipe_fd[1]);
            }

            /*
             * Built-ins inside a pipeline must execute
             * inside the child process.
             */
            if (builtin_is_command(current->argv[0])) {

                int status = builtin_execute(current);

                if (status >= 0) {
                    exit(status);
                }
            }

            execute_child(current);
        }

        /*
         * Parent process.
         */
        child_pids[child_index++] = pid;

        if (previous_pipe_read != -1) {
            close(previous_pipe_read);
        }

        if (pipe_fd[1] != -1) {
            close(pipe_fd[1]);
        }

        previous_pipe_read = pipe_fd[0];

        current = current->next;
    }

    if (previous_pipe_read != -1) {
        close(previous_pipe_read);
    }

    /*
     * Background command:
     *
     * Do not wait for the child processes.
     */
    if (commands->background) {

        printf("[background] started");

        for (int i = 0; i < child_index; i++) {
            printf(" %d", child_pids[i]);
        }

        printf("\n");

        free(child_pids);

        return 0;
    }

    /*
     * Foreground command:
     *
     * Wait for every process in the pipeline.
     */
    int final_status = 0;

    for (int i = 0; i < child_index; i++) {

        int status;

        if (waitpid(child_pids[i], &status, 0) < 0) {
            perror("shellforge: waitpid");
            final_status = 1;
            continue;
        }

        if (i == child_index - 1) {

            if (WIFEXITED(status)) {
                final_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                final_status = 128 + WTERMSIG(status);
            }
        }
    }

    free(child_pids);

    return final_status;
}
