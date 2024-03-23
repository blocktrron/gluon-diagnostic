#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include "gluon-diagnostic.h"
#include "daemon.h"

#define UPDATE_INTERVAL (30 * 1000)

#define HOSTNAME_PATH "/home/dbauer/test"
#define BEACON_BUFFER_SIZE 512

struct gluon_beacon_information {
	struct {
		uint8_t *buf;
		size_t len;  /* Content Length*/
		size_t size; /* Buffer Size */
	} output;
};

struct ubus_context *ubus_ctx;


struct gluon_beacon_information gbi = {};
extern struct gluon_beacon_information_source information_sources[];

int create_vendor_element_buf() {
	uint8_t *element_length;

	/* Free buffer if it exists*/
	if (gbi.output.buf) {
		free(gbi.output.buf);
	}

	/* Allocate Buffer and Init sizes */
	gbi.output.buf = calloc(BEACON_BUFFER_SIZE, 1);
	if (!gbi.output.buf) {
		return -1;
	}
	gbi.output.size = BEACON_BUFFER_SIZE;
	gbi.output.len = 0;

	/* Tag-Type(1) + Teg-Length(1) + OUI(3) + type(1) + {data} */
	gbi.output.len = 1 + 1 + 3 + 1;

	/* Header */
	gbi.output.buf[0] = 0xDD;
	gbi.output.buf[1] = 0x00; /* Set Later! */
	/* OUI */
	gbi.output.buf[2] = 0x00;
	gbi.output.buf[3] = 0x20;
	gbi.output.buf[4] = 0x91;
	/* OUI-Type */
	gbi.output.buf[5] = 0x04;

	/* Length now matches content. Make sure to update on each new information! */
	
	/* Loop through all information-sources */
	for (int i = 0; information_sources[i].name; i++) {
		log_debug("Collecting information id=%d name=%s", information_sources[i].type, information_sources[i].name);
		/* Check if we have space for T + L + {data} */
		if (gbi.output.len + 3 > gbi.output.size) {
			log_error("Buffer too small for id=%d name=%s", information_sources[i].type, information_sources[i].name);
			break;
		}

		/* Set T and L placeholder */
		gbi.output.buf[gbi.output.len] = information_sources[i].type;
		gbi.output.buf[gbi.output.len + 1] = 0x00;

		/* Save for later */
		element_length = &gbi.output.buf[gbi.output.len + 1];

		/* Collect Information */
		int ret = information_sources[i].collect(&gbi.output.buf[gbi.output.len + 2], gbi.output.size - gbi.output.len - 2);
		if (ret == 0) {
			/* No Information available */
			log_error("No Information available for id=%d name=%s", information_sources[i].type, information_sources[i].name);
			continue;
		} else if (ret > 0xff) {
			/* Too much Information */
			log_error("Too much Information for id=%d name=%s", information_sources[i].type, information_sources[i].name);
			return -ENOMEM;
		} else if (ret < 0) {
			/* Error */
			log_error("Error collecting Information for id=%d name=%s code=%d", information_sources[i].type, information_sources[i].name, ret);
			continue;
		}

		/* Update Length of Field*/
		*element_length = (uint8_t)ret;

		/* Update total Length */
		gbi.output.len += ret + 2;

		log_debug("Add Element to beacon id=%d name=%s element_length=%d total_length=%d", information_sources[i].type, information_sources[i].name, ret, gbi.output.len);
	}

	/* Set Length */
	gbi.output.buf[1] = gbi.output.len - 2;
	log_debug("Set length of beacon element element_length=%u total_length=%d", gbi.output.buf[1], gbi.output.len);

	return 0;
}

int buffer_to_hexstring(uint8_t *buf, size_t len, uint8_t *hexstring) {
	for (size_t i = 0; i < len; i++) {
		sprintf((char *)&hexstring[i * 2], "%02x", buf[i]);
	}
	return 0;
}

static void collect_information(struct uloop_timeout *timeout) {
	struct gluon_diagnostic *instance = container_of(timeout, struct gluon_diagnostic, update_timeout);
	if (create_vendor_element_buf() < 0) {
		goto out_free;
	}
	
	/* Allocate output buffer */
	size_t buf_len = gbi.output.len * 2 + 1;
	uint8_t *buf_hex = malloc(buf_len);
	
	buffer_to_hexstring(gbi.output.buf, gbi.output.len, buf_hex);

	/* Update nodes */
	log_debug("Update %s", buf_hex);
	gluon_diagnostic_interface_update(instance->ubus_ctx, (char *)buf_hex);

out_free:
	free(buf_hex);
	uloop_timeout_set(timeout, UPDATE_INTERVAL);
}

static int start_daemon() {
	struct gluon_diagnostic instance;

	uloop_init();

	instance.ubus_ctx = ubus_connect(NULL);
	if (!instance.ubus_ctx) {
		fprintf(stderr, "Failed to connect to ubus");
		return -1;
	}

	/* Init ubus */
	ubus_add_uloop(instance.ubus_ctx);
	gluon_diagnostic_ubus_init(instance.ubus_ctx);

	/* Add Information gathering timer */
	instance.update_timeout.cb = collect_information;
	uloop_timeout_set(&instance.update_timeout, 5 * 1000);

	uloop_run();

	/* Terminate */
	uloop_done();
	return 0;
}

int main(int argc, char *argv[]) {
	start_daemon();
	return 0;
}
