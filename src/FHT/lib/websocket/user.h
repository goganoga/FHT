/**
 *
 * filename: user.h
 * summary:
 * author: caosiyang
 * email: csy3228@gmail.com
 *
 */
#ifndef USER_H
#define USER_H

#include "connection.h"
#include <functional>

typedef struct user {
	uint32_t id;
	ws_conn_t *wscon;
	string msg;
	std::function<void(void)> close_bind;
	std::function<void(std::string)> read_bind;
} user_t;


user_t *user_create();

void user_disconnect(user_t* user);

void user_disconnect_cb(void* arg);

void user_destroy(user_t *user);


void frame_recv_cb(void *arg);


#endif
