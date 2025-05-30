#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

typedef struct BlockHeader {
    size_t size;
    int free;
    struct BlockHeader *prev;
    struct BlockHeader *next;
} Header;

Header *head = nullptr;
void *mappedRegion = NULL;
size_t regionSize = 0;

void *myMalloc(size_t size) {
    if (size == 0) return NULL;

    if (mappedRegion == NULL) {
        regionSize = 4 * 1024;
        mappedRegion = mmap(NULL, regionSize,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mappedRegion == MAP_FAILED) {
            perror("mmap failed");
            return NULL;
        }
        head = (Header *)mappedRegion;
        head->size = regionSize - sizeof(Header);
        head->free = 1;
        head->prev = nullptr;
        head->next = nullptr;
    }

    size = (size + 7) & ~7;

    Header *current = head;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            if (current->size > size + sizeof(Header)) {
                Header *newBlock = (Header *)((char *)current + sizeof(Header) + size);
                newBlock->size = current->size - size - sizeof(Header);
                newBlock->free = 1;
                newBlock->prev = current;
                newBlock->next = current->next;
                if (current->next != NULL) current->next->prev = newBlock;
                current->next = newBlock;
                current->size = size;
                current->free = 0;
            } else {
                current->free = 0;
            }
            return (char *)current + sizeof(Header);
        }
        current = current->next;
    }

    return NULL;
}

void myFree(void *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "Free pointer is NULL\n");
        return;
    }
    if ((uint64_t)ptr % 8 != 0) {
        fprintf(stderr, "Free pointer is not aligned\n");
        return;
    }

    Header *block = (Header *)((char *)ptr - sizeof(Header));
    if ((block->prev != NULL && block->prev->next != block) ||
        (block->next != NULL && block->next->prev != block) ||
        block->size > regionSize || block->free != 0) {
        fprintf(stderr, "Free pointer is incorrect\n");
        return;
    }
    block->free = 1;

    if (block->prev != NULL && block->prev->free) {
        Header *prev = block->prev;
        prev->size += block->size + sizeof(Header);
        prev->next = block->next;
        if (block->next != NULL) block->next->prev = prev;
        block = prev;
    }

    if (block->next != NULL && block->next->free) {
        Header *next = block->next;
        block->size += next->size + sizeof(Header);
        block->next = next->next;
        if (next->next != NULL) next->next->prev = block;
    }
}

int main() {
    /*
    void *ptr1 = myMalloc(1024);
    if (ptr1) printf("ptr1 OK\n");

    myFree(ptr1);

    char *ptr2 = myMalloc(200);
    memset(ptr2, 'a', 200);
    myFree(ptr2 + 56);
    */

    void *p1 = myMalloc(1024);
    if (!p1) fprintf(stderr, "p1 was not allocated\n");
    void *p2a = myMalloc(512);
    if (!p2a) fprintf(stderr, "p2a was not allocated\n");
    void *p2b = myMalloc(512);
    if (!p2b) fprintf(stderr, "p2b was not allocated\n");
    void *p3 = myMalloc(1536);
    if (!p3) fprintf(stderr, "p3 was not allocated\n");

    myFree(p2a);
    myFree(p2b);  // joining blocks for p2
    void *p2 = myMalloc(1024);
    if (!p2) fprintf(stderr, "p2 was not allocated\n");

    void *p4 = myMalloc(512);  // not enough memory because of headers
    if (!p4) fprintf(stderr, "p4 was not allocated\n");

    void *p5 = myMalloc(512 - sizeof(Header) * 5);
    if (!p5) fprintf(stderr, "p5 was not allocated\n");

    void *p6 = myMalloc(1);  // heap is full
    if (!p6) fprintf(stderr, "p6 was not allocated\n");

    myFree(p1);
    myFree(p2);
    myFree(p3);
    // myFree(p4);
    myFree(p5);
    // myFree(p6);


    return 0;
}
