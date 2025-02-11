1.
fgets() is a good choice for reading user input because it reads the entire line at once. fgets() ensures that input does
not cause buffer overflows
2.
We use malloc() to allocate memory for cmd_buff because the shell needs flexibility in handling different input sizes.
If we used an fixed array, it will waste memory for small commands and risk buffer overflows for long commands.
3.
Trimming leading and trailing spaces ensure that parsed commands are clean and properly formatted before execution. 
For example " ls" will be seen as an invalid command because of that leading space. 
4.
a) Output Redirection Ex: ls -l > output.txt , challenge: must properly open and close the file descriptor 
   Input Redirection Ex: sort < data.txt , challenge: must open the file for reading and correctly replace STDIN
   Error Redirection Ex: gcc program.c 2> errors.log , challenge: must correctly differentiate STDERR from STDOUT
b) Key differences: 
   Redirection is more for commands to files
   Piping is more for commands to another command
c) Keeping STDERR and STDOUT seperate allows for better error handling, debugging, and output management
d) Our shell should handle errors by providing feedback and I do believe it should supporting merging STDOUT and STDERR in some cases.
   I do not know how to handle merging.