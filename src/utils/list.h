#ifndef __LIST_H__
#define __LIST_H__

#define ITER_LIST(item, list) for (item = list->head; item; item = item->next)

/**
 * An item in a linked list.
 */
typedef struct item_s item_t;
struct item_s {
    void* content;
    item_t* next;
};

/**
 * A linked list that can store generic type data.
 */
typedef struct list_s list_t;
struct list_s {
    item_t* head;
    int len;

    /**
     * Add data to the list
     *
     * An item will be created to carry the content of data and inserted to the
     * head of the list.
     *
     * @param content A pointer to the content of data
     */
    void (*add)(list_t* self, void* content);

    /**
     * Remove the indexth item from the list
     *
     * @return 0 on success. -1 if the index is out of range
     */
    int (*remove)(list_t* self, int index);

    /**
     * Get data in the indexth item from the list
     *
     * @return A pointer to the data on success. NULL if the index is out of
     *         range
     */
    void* (*get)(list_t* self, int index);

    /**
     * Free the list.
     *
     * All items in the list will be free. And if the function free_content is
     * not NULL, it will be used the free the data carried by items. Otherwise
     * those data will not be free.
     */
    void (*free)(list_t* self, void (*free_content)(void*));
};


list_t* create_list();

void list_add(list_t*, void*);
int list_remove(list_t*, int);
void* list_get(list_t*, int);
void list_free(list_t*, void (void*));

#endif
