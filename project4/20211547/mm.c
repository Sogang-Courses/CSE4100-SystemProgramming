/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your information in the following struct.
 ********************************************************/
team_t team = {
    /* Your student ID */
    "20211547",
    /* Your full name*/
    "Jiwon Shin",
    /* Your email address */
    "shinzz410@sogang.ac.kr",
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* prj4 신지원) 전역 변수 선언 */
static char *heap_listp = NULL;
static void *seg_list = NULL;

#define WSIZE       4       // 워드 사이즈 (header/footer)
#define DSIZE       8       // 더블워드 사이즈 (alignment)
#define CHUNKSIZE   (1 << 12)  // 초기 힙 확장 크기 (4KB)
#define LISTLIMIT   20      // segregated list의 최대 index 수
#define MINBLOCKSIZE (3 * DSIZE)
//#define MINBLOCKSIZE (2 * DSIZE)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define PACK(size, alloc)   ((size) | (alloc))              // size + alloc flag
#define GET(p)              (*(unsigned int *)(p))          // 포인터 p의 값 읽기
#define PUT(p, val)         (*(unsigned int *)(p) = (val))  // 포인터 p에 값 쓰기
#define GET_SIZE(p)         (GET(p) & ~0x7)                 // size 정보만 추출
#define GET_ALLOC(p)        (GET(p) & 0x1)                  // alloc 정보만 추출

#define HDRP(bp)           ((char *)(bp) - WSIZE)
#define FTRP(bp)           ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp)      ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)      ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define SET_SEGI(base, i, val)   (*((void **)((char *)(base) + (i) * DSIZE)) = (val))
#define GET_SEGI(base, i)        (*((void **)((char *)(base) + (i) * DSIZE)))
#define NEXT_FREE(bp) (*(void **)(bp))
#define PREV_FREE(bp) (*(void **)((char *)(bp) + DSIZE))

/* prj4 신지원) 함수 선언 */
int mm_init(void);
static inline void *extend_heap(size_t size);
static void insert_node(void *bp);
static void remove_node(void *bp);
static inline void *find_fit(size_t asize);
static inline void place(void *bp, size_t asize);
static inline void *coalesce(void *bp);
static unsigned get_list_idx(size_t size);

void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* prj4 신지원) malloc 패키지 초기화 함수 */
    heap_listp = NULL;
    seg_list = NULL;

    /* prj4 신지원) segregated list, 리스트헤더, 힙 초기 */    
    if ((seg_list = mem_sbrk(LISTLIMIT * DSIZE)) == (void *)-1) return -1;
    for (int i = 0; i < LISTLIMIT; ++i) SET_SEGI(seg_list, i, NULL);
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) return -1;

    PUT(heap_listp, 0);    
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); 
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); 
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); 
    heap_listp += (2 * WSIZE);

    /* prj4 신지원) 초기 free block 할당*/
    if (extend_heap(CHUNKSIZE) == NULL) return -1;

    return 0;
}

/* prj4 신지원) 힙을 size 바이트만큼 확장하고 새 free block을 반환 */
static inline void *extend_heap(size_t size) 
{
    char *bp;

    /* prj4 신지원) mem_sbrk를 이용해 힙 공간 확장 */
    if ((long)(bp = mem_sbrk(size)) == -1) return NULL;

    /* prj4 신지원) 확장한 영역에 free block의 header, footer, epilogue 설정 */
    PUT(HDRP(bp), PACK(size, 0));       
    PUT(FTRP(bp), PACK(size, 0));                   
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));  

    /* prj4 신지원) 인접 블록과 coalesce 및 segregated list에 추가 */
    bp = coalesce(bp);
    insert_node(bp);

    return bp;
}

/* prj4 신지원) 새 free block을 size에 따라 적절한 segregated list에 정렬 삽입 */
static void insert_node(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int idx = get_list_idx(size);
    void *prev = NULL;
    void *curr = GET_SEGI(seg_list, idx);

    // prj4 신지원) 해당 리스트에서 정렬된 위치 찾
    while (curr != NULL && GET_SIZE(HDRP(curr)) < size) {
        prev = curr;
        curr = NEXT_FREE(curr);
    }

    PREV_FREE(bp) = prev;
    NEXT_FREE(bp) = curr;

    if (prev != NULL) NEXT_FREE(prev) = bp;
    else SET_SEGI(seg_list, idx, bp);

    if (curr != NULL) PREV_FREE(curr) = bp;
}
/* prj4 신지원) LIFO 로 구현시throughput 하락 
void insert_node(void *bp) {
    int idx = get_list_idx(GET_SIZE(HDRP(bp)));
    void *head = GET_SEGI(seg_list, idx);

    NEXT_FREE(bp) = head;
    PREV_FREE(bp) = NULL;
    if (head) PREV_FREE(head) = bp;

    SET_SEGI(seg_list, idx, bp);
}*/

/* prj4 신지원) segregated list에서 주어진 free bp을 제거 */
static void remove_node(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int idx = get_list_idx(size);
    void *prev = PREV_FREE(bp);
    void *next = NEXT_FREE(bp);

    if (prev != NULL) NEXT_FREE(prev) = next;
    else SET_SEGI(seg_list, idx, next);

    if (next != NULL) PREV_FREE(next) = prev;

    PREV_FREE(bp) = NULL;
    NEXT_FREE(bp) = NULL;
}

