#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include <linux/unistd.h>
#include <linux/kernel.h>

#include "batadv.h"
#include "util.h"
#include "information.h"

#ifndef BUILD_MONITOR
#define INFORMATION_SOURCE(__name, __type) \
	{ \
		.name = #__name, \
		.type = __type, \
		.collect = gluon_beacon_diagnostic_information_##__name##_collect, \
	}
#else
#define INFORMATION_SOURCE(__name, __type) \
	{ \
		.name = #__name, \
		.type = __type, \
		.parse = gluon_beacon_diagnostic_information_##__name##_parse, \
	}
#endif

#ifndef BUILD_MONITOR
int gluon_beacon_diagnostic_information_hostname_collect(uint8_t *buffer, size_t buffer_size) {
	int ret;

	ret = gethostname((char *)buffer, buffer_size);
	if (ret) {
		return ret;
	}

	return strlen((char *)buffer);
}

int gluon_beacon_diagnostic_information_node_id_collect(uint8_t *buffer, size_t buffer_size) {
	char *node_id_ascii;
	size_t node_id_ascii_len;
	int ret;

	/* len_out = ETH_ALEN */
	if (buffer_size < 6) {
		return -1;
	}

	ret = gd_read_file("/lib/gluon/core/sysconfig/primary_mac", &node_id_ascii, &node_id_ascii_len);
	if (ret) {
		return ret;
	}

	if (node_id_ascii_len < 18) {
		free(node_id_ascii);
		return -1;
	}

	ret = gd_parse_mac_address_ascii(node_id_ascii, buffer);
	free(node_id_ascii);
	if (ret < 0) {
		return -1;
	}

	return 6;
}

int gluon_beacon_diagnostic_information_batman_adv_collect(uint8_t *buffer, size_t buffer_size) {
	struct gluon_diagnostic_batadv_neighbor_stats stats = {};
	int ret;

	if (buffer_size < 3) {
		return -1;
	}

	ret = gluon_diagnostic_get_batadv_neighbor_stats(&stats);
	if (ret)
		return -1;
	

	buffer[0] = stats.vpn.connected ? 1 : 0;
	buffer[1] = stats.vpn.tq;
	buffer[2] = stats.originator_count;

	return 3;
}

int gluon_beacon_diagnostic_information_uptime_collect(uint8_t *buffer, size_t buffer_size) {
	struct sysinfo s_info;
	uint32_t uptime_minutes;
	int ret;

	ret = sysinfo(&s_info);
	if (ret) {
		return -1;
	}

	uptime_minutes = htonl(s_info.uptime / 60);

	memcpy(buffer, &uptime_minutes, sizeof(uptime_minutes));

	return sizeof(uptime_minutes) / sizeof(uint8_t);
}

#else

int gluon_beacon_diagnostic_information_hostname_parse(const uint8_t *buffer, size_t buffer_size) {
	char hostname[255] = {};
	const uint8_t *ie_buf = &buffer[2];
	uint8_t ie_len = buffer[1];

	memcpy(hostname, ie_buf, ie_len);
	printf("Hostname: %s\n", hostname);
	return 0;
}

int gluon_beacon_diagnostic_information_node_id_parse(const uint8_t *buffer, size_t buffer_size) {
	const uint8_t *ie_buf = &buffer[2];
	uint8_t ie_len = buffer[1];
	
	/* We Print Node-ID on top */

	return 0;
}

int gluon_beacon_diagnostic_information_batman_adv_parse(const uint8_t *buffer, size_t buffer_size) {
	const uint8_t *ie_buf = &buffer[2];
	uint8_t ie_len = buffer[1];
	
	printf("VPN connected: %s\n", ie_buf[0] ? "Yes" : "No");
	printf("VPN-TQ: %d\n", ie_buf[1]);
	printf("Neighbor count: %d\n", ie_buf[2]);

	return 0;
}

int gluon_beacon_diagnostic_information_uptime_parse(const uint8_t *buffer, size_t buffer_size) {
	const uint8_t *ie_buf = &buffer[2];
	uint8_t ie_len = buffer[1];
	uint32_t uptime_minutes;

	memcpy(&uptime_minutes, ie_buf, sizeof(uptime_minutes));
	printf("Uptime: %d minutes\n", ntohl(uptime_minutes));

	return 0;
}

#endif


struct gluon_beacon_information_source information_sources[] = {
	INFORMATION_SOURCE(hostname, 0),
	INFORMATION_SOURCE(node_id, 1),
	INFORMATION_SOURCE(uptime, 2),
	INFORMATION_SOURCE(batman_adv, 20),
	{},
};
