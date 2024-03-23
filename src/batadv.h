#pragma once

#include <stdint.h>

struct gluon_diagnostic_batadv_neighbor_stats {
	uint16_t originator_count;
	uint16_t neighbor_count;
	struct {
		int connected;
		int tq;
	} vpn;
};

int gluon_diagnostic_get_batadv_neighbor_stats(struct gluon_diagnostic_batadv_neighbor_stats *stats);
int gluon_diagnostic_get_batadv_clients();
