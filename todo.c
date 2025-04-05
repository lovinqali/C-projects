#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#define MAX_FILES 10
#define TODO_DIR "/home/ali/challenges/todo/"

FILE *fptr;
char input[255], job[8] = "",string[255];
int val;
char listout[1000];
char* files[MAX_FILES];
int file = -1;

void init(void);
void list_available_files(void);
void cleanup(void);
void files_cmd();


char *concat(char buffer[511],char str1[255],char str2[255]) {
    int i = 0, j = 0;
    while (str1[i] != '\0') {
        buffer[i] = str1[i];
        i++;
    }
    while(str2[j] != '\0') {
        buffer[i] = str2[j];
        j++;
        i++;
    }
    buffer[i] = '\0';
    return buffer;
}

void selectfile(char string[255]) {
    int i = 0;
    char* header;
    char name[500];
    header = TODO_DIR;

    if (string[0] == '\0') {
        printf("Usage: select <filename>\n");
        sleep(1);
        return;
    }

    strcpy(name, concat(name, header, string));
    strcpy(name, concat(name, name, ".txt"));

    for(i=0; i < MAX_FILES ; i++) {
        if (files[i] != NULL && strcmp(name, files[i]) == 0) {
            file = i;
            return;
        }
    }

    printf("File '%s' not found.\n", string);
    list_available_files();
    sleep(2);
    return;
}


void create_file(char string[255]) {
    int i = 0;
    char* header;
    char name[511];
    int existing_index = -1;
    int empty_slot = -1;

    header = TODO_DIR;

    if (string[0] == '\0') {
        printf("Usage: create <filename>\n");
        sleep(1);
        return;
    }

    concat(name, header, string);
    concat(name, name, ".txt");

    for(i=0; i < MAX_FILES ; i++) {
        if (files[i] == NULL && empty_slot == -1) {
            empty_slot = i;
        }
        if (files[i] != NULL && strcmp(name, files[i]) == 0) {
            existing_index = i;
        }
    }

    if (existing_index != -1) {
        printf("File '%s' already exists.\n", string);
        file = existing_index;
        printf("Selected existing file.\n");
        sleep(1);
        return;
    }

    if (empty_slot == -1) {
        printf("No available space to create new file (max %d).\n", MAX_FILES);
        sleep(2);
        return;
    }

    fptr = fopen(name, "w");
    if (fptr == NULL) {
        perror("Error creating file on disk");
        sleep(2);
        return;
    }
    fclose(fptr);

    files[empty_slot] = malloc(strlen(name) + 1);
    if (files[empty_slot] == NULL) {
        printf("Error: Failed to allocate memory for filename!\n");

        sleep(2);
        return;
    }
    strcpy(files[empty_slot], name);
    file = empty_slot;
    printf("Created '%s'.\n", string);
    sleep(1);
    return;
}

int parse(char input[255], char job[8],char string[255]) {
    int i = 0, j = 0;
    while (input[i] != '\0' && input[i] == ' ') i++;

    while (input[i] != ' ' && input[i] != '\0') {
        if (j < 7) {
            job[j] = input[i];
            j++;
        }
        i++;
    }
    job[j] = '\0';

    while (input[i] != '\0' && input[i] == ' ') i++;

    j = 0;
    while (input[i] != '\0') {
        if (j < 254) {
            string[j] = input[i];
            j++;
        }
        i++;
    }
    string[j] = '\0';

    while(j > 0 && string[j-1] == ' ') {
        string[j-1] = '\0';
        j--;
    }


    if (job[0] != '\0' && string[0] == '\0') {
        return 1;
    } else if (job[0] != '\0' && string[0] != '\0') {
        return 0;
    } else {
        job[0] = '\0';
        string[0] = '\0';
        return 1;
    }
}

int remLine(char *filename, int line_to_remove) {
    if (filename == NULL) {
        fprintf(stderr, "remLine Error: filename is NULL.\n");
        return -1;
    }

    FILE *file_ptr_read;
    FILE *file_ptr_write;
    char temp_filename[300];
    char buffer[1024];
    int current_line = 0;
    int found = 0;

    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    file_ptr_read = fopen(filename, "r");
    if (file_ptr_read == NULL) {
        perror("remLine Error opening source file");
        fprintf(stderr, "Filename: %s\n", filename);
        return -1;
    }
    file_ptr_write = fopen(temp_filename, "w");
    if (file_ptr_write == NULL) {
        perror("remLine Error opening temp file");
        fclose(file_ptr_read);
        return -1;
    }


    while (fgets(buffer, sizeof(buffer), file_ptr_read) != NULL) {
        current_line++;
        if (current_line != line_to_remove) {
            if (fputs(buffer, file_ptr_write) == EOF) {
                perror("Error writing to temp file");
                fclose(file_ptr_read);
                fclose(file_ptr_write);
                remove(temp_filename);
                return -1;
            }
        } else {
            found = 1;
        }
    }

    fclose(file_ptr_read);
    fclose(file_ptr_write);

    if (!found && line_to_remove > 0) {
        printf("Warning: Line %d not found in file %s.\n", line_to_remove, filename);
        remove(temp_filename);
        return 1;
    } else if (found) {
        if (remove(filename) != 0) {
            perror("Error removing original file");
            return -1;
        }
        if (rename(temp_filename, filename) != 0) {
            perror("Error renaming temp file");
            fprintf(stderr, "CRITICAL ERROR: Original file '%s' deleted, but temp file '%s' could not be renamed.\n", filename, temp_filename);
            return -1;
        }
    } else {
        remove(temp_filename);
    }
    return 0;
}

