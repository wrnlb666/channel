#ifndef __CHANNEL_H__
#define __CHANNEL_H__


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct chan chan_t;

chan_t* chan_init(size_t buf_size);
void chan_deinit(chan_t* ch);
size_t chan_len(chan_t* ch);
size_t chan_cap(chan_t* ch);
void chan_push(chan_t* ch, void* data);
void* chan_pop(chan_t* ch);


#endif  // __CHANNEL_H__
