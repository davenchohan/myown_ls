/*
 * Filename: myls.c
 * Description: myls command
 * Date: July 19, 2022
 * Name: Daven Chohan
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <time.h> 

// Function Declarations
int file_exists(char* file_name, int count, int index, int long_list, int recursive, _Bool last);
void get_mode(int st_mode, char str_mode[]);
void print_output(struct dirent *dir, int index, int long_list, int recursive, char* directory);
void ls(char* directory, int count, int index, int long_list, int recursive, _Bool last);
void prepare_ls(char** arguments, int count, int index, int long_list, int recursive);


void get_mode(int st_mode, char str_mode[]){
    strcpy(str_mode, "----------"); // Default
    //File type
    if (S_ISDIR(st_mode))
    {
        str_mode[0] = 'd'; // If file is a directory
    }
    else if (S_ISCHR(st_mode))
    {
        str_mode[0] = 'c'; // If file is a character device
    }
    else if (S_ISBLK(st_mode))
    {
        str_mode[0] = 'b'; // If file is a block device
    }
    else if (S_ISLNK(st_mode))
    {
        str_mode[0] = 'l'; // If file is a symbolic link
    }

    //Owner
    if (st_mode & S_IRUSR)
    {
        str_mode[1] = 'r';
    }
    if (st_mode & S_IWUSR)
    {
        str_mode[2] = 'w';
    }
    if (st_mode & S_IXUSR)
    {
        str_mode[3] = 'x';
    }

    //Group
    if (st_mode & S_IRGRP)
    {
        str_mode[4] = 'r';
    }
    if (st_mode & S_IWGRP)
    {
        str_mode[5] = 'w';
    }
    if (st_mode & S_IXGRP)
    {
        str_mode[6] = 'x';
    }

    //Others
    if (st_mode & S_IROTH)
    {
        str_mode[7] = 'r';
    }
    if (st_mode & S_IWOTH)
    {
        str_mode[8] = 'w';
    }
    if (st_mode & S_IXOTH)
    {
        str_mode[9] = 'x';
    }
}

void print_output(struct dirent *dir, int index, int long_list, int recursive, char* directory){
    char* d_name = dir->d_name;
    if (index==1)
    {
        printf("%d\t", (int) dir->d_ino);
    }
    if (long_list==1)
    {
        struct stat sb;
        char* temp_directory = malloc((strlen(directory)) + 1024);
        strcpy(temp_directory,directory);
        char slash[2];
        strcpy(slash, "/");
        if (temp_directory[strlen(temp_directory)-1] != '/')
        {
            strcat(temp_directory, slash);
        }
        strcat(temp_directory, d_name);
        if (lstat(temp_directory, &sb) == -1) { // Using lstat because of symbolic links
            if (lstat(d_name, &sb) == -1) {
                perror("stat");
            }
        }
        char str_mode[11];
        get_mode(sb.st_mode, str_mode);
        // Print file type and mode
        printf("%s\t", str_mode);
        // Print number of hard links
        printf("%ld\t", sb.st_nlink);
        // Print the username of the owner of the file
        struct passwd *pw = getpwuid(sb.st_uid);
        printf("%s\t", pw->pw_name);
        // Print the group of the file
        struct group *grp = getgrgid(sb.st_gid);
        printf("%s\t", grp->gr_name);
        // Print the size of the file
        printf("%ld\t", sb.st_size);
        // Print the last modified date
        char time[250];
        struct tm *tm;
        tm = localtime(&sb.st_mtime);
        strftime(time, sizeof(time), "%b %e %Y %H:%M", tm);
        printf("%s\t", time);
        free(temp_directory);
    }
}

void recursive_call(char* directory, int index, int long_list, int recursive){
    if (recursive == 1)
    {
        struct dirent* dir;
        DIR *d = opendir(directory);
        struct dirent **outputs;

        char* temp_directory = malloc((strlen(directory)) + 1024);
        strcpy(temp_directory,directory);
        char slash[2];
        strcpy(slash, "/");
        if (temp_directory[strlen(temp_directory)-1] != '/')
        {
            strcat(temp_directory, slash);
        }

        int files = scandir(temp_directory, &outputs, 0, alphasort);
        if (files < 0)
            perror("scandir");
        else {
            for (int i = 0; i < files; i++) {
                if (outputs[i]->d_name[0] != '.')
                {
                    if (outputs[i]->d_type == DT_DIR){
                        char* temp_directory2 = malloc((strlen(temp_directory)) + 1024);
                        strcpy(temp_directory2,temp_directory);
                        strcat(temp_directory2, outputs[i]->d_name);
                        printf("\n");
                        ls(temp_directory2, 2, index, long_list, recursive, 1);
                        free(temp_directory2);
                    }
                }
                free(outputs[i]);
            }
        }
        free(outputs);
        free(temp_directory);
        closedir(d);
    }
}

int file_exists(char* file_name, int count, int index, int long_list, int recursive, _Bool last){
    struct dirent* dir;
    DIR *d = opendir(".");
    while((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, file_name) == 0)
        {
            if (count > -1)
            {
                print_output(dir, index, long_list, recursive, file_name);
                printf("%s\n", dir->d_name);
            }
            if (!last && count > 1)
            {
                printf("\n");
            }
            if (dir->d_type == DT_REG)
            {
                closedir(d);
                return 1;
            }
            closedir(d);
            return 0;
        }
    }
    closedir(d);
    char* single_file = strrchr(file_name, '/');
    if (single_file == NULL)
    {
        if (count > -1)
        {
            printf("%s\n", "Error : Nonexistent files or directories");
        }
        return 0;
    }
    char* temp_file_name = strdup(file_name);
    char* split_dir = strstr(temp_file_name, single_file);
    char* single_file2 = strdup(single_file);
    *split_dir = '\0';
    int file_flag = 0;
    if (single_file2[0] == '/') // If the file string starts with '/' ignore the '/' when searching
    {
        single_file2++;
        file_flag = 1;
    }
    d = opendir(temp_file_name);
    while((dir = readdir(d)) != NULL) {
        char* d_name = dir->d_name;
        if (strcmp(d_name, single_file2) == 0)
        {
            if (file_flag == 1)
            {
                single_file2--;
            }
            if (count > -1)
            {
                print_output(dir, index, long_list, recursive, temp_file_name);
                printf("%s", temp_file_name);
                printf("%s\n", single_file2);
            }
            free(single_file2);
            if (!last && count > 1)
            {
                printf("\n");
            }
            if (dir->d_type == DT_REG)
            {
                closedir(d);
                free(temp_file_name);
                return 1;
            }
            closedir(d);
            free(temp_file_name);
            return 0;
        }
    }
    closedir(d);
    if (file_flag == 1)
    {
        single_file2--;
    }
    free(single_file2);
    free(temp_file_name);
    if (count > -1)
    {
        printf("%s\n", "Error : Nonexistent files or directories");
    }
    //exit(EXIT_FAILURE);
    return 0;
}

//Learned directory types from: http://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
//Learned about file permissions from: https://linuxhint.com/linux_file_permissions/
//Learned about scandir from: https://linux.die.net/man/3/scandir
//Learned about stat from: https://www.man7.org/linux/man-pages/man2/stat.2.html
//Learned about file type and mode from: https://www.man7.org/linux/man-pages/man7/inode.7.html
//Learned about getting username and group from infodemo.c provided by Hazra Imran
//Learned about strftime() from: https://linux.die.net/man/3/strftime
void ls(char* directory, int count, int index, int long_list, int recursive, _Bool last){
    struct dirent* dir;
    DIR *d = opendir(directory);
    if (!d)
    {
        file_exists(directory, count, index, long_list, recursive, last);
    } else {
        if (count > 1)
        {
            printf("%s%c\n", directory, ':');
        } 
        struct dirent **outputs;

        int files = scandir(directory, &outputs, 0, alphasort);
        if (files < 0)
            perror("scandir");
        else {
            for (int i = 0; i < files; i++) {
                if (outputs[i]->d_name[0] != '.')
                {
                    print_output(outputs[i], index, long_list, recursive, directory);
                    printf("%s\n", outputs[i]->d_name);
                }
                free(outputs[i]);
            }
            recursive_call(directory, index, long_list, recursive);
        }
        free(outputs);
        if (!last && count > 1)
        {
            printf("\n");
        } 
        closedir(d);
    }
}

int sort_helper (const void *x, const void *y) {
    char * char_x = (char*)x;
    char * char_y = (char*)y;
    int flag_x = 0;
    int flag_y = 0;

    int is_x_file = file_exists(char_x, -1, 0, 0, 0, 1);
    int is_y_file = file_exists(char_y, -1, 0, 0, 0, 1);

    int return_value;
    // Print files before directories just like the linux command ls does
    if (is_x_file == 1 && is_y_file == 1) // If both are files sort lexicographically
    {
        if (char_x[0] == '/') // If the address starts with '/' ignore the '/' when sorting
        {
            char_x[0] = char_x[1];
            flag_x = 1;
        }
        if (char_y[0] == '/')
        {
            char_y[0] = char_y[1];
            flag_y = 1;
        }
        return_value = strcmp(char_x ,char_y);
        if (flag_x == 1)
        {
            char_x[0] = '/';
        }
        if (flag_y == 1)
        {
            char_y[0] = '/';
        }
    }
    else if (is_y_file == 1)
    {
        return_value = 1;
    }
    else if (is_x_file == 1)
    {
        return_value = -1;
    }
    else { 
        return_value = strcmp(x ,y); // If both are not files sort lexicographically
    }

    return return_value; 
}  

void prepare_ls(char** arguments, int count, int index, int long_list, int recursive) {
    int max_arg_size = 1024;
    char array[count][max_arg_size];

    for(int i=0;i<count;i++){
      strcpy(array[i], arguments[i]);
    }

    qsort(array, count, max_arg_size, sort_helper);

    for (int i = 0; i < count; ++i)
    {
        if ((i+1)==count)
        {
            ls(array[i], count, index, long_list, recursive, 1); // Do not print a \n at the end
        } else {
            ls(array[i], count, index, long_list, recursive, 0); 
        }
    }
}

//Main function to run the program
void main(int argc, char *argv[]){
    char** arguments = malloc(argc*sizeof(char*)); //Directories or files
    int index = 0; // -i parameter
    int long_list = 0; // -l parameter
    int recursive = 0; // -R parameter
    int count = 0; // Number of directories or files
    for (int i = 0; i < argc-1; ++i)
    {
        char* argument = argv[i+1];
        if (argument[0] == '-')
        {
            for (int j = 1; j < strlen(argument); ++j)
            {
                if (argument[j] == 'i')
                {
                    index = 1;
                }
                else if (argument[j] == 'R')
                {
                    recursive = 1;
                }
                else if (argument[j] == 'l')
                {
                    long_list = 1;
                }        
                else 
                {
                    printf("%s\n", "Error : Unsupported Option");
                    free(arguments);
                    exit(EXIT_FAILURE);
                }
            }
        } 
        else 
        {
            count++;
            arguments[count-1] = argv[i+1];
        }

    }
    if (count != 0) //Just list the directory the program currently is in
    {
        prepare_ls(arguments, count, index, long_list, recursive);
    }
    else {
        count++;
        arguments[0] = ".";
        prepare_ls(arguments, count, index, long_list, recursive);
    }
    free(arguments);
    return;
}


