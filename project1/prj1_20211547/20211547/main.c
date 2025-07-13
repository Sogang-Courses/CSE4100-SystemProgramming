/*prj1 신지원) main.c 함수 구현 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "list.h"
#include "hash.h"
#include "bitmap.h"

#define MAX_OBJECTS 10
#define MAX_NAME 32
#define MAX_INPUT 100


/*prj1 신지원) list/hash/bitmap 자료 구조 선언 */
struct named_list {
  char name[MAX_NAME];
  struct list list;
};

struct named_hash {
  char name[MAX_NAME];
  struct hash hash;
};

struct named_bitmap {
  char name[MAX_NAME];
  struct bitmap *bitmap;
};


/* prj1 신지원) list/hash/bitmap 전역 변수 선언 */
struct named_list *lists[MAX_OBJECTS];
int list_count = 0;

struct named_hash *hashes[MAX_OBJECTS];
int hash_count = 0;

struct named_bitmap *bitmaps[MAX_OBJECTS];
int my_bitmap_count = 0;

struct named_list *find_list(const char *name) {
  for (int i = 0; i < list_count; i++) {
    if (!strcmp(lists[i]->name, name))
      return lists[i];
  }
  return NULL;
}

struct named_hash *find_hash(const char *name) {
  for (int i = 0; i < hash_count; i++) {
    if (!strcmp(hashes[i]->name, name))
      return hashes[i];
  }
  return NULL;
}

struct named_bitmap *find_bitmap(const char *name) {
  for (int i = 0; i < my_bitmap_count; i++) {
    if (!strcmp(bitmaps[i]->name, name))
      return bitmaps[i];
  }
  return NULL;
}


/* prj1 신지원) list 커스텀 함수 */
bool my_list_less(const struct list_elem *a, const struct list_elem *b, void *aux) {
    struct list_item *item_a = list_entry(a, struct list_item, elem);
    struct list_item *item_b = list_entry(b, struct list_item, elem);
    return item_a->data.val < item_b->data.val;
}


/* prj1 신지원) hast 커스텀 함수*/ 
unsigned my_hash_func(const struct hash_elem *e, void *aux) {
  return hash_int(e->val);
}

bool my_hash_less(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
  return a->val < b->val;
}

void hash_action_free(struct hash_elem *e, void *aux) {
  (void) aux;
  free(e);
}

/* prj1 신지원) main 함수 시작: command 처리 */

