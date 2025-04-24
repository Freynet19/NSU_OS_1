#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void reverseString(const char *src, char *dst, size_t len) {
    for (size_t i = 0; i < len; i++) {
        dst[i] = src[len - 1 - i];
    }
    dst[len] = '\0';
}

void reverseBuffer(char *buffer, size_t size) {
    for (size_t i = 0; i < size / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[size - 1 - i];
        buffer[size - 1 - i] = temp;
    }
}

int reverseCopyFile(const char *source, const char *dest) {
    int srcFD = open(source, O_RDONLY);
    if (srcFD == -1) {
        perror("open source failed");
        return 0;
    }

    struct stat s
t;
    if (fstat(srcFD, &st) != 0 || !S_ISREG(st.st_mode)) {
        perror("fstat failed");
        close(srcFD);
        return 0;
    }
    const off_t fileSize = st.st_size;

    int destFD = open(dest, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (destFD == -1) {
        perror("open dest failed");
        close(srcFD);
        return 0;
    }

    const size_t chunkSize = 1 * 1024 * 1024;
    char *buffer = malloc(chunkSize);
    if (!buffer) {
        perror("malloc failed");
        close(srcFD);
        close(destFD);
        return 0;
    }

    int success = 1;
    off_t pos = fileSize;
    while (pos > 0) {
        const size_t chunk = pos > (off_t)chunkSize ? chunkSize : (size_t)pos;
        pos -= chunk;

        if (lseek(srcFD, pos, SEEK_SET) == -1) {
            perror("lseek failed");
            success = 0;
            break;
        }

        ssize_t totalRead = 0;
        while (totalRead < (ssize_t)chunk) {
            ssize_t readNow = read(srcFD, buffer + totalRead, chunk - totalRead);
            if (readNow == -1) {
                perror("read failed");
                success = 0;
                break;
            }
            if (readNow == 0) break;
            totalRead += readNow;
        }

        if (totalRead != (ssize_t)chunk) {
            success = 0;
            break;
        }

        reverseBuffer(buffer, chunk);

        ssize_t totalWritten = 0;
        while (totalWritten < (ssize_t)chunk) {
            ssize_t writtenNow = write(destFD, buffer + totalWritten, chunk - totalWritten);
            if (writtenNow == -1) {
                perror("write failed");
                success = 0;
                break;
            }
            totalWritten += writtenNow;
        }
    }

    free(buffer);
    close(srcFD);
    close(destFD);
    return success;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s directory\n", argv[0]);
        return 1;
    }

    char *sourceDir = argv[1];
    struct stat st;

    // получить метаданные
    if (stat(sourceDir, &st) == -1) {
        perror("stat failed");
        return 1;
    }

    // является ли директорией
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: %s is not a directory\n", sourceDir);
        return 1;
    }

    // копирование пути для редактирования
    char *src = strdup(sourceDir);
    if (!src) {
        perror("strdup (sourceDir) failed");
        return 1;
    }

    // удаление слэшей на конце
    size_t len = strlen(src);
    while (len > 1 && src[len - 1] == '/') {
        src[--len] = '\0';
    }

    // разделение на родительский и целевой каталоги
    const char *lastSlash = strrchr(src, '/');
    const char *base;
    char *parentDir;

    if (!lastSlash) {  // относительный путь
        parentDir = ".";
        base = src;
    } else if (lastSlash == src) {  // корневой каталог
        parentDir = "/";
        base = lastSlash + 1;
    } else {
        parentDir = strndup(src, lastSlash - src);
        base = lastSlash + 1;
    }

    if (!parentDir) {
        perror("strndup (parentDir) failed");
        free(src);
        return 1;
    }

    // реверс имени целевого каталога
    size_t baseLen = strlen(base);
    char reversedBase[NAME_MAX + 1];
    reverseString(base, reversedBase, baseLen);

    // создание нового пути
    char newDir[PATH_MAX];
    snprintf(newDir, PATH_MAX, "%s/%s", parentDir, reversedBase);

    // берем исходные права доступа
    if (mkdir(newDir, st.st_mode) == -1) {
        perror("mkdir failed");
        free(src);
        free(parentDir);
        return 1;
    }

    // открываем исходный каталог
    DIR *dir = opendir(sourceDir);
    if (!dir) {
        perror("opendir failed");
        free(src);
        free(parentDir);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char sourcePath[PATH_MAX];
        snprintf(sourcePath, PATH_MAX, "%s/%s", sourceDir, entry->d_name);

        struct stat entrySt;
        if (lstat(sourcePath, &entrySt) == -1) {
            perror("lstat failed");
            continue;
        }

        if (!S_ISREG(entrySt.st_mode)) {
            continue;
        }

        // реверс имени файла
        size_t nameLen = strlen(entry->d_name);
        char reversedName[NAME_MAX + 1];
        reverseString(entry->d_name, reversedName, nameLen);

        // формирование destPath
        size_t needed = snprintf(NULL, 0, "%s/%s", newDir, reversedName) + 1;
        char *destPath = malloc(needed);
        if (!destPath) {
            perror("malloc failed");
            continue;
        }
        snprintf(destPath, needed, "%s/%s", newDir, reversedName);

        // копирование реверсированного содержимого
        if (!reverseCopyFile(sourcePath, destPath)) {
            fprintf(stderr, "Failed to copy %s to %s\n", sourcePath, destPath);
        }

        free(destPath);
    }

    closedir(dir);
    free(src);
    free(parentDir);

    return 0;
}
