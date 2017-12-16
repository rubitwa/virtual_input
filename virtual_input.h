#ifndef _VIRTUAL_INPUT_H_
#define _VIRTUAL_INPUT_H_

#include <linux/input.h>

// virtual key node
struct vk_node {
	char *key;
	int scancode;
	struct vk_node *next;
};

// virtual device config
struct vd_config {
	char *name;
	char *input;
	struct vk_node *vks;
	// for skip vks iteration
	int *table;
	unsigned int min;
	unsigned int max;
};

void fprint_namespace(void);
int get_input_code(const char *key);
const char *get_input_name(int code);

int vd_config_read(FILE * f, struct vd_config *config);
int vd_config_add_button(struct vd_config *config, char *key, int scancode);
void vd_config_table_rebuild(struct vd_config *config);
int vd_create(struct vd_config *config);
void vd_send_event(int fd, int type, int code, int value);
void vd_destroy(int fd);

static void interrupt_handler(int sig);
int test_grab(int fd, int grab_flag);

#endif