#include <stdlib.h>
#include "list.h"

list_t* create_list() {
    list_t* list = malloc(sizeof(list_t));

    list->len = 0;
    list->head = NULL;

    list->add = list_add;
    list->remove_c = list_remove_c;
    list->remove_i = list_remove_i;
    list->get_c = list_get_c;
    list->get_i = list_get_i;
    list->free = list_free;

    return list;
}

void list_add(list_t* self, void* content) {
    item_t* item = malloc(sizeof(item_t));
    item->content = content;

    if (self->head)
        item->next = self->head;
    else
        item->next = NULL;
    self->head = item;
    self->len++;
}

static void remove_from_list(item_t* target, list_t* list, item_t* prev) {
    if (prev == NULL)
        list->head = target->next;
    else
        prev->next = target->next;
    list->len--;
    free(target);
}

int list_remove_c(list_t* self, int (*condition)(void*)) {
    item_t* item, *prev;

    ITER_LIST(item, self) {
        if (condition(item)) {
            remove_from_list(item, self, prev);
            return 0;
        }
        prev = item;
    }

    return -1;
}

int list_remove_i(list_t* self, int index) {
    item_t* item, *prev;
    int i = 0;

    prev = NULL;
    ITER_LIST(item, self) {
        if (i++ == index) {
            remove_from_list(item, self, prev);
            return 0;
        }
        prev = item;
    }

    return -1;
}

void* list_get_c(list_t* self, int (*condition)(void*)) {
    item_t* item;

    ITER_LIST(item, self)
        if (condition(item->content))
            return item->content;

    return NULL;
}

void* list_get_i(list_t* self, int index) {
    item_t* item;
    int i  = 0;

    ITER_LIST(item, self)
        if (i++ == index)
            return item->content;

    return NULL;
}

void list_free(list_t* self, void (*free_content)(void*)) {
    item_t* item;

    ITER_LIST(item, self) {
        if (free_content)
            free_content(item->content);
        free(item);
    }

    free(self);
}
