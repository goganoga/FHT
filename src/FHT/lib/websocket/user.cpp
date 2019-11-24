#include "user.h"


user_t *user_create() {
	user_t *user = new (nothrow) user_t;
	if (user) {
		user->id = 0;
		user->wscon = ws_conn_new();
		user->msg = "";
	}
	return user;
}


void user_destroy(user_t *user) {
	if (user) {
		if (user->wscon) {
			ws_conn_free(user->wscon);
		}
		auto a = std::move(user->close_bind);
		auto b = std::move(user->read_bind);
		delete user;
	}
}

void user_disconnect_cb(void* arg) {
	// LOG >> std::cout << __func__ << std::endl;
	user_t* user = (user_t*)arg;
	if (user->close_bind)
		user->close_bind();
	user_disconnect(user);
}

void user_disconnect(user_t* user) {
	if (user) {
		user_destroy(user);
	}
}

void frame_recv_cb(void *arg) {
	user_t *user = (user_t*)arg;
	if (user->wscon->frame->payload_len > 0) {
		user->msg += string(user->wscon->frame->payload_data, user->wscon->frame->payload_len);
	}
	if (user->wscon->frame->fin == 1) {
		if (user->read_bind)
			user->read_bind(user->msg);

		user->msg = "";
	}
}
