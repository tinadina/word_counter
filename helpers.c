#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
* Counts the words in the string in a simple manner.
* The counting is done by looking for space and newline 
* characters until the string terminator is reached.
*
* @param text: the string to count the words in.
* 
* @returns the total number of words in the string.
*/
int wordCount(char *text) {
	int counter = 0;
	int i=0;
	char pre_char = text[i];
	while(text[i] != '\0'){
		if ((text[i] == ' ' || text[i] == '\n') 
				&& (pre_char != ' ' && pre_char != '\n')){
			counter++;
		}

		pre_char = text[i];
		i++;
	}

	// Count the last word in the text if it does not 
	// end with a space or newline.
    if (pre_char != ' ' && pre_char != '\n' && i != 0){
        counter++;
    }

	return counter;
}


/**
* Checks if the <input_file> is a txt file or not
* by looking for '.txt' at the end.
*
* @param fileName: the name of the file to be checked.
* 
* @returns 1 if file is a .txt file and returns 0 if not.
*/
int validateTextFile(char *fileName) {
	int len = strlen(fileName);
	if (len < 4) {
		return 0;
	} else {
		char *lastFoure = &fileName[len-4];
		if(strcmp(lastFoure, ".txt") == 0){
			return 1;
		}
		return 0;
	}
	
}


/**
* Returns the total size (length of the file), 
* meaning the total number of chars in the file.
*
* @param file: file which length of chars will be calculated.
* 
* @returns length of file in chars (long).
*/
long fileLength(FILE *file) {

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);
	return length;
}

/**
 * Saves a result with an int type to a given textfile.
 * 
 * @param fileName: The textfile name to save the result.
 * Please just pass the textfile name so that it can be
 * located in the same directory as the executable.
 * 
 * @param result: The value with long int type to be saved.
 * Don't worry about that the case that the result is a
 * value like 0xxxxxxxx, which can be obtained, for example,
 * by giving your program a number 1xxxxxxxx and do the 
 * addition operation for 9 times. The grading team will not 
 * test your program with this special case.
 */

void saveResult(char *fileName, long int result)
{
	FILE * fp;

   fp = fopen (fileName, "w");
   fprintf(fp, "%ld", result);

   fclose(fp);
}