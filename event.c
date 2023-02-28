#include <stdlib.h>
#include "event.h"
#include "includes.h"

#define IOCP_BUFFER_SIZE 512

event_system_error_callback_t g_event_error_cb;

struct loop_node {
	struct loop_node* next, * prev;
	fd_t fd;
	event_callback_t user_callback;
	void* user_data;
	u32 ref_counts;
	u8 event_state;
};

struct loop_list {
	struct loop_node* head, * tail;
	usize nodes_count;
};

static struct loop_list* event_loop_list_new() {
	struct loop_list* list = (struct loop_list*)malloc(sizeof(struct loop_list));
	
	memset(list, 0, sizeof(struct loop_list));

	return list;
}

static void event_loop_list_push(struct loop_list* list, struct loop_node* node) {
	node->next = NULL;
	node->prev = NULL;

	if (list->tail) {
		list->tail->next = node;
		node->prev = list->tail;
	}
	else {
		list->head = node;
	}

	list->tail = node;

	++list->nodes_count;
}

static u8 event_loop_list_empty(struct loop_list* list) {
	return list->nodes_count == 0;
}

static void event_loop_list_pop(struct loop_list* list, struct loop_node** node) {
	if (!list->head) {
		*node = NULL;
		return;
	}

	*node = list->head;

	list->head = (*node)->next;

	if (!list->head) {
		list->tail = NULL;
	}

	--list->nodes_count;
}

static struct loop_node* event_loop_list_node_new() {
	struct loop_node* new_node = (struct loop_node*)malloc(sizeof(struct loop_node));

	memset(new_node, 0, sizeof(struct loop_node));
	new_node->ref_counts = 1;

	return new_node;
}

static void event_loop_list_node_force_free(struct loop_node* node) {
	free(node);
}

static void event_loop_list_node_free(struct loop_node* node) {
	if (--node->ref_counts == 0) {
		event_loop_list_node_force_free(node);
	}
}

static void event_loop_list_remove(struct loop_list* list, struct loop_node* node) {
	if (list->head == node) {
		struct loop_list** retire_node;
		event_loop_list_pop(list, &retire_node);
	}
	else if (list->tail == node) {
		if (list->tail == list->head) {
			struct loop_list** retire_node;
			event_loop_list_pop(list, &retire_node);
		}
		else {
			list->tail = list->tail->prev;
		}
	}
	else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	event_loop_list_node_free(node);

	--list->nodes_count;
}

static void event_loop_list_free(struct loop_list* list) {
	if (list->nodes_count > 0) {
		for (struct loop_node* ptr = list->head; ptr != NULL;) {
			struct loop_node* next = ptr->next;
			event_loop_list_node_free(ptr);
			ptr = next;
		}
	}

	free(list);
}

struct event_loop {
	fd_t iocp_fd;
	struct loop_list* events;
	u8 started;
};

static void event_loop_proactor_worker(struct event_loop* loop) {
	LPOVERLAPPED overlapped;

	fd_t completion_port = loop->iocp_fd;

	DWORD bytes_transferred = 0;

	PULONG completion_key;

	while (loop->started && !event_loop_list_empty(loop->events)) {
		if (GetQueuedCompletionStatus(completion_port, &bytes_transferred, &completion_key, &overlapped, 100)) {
			struct loop_node* event_data = (struct loop_node*)completion_key;

			if (bytes_transferred == 0) {
				g_event_error_cb(loop, event_data);
				continue;
			}

			if (event_data->event_state != EVENT_STATE_PENDING) {
				continue;
			}

			event_callback_t callback = event_data->user_callback;

			if (callback) {
				event_data->user_callback = NULL;
				event_data->event_state = EVENT_STATE_ACTIVE;
				callback(loop, bytes_transferred, event_data->user_data);
			}
		}
	}

	loop->started = 0;
}

event_loop_t* event_loop_new() {
	fd_t iocp_fd = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
	if (!iocp_fd) return NULL;

	event_loop_t* loop = (event_loop_t*)malloc(sizeof(struct event_loop));
	memset(loop, 0, sizeof(struct event_loop));
	loop->iocp_fd = iocp_fd;
	loop->events = event_loop_list_new();

	return loop;
}

event_key_t event_add(fd_t fd, event_loop_t* loop) {
	struct loop_node* new_node = event_loop_list_node_new();
	new_node->fd = fd;
	++new_node->ref_counts;

	CreateIoCompletionPort(fd, loop->iocp_fd, new_node, 0);

	new_node->event_state = EVENT_STATE_ACTIVE;
	
	event_loop_list_push(loop->events, new_node);

	return new_node;
}

void event_submit(event_key_t event_key, event_callback_t event_cb) {
	struct loop_node* node = (struct loop_node*)event_key;

	node->event_state = EVENT_STATE_PENDING;
	node->user_callback = event_cb;
}

void event_close(event_loop_t* loop, event_key_t event_key) {
	struct loop_node* node = (struct loop_node*)event_key;

	CancelIo(node->fd);
	event_loop_list_remove(loop->events, node);

	event_loop_list_node_force_free(node);
}

void event_force_close(event_key_t event_key) {
	struct loop_node* node = (struct loop_node*)event_key;

	CancelIo(node->fd);

	event_loop_list_node_force_free(node);
}

void event_loop_run(event_loop_t* loop) {
	loop->started = 1;
	event_loop_proactor_worker(loop);
}

void event_loop_stop(event_loop_t* loop) {
	loop->started = 0;
}

void event_loop_free(event_loop_t* loop) {
	CloseHandle(loop->iocp_fd);
	event_loop_list_free(loop->events);
	free(loop);
}

void event_set_user_data(event_key_t event_key, void* user_data) {
	((struct loop_node*)event_key)->user_data = user_data;
}

void event_loop_default_error_handler(event_loop_t* loop, void* event_key) {
	abort();
}

void event_loop_set_error_handler(event_system_error_callback_t error_cb) {
	g_event_error_cb = error_cb;
}