void list_available_files() {
    printf("Available files:\n");
    int found_any = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i] != NULL) {
            char display_name[256];
            const char *start_of_base = strrchr(files[i], '/');

            if (start_of_base != NULL) {
                start_of_base++;
            } else {
                start_of_base = files[i];
            }
            strncpy(display_name, start_of_base, sizeof(display_name) - 1);
            display_name[sizeof(display_name) - 1] = '\0';
            char *dot = strstr(display_name, ".txt");
            if (dot != NULL && dot == display_name + strlen(display_name) - 4) {
                *dot = '\0';
            }
            printf("  %s\n", display_name);
            found_any = 1;
        }
    }
    if (!found_any) {
        printf("  (None found in %s)\n", TODO_DIR);
        printf("Use 'create <name>' to make one.\n");
    } else {
        printf("Use 'select <name>' to choose a file.\n");
    }
}


void listTasks() {
    if (file < 0 || file >= MAX_FILES || files[file] == NULL) {
        printf("No file selected.\n");
        list_available_files();
        return;
    }

    fptr = fopen(files[file], "r");
    if(fptr == NULL) {
        printf("Error: Cannot open currently selected file: %s\n", files[file]);
        free(files[file]);
        files[file] = NULL;
        file = -1;
        return;
    }

    const char *base_name_title = strrchr(files[file], '/');
    if (base_name_title != NULL) base_name_title++; else base_name_title = files[file];

    printf("Current File: %s\n-----------------------\n", base_name_title);
    int line_num = 1;
    while(fgets(listout, sizeof(listout), fptr)) {
        listout[strcspn(listout, "\n")] = 0;
        printf("%d: %s\n", line_num++, listout);
    }
    if (line_num == 1) {
        printf("(empty list)\n");
    }
    fclose(fptr);
}

void push(char *filename_unused, char string[255]) {
    if (file < 0 || file >= MAX_FILES || files[file] == NULL) {
        printf("Error: No file selected. Use 'select <name>'.\n");
        return;
    }
    if (string[0] == '\0') {
        printf("Usage: push <task description>\n");
        return;
    }
    fptr = fopen(files[file], "a");
    if(fptr == NULL) {
        printf("Error: Cannot open file for append: %s\n", files[file]);
        return;
    }
    fprintf(fptr, "%s\n", string);
    fclose(fptr);
}

void tick(char *filename_unused, char string[255]) {
    if (file < 0 || file >= MAX_FILES || files[file] == NULL) {
        printf("Error: No file selected. Use 'select <name>'.\n");
        return;
    }

    if (string[0] == '\0') {
        printf("Usage: tick <line number>.\n");
        return;
    }
    for(int k=0; string[k] != '\0'; k++) {
        if (string[k] < '0' || string[k] > '9') {
            printf("Error: '%s' is not a valid line number.\n", string);
            return;
        }
    }


    int line_to_remove = atoi(string);
    if (line_to_remove <= 0) {
        printf("Error: Line number must be positive.\n");
        return;
    }
    remLine(files[file], line_to_remove);
}

