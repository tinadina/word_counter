#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/shm.h>
#include <errno.h>
#include <assert.h>
#include <dirent.h>
#include <limits.h>

#include "helpers.h"

#define PARENT_SEMAPHORE "/parent_semaphore_37_Dina"
#define CHILD_SEMAPHORE "/child_semaphore_37_Dina"
#define SHARED_MEMORY_SIZE 1048576
/**
 * @brief This function recursively traverse the source directory.
 * 
 * @param dir_name : The source directory name.
 */
void traverseDir(char *dir_name);  

// char Array to keep the file paths of a given directory and their count.
char file_paths[100][PATH_MAX];
int file_count = 0;

sem_t *sem_parent;
sem_t *sem_child;
char *shared_memory;

int main(int argc, char **argv) {
	int process_id; // Process identifier 
	
    // The source directory. 
    // It can contain the absolute path or relative path to the directory.
	char *dir_name = argv[1];

	if (argc < 2) {
		printf("Main process: Please enter a source directory name.\nUsage: ./main <dir_name>\n");
		exit(-1);
	}
 
  //if semaphores exists, unlink them.
  sem_unlink(PARENT_SEMAPHORE);
  sem_unlink(CHILD_SEMAPHORE);
 
  //creating Shared Memory of size 1MB
  int shm_id = shmget(IPC_PRIVATE, SHARED_MEMORY_SIZE , 0666|IPC_CREAT);
  if (shm_id == -1) {
      printf("shmget error");
      exit(-1);
  }
  
  shared_memory = (char *)shmat(shm_id, NULL, 0); 
  
  if (shared_memory == (char *)(-1)) {
      printf("shmat error");
      exit(-1);
  }
  
  sem_parent = sem_open(PARENT_SEMAPHORE, O_CREAT | O_EXCL, S_IRUSR|S_IWUSR, 1);
  if (sem_parent == SEM_FAILED) {
      printf("sem_open/sem_parent error");
      exit(-1);
  }
  sem_child = sem_open(CHILD_SEMAPHORE, O_CREAT | O_EXCL, S_IRUSR|S_IWUSR, 0);
  if (sem_child == SEM_FAILED) {
      printf("sem_open/sem_child error");
      exit(-1);
  }
  
  //Collect all text file paths in char array
	traverseDir(dir_name);
	
  switch (process_id = fork()) {

	default:
		/*
			Parent Process
		*/
		printf("Parent process: My ID is %jd\n", (intmax_t) getpid());
   
    int i = 0;
    while (1) {
        sem_wait(sem_parent);
        
        //Parent proces reads the array file path content and writes it to shared memory.
        FILE *file = fopen(file_paths[i], "r");
        if (file == NULL) {
            fprintf(stderr, "Failed to open file: %s\n", file_paths[i]);
            continue;
        }
        
        long file_size = fileLength(file);
        
        if (file_size > SHARED_MEMORY_SIZE) {
          
            //If file_size is greater than size of shared memory
            //we divide the file to chunks and save them to buffer
            char buffer[SHARED_MEMORY_SIZE];
            size_t offset = 0;
            size_t chunk_size;
  
            while ((chunk_size = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
                //Write current chunk to shared memory and pass it to chil process
                memcpy(shared_memory , buffer, chunk_size);
                sem_post(sem_child);
                sem_wait(sem_parent);
                
                //Move to next chunk in the file
                offset += chunk_size;
                fseek(file, offset, SEEK_SET);
            }
        }
        else {
            //If file_size is less than or equal to share memory size
            //we directly write the file content into shared memory
            size_t bytes_read = fread(shared_memory, sizeof(char), SHARED_MEMORY_SIZE, file);
            if (bytes_read == 0) {
                fprintf(stderr, "Failed to read file: %s\n", file_paths[i]);
            } else {
                //signal child that file content is written into shared memory.
                sem_post(sem_child);
            }
        }
        
        fclose(file);
    
        i++;
        if (i >= file_count) {
            break;
        }
    }
    
    //Signal Child that execution is over
    sem_post(sem_child);

    //Close and delete semaphore as well as shared memory
    sem_close(sem_parent);
    sem_close(sem_child);
    sem_unlink(PARENT_SEMAPHORE);
    sem_unlink(CHILD_SEMAPHORE);
    shmdt(shared_memory);
    shmctl(shm_id, IPC_RMID, NULL);

		printf("Parent process: Finished.\n");
		break;

	case 0:
		/*
			Child Process
		*/

		printf("Child process: My ID is %jd\n", (intmax_t) getpid());
    int totalWords = 0;

    //Open p2_result.txt in order to write the totalWords count
    FILE* result_file_child = fopen("p2_result.txt", "w");
    if (result_file_child == NULL) {
        printf("Failed to open result file");
        exit(-1);
    }

    while (1) {
        sem_wait(sem_child);
        
        //If shared memory is empty, then parent finished reading file paths
        if (shared_memory[0] == '\0') {
            sem_post(sem_parent);
            break;
        }
        
        //Count words in shared memory and add up to word counter
        totalWords += wordCount(shared_memory);
        
        //Clear the shared memrory and signal parent so it can write another file
        memset(shared_memory, 0, SHARED_MEMORY_SIZE);
        sem_post(sem_parent);
        
    }
    
    //Write totalWords and close file
    fprintf(result_file_child, "%d", totalWords);
    fclose(result_file_child);
    
		printf("Child process: Finished.\n");
		exit(0);

	case -1:
		/*
		Error occurred.
		*/
		printf("Fork failed!\n");
		exit(-1);
	}

	exit(0);
}

/**
 * @brief This function recursively traverse the source directory.
 * 
 * @param dir_name : The source directory name.
 */
 
void traverseDir(char *dir_name) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(dir_name);
    if (dir == NULL) {
        perror("Unable to open directory");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) {
        //if directory is readed
        if (entry->d_type == DT_DIR) {
            //skip directories "." and ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            //Recursive traverse
            char sub_dir_path[PATH_MAX];
            snprintf(sub_dir_path, sizeof(sub_dir_path), "%s/%s", dir_name, entry->d_name);
            traverseDir(sub_dir_path);
            
        }
        //if file is readed
        else if (entry->d_type == DT_REG) {
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir_name, entry->d_name);
            
            //Add file to array only if it is .txt file
            if (strstr(entry->d_name, ".txt") != NULL) {
                if (file_count < 100) {
                    strcpy(file_paths[file_count], file_path);
                    file_count++;
                } else {
                    fprintf(stderr, "Maximum files exceeded. Skipping file: %s\n", file_path);
                }
            }
        }
    }

    closedir(dir);
}