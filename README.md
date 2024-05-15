# Project 1 - Operating Systems

## Option 2

Recursive file copy program [24p]
The program takes two arguments: a source directory and a destination directory. The destination directory cannot exist. The program copies all the files from the source directory to the destination directory. If a source directory contains a directory, this directory is copied to the destination together with the contents. This procedure is applied recursively.

The SIGINT signal (Ctrl + C) should stop the currently performed copy operation and delete the incomplete destination file. This situation should be logged in the system log (syslog).

[10p] Additional option -a preserves the attributes of files. That is all the attributes of files in a destination directory (owner, group, access mode, three access times) are the same as the attributes of corresponding files in the source directory.

## Team

- Jorge Ortega Izquierdo
- Adrián Anta Gil

## How to run program

- Step 1

  ```bash
  gcc -o recursive_copy recursive_copy.c
  ```

- Step 2

  Note that if `./test_dir/destination` already exists you shoukd either delete it or change the destination directory.

  ```bash
  ./recursive_copy ./test_dir/source ./test_dir/destination
  ```

## Explanation

The program takes 2 arguments, a source directory (to copy) and a destination directory (that doesn't exit), the source will be copied in the destination directory and each directory in the source will be copied using the same function recursively.
