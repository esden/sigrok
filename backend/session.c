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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "sigrok.h"
#include "device.h"
#include "session.h"
#include "analyzer.h"

#include <zip.h>

/* there can only be one session at a time */
struct session *session;


struct session *session_load(char *filename)
{
	struct session *session;

	/* TODO: implement */
	session = NULL;

	return session;
}


struct session *session_new(void)
{

	session = g_malloc0(sizeof(struct session));

	return session;
}


void session_destroy(void)
{

	g_slist_free(session->devices);

	/* TODO: loop over protocols and free them */

	g_free(session);

}


void session_device_clear(void)
{

	g_slist_free(session->devices);
	session->devices = NULL;

}


int session_device_add(struct device *device)
{
	int ret;

	ret = device->plugin->open(device->plugin_index);
	if(ret == SIGROK_OK)
		session->devices = g_slist_append(session->devices, device);

	return ret;
}


void session_pa_clear(void)
{

	/* the protocols are pointers to the global set of PA plugins, so don't free them */
	g_slist_free(session->analyzers);
	session->analyzers = NULL;

}


void session_pa_add(struct analyzer *an)
{

	session->analyzers = g_slist_append(session->analyzers, an);

}


void session_output_clear(void)
{

	if(session->output_filename)
	{
		g_free(session->output_filename);
		session->output_filename = NULL;
	}

	session->output_callbacks = NULL;

}


void session_output_add_file(char *filename)
{

	if(session->output_filename)
		g_free(session->output_filename);

	session->output_filename = g_strdup(filename);

}


void session_output_add_callback(output_callback callback)
{

	session->output_callbacks = g_slist_append(session->output_callbacks, callback);

}


int session_start(void)
{
	struct device *device;
	GSList *l;
	int ret;

	g_message("starting acquisition");
	for(l = session->devices; l; l = l->next)
	{
		device = l->data;
		if( (ret = device->plugin->start_acquisition(device->plugin_index, device)) != SIGROK_OK)
			break;
	}

	return ret;
}


void session_stop(void)
{
	struct device *device;
	GSList *l;

	if(session->output_file)
		fclose(session->output_file);

	g_message("stopping acquisition");
	for(l = session->devices; l; l = l->next)
	{
		device = l->data;
		device->plugin->stop_acquisition(device->plugin_index, device);
	}

}


void session_bus(struct device *device, struct datafeed_packet *packet)
{
	GSList *l;
	output_callback cb;

	/* TODO: send packet through PA pipe, and send the output of that to
	 * the callbacks
	 */

	for(l = session->output_callbacks; l; l = l->next)
	{
		cb = l->data;
		cb(device, packet);
	}

}


void make_metadata(char *filename)
{
	GSList *l, *p;
	struct device *device;
	struct probe *probe;
	FILE *f;
	int devcnt;

	f = fopen(filename, "wb");

	/* general */

	/* devices */
	devcnt = 1;
	for(l = session->devices; l; l = l->next) {
		device = l->data;
		fprintf(f, "[device]\n");
		fprintf(f, "driver = %s\n", device->plugin->name);
		if(device->datastore)
			fprintf(f, "capturefile = raw-%d\n", devcnt);
		for(p = device->probes; p; p = p->next)	{
			probe = p->data;
			if(probe->enabled)
			{
				fprintf(f, "probe %d", probe->index);
				if(probe->name)
					fprintf(f, " name \"%s\"", probe->name);
				if(probe->trigger)
					fprintf(f, " trigger \"%s\"", probe->trigger);
				fprintf(f, "\n");
			}
		}
		devcnt++;
	}

	/* TODO: protocol analyzers */

	fclose(f);

}


int session_save(char *filename)
{
	GSList *l, *d;
	struct device *device;
	struct datastore *ds;
	struct zip *zipfile;
	struct zip_source *src;
	int bufcnt, devcnt, tmpfile, ret, error;
	char version[1], rawname[16], metafile[32], *buf;

	/* quietly delete it first, libzip wants replace ops otherwise */
	unlink(filename);

	if( !(zipfile = zip_open(filename, ZIP_CREATE, &error)) )
		return SIGROK_NOK;

	/* version */
	version[0] = '1';
	if( !(src = zip_source_buffer(zipfile, version, 1, 0)) )
		return SIGROK_NOK;
	if(zip_add(zipfile, "version", src) == -1) {
		g_message("error saving version into zipfile: %s", zip_strerror(zipfile));
		return SIGROK_NOK;
	}

	/* metadata */
	strcpy(metafile, "sigrok-meta-XXXXXX");
	if( (tmpfile = mkstemp(metafile)) == -1)
		return SIGROK_NOK;
	close(tmpfile);
	make_metadata(metafile);
	if( !(src = zip_source_file(zipfile, metafile, 0, -1)) )
		return SIGROK_NOK;
	if(zip_add(zipfile, "metadata", src) == -1)
		return SIGROK_NOK;
	unlink(metafile);

	/* raw */
	devcnt = 1;
	for(l = session->devices; l; l = l->next) {
		device = l->data;
		ds = device->datastore;
		if(ds) {
			buf = malloc(ds->num_units * ds->ds_unitsize + DATASTORE_CHUNKSIZE);
			bufcnt = 0;
			for(d = ds->chunklist; d; d = d->next) {
				memcpy(buf + bufcnt, d->data, DATASTORE_CHUNKSIZE);
				bufcnt += DATASTORE_CHUNKSIZE;
			}
			if( !(src = zip_source_buffer(zipfile, buf, ds->num_units * ds->ds_unitsize, TRUE)) )
				return SIGROK_NOK;
			snprintf(rawname, 15, "raw-%d", devcnt);
			if(zip_add(zipfile, rawname, src) == -1)
				return SIGROK_NOK;
		}
		devcnt++;
	}

	if( (ret = zip_close(zipfile)) == -1) {
		g_message("error saving zipfile: %s", zip_strerror(zipfile));
		return SIGROK_NOK;
	}

	return SIGROK_OK;
}








