# PennOS (23sp-pennOS-group-24)
### Authors

| Name | PennKey |
| --- | --- |
| Ruifan Wang | wang321 |
| Jiaqi Xie | jiaqixie |
| Zhiyuan Liang  | liangzhy |
| Shuo Sun | [PennKey] |


### Source Files
    
    ├── bin/                     # Compiled binaries
    |    |── pennFAT
    |    └── pennOS
    ├── src/                     # Source code
    |    |── kernel/         
    |    |      |── behavior.c
    |    |      |── behavior.h
    |    |      |── global.c
    |    |      |── global.h
    |    |      |── global2.h
    |    |      |── job.c
    |    |      |── job.h
    |    |      |── kernel.c
    |    |      |── kernel.h
    |    |      |── log.c
    |    |      |── log.h
    |    |      |── parser.h
    |    |      |── parser.c
    |    |      |── perrno.h
    |    |      |── perrno.c
    |    |      |── programs.c
    |    |      |── programs.h
    |    |      |── scheduler.c
    |    |      |── scheduler.h
    |    |      |── shell.c
    |    |      |── shell.h     
    |    |      |── stress.c
    |    |      |── stress.h     
    |    |      |── user.c
    |    |      |── user.h   
    |    |      |── utils.c
    |    |      └── utils.h  
    |    └── PennFAT/
    |           |── FAT.c
    |           |── FAT.h
    |           |── fd-table.c
    |           |── fd-table.h     
    |           |── filesys.c
    |           |── filesys.h     
    |           |── interface.c
    |           |── interface.h   
    |           |── pennFAT.c
    |           |── pennFAT.h  
    |           |── utils.c
    |           └── utils.h    
    ├── doc/                     # Documentation files 
    |    └── doc.pdf
    ├── log/                     # PennOS logs
    |    └── log.txt
    └── README.md

### Extra Credit Answers

- N/A 
  
### Compilation Instructions

* Compile by running (make sure you are in the root directory)
```
make
```
* Run the PennOS
```
./bin/pennOS [filesystem] [logfile]
```
* Or the PennFAT
```
./bin/pennFAT
```

### Additional Logging Events
- DQ_READY_SCHD: removed a process from the ready queue. 
- EQ_READY_TOUT: added a process to the ready queue. 

### Overview of Work Accomplished

PennOS is an UNIX-like operating system created by our team. It has a file system called PennFAT, a priority-based scheduler, and allows users to interact with it through a shell. There are two states in PennOS: kernel land and user land, which can be switched using custom system calls. The priority scheduler takes 3-level priorities into account and is implemented using the ucontext library and alarm clock. Users can interact with PennOS using a basic shell that accepts commands and built-ins. The system handles signals from the host operating system and provides a custom signal command called "kill." Additionally, it includes logging functionality for both the scheduler and kernel level functions, and an error handling mechanism. A Companion Document in PDF format located in the ./doc directory provides more detailed information about the OS API and functionality. PennOS also supports a FAT file system, allowing basic interactions between the scheduler and the file system.

### Shell Built-in Functions

### Description of Code and Code Layout

[Provide a detailed description of the code, including any algorithms or data structures used. Explain the layout of the code, including the purpose of each file and how they interact with each other.]

### General Comments

[Include any general comments or observations about your code, including any challenges that you faced while working on the project. This can help us better understand your thought process and approach to the project.]

