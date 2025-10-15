void pipeline(char* input) {
    int n = 1;
    for (int i = 0; input[i]; i++)
        if (input[i] == '|') n++;

    char** commands = malloc(n * sizeof(char*));
    if (!commands) {
        perror("malloc commands");
        exit(EXIT_FAILURE);
    }

    // ✅ duplicate input to avoid destroying original
    char *temp = strdup(input);
    if (!temp) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    // ✅ split input into individual commands
    char *saveptr;
    char *command = strtok_r(temp, "|", &saveptr);
    int i = 0;
    while (command != NULL && i < n) {
        while (*command == ' ') command++; // trim leading spaces
        commands[i++] = command;
        command = strtok_r(NULL, "|", &saveptr);
    }

    int (*pipes)[2] = NULL;
    if (n > 1) {
        pipes = malloc((n - 1) * sizeof(int[2]));
        if (!pipes) {
            perror("malloc pipes");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < n - 1; i++)
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
    }

    // ✅ create child processes
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // child
            if (i > 0)
                dup2(pipes[i - 1][0], STDIN_FILENO);
            if (i < n - 1)
                dup2(pipes[i][1], STDOUT_FILENO);

            if (n > 1) {
                for (int j = 0; j < n - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            // tokenize arguments
            char* args[100];
            char* argsptr;
            char* token = strtok_r(commands[i], " \t", &argsptr);
            int k = 0;
            while (token != NULL && k < 99) {
                args[k++] = token;
                token = strtok_r(NULL, " \t", &argsptr);
            }
            args[k] = NULL;

            if (args[0] == NULL) exit(EXIT_SUCCESS);

            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // ✅ parent closes pipes and waits
    if (n > 1) {
        for (int i = 0; i < n - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }

    for (int i = 0; i < n; i++)
        wait(NULL);

    free(commands);
    free(temp);
    free(pipes);
}
