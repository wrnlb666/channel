#include "channel.h"


typedef struct ch_node {
    void*               data;
    struct ch_node*     next;
} ch_node_t;

typedef struct ch_list {
    size_t      size;
    ch_node_t*  head;
    ch_node_t*  tail;
} ch_list_t;

typedef struct chan {
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;
    size_t              buf_size;
    ch_list_t           list;       // buffered list
    ch_list_t           free_list;  // free list for buffered list, reduce allocation frequency
} chan_t;


static inline ch_node_t* ch_node_alloc(chan_t* ch) {
    ch_list_t* list = &ch->free_list;
    if (list->size == 0) {
        return malloc(sizeof (ch_node_t));
    }
    ch_node_t* node = list->head;
    list->head = list->head->next;
    list->size -= 1;
    return node;
}


static inline ch_node_t* ch_node_init(chan_t* ch, void* data) {
    ch_node_t* node = ch_node_alloc(ch);
    *node = (ch_node_t) {
        .data = data,
        .next = NULL,
    };
    return node;
}

static inline void ch_list_push(chan_t* ch, void* data) {
    ch_list_t* list = &ch->list;
    if (list->head == NULL) {
        list->head = list->tail = ch_node_init(ch, data);
        list->size += 1;
        return;
    }
    list->tail->next = ch_node_init(ch, data);
    list->tail = list->tail->next;
    list->size += 1;
    return;
}


static inline void* ch_list_pop(chan_t* ch) {
    ch_list_t* list = &ch->list;
    ch_list_t* free_list = &ch->free_list;

    // delete node from list
    ch_node_t* node = list->head;
    void* data = node->data;
    list->head = node->next;
    list->size -= 1;
    
    // add node to free_list
    node->next = free_list->head;
    free_list->head = node;
    free_list->size += 1;

    return data;
}

static inline void ch_list_deinit(ch_list_t* list) {
    ch_node_t* node = list->head;
    ch_node_t* next;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
}

chan_t* chan_init(size_t buf_size) {
    chan_t* ch = malloc(sizeof (chan_t));
    if (pthread_mutex_init(&ch->mutex, NULL) != 0) {
        free(ch);
        return NULL;
    }
    if (pthread_cond_init(&ch->cond, NULL) != 0) {
        pthread_mutex_destroy(&ch->mutex);
        free(ch);
        return NULL;
    }
    ch->buf_size = buf_size;
    ch->list = (ch_list_t){};
    ch->free_list = (ch_list_t){};
    return ch;
}

void chan_deinit(chan_t* ch) {
    pthread_mutex_lock(&ch->mutex);
    pthread_cond_destroy(&ch->cond);
    ch_list_deinit(&ch->list);
    ch_list_deinit(&ch->free_list);
    pthread_mutex_destroy(&ch->mutex);
    free(ch);
}

size_t chan_len(chan_t* ch) {
    return ch->list.size > ch->buf_size ? ch->buf_size : ch->list.size;
}

size_t chan_cap(chan_t* ch) {
    return ch->buf_size;
}

void chan_push(chan_t* ch, void* data) {
    pthread_mutex_lock(&ch->mutex);
    ch_list_push(ch, data);
    pthread_cond_signal(&ch->cond);
    while (ch->list.size > ch->buf_size) {
        pthread_cond_wait(&ch->cond, &ch->mutex);
    }
    pthread_mutex_unlock(&ch->mutex);
}

void* chan_pop(chan_t* ch) {
    pthread_mutex_lock(&ch->mutex);
    while (ch->list.size == 0) {
        pthread_cond_wait(&ch->cond, &ch->mutex);
    }
    void* data = ch_list_pop(ch);
    pthread_cond_signal(&ch->cond);
    pthread_mutex_unlock(&ch->mutex);
    return data;
}