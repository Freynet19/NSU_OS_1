#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define LINE_BUF_SIZE 1024
#define OUTPUT_BUF_SIZE 1024

ssize_t safeRead(int fd, void *buf, size_t count) {
    size_t totalRead = 0;
    while (totalRead < count) {
        ssize_t readNow = read(fd, (char*)buf + totalRead, count - totalRead);
        if (readNow < 0) return -1;
        if (readNow == 0) break;
        totalRead += readNow;
    }
    return (ssize_t)totalRead;
}

ssize_t safeWrite(int fd, const void *buf, size_t count) {
    size_t totalWritten = 0;
    while (totalWritten < count) {
        ssize_t writtenNow = write(fd, (const char*)buf + totalWritten, count - totalWritten);
        if (writtenNow < 0) return -1;
        totalWritten += writtenNow;
    }
    return (ssize_t)totalWritten;
}

void printEntry(uint64_t addr, uint64_t pageEntry) {
    char outputBuffer[OUTPUT_BUF_SIZE];
    int outputLength = snprintf(outputBuffer, sizeof(outputBuffer),
        "0x%lx: 0x%016lx\n", addr, pageEntry);
    safeWrite(STDOUT_FILENO, outputBuffer, outputLength);

    outputLength = snprintf(outputBuffer, sizeof(outputBuffer),
        " -> Bit 63 \t\tPage present: \t%llu\n", (pageEntry & (1ULL << 63)) >> 63);
    safeWrite(STDOUT_FILENO, outputBuffer, outputLength);

    outputLength = snprintf(outputBuffer, sizeof(outputBuffer),
        " -> Bit 62 \t\tPage swapped: \t%llu\n", (pageEntry & (1ULL << 62)) >> 62);
    safeWrite(STDOUT_FILENO, outputBuffer, outputLength);

    outputLength = snprintf(outputBuffer, sizeof(outputBuffer),
        " -> Bit 61 \t\tFile-page or shared-anon: \t%llu\n", (pageEntry & (1ULL << 61)) >> 61);
    safeWrite(STDOUT_FILENO, outputBuffer, outputLength);

    outputLength = snprintf(outputBuffer, sizeof(outputBuffer),
        " -> Bits 57-60 \tReserved (zero)\n");
    safeWrite(STDOUT_FILENO, outputBuffer, outputLength);

    outputLength = snprintf(outputBuffer, sizeof(outputBuffer),
        " -> Bit 56 \t\tExclusively mapped: \t%llu\n", (pageEntry & (1ULL << 56)) >> 56);
    safeWrite(STDOUT_FILENO, outputBuffer, outputLength);

    outputLength = snprintf(outputBuffer, sizeof(outputBuffer),
        " -> Bit 55 \t\tPTE is soft-dirty: \t%llu\n", (pageEntry & (1ULL << 55)) >> 55);
    safeWrite(STDOUT_FILENO, outputBuffer, outputLength);

    outputLength = snprintf(outputBuffer, sizeof(outputBuffer),
        " -> Bits 0-54 \tPFN/SWAP: \t0x%014llx\n", (pageEntry & ((1ULL << 55) - 1)));
    safeWrite(STDOUT_FILENO, outputBuffer, outputLength);
}

int main(int argc, char *argv[]) {
    char *pidStr = argc > 1 ? argv[1] : "self";

    char mapsPath[64];
    char pagemapPath[64];
    snprintf(mapsPath, sizeof(mapsPath), "/proc/%s/maps", pidStr);
    snprintf(pagemapPath, sizeof(pagemapPath), "/proc/%s/pagemap", pidStr);

    int mapsFD = open(mapsPath, O_RDONLY);
    if (mapsFD < 0) {
        perror("Failed to open maps file");
        return EXIT_FAILURE;
    }

    int pagemapFD = open(pagemapPath, O_RDONLY);
    if (pagemapFD < 0) {
        perror("Failed to open pagemap file");
        close(mapsFD);
        return EXIT_FAILURE;
    }

    char line[LINE_BUF_SIZE];
    int lineIdx = 0;

    while (1) {
        char currentChar;
        ssize_t readResult = safeRead(mapsFD, &currentChar, 1);
        if (readResult == 0) break;
        if (readResult < 0) {
            perror("Error reading maps file");
            break;
        }

        if (currentChar == '\n') {
            line[lineIdx] = '\0';
            uint64_t startAddr, endAddr;
            if (sscanf(line, "%lx-%lx", &startAddr, &endAddr) != 2) {
                lineIdx = 0;
                continue;
            }

            size_t pageSize = (size_t)sysconf(_SC_PAGE_SIZE);
            for (uint64_t addr = startAddr; addr < endAddr; addr += pageSize) {
                off_t offset = addr / pageSize * sizeof(uint64_t);
                if (lseek64(pagemapFD, offset, SEEK_SET) == (off_t)-1) {
                    perror("Failed to seek pagemap");
                    continue;
                }

                uint64_t pageEntry;
                ssize_t bytesRead = safeRead(pagemapFD, &pageEntry, sizeof(pageEntry));
                if (bytesRead == 0) break;
                if (bytesRead != sizeof(pageEntry)) {
                    perror("Failed to read pagemap entry");
                    continue;
                }

                printEntry(addr, pageEntry);
            }
            lineIdx = 0;
        } else if (lineIdx < LINE_BUF_SIZE - 1) {
            line[lineIdx++] = currentChar;
        } else {
            fprintf(stderr, "Buffer overflow\n");
            lineIdx = 0;
        }
    }

    close(mapsFD);
    close(pagemapFD);
    return EXIT_SUCCESS;
}
