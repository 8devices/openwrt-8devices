/*
 *Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 *Permission to use, copy, modify, and/or distribute this software for any
 *purpose with or without fee is hereby granted, provided that the above
 *copyright notice and this permission notice appear in all copies.
 *
 *THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE
 */

#include <gps.h>
#include <gpsdclient.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MODE_STRING_COUNT 4
#define STATUS_STRING_COUNT 3

static char *status_str[STATUS_STRING_COUNT] =
{
	"status_no_fix",
	"status_fix",
	"status_dgps_fix"
};

int main(void)
{
	struct gps_data_t gps_info;
	struct fixsource_t gpsd_source;
	int status, flags, indoor_dep, orientation;
	bool latlon_set = false, alti_set = false, hdop_set = false, pdop_set = false, vdop_set = false, is_data_valid = false;
	FILE *fd;
	char path[] = "/etc/afc/location-ipq_enc.conf";
	char decrypted_path[] = "/etc/afc/location-ipq.conf";
	bool file_found = true, update_pending = true;

	(void)gpsd_source_spec(NULL, &gpsd_source);

	flags = WATCH_ENABLE | WATCH_JSON;

	status = gps_open(gpsd_source.server, gpsd_source.port, &gps_info);

	if (status != 0) {
		printf("No GPSD running or network error:%d,%s\n",errno,gps_errstr(errno));
		exit(EXIT_FAILURE);
	}
	if (access(path, F_OK) != 0){
		printf("Error: Location config file not found!!\n");
		file_found = false;
	}

	(void)gps_stream(&gps_info, flags, gpsd_source.device);
	//printf("\nWaiting for GPS data!!\n");
	/* Wait for data from GPSD for a maximum of 10 seconds */
	while ((gps_waiting(&gps_info, 10000000)) && (is_data_valid == false)) {
		if (-1 == gps_read(&gps_info))
		{
			printf("Read failure!\n");
			exit(EXIT_FAILURE);
		}
		if (PACKET_SET == (PACKET_SET & gps_info.set))
		{
			if (MODE_SET != (MODE_SET & gps_info.set)) {
				continue;
			}
			/* Checking if the mode is within the range */
			if ((gps_info.fix.mode < 0) || (gps_info.fix.mode >= MODE_STRING_COUNT))
			{
				gps_info.fix.mode = 0;
			}

			/* Updating the indoor deployment value to outdoor(2)*/
			indoor_dep = 2;
			/* Updating the orientation to zero degree as the horizontal area covered by GPS is a circle */
			orientation = 0;

			if (LATLON_SET == (LATLON_SET & gps_info.set))
			{
				if (isfinite(gps_info.fix.latitude) && isfinite(gps_info.fix.longitude))
				{
					latlon_set = true;
					/* Display data from the GPS receiver if valid */
					printf("\nLatitude: %.6f Longitude: %.6f\n",
					gps_info.fix.latitude, gps_info.fix.longitude);
				}
				else
				{
					latlon_set = false;
					printf("\nLatitude and Longitude: Data not found\n");
				}
			}

			else
			{
				printf("\nLatLon not set!\n");
				latlon_set = false;
			}
			if (ALTITUDE_SET == (ALTITUDE_SET & gps_info.set))
			{
				if (isfinite(gps_info.fix.altitude))
				{
					alti_set = true;
					printf("Height: %.6f \n",gps_info.fix.altitude);
				}
				else
				{
					alti_set = false;
					printf("Height: Data not found\n");
				}
			}
			else
			{
				printf("Altitude not set\n");
				alti_set = false;
			}
			if (isfinite(gps_info.dop.hdop))
			{
				hdop_set = true;
				printf("Major axis: %.1f\n", gps_info.dop.hdop);
			}
			else
			{
				hdop_set = false;
				printf("Major axis : Data not found\n");
			}
			if (isfinite(gps_info.dop.pdop))
			{
				pdop_set = true;
				printf("Minor axis: %.1f\n", gps_info.dop.pdop);
			}
			else
			{
				pdop_set = false;
				printf("Minor axis: Data not found\n");
			}
			if (isfinite(gps_info.dop.vdop))
			{
				vdop_set = true;
				printf("Vertical Uncertainty: %.1f\n", gps_info.dop.vdop);
			}
			else
			{
				vdop_set = false;
				printf("vertical Uncertainty: Data not found\n");
			}
			printf("Indoor deployment: %d\n",indoor_dep);
			printf("Orientation: %d\n",orientation);
			if ((latlon_set == true) && (alti_set == true) && (hdop_set == true) && (pdop_set == true) && (vdop_set == true))
			{
				is_data_valid = true;
				printf("GPS Data fields are set!!\n");
			}
		}
		else
		{
			printf("No data packets received...\n");
		}
		if ((file_found == true) && (is_data_valid == true))
		{
			system("/usr/sbin/encrypt_client_app decrypt location");
			system("sync");
			if ((fd = fopen(decrypted_path, "w")) != NULL){
				printf("Updating %s\n",decrypted_path);
				fprintf(fd,"#############################\n# Location Configuration file.\n#############################\n\n");
				fprintf(fd,"# Common Location Fields\nlocation_object_ellipse = 1\n\nlocation_object_linear_polygon = 0\n\nlocation_object_radial_polygon = 0\n\n");
				fprintf(fd,"location_height = %.6f\n\nlocation_vertical_uncertainity = %.1f\n\nlocation_indoordep = %d\n\n",gps_info.fix.altitude,gps_info.dop.vdop,indoor_dep);
				fprintf(fd,"# Ellipse location fields\nellipse_minor_axis = %.1f\n\nellipse_major_axis = %.1f\n\nellipse_orientation = %d\n\nellipse_longitude = %.6f\n\nellipse_latitude = %.6f\n\n",gps_info.dop.pdop,gps_info.dop.hdop,orientation,gps_info.fix.longitude,gps_info.fix.latitude);
				fprintf(fd,"# Linear Polygon location fields\nlinear_polygon_longitude = -121.9149914479106\n\nlinear_polygon_latitude = 37.364992615471664\n\n");
				fprintf(fd,"# Radial Polygon location fields\nradial_polygon_longitude = -121.9149914479106\n\nradial_polygon_lattitude = 37.364992615471664\n\nradial_polygon_length = 16.1\n\nradial_polygon_angle = 30.5\n");
				update_pending = false;
				fclose(fd);
			}
			else
			{
				printf("Error opening %s",decrypted_path);
				exit(EXIT_FAILURE);
			}
			system("/usr/sbin/encrypt_client_app encrypt location");
			system("sync");
		}
	}
	sleep(1);
	flags = WATCH_DISABLE;
	(void)gps_stream(&gps_info, flags, gpsd_source.device);
	(void)gps_close(&gps_info);
	if (update_pending == false)
	{
		printf("Location config file updated...Exiting!!\n");
		exit(EXIT_SUCCESS);
	}
	else if (file_found == false)
        {
                printf("\nFile update failed: Location config file not found\n");
		exit(EXIT_SUCCESS);
        }
	else
	{
		printf("GPS Wait timed out..Retrying!!\n");
		exit(EXIT_FAILURE);
	}
}

