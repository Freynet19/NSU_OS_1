#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>

void createDir(char *argv[]);
void listDir(char *argv[]);
void removeDir(char *argv[]);
void createFile(char *argv[]);
void catFile(char *argv[]);
void removeFile(char *argv[]);
void createSymlink(char *argv[]);
void readSymlink(char *argv[]);
void followSymlink(char *argv[]);
void removeSymlink(char *argv[]);
void createHardlink(char *argv[]);
void removeHardlink(char *argv[]);
void statFile(char *argv[]);
void chmodFile(char *argv[]);

typedef struct {
    const char *name;
    void (*function)(char *argv[]);
    int requiredArgs;
} Command;

Command commandMap[] = {
    {"create_dir", createDir, 1},
    {"list_dir", listDir, 1},
    {"remove_dir", removeDir, 1},
    {"create_file", createFile, 1},
    {"cat_file", catFile, 1},
    {"remove_file", removeFile, 1},
    {"create_symlink", createSymlink, 2},
    {"read_symlink", readSymlink, 1},
    {"follow_symlink", followSymlink, 1},
    {"remove_symlink", removeSymlink, 1},
    {"create_hardlink", createHardlink, 2},
    {"remove_hardlink", removeHardlink, 1},
    {"stat_file", statFile, 1},
    {"chmod_file", chmodFile, 2},
};

int main(int argc, char *argv[]) {
    char *progName = basename(argv[0]);

    for (size_t i = 0; i < sizeof(commandMap) / sizeof(Command); i++) {
        if (strcmp(progName, commandMap[i].name) == 0) {
            if (argc < commandMap[i].requiredArgs + 1) {
                fprintf(stderr, "Usage: %s", progName);
                for (int j = 0; j < commandMap[i].requiredArgs; j++) {
                    fprintf(stderr, " <arg%d>", j+1);
                }
                fprintf(stderr, "\n");
                return EXIT_FAILURE;
            }
            commandMap[i].function(argv);
            return EXIT_SUCCESS;
        }
    }

    fprintf(stderr, "Unknown command: %s\n", progName);
    return EXIT_FAILURE;
}

void createDir(char *argv[]) {
    const char *path = argv[1];
    struct stat st;

    if (lstat(path, &st) == 0) {
        fprintf(stderr, "Path already exists\n");
        exit(EXIT_FAILURE);
    }

    if (mkdir(path, 0755) != 0) {
        perror("mkdir failed");
        exit(EXIT_FAILURE);
    }
}

void listDir(char *argv[]) {
    const char *path = argv[1];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Not a directory\n");
        exit(EXIT_FAILURE);
    }

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s\n", entry->d_name);
        }
    }
    closedir(dir);
}

void removeDir(char *argv[]) {
    const char *path = argv[1];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Not a directory\n");
        exit(EXIT_FAILURE);
    }

    if (rmdir(path) != 0) {
        perror("rmdir failed");
        exit(EXIT_FAILURE);
    }
}

void createFile(char *argv[]) {
    const char *path = argv[1];
    int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, 0644);
    if (fd == -1) {
        perror("create file failed");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void catFile(char *argv[]) {
    const char *path = argv[1];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Not a regular file\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(path, "r");
    if (!file) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        fputs(buffer, stdout);
    }
    fclose(file);
}

void removeFile(char *argv[]) {
    const char *path = argv[1];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    if (!S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode)) {
        fprintf(stderr, "Not a file or symlink\n");
        exit(EXIT_FAILURE);
    }

    if (unlink(path) != 0) {
        perror("unlink failed");
        exit(EXIT_FAILURE);
    }
}

void createSymlink(char *argv[]) {
    const char *target = argv[1];
    const char *linkpath = argv[2];

    if (symlink(target, linkpath) != 0) {
        perror("symlink failed");
        exit(EXIT_FAILURE);
    }
}

void readSymlink(char *argv[]) {
    const char *path = argv[1];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    if (!S_ISLNK(st.st_mode)) {
        fprintf(stderr, "Not a symlink\n");
        exit(EXIT_FAILURE);
    }

    char buffer[PATH_MAX];
    ssize_t len = readlink(path, buffer, sizeof(buffer)-1);
    if (len == -1) {
        perror("readlink failed");
        exit(EXIT_FAILURE);
    }
    buffer[len] = '\0';
    printf("%s\n", buffer);
}

void followSymlink(char *argv[]) {
    const char *path = argv[1];
    struct stat st;
    char targetPath[PATH_MAX];

    if (lstat(path, &st) != 0) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    if (!S_ISLNK(st.st_mode)) {
        fprintf(stderr, "Not a symbolic link\n");
        exit(EXIT_FAILURE);
    }

    ssize_t len = readlink(path, targetPath, sizeof(targetPath)-1);
    if (len == -1) {
        perror("readlink failed");
        exit(EXIT_FAILURE);
    }
    targetPath[len] = '\0';

    FILE *file = fopen(targetPath, "r");
    if (!file) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        fputs(buffer, stdout);
    }
    fclose(file);
}

void removeSymlink(char *argv[]) {
    const char *path = argv[1];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    if (!S_ISLNK(st.st_mode)) {
        fprintf(stderr, "Not a symlink\n");
        exit(EXIT_FAILURE);
    }

    if (unlink(path) != 0) {
        perror("unlink failed");
        exit(EXIT_FAILURE);
    }
}

void createHardlink(char *argv[]) {
    const char *oldpath = argv[1];
    const char *newpath = argv[2];
    struct stat st;

    if (stat(oldpath, &st) != 0) {
        perror("stat failed");
        exit(EXIT_FAILURE);
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Source is not a regular file\n");
        exit(EXIT_FAILURE);
    }

    if (link(oldpath, newpath) != 0) {
        perror("link failed");
        exit(EXIT_FAILURE);
    }
}

void removeHardlink(char *argv[]) {
    removeFile(argv);
}

void statFile(char *argv[]) {
    const char *path = argv[1];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }

    printf("Permissions: %o\n", st.st_mode & 0777);
    printf("Hard links: %ld\n", (long)st.st_nlink);
}

void chmodFile(char *argv[]) {
    const char *modeStr = argv[1];
    const char *path = argv[2];
    struct stat st;
    char *endptr;

    if (stat(path, &st) != 0) {
        perror("stat failed");
        exit(EXIT_FAILURE);
    }

    mode_t mode = strtol(modeStr, &endptr, 8);
    if (*endptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "Invalid mode\n");
        exit(EXIT_FAILURE);
    }

    if (chmod(path, mode) != 0) {
        perror("chmod failed");
        exit(EXIT_FAILURE);
    }
}
