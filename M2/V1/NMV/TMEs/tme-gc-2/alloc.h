#ifndef _ALLOC_H_
#define _ALLOC_H_

struct object_header {
  struct object_header * prev; // header precedent
  struct object_header * next; // header suivant
  int     color;
  int is_racine;
  unsigned int object_size;       // gives the exact object size
};

struct object_header *pre_malloc(unsigned int size);
void                  pre_free(void *ptr);
struct object_header *toHeader(void *ptr);
void *                toObject(struct object_header *header);
void                  print_stats();

#endif