/* prj4 신지원) 주어진 asize를 만족하는 free block을 segregated list에서 탐색 */
static inline void *find_fit(size_t asize)
{
    void *bp = NULL;
    unsigned int idx = get_list_idx(asize);

    for (int i = idx; i < LISTLIMIT; ++i) {
        for (bp = GET_SEGI(seg_list, i); bp != NULL; bp = NEXT_FREE(bp)) {
            if (GET_SIZE(HDRP(bp)) >= asize)
                return bp;
        }
    }
    return NULL;
}

/* prj4 신지원) 주어진 free block(bp)에 asize만큼 할당하고 나머지를 분할 처리 */
static inline void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    remove_node(bp);

    if ((csize - asize) < MINBLOCKSIZE) {
        /* prj4 신지원) 나머지를 나누기엔 작아서 통째로 할당 */
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    } 
    else {
        /* prj4 신지원) 할당 + 분할된 나머지 블록 생성 */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        void *split_bp = NEXT_BLKP(bp);
        size_t split_size = csize - asize;

        PUT(HDRP(split_bp), PACK(split_size, 0));
        PUT(FTRP(split_bp), PACK(split_size, 0));

        insert_node(coalesce(split_bp));
    }
}

/* prj4 신지원) 인접 free block 과 병합하여 coalescing 수행 */
static inline void *coalesce(void *bp) 
{   
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* prj4 신지원) case 1: 앞뒤 모두 할당 상태 */
    if (prev_alloc && next_alloc) {
        return bp;
    } 
    /* prj4 신지원) case 2: 뒤쪽만 free */
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        remove_node(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } 
    /* prj4 신지원) case 3: 앞쪽만 free */
    else if (!prev_alloc && next_alloc) {
        bp = PREV_BLKP(bp);
        size += GET_SIZE(HDRP(bp));
        remove_node(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } 
    /* prj4 신지원) case 4: 앞뒤 모두 free */
    else {
        void *prev_bp = PREV_BLKP(bp);
        void *next_bp = NEXT_BLKP(bp);
        size += GET_SIZE(HDRP(prev_bp)) + GET_SIZE(FTRP(next_bp));
        remove_node(prev_bp);
        remove_node(next_bp);
        PUT(HDRP(prev_bp), PACK(size, 0));
        PUT(FTRP(next_bp), PACK(size, 0));
        bp = prev_bp;
    }

    return bp;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
/*void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}*/

/* prj4 신지원) 요청한 size 만큼 메모리를 할당하도록 구현된 새로운 malloc 함수 */
void *mm_malloc(size_t size) {
    size_t asize;      
    size_t extendsize;
    char *bp;

    if (size == 0) return NULL;

    /* prj4 신지원) 8바이트 정렬 */
    if (size <= DSIZE) asize = 3 * DSIZE;
    //if (size <= DSIZE) asize = 2 * DSIZE;
    else asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);

    /* prj4 신지원) 적절한 free 블록을 찾거나 없으면 힙 확장 */
    if ((bp = find_fit(asize)) == NULL) {
        extendsize = MAX(asize, CHUNKSIZE);
        if ((bp = extend_heap(extendsize)) == NULL) return NULL;
    }

    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if (ptr == NULL) return;
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    /* prj4 신지원) 인접 블록과 병합 후 freelist에 삽입 */
    ptr = coalesce(ptr);
    insert_node(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
/*void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}*/

/* prj4 신지원) 기존 블록 bp를 size 만큼 재할당하도록 구현한 새로운 realloc 함수 */
void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL) return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    size_t old_size = GET_SIZE(HDRP(ptr));
    size_t asize;

    if (size <= DSIZE) asize = 3 * DSIZE;
    //if (size <= DSIZE) asize = 2 * DSIZE;
    else asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);

    if (asize <= old_size) return ptr;
    
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));

    if (!next_alloc && old_size + next_size >= asize) {
        remove_node(NEXT_BLKP(ptr));
        size_t total_size = old_size + next_size;
        PUT(HDRP(ptr), PACK(total_size, 1));
        PUT(FTRP(ptr), PACK(total_size, 1));
        return ptr;
    }

    /* prj4 신지원) 새로운 블록을 malloc하고 기존 데이터 복사 */
    void *new_ptr = mm_malloc(size);
    if (new_ptr == NULL) return NULL;
    
    memcpy(new_ptr, ptr, old_size - DSIZE);
    mm_free(ptr);
    return new_ptr;
}

/* prj4 신지원) size에 따라 적절한 segregated list index 반환 */
/*
static unsigned get_list_idx(size_t size) {
    if(size <= 32)       return 0;
    else if(size <= 64)  return 1;
    else if(size <= 128) return 2;
    else if(size <= 256) return 3;
    else if(size <= 512) return 4;
    else if(size <= 1024) return 5;
    else if(size <= 2048) return 6;
    else if(size <= 4096) return 7;
    else if(size <= 8192) return 8;
    else if(size <= 16384) return 9;
    else if(size <= 32768) return 10;
    else if(size <= 65536) return 11;
    else if(size <= 131072) return 12;
    else if(size <= 262144) return 13;
    else if(size <= 524288) return 14;
    else return 15;
}*/

/* prj4 신지원) while 문으로 개선 */
static unsigned get_list_idx(size_t size) {
    unsigned idx = 0;
    size_t s = size;

    while (idx < LISTLIMIT - 1 && s > 1) {
        s >>= 1;
        idx++;
    }
    return idx;
}