int main(void) {
  char input[MAX_INPUT];

  while(1) {
    if (!fgets(input, sizeof(input), stdin)) break;
    
    char *cmd = strtok(input, " \n");
    if (!cmd) continue;

    if (strcmp(cmd, "quit") == 0) {
      break;
    }
    
    // prj1 신지원) ----- CREATE -----
    if (strcmp(cmd, "create") == 0) {
      char *type = strtok(NULL, " \n");
      char *name = strtok(NULL, " \n");

      if (strcmp(type, "list") == 0 && list_count < MAX_OBJECTS) {
        struct named_list *new_list = malloc(sizeof(struct named_list));
        strncpy(new_list->name, name, MAX_NAME);
        list_init(&new_list->list);
        lists[list_count++] = new_list;
      }

      else if (strcmp(type, "hashtable") == 0 && hash_count < MAX_OBJECTS) {
        struct named_hash *new_hash = malloc(sizeof(struct named_hash));
	if (new_hash == NULL) {
          fprintf(stderr, "Failed to allocate hash table\n");
          continue;
        }
        
	strncpy(new_hash->name, name, MAX_NAME);
	new_hash->name[MAX_NAME - 1] = '\0';
	hash_init(&new_hash->hash, my_hash_func, my_hash_less, NULL);
	hashes[hash_count++] = new_hash;
      }

      else if (strcmp(type, "bitmap") == 0 && my_bitmap_count < MAX_OBJECTS) {
        char *bit_cnt_str = strtok(NULL, " \n");
        if (bit_cnt_str != NULL) {
          int bit_cnt = atoi(bit_cnt_str);
          struct named_bitmap *new_bitmap = malloc(sizeof(struct named_bitmap));
          strncpy(new_bitmap->name, name, MAX_NAME);
          new_bitmap->bitmap = bitmap_create(bit_cnt);
          bitmaps[my_bitmap_count++] = new_bitmap;
        }
      }
    }
    

    // prj1 신지원) ----- DELETE -----
    else if (strcmp(cmd, "delete") == 0) { 
      char *list_name = strtok(NULL, " \n");

      for (int i = 0; i < list_count; i++) {
        if (!strcmp(lists[i]->name, list_name)) {
          free(lists[i]);
          for (int j = i; j < list_count - 1; j++)
            lists[j] = lists[j + 1];
          list_count--;
          break;
        }
      }

      for (int i = 0; i < hash_count; i++) {
        if (!strcmp(hashes[i]->name, list_name)) {
          free(hashes[i]);
          for (int j = i; j < hash_count - 1; j++)
            hashes[j] = hashes[j + 1];
          hash_count--;
          break;
        }
      }

      for (int i = 0; i < my_bitmap_count; i++) {
        if (!strcmp(bitmaps[i]->name, list_name)) {
          bitmap_destroy(bitmaps[i]->bitmap);
          free(bitmaps[i]);
          for (int j = i; j < my_bitmap_count - 1; j++)
            bitmaps[j] = bitmaps[j + 1];
          my_bitmap_count--;
          break;
        }
      }
    }
    

    // prj1 신지원) ----- DUMPDATA -----
    else if (strcmp(cmd, "dumpdata") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      if (l != NULL) {
        struct list_elem *e;
        for (e = list_begin(&l->list); e != list_end(&l->list); e = list_next(e)) {
          struct list_item *item = list_entry(e, struct list_item, elem);
          printf("%d ", item->data.val);
        }
        printf("\n");
      }

      for (int i = 0; i < hash_count; i++) {
        if (!strcmp(hashes[i]->name, list_name)) {
          struct hash_iterator iter;
          hash_first(&iter, &hashes[i]->hash);
          while (hash_next(&iter)) {
            struct hash_elem *e = hash_cur(&iter);
	    printf("%d ", e->val);
	  }
          printf("\n");
        }
      }

      for (int i = 0; i < my_bitmap_count; i++) {
        if (!strcmp(bitmaps[i]->name, list_name)) {
          size_t size = bitmap_size(bitmaps[i]->bitmap);
          for (size_t j = 0; j < size; j++) {
            printf("%d", bitmap_test(bitmaps[i]->bitmap, j) ? 1 : 0);
          }
          printf("\n");
        }
      }
    }

    /* list method start */
    /* 리스트 순서
     * list_push_back
     * list_push_front
     * list_pop_front
     * list_pop_back
     * list_front
     * list_back
     * list_empty
     * list_size
     * list_max
     * list_min
     * list_unique
     * list_insert
     * list_insert_ordered
     * list_remove
     * list_reverse
     * list_sort
     * list_splice
     * list_swap
     * list_shuffle
     */

    else if (strcmp(cmd, "list_push_back") == 0) {
      char *list_name = strtok(NULL, " \n");
      char *value_str = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      
      if (l && value_str) {
        struct list_item *item = malloc(sizeof(struct list_item));
        if (item == NULL) {
          fprintf(stderr, "Memory allocation failed\n");
          continue;
        }
	item->type = VAL;
       	item->data.val = atoi(value_str);
        list_push_back(&l->list, &item->elem);
      }
    }
  
    else if (strcmp(cmd, "list_push_front") == 0) {
      char *list_name = strtok(NULL, " \n");
      char *value_str = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);

      if (l && value_str) {
        struct list_item *item = malloc(sizeof(struct list_item));
        item->type = VAL;
	item->data.val = atoi(value_str);
        list_push_front(&l->list, &item->elem);
      }
    }

    else if (strcmp(cmd, "list_pop_front") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);

      if( l && !list_empty(&l->list)) {
        struct list_elem *e = list_pop_front(&l->list);
	struct list_item *item = list_entry(e, struct list_item, elem);
	free(item);
      }
    }
    

    else if (strcmp(cmd, "list_pop_back") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);

      if( l && !list_empty(&l->list)) {
        struct list_elem *e = list_pop_back(&l->list);
        struct list_item *item = list_entry(e, struct list_item, elem);
        free(item);
      }
    }

    else if (strcmp(cmd, "list_front") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);

      if( l && !list_empty(&l->list)) {
        struct list_item *item = list_entry(list_front(&l->list), struct list_item, elem);
	item->type = VAL;
	printf("%d\n", item->data.val);
      }
    }

    else if (strcmp(cmd, "list_back") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);

      if( l && !list_empty(&l->list)) {
        struct list_item *item = list_entry(list_back(&l->list), struct list_item, elem);
	item->type = VAL;
	printf("%d\n", item->data.val);
      }
    }

    else if (strcmp(cmd, "list_empty") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      if (l) {
        if( list_empty(&l->list)) {
		printf("true\n");
	} else {
	  printf("false\n");
	}
      }	
    }

    else if (strcmp(cmd, "list_size") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      if (l) {
        printf("%zu\n", list_size(&l->list));
      }
    }
    
    else if (strcmp(cmd, "list_max") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      if (l) {
        struct list_elem *max_elem = list_max(&l->list, my_list_less, NULL);
	if (max_elem) {
	  struct list_item *item = list_entry(max_elem, struct list_item, elem);
	  item->type = VAL;
	  printf("%d\n", item->data.val);
	}
      }
    }

    else if (strcmp(cmd, "list_min") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      if (l) {
        struct list_elem *min_elem = list_min(&l->list, my_list_less, NULL);
        if (min_elem) {
          struct list_item *item = list_entry(min_elem, struct list_item, elem);
          item->type = VAL;
	  printf("%d\n", item->data.val);
        }
      }
    }
    
    else if (strcmp(cmd, "list_unique") == 0) {
      char *list_name = strtok(NULL, " \n");
      char *dup_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      struct named_list *dup = dup_name ? find_list(dup_name) : NULL;

      if (l) {
        list_unique(&l->list, dup ? &dup->list : NULL, my_list_less, NULL);
      }
    }
    
    else if (strcmp(cmd, "list_insert") == 0) {
      char *list_name = strtok(NULL, " \n");
       struct named_list *l = find_list(list_name);

       char *index_str = strtok(NULL, " \n");
       char *value_str = strtok(NULL, " \n");

       if (l && index_str && value_str) {
         int index = atoi(index_str);
	 struct list_elem *e = list_begin(&l->list);
	 for (int i = 0; i < index && e != list_end(&l->list); i++) {
           e = list_next(e);
         }

	 if(e) {
	   struct list_item *item = malloc(sizeof(struct list_item));
	   item->type = VAL;
	   item->data.val = atoi(value_str);
           list_insert(e, &item->elem);
	 }
       }
    }
    
    else if (strcmp(cmd, "list_insert_ordered") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      char *value_str = strtok(NULL, " \n");
      
      if( l && value_str ) {
        struct list_item *item = malloc(sizeof(struct list_item));
	item->type = VAL;
	item->data.val = atoi(value_str);
	list_insert_ordered(&l->list, &item->elem, my_list_less, NULL);
      }
    }

    else if (strcmp(cmd, "list_remove") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      char *index_str = strtok(NULL, " \n");

      if (l && index_str) {
        int index = atoi(index_str);
	struct list_elem *e = list_begin(&l->list);
	for (int i = 0; i < index && e != list_end(&l->list); i++) {
           e = list_next(e);
         }

	if( e && e != list_end(&l->list)) {
	  struct list_item *item = list_entry(e, struct list_item, elem);
	  list_remove(e);
	  free(item);
	}
      }
    }
    
    else if (strcmp(cmd, "list_reverse") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      if (l) {
        list_reverse(&l->list);
      }
    }
    
    else if (strcmp(cmd, "list_sort") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      if (l) {
        list_sort(&l->list, my_list_less, NULL);
      }
    } 

    else if (strcmp(cmd, "list_splice") == 0) {
      char *dst_name = strtok(NULL, " \n");
      char *dst_idx_str = strtok(NULL, " \n");
      char *src_name = strtok(NULL, " \n");
      char *start_idx_str = strtok(NULL, " \n");
      char *end_idx_str = strtok(NULL, " \n");
    
      struct named_list *dst = find_list(dst_name);
      struct named_list *src = find_list(src_name);

      if (dst && src && dst_idx_str && start_idx_str && end_idx_str) {
        int dst_idx = atoi(dst_idx_str);
	int start_idx = atoi(start_idx_str);
	int end_idx = atoi(end_idx_str);

        if (start_idx > end_idx) continue;

	struct list_elem *dst_e = list_begin(&dst->list);
        for (int i = 0; i < dst_idx && dst_e != list_end(&dst->list); i++) {
          dst_e = list_next(dst_e);
        }

	struct list_elem *start_e = list_begin(&src->list);
	for (int i = 0; i < start_idx && start_e != list_end(&src->list); i++) {
          start_e = list_next(start_e);
        }
       
	struct list_elem *end_e;
	if (end_idx >= list_size(&src->list)) {
	  end_e = list_end(&src->list);
	} else {
	  end_e = list_begin(&src->list);
	  for (int i = 0; i < end_idx && end_e != list_end(&src->list); i++) {  
	    end_e = list_next(end_e);
	  }
	}
        
	if (dst_e && start_e && end_e) {
	  list_splice(dst_e, start_e, end_e);
        }
      }
    }

    else if (strcmp(cmd, "list_swap") == 0) {
      char *list_name = strtok(NULL, " \n");
      char *idx1_str = strtok(NULL, " \n");
      char *idx2_str = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);

      if (l) {
        int idx1 = atoi(idx1_str);
	int idx2 = atoi(idx2_str);
        struct list_elem *a = list_begin(&l->list);
        struct list_elem *b = list_begin(&l->list);
        
	for (int i = 0; i < idx1 && a != list_end(&l->list); i++) a = list_next(a);
        for (int i = 0; i < idx2 && b != list_end(&l->list); i++) b = list_next(b);
        if (a != list_end(&l->list) && b != list_end(&l->list)) {
          list_swap(a, b);
        }
      }
    }

    else if (strcmp(cmd, "list_shuffle") == 0) {
      char *list_name = strtok(NULL, " \n");
      struct named_list *l = find_list(list_name);
      if (l) {
        list_shuffle(&l->list);
      }
    }

    /* list method end */
    /* hash method start */
    /* 해쉬 순서
     * hash_apply
     * hash_delete
     * hash_find
     * hash_insert
     * hash_replace
     * hash_etc - hash_empty
     * hash_etc - hash_size
     * hash_etc - hash_clear  */
    
    else if (strcmp(cmd, "hash_apply") == 0) {
      char *hash_name = strtok(NULL, " \n");
      char *hash_mode = strtok(NULL, " \n");
      struct named_hash *h = find_hash(hash_name);

      if (!strcmp(hash_mode, "square"))
        hash_apply(&h->hash, hash_square);
      else if (!strcmp(hash_mode, "triple"))
        hash_apply(&h->hash, hash_triple);   
    }

    else if (strcmp(cmd, "hash_delete") == 0) {
      char *hash_name = strtok(NULL, " \n");
      char *hash_mode = strtok(NULL, " \n");
      int val = atoi(hash_mode);
      struct hash_elem temp;
      memset(&temp, 0, sizeof(temp));
      temp.val = val;
      
      struct named_hash *h = find_hash(hash_name);
      if (h) { 
	struct hash_elem *found = hash_find(&h->hash, &temp);
        if(found) {
	  hash_delete(&h->hash, found);
	  free(found);
	}
      }
    }

    else if (strcmp(cmd, "hash_find") == 0) {
      char *hash_name = strtok(NULL, " \n");
      char *hash_mode = strtok(NULL, " \n");
      int val = atoi(hash_mode);
      struct hash_elem temp;
      memset(&temp, 0, sizeof(temp));
      temp.val = val;

      struct named_hash *h = find_hash(hash_name);
      if (h) {
        struct hash_elem *found = hash_find(&h->hash, &temp);
	if (found != NULL) {
          printf("%d\n", found->val);
        }
      }
    }
    
    else if (strcmp(cmd, "hash_insert") == 0) {
      char *hash_name = strtok(NULL, " \n");
      char *hash_mode = strtok(NULL, " \n");
      int val = atoi(hash_mode);
      struct named_hash *h = find_hash(hash_name);

      if(h) {
        struct hash_elem *elem = malloc(sizeof(struct hash_elem));
	elem->val = val;
	hash_insert(&h->hash, elem);
      }
    }

    else if (strcmp(cmd, "hash_replace") == 0) {
      char *hash_name = strtok(NULL, " \n");
      char *hash_mode = strtok(NULL, " \n");
      int val = atoi(hash_mode);
      
      struct named_hash *h = find_hash(hash_name);
      if (h) {
        struct hash_elem *elem = malloc(sizeof(struct hash_elem));
        if (elem == NULL) {
          fprintf(stderr, "Memory allocation failed\n");
          continue;
        }

	elem->val = val;
	struct hash_elem *old = hash_replace(&h->hash, elem);
	if (old != NULL) {
		free(old);
	}
      }
    }
    
    else if (strcmp(cmd, "hash_empty") == 0) {
      char *hash_name = strtok(NULL, " \n");
      struct named_hash *h = find_hash(hash_name);
      
      if (h) {
        if(hash_empty(&h->hash)) 
	  printf("true\n");
	else {
	  printf("false\n");
	}
      }
    }
    
    else if (strcmp(cmd, "hash_size") == 0) {
      char *hash_name = strtok(NULL, " \n");
      struct named_hash *h = find_hash(hash_name);

      if (h) {
        printf("%zu\n", hash_size(&h->hash));
      }
    }
    
    else if (strcmp(cmd, "hash_clear") == 0) {
      char *hash_name = strtok(NULL, " \n");
      struct named_hash *h = find_hash(hash_name);
      if (h) hash_clear(&h->hash, hash_action_free);
    }

    /* hash method end */
    /* bitmap method start */
    /* 비트맵 순서
     * bitmap_mark
     * bitmap_reset
     * bitmap_all
     * bitmap_any
     * bitmap_contains
     * bitmap_none
     * bitmap_set
     * bitmap_set_all
     * bitmap_set_multiple
     * bitmap_flip
     * bitmap_count
     * bitmap_dump
     * bitmap_scan
     * bitmap_scan_and_flip
     * bitmap_test
     * bitmap_size
     * bitmap_expand */	
    
    else if (strcmp(cmd, "bitmap_mark") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int idx = atoi(strtok(NULL, " \n"));
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) bitmap_mark(b->bitmap, idx);
    }

    else if (strcmp(cmd, "bitmap_reset") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int idx = atoi(strtok(NULL, " \n"));
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) bitmap_reset(b->bitmap, idx);
    }

    else if (strcmp(cmd, "bitmap_all") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      char *start_str = strtok(NULL, " \n");
      char *count_str = strtok(NULL, " \n");

      int start = atoi(start_str);
      int count = atoi(count_str);

      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) {
        if(bitmap_all(b->bitmap, start, count)) printf("true\n");
        else printf("false\n");
      }
    }

    else if (strcmp(cmd, "bitmap_any") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      char *start_str = strtok(NULL, " \n");
      char *count_str = strtok(NULL, " \n");

      int start = atoi(start_str);
      int count = atoi(count_str);

      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) {
        if(bitmap_any(b->bitmap, start, count)) printf("true\n");
        else printf("false\n");
      }
    }

    else if (strcmp(cmd, "bitmap_contains") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      char *start_str = strtok(NULL, " \n");
      char *count_str = strtok(NULL, " \n");
      char *val_str = strtok(NULL, " \n");

      int start = atoi(start_str);
      int count = atoi(count_str);
      bool value = (strcmp(val_str, "true") == 0);
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) {
        if(bitmap_contains(b->bitmap, start, count, value)) printf("true\n");
        else printf("false\n");
      }
    }

    else if (strcmp(cmd, "bitmap_none") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      char *start_str = strtok(NULL, " \n");
      char *count_str = strtok(NULL, " \n");

      int start = atoi(start_str);
      int count = atoi(count_str);

      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) {
        if(bitmap_none(b->bitmap, start, count)) printf("true\n");
	else printf("false\n");
      }
    }

    else if (strcmp(cmd, "bitmap_set") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int idx = atoi(strtok(NULL, " \n"));
      bool value = strcmp(strtok(NULL, " \n"), "true") == 0;
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) bitmap_set(b->bitmap, idx, value);
    }

    else if (strcmp(cmd, "bitmap_set_all") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      bool value = strcmp(strtok(NULL, " \n"), "true") == 0;
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) bitmap_set_all(b->bitmap, value);
    }

    else if (strcmp(cmd, "bitmap_set_multiple") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int start = atoi(strtok(NULL, " \n"));
      int count = atoi(strtok(NULL, " \n"));
      bool value = strcmp(strtok(NULL, " \n"), "true") == 0;
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) bitmap_set_multiple(b->bitmap, start, count, value);
    }

    else if (strcmp(cmd, "bitmap_flip") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int idx = atoi(strtok(NULL, " \n"));
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) bitmap_flip(b->bitmap, idx);
    }

    else if (strcmp(cmd, "bitmap_count") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int start = atoi(strtok(NULL, " \n"));
      int count = atoi(strtok(NULL, " \n"));
      bool value = strcmp(strtok(NULL, " \n"), "true") == 0;
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) printf("%zu\n", bitmap_count(b->bitmap, start, count, value));
    }

    else if (strcmp(cmd, "bitmap_dump") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) bitmap_dump(b->bitmap);
    }

    else if (strcmp(cmd, "bitmap_scan") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int start = atoi(strtok(NULL, " \n"));
      int count = atoi(strtok(NULL, " \n"));
      bool value = strcmp(strtok(NULL, " \n"), "true") == 0;
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) printf("%zu\n", bitmap_scan(b->bitmap, start, count, value));
    }

    else if (strcmp(cmd, "bitmap_scan_and_flip") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int start = atoi(strtok(NULL, " \n"));
      int count = atoi(strtok(NULL, " \n"));
      bool value = strcmp(strtok(NULL, " \n"), "true") == 0;
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) printf("%zu\n", bitmap_scan_and_flip(b->bitmap, start, count, value));
    }

    else if (strcmp(cmd, "bitmap_test") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int idx = atoi(strtok(NULL, " \n"));
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (bitmap_test(b->bitmap, idx)) printf("true\n");
      else printf("false\n");
    }

    else if (strcmp(cmd, "bitmap_size") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) printf("%zu\n", bitmap_size(b->bitmap));
    }

    else if (strcmp(cmd, "bitmap_expand") == 0) {
      char *bitmap_name = strtok(NULL, " \n");
      int size = atoi(strtok(NULL, " \n"));
      struct named_bitmap *b = find_bitmap(bitmap_name);
      if (b) bitmap_expand(b->bitmap, size);
    }

    /* bitmap method end */
  }
} 
