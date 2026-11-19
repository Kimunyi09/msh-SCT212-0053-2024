			Custom Linux Mini-Shell
	
Nancy Wanjiku Kimunyi
SCT212-0053/20224

		Features Implemented
Task 1: Basic interactive read-eval-print loop using getline and clean EOF (Ctrl-D) handling — Complete
Task 2: Whitespace tokenization into a NULL-terminated argv array using strtok — Complete
Task 3: External command execution via fork, execvp, and waitpid — Complete
Task 4: Built-in commands (exit, help, and cd with HOME fallback) — Complete
Task 5: Dynamic exit status reporting in the prompt (msh[status]> ) using WIFEXITED and WIFSIGNALED — Complete
Task 6: Background job execution (&) with non-blocking zombie reaping (WNOHANG) — Complete
Task 7: Standard output redirection (>) to files using open and dup2 — Complete
Task 8: SIGINT (Ctrl-C) signal handling so interrupts terminate only foreground commands — Complete

		Design Explanation
msh executes external commands using the standard Unix fork–exec–wait lifecycle. After reading a line of input with getline and splitting it into an argument vector (argv) with strtok, the parent shell calls fork() to create an identical child process. Inside the child (pid == 0), msh configures any requested file redirections and signal dispositions before calling execvp(argv[0], argv). execvp searches the system PATH and replaces the child's memory image with the target executable. If execvp fails (such as when a command does not exist), the child reports the error with perror and terminates immediately using _exit(127) so it does not flush parent stdio buffers or enter the parent's prompt loop. Meanwhile, for foreground commands, the parent calls waitpid(pid, &status, 0) to pause until the child finishes and decodes its exit code or terminating signal into last_status.

Output redirection (>) is also performed exclusively inside the child process between fork() and execvp(). Opening the target file and calling dup2(fd, STDOUT_FILENO) in the child redirects only that command's standard output, leaving the parent shell's own stdout attached to the terminal for future prompts.

cd and exit must be implemented as shell built-ins rather than external programs because child processes inherit a copy of the parent's state and cannot modify the parent shell itself. If cd ran in a child process, chdir() would only change the child's working directory before the child exited, leaving the shell in its original directory. Similarly, calling exit() in a child would only terminate that child rather than closing the shell session.

Finally, msh prevents zombie processes in two ways: foreground commands are synchronously reaped via blocking waitpid(), while background commands (&) are reaped asynchronously at the top of every prompt cycle using a non-blocking while (waitpid(-1, &st, WNOHANG) > 0) loop.

Input				Expected behaviour		Actual Results
cd				take me back to /home/kmunyi	took me back to /home/kmunyi		
ls /nonexistent			Error from ls; non-zero		No such file or directory				
				status
nosuchcommand			Error message; status 127	No such file or directory
cd /tmp then pwd		Prints /tmp			the pwd after using cd is /tmp
echo hello world		Prints text; status 0		prints hello world
sleep 3 &			Prompt returns at once;		[started pid 236323]
				PID shown
echo hi > f.txt then cat f.txt 	File contains hi		after cat 'hi' is displayed
sleep 100 then Ctrl-C 		Sleep stops; shell survives;	exit status changes
				status 130
Ctrl-D at the prompt		Shell exits cleanly		shell exits cleanly
Press Enter on a blank line	Ignores empty input and cleanly msh>is redisplayed in next lin
				redisplays msh>	
help				Prints the list of supported 	built in commands(cd, 
				built-in commands		help,exit) are displayed


 







