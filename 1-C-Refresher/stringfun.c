#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);
//prototypes for functions to handle required functionality
int count_words(char *, int, int);
void reverse_string(char *, int);
void print_words(char *, int, int);
int replace_string(char *, int, char *, char *);

// Implementation of setup_buff()
int setup_buff(char *buff, char *user_str, int len) {
    // Copy user_str to buff and removes extra whitespace
    char *src = user_str;
    char *dst = buff;
    int count = 0;
    int was_whitespace = 1;

    while (*src && count < len) {
        if (*src != ' ' && *src != '\t') {
            *dst++ = *src;
            count++;
            was_whitespace = 0;
        } else if (!was_whitespace) {
            *dst++ = ' ';
            count++;
            was_whitespace = 1;
        }
        src++;
    }

    if (*src) return -1;  // Input string is too long

    while (count < len) {
        *dst++ = '.';
        count++;
    }

    return dst - buff;  // Return the length of the user string
}

// Implementation of print_buff()
void print_buff(char *buff, int len) {
    printf("Buffer:  ");
    for (int i = 0; i < len; i++) {
        putchar(*(buff + i));
    }
    putchar('\n');
}

// Implementation of usage()
void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

// Implementation of count_words()
int count_words(char *buff, int len, int str_len) {
    int count = 0;
    int in_word = 0;

    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) != ' ' && *(buff + i) != '.') {
            if (!in_word) {
                in_word = 1;
                count++;
            }
        } else {
            in_word = 0;
        }
    }

    return count;
}

// Implementation of reverse_string()
void reverse_string(char *buff, int str_len) {
    char *start = buff;
    char *end = buff + str_len - 1;
    char temp;

    while (start < end) {
        temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

// Implementation of print_words()
void print_words(char *buff, int len, int str_len) {
    printf("Word Print\n");
    printf("----------\n");

    int word_start = 0;
    int word_length = 0;
    int word_index = 1;

    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) != ' ' && *(buff + i) != '.') {
            if (word_length == 0) {
                word_start = i;
            }
            word_length++;
        } else if (word_length > 0) {
            printf("%d. ", word_index++);
            for (int j = 0; j < word_length; j++) {
                putchar(*(buff + word_start + j));
            }
            printf(" (%d)\n", word_length);
            word_length = 0;
        }
    }

    if (word_length > 0) {
        printf("%d. ", word_index++);
        for (int j = 0; j < word_length; j++) {
            putchar(*(buff + word_start + j));
        }
        printf(" (%d)\n", word_length);
    }
}

// Implementation of replace_string()
int replace_string(char *buff, int len, char *search, char *replace) {
    char temp[BUFFER_SZ]; // Temporary buffer to construct the new string
    char *buff_ptr = buff; // Pointer to traverse the main buffer
    char *temp_ptr = temp; // Pointer to construct the temporary buffer
    char *search_start = NULL; // Pointer to the start of the search string in the buffer
    char *search_ptr, *replace_ptr; // Pointers for search and replace strings

    // Locate the search string in the buffer
    while (*buff_ptr != '\0' && buff_ptr < buff + len) {
        search_ptr = search;
        search_start = buff_ptr;

        // Check if the search string matches at the current position
        while (*buff_ptr == *search_ptr && *search_ptr != '\0' && buff_ptr < buff + len) {
            buff_ptr++;
            search_ptr++;
        }

        // If the search string is found, break
        if (*search_ptr == '\0') {
            break;
        }

        // Otherwise, reset and move to the next character
        search_start = NULL;
        buff_ptr++;
    }

    // If the search string is not found
    if (search_start == NULL) {
        printf("Word not found.\n");
        return -1;
    }

    // Copy everything before the search string to temp
    buff_ptr = buff;
    while (buff_ptr < search_start && temp_ptr < temp + len) {
        *temp_ptr++ = *buff_ptr++;
    }

    // Copy the replacement string to temp
    replace_ptr = replace;
    while (*replace_ptr != '\0' && temp_ptr < temp + len) {
        *temp_ptr++ = *replace_ptr++;
    }

    // Skip the search string in the buffer
    buff_ptr = search_start;
    search_ptr = search;
    while (*search_ptr != '\0' && buff_ptr < buff + len) {
        buff_ptr++;
        search_ptr++;
    }

    // Copy everything after the search string to temp
    while (*buff_ptr != '\0' && temp_ptr < temp + len) {
        *temp_ptr++ = *buff_ptr++;
    }

    
    while (temp_ptr < temp + len) {
        *temp_ptr++ = '.';
    }

    // Copy temp back to the original buffer
    temp_ptr = temp;
    buff_ptr = buff;
    while (buff_ptr < buff + len) {
        *buff_ptr++ = *temp_ptr++;
    }

    return 0;
}


int main(int argc, char *argv[]) {
    char *buff;
    char *input_string;
    char opt;
    int rc;
    int user_str_len;

    // TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    // Answer: The if statement ensures argc is >= 2, so argv[1] exists. 
    // The condition *argv[1] != '-' validates the first argument starts with '-'.
    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = *(argv[1] + 1);

    // Handle the help flag
    if (opt == 'h') {
        usage(argv[0]);
        exit(0);
    }

    // TODO:  #2 Document the purpose of the if statement below
    // Answer: Makes the user provide a second argument, which is the required input string for processing.
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2];

    // TODO #3: Allocate space for the buffer
    buff = malloc(BUFFER_SZ);
    if (!buff) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0) {
        printf("Error setting up buffer, error = %d\n", user_str_len);
        free(buff);
        exit(3);
    }

    switch (opt) {
        case 'c':  // Count words
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            printf("Word Count: %d\n", rc);
            break;

        case 'r':  // Reverse string
            reverse_string(buff, user_str_len);
            printf("Reversed String: ");
            for (int i = 0; i < user_str_len; i++) {
                putchar(*(buff + i));
            }
            putchar('\n');
            break;

        case 'w':  // Print words
            print_words(buff, BUFFER_SZ, user_str_len);
            break;

        case 'x':  // Replace string
            if (argc < 5) {
                printf("Error: Insufficient arguments for -x option.\n");
                usage(argv[0]);
                free(buff);
                exit(1);
            }
            rc = replace_string(buff, BUFFER_SZ, argv[3], argv[4]);
            if (rc == 0) {
                printf("Modified String: %s\n", buff);
            }
            break;

        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    // TODO #6: Free the buffer before exiting
    print_buff(buff, BUFFER_SZ);
    free(buff);
    return 0;
}

// TODO #7: Why pass both pointer and length?
// Answer: Passing both ensures the function works with dynamic memory and avoids buffer overruns.
// The length sets a clear limit, making the code safer and avoiding buffer overflows.
