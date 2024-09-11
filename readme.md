# Word Counter README

## Overview

This C program implements a word counter that automatically counts the number of words in text files (.txt) located in a specified directory. The program utilizes a parent-child process model, where the parent process reads the text files and the child process counts the words using a shared memory mechanism.

## Features

- Counts the number of words in `.txt` files within a specified directory.
- Utilizes two processes: a parent process for file handling and a child process for word counting.
- Communicates between processes using shared memory.

## Requirements

- A C compiler (e.g., gcc)
- POSIX compliant operating system (Linux/Unix)

## Installation

1. Clone the repository:
2. Compile the program:

   ```bash
   gcc problem2.c helpers.c -o problem2 -I. -pthread
   ```
3. Run the program:
    ```bash
    ./problem2 <directory_path>
    ```
Replace `<directory_path>` with the path to the directory containing the `.txt` files you want to analyze.

## Functionality

1. **Parent Process**:
   - Reads all `.txt` files in the specified directory.
   - Passes the file names to the child process through shared memory.

2. **Child Process**:
   - Receives the file names from the parent process.
   - Calls the `wordCount()` function defined in `helpers.c` to count the words in each file.
   - Outputs the total word count for each file.

## Directory Traversal

The program utilizes the `traverseDir()` function to find all text files under the specified directory. This function employs the `dirent` structure to recursively traverse the directory. When `dirent->d_type` indicates a folder, it performs a recursive traversal. If it encounters a file, it checks for the `.txt` extension and adds the file path to a global array `char file_paths[100][PATH_MAX]`.

## Process Synchronization

Synchronization between the parent and child processes is achieved using two semaphores, `sem_parent` and `sem_child`, similar to the Readers-Writers problem. Before reading a file from `file_paths[i]`, the parent waits on `sem_parent` to ensure no other process is accessing shared memory. After reading the file path into shared memory, it signals the child process. The child then waits for `sem_child` to read from shared memory and count the words, clearing the memory before signaling the parent again for the next iteration.

## Handling Large Files

To manage cases where the total size of a text file exceeds the buffer size, the program uses the `fileLength()` helper function to calculate the file size. If the size exceeds `SHARED_MEMORY_SIZE` (1MB), it reads the file in chunks. The parent process uses a local array `char buffer[SHARED_MEMORY_SIZE]` along with `size_t` variables `offset` and `chunk_size`. It enters a loop to read data in 1MB chunks, using `memcpy` to write to shared memory. After each chunk, the child counts the words and clears the memory, and the parent updates the `offset` using `fseek()` to continue reading until the entire file is processed.

## Implemented Functions
- **int main(int argc, char *argv)**: 

Initializes semaphores, shared memory, and other necessary variables. It forks into a parent and child process. The parent reads file contents into shared memory, while the child counts words and writes results to `p2_result.txt`.

- **void traverseDir(char *dir_name)**: 
Recursively traverses the directory, collecting paths of all text files (with a `.txt` extension) into the `file_paths` array and tracking the number of files in `file_count`.