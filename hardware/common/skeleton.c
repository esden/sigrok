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

#include "sigrok.h"
#include "hwplugin.h"
#include "hwcommon.h"
#include "session.h"



int hw_init(char *deviceinfo)
{

}


int hw_opendev(int device_index)
{

}


void hw_closedev(int device_index)
{

}


void hw_cleanup(void)
{

}


char *hw_get_identifier(int device_index)
{

}


char *hw_get_device_info(int device_index, int device_info_id)
{

}


int hw_get_status(int device_index)
{

}


int *hw_get_capabilities(void)
{

}


int hw_set_configuration(int device_index, int capability, char *value)
{

}


int hw_start_acquisition(int device_index, gpointer session_device_id)
{

}


void hw_stop_acquisition(int device_index, gpointer session_device_id)
{

}



struct device_plugin plugin_info = {
	"skeleton",
	1,
	hw_init,
	hw_cleanup,

	hw_opendev,
	hw_closedev,
	hw_get_device_info,
	hw_get_status,
	hw_get_capabilities,
	hw_set_configuration,
	hw_start_acquisition,
	hw_stop_acquisition
};

