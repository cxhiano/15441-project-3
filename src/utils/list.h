#ifndef __LIST_H__
#define __LIST_H__

#define ITER_LIST(item, list) for (item = list->head; item; item = item->next)

typedef struct item_s item_t;
struct item_s {
    void* content;
    item_t* next;
};

typedef struct list_s list_t;
struct list_s {
    item_t* head;
    int len;

    void (*add)(list_t* self, void* content);
    int (*remove_c)(list_t* self, int (*condition)(void*));
    int (*remove_i)(list_t* self, int index);
    void* (*get_c)(list_t* self, int (*condition)(void*));
    void* (*get_i)(list_t* self, int index);
    void (*free)(list_t* self, void (*free_content)(void*));
};


list_t* create_list();

void list_add(list_t*, void*);
int list_remove_c(list_t*, int (void*));
int list_remove_i(list_t*, int);
void* list_get_c(list_t*, int (void*));
void* list_get_i(list_t*, int);
void list_free(list_t*, void (void*));

#endif