void print_help() {
    printf("Available commands:\n");
    printf("  files               - List available todo files\n");
    printf("  create <name>       - Creates a new todo list and selects it\n");
    printf("  select <name>       - Selects an existing list\n");
    printf("  push <description>  - Add task to the selected list\n");
    printf("  tick <line number>  - Remove task by line number from selected list\n");
    printf("  clear               - Clears all tasks from selected list\n");
    printf("  list                - Refresh and show tasks in current list\n");
    printf("  help                - Show this help message\n");
    printf("  quit                - Exit the program\n");
    printf("\nPress Enter to continue...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar();
}

void clear() {
    if (file < 0 || file >= MAX_FILES || files[file] == NULL) {
        printf("Error: No file selected. Use 'select <name>'.\n");
        return;
    }
    fptr = fopen(files[file], "w");
    if (fptr == NULL) {
        perror("Delete error opening file");
        return;
    }
    fclose(fptr);
    printf("Cleared content of %s\n", files[file]);
    sleep(1);
}

void delete(char* filename) {
    char name[511];
    int i = 0, j;
    int found_index = -1;
    concat(name, TODO_DIR, filename);
    concat(name, name, ".txt");
    for (i = 0; i<MAX_FILES ; i++) {
        if (files[i] != NULL && strcmp(name, files[i]) == 0) {
            free(files[i]);
            files[i] = NULL;
            found_index = i;
            break;
        }
    }
    if (found_index != -1) {
        for (j = found_index; j < MAX_FILES - 1; j++) {
            files[j] = files[j + 1];
        }
        files[MAX_FILES - 1] = NULL;
        if (file == found_index) {
            file = -1;
            printf("Note: The currently selected file was deleted.\n");
        } else if (file > found_index) {
            file--;
        }
    }
    if (remove(name) != 0) {
        fprintf(stderr, "Error deleting file '%s': ", name);
    } else {
        printf("Attempted to delete '%s'\n", name);
    }
    sleep(1);
    files_cmd();
}

void files_cmd() {
    char files_input[255];
    char files_job[8];
    char files_string[255];
    while (1) {
        printf("\033[2J\033[H");
        fflush(stdout);
        list_available_files();
        printf("-----------------------\n");
        printf("Select file -> ");
        fflush(stdout);
        if (fgets(files_input, sizeof(files_input), stdin) == NULL) {
            strcpy(job, "quit");
            printf("\nEOF detected. Quitting.\n");
            return;
        }
        files_input[strcspn(files_input, "\n")] = '\0';
        if (files_input[0] == '\0' || files_input[0] == ' ') {
            continue;
        }

        parse(files_input, files_job, files_string);
        if (strcmp(files_job, "select") == 0) {
            selectfile(files_string);
            if (file >= 0) {
                break;
            } else {
                continue;
            }
        } else if (strcmp(files_job, "create") == 0) {
            create_file(files_string);
            sleep(1);
        } else if (strcmp(files_job, "delete") == 0) {
            delete(files_string);
        } else if (strcmp(files_job, "quit") == 0) {
            strcpy(job, "quit");
            return;
        } else if (strcmp(files_job, "help") == 0) {
            print_help();
        }
        else {
            printf("Invalid command. Use 'select', 'create', 'delete', or 'quit'.\n");
            sleep(2);
        }
    }
}

void options(char job[8], char string[255]) {
    if (strcmp(job, "push") == 0 || strcmp(job, "add") == 0) {
        push(NULL, string);
    } else if (strcmp(job, "tick") == 0) {
        tick(NULL, string);
    } else if (strcmp(job, "help") == 0) {
        print_help();
    } else if (strcmp(job, "clear") == 0) {
        clear();
    } else if (strcmp(job, "create") == 0) {
        create_file(string);
    } else if (strcmp(job, "select") == 0) {
        selectfile(string);
    } else if (strcmp(job, "files") == 0) {
        files_cmd();
    } else if (strcmp(job, "delete") == 0) {
        delete(string);
    }
    else {
        if (job[0] != '\0') {
            printf("Unknown command: '%s'. Type 'help' for commands.\n", job);
            sleep(1);
        }
    }
}
void cleanup() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i] != NULL) {
            free(files[i]);
            files[i] = NULL;
        }
    }
}

void init() {
    file = -1;

    for(int i=0; i<MAX_FILES; ++i) {
        files[i] = NULL;
    }
    DIR *dir_stream;
    struct dirent *entry;
    int count = 0;
    char full_path[512];

    dir_stream = opendir(TODO_DIR);
    if (dir_stream != NULL) {
        errno = 0;
        while ((entry = readdir(dir_stream)) != NULL && count < MAX_FILES) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            size_t name_len = strlen(entry->d_name);
            if (name_len > 4 && strcmp(entry->d_name + name_len - 4, ".txt") == 0) {
                snprintf(full_path, sizeof(full_path), "%s%s", TODO_DIR, entry->d_name);
                files[count] = malloc(strlen(full_path) + 1);
                if (files[count] != NULL) {
                    strcpy(files[count], full_path);
                    count++;
                } else {
                    fprintf(stderr, "Failed malloc loading %s\n", full_path);
                }
            }
            errno = 0;
        }
        if (errno != 0 && entry == NULL) {
            perror("Error reading directory");
        }
        closedir(dir_stream);
    } else {
        perror("Could not open TODO directory");
        fprintf(stderr, "Directory path: %s\n", TODO_DIR);
    }
}
void run() {
    while (strcmp(job, "quit") != 0) {
    printf("\033[2J\033[H");
    fflush(stdout);
    listTasks();
    printf("-----------------------\n");
    printf("->");
    fflush(stdout);

    if (fgets(input, sizeof(input) ,stdin) == NULL) {
        strcpy(job, "quit");
        printf("\nEOF detected. Quitting.\n");
        continue;
    }
    input[strcspn(input, "\n")] = '\0';

    if (input[0] == '\0' || input[0] == ' ') {
        continue;
    }

    parse(input, job , string);

    if (strcmp(job,"quit") != 0) {
        options(job, string);
    }
    }
}

int main(){
    init();
    atexit(cleanup);
    printf("\033[2J\033[H");
    fflush(stdout);
    files_cmd();
    if (strcmp(job, "quit") != 0) {
        run();
    }
    printf("\033[2J\033[H");
    fflush(stdout);
    return 0;
}
