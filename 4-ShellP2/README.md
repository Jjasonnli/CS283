1. 
We use fork before calling execvp to ensure that the shell running still. It makes it so that the shell can keep running commands
2. 
If fork() fails it returns -1. My code checks for this and prints an error message.
3. 
execvp() searches for commands in the directories listed in the PATH environment.
4.
wait() makes the parent process pause until the child process finishes. If we don't call it, child processes will still be in memory which wastes resources.
5. 
WEXITSTATUS() gets the exit code of a finished child process. This helps the shell check if a command ran successfully or failed.
6.
It keeps spaces inside quotes while trimming extra spaces elsewhere. This ensures that commands like echo "hello world" keep the spaces inside the quotes as part of one argument.
7.
I switched from a list-based structure to a single command buffer (cmd_buff_t). The biggest challenge was handling spaces and quotes correctly while keeping the parsing clean.
8.
a)
Signals let processes communicate asynchronously (e.g., stop, continue, or kill a process). Unlike pipes or shared memory, signals don’t transfer data but send control messages.

b)
SIGKILL : Instantly kills a process (cannot be ignored).
SIGTERM : Requests a process to end (can be handled).
SIGINT : Interrupts a process (e.g., Ctrl+C to stop a program).

c)
SIGSTOP pauses a process. It cannot be caught or ignored, ensuring any process can be stopped if needed.
