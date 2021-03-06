/*
 * This file is part of the sigrok project.
 *
 * Copyright (C) 2010 Bert Vermeulen <bert@biot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SIGROK_DEVICE_H
#define SIGROK_DEVICE_H

#include <glib.h>
#include "hwplugin.h"
#include "datastore.h"


/* this represents a generic device connected to the system. For device-specific
 * information, ask the plugin. The plugin_index refers the device index within
 * thgat plugin; it may be handling more than one device. All relevant plugin calls
 * take a device_index parameter for this.
 */
struct device {
	/* which plugin handles this device */
	struct device_plugin *plugin;
	/* a plugin may handle multiple devices of the same type */
	int plugin_index;
	/* list of struct probe* */
	GSList *probes;
	/* data acquired by this device, if any */
	struct datastore *datastore;
};

struct probe {
	int index;
	gboolean enabled;
	char *name;
	char *trigger;
};

extern GSList *devices;

void device_scan(void);
void device_close_all(void);
GSList *device_list(void);
struct device *device_new(struct device_plugin *plugin, int plugin_index);
void device_clear(struct device *device);
void device_destroy(struct device *dev);

void device_probe_clear(struct device *device, int probenum);
void device_probe_add(struct device *device, char *name);
struct probe *probe_find(struct device *device, int probenum);
void device_probe_name(struct device *device, int probenum, char *name);

void device_trigger_clear(struct device *device);
void device_trigger_set(struct device *device, int probenum, char *trigger);

#endif
