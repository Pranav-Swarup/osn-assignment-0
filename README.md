# Assignment 0

- Pranav Swarup Kumar 
- 2025121011

## Part 2 Discussion

In Task 2, the child process `writer` writes its parent's Process ID to `newfile.txt`.
Since the parent is active and waiting, the child sees the actual parent PID.

But in Task 3, the Parent exits without waiting for the child process. And hence, `orphans` it. When the child process goes to print the Parent's Process ID, it's different. In Linux, it was `PID: 1` and on WSL on my system, it showed `PID: 266`

This is because the `init/systemd` adopts the orphaned child process. This way the OS cleans up resources and the process table.

### -> Role of `init/systemd`

init/systemd is the first process started by the kernel at boot (PID 1).

One of it's job is to adopt orphaned processes when a parent dies, it becomes the new parent.It periodically calls `wait()` on its adopted children to clean up their resources and prevent zombies.

## Footnotes

As per the A.I Policy for the course, I have attached the PDF with all of the prompts and their respective outputs. I have also attached a sample input output PDF file. 
Within each directory, input.txt and output.txt contain the sequence of inputs and the overall output when the respective C-file is run. 

[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/WVXI1UQX)

#### Organisation: [IIIT-H](https://www.iiit.ac.in/)