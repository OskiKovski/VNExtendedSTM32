#include "gps.h"

struct gps_state gps_init(UART_HandleTypeDef * uart) {
	struct gps_state state;

	state.uart = uart;
	for(uint8_t i=0; i<100; i++) state.line_buffer[i] = '\0';
	state.writer_position = 0;
	state.reader_position = 0;
	for(uint8_t i=0; i<30; i++) state.field_buffer[i] = '\0';
	state.field_position = 0;

	state.date_day = 0;
	state.date_mounth = 0;
	state.date_year = 0;
	state.time_hour = 0;
	state.time_min = 0;
	state.time_sec = 0;

	state.latitude = 0.0;
	state.latitude_direction = '?';
	state.longitude = 0.0;
	state.longitude_direction = '?';
	state.altitude = 0.0;

	state.speed_knots = 0.0;
	state.speed_kilometers = 0.0;

	state.satelites_number = 0;
	state.quality = '?';
	state.dop = 0.0;
	state.hdop = 0.0;
	state.vdop = 0.0;

	return state;
}

void gps_recv_char(struct gps_state * state, uint8_t recv_char) {
	if (state->writer_position == 0 && recv_char == '$') {
		state->writer_position++;
	} else if (state->writer_position >= 1 && state->writer_position < 99) {
		if (recv_char == '\r' || recv_char == '\n') {
			state->line_buffer[state->writer_position - 1] = '\0';
			state->writer_position = 0;
			gps_process_line(state);
		} else {
			state->line_buffer[state->writer_position - 1] = recv_char;
			state->writer_position++;
		}
	} else {
		state->writer_position = 0;
	}
}

void gps_read_field(struct gps_state * state) {
	state->field_position = 0;
	while(state->line_buffer[state->reader_position] != ',' && state->line_buffer[state->reader_position] != '\0'
			&& state->field_position < 29) {
		state->field_buffer[state->field_position] = state->line_buffer[state->reader_position];
		state->reader_position++;
		state->field_position++;
	}
	state->field_buffer[state->field_position] = '\0';
	state->reader_position++;
}

void gps_process_line(struct gps_state * state) {
	state->reader_position = 0;
	gps_read_field(state);
	     if(strcmp(state->field_buffer, "GPRMC") == 0) gps_process_gprmc(state);
	else if(strcmp(state->field_buffer, "GPVTG") == 0) gps_process_gpvtg(state);
	else if(strcmp(state->field_buffer, "GPGGA") == 0) gps_process_gpgga(state);
	else if(strcmp(state->field_buffer, "GPGSA") == 0) gps_process_gpgsa(state);
}

void gps_process_gprmc(struct gps_state * state)
{
	//$GPRMC,212846.00,A,5025.81511,N,01639.92090,E,0.196,,140417,,,A*73
	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) {
		uint32_t tmp;
		sscanf(state->field_buffer, "%d", &tmp);
		state->time_sec = tmp % 100;
		state->time_min = (tmp / 100) % 100;
		state->time_hour = (tmp / 10000) % 100;
	}

	gps_read_field(state);

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%lf", &(state->latitude));

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%c", &(state->latitude_direction));

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%lf", &(state->longitude));

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%c", &(state->longitude_direction));

	gps_read_field(state);
	gps_read_field(state);

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) {
		uint32_t tmp;
		sscanf(state->field_buffer, "%d", &tmp);
		state->date_year = tmp % 100;
		state->date_mounth = (tmp / 100) % 100;
		state->date_day = (tmp / 10000) % 100;
	}
}

void gps_process_gpvtg(struct gps_state * state)
{
	//$GPVTG,,T,,M,0.196,N,0.363,K,A*2B
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%lf", &(state->speed_knots));

	gps_read_field(state);

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%lf", &(state->speed_kilometers));
}

void gps_process_gpgga(struct gps_state * state)
{
	//$GPGGA,212846.00,5025.81511,N,01639.92090,E,1,04,4.72,281.1,M,42.0,M,,*5F
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%d", &(state->quality));

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%d", &(state->satelites_number));

	gps_read_field(state);

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%lf", &(state->altitude));
}

void gps_process_gpgsa(struct gps_state * state)
{
	//$GPGSA,A,3,10,18,21,15,,,,,,,,,6.79,4.72,4.89*01
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);
	gps_read_field(state);

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%lf", &(state->dop));

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%lf", &(state->hdop));

	gps_read_field(state);
	if(strlen(state->field_buffer) > 0) sscanf(state->field_buffer, "%lf", &(state->vdop));
}

float radians(float degrees) {
  return degrees * M_PI / 180.0;
}

float degrees(float radians) {
  return radians * 180.0 / M_PI;
}

float course_to (float lat1, float long1, float lat2, float long2)
{
  // returns course in degrees (North=0, West=270) from position 1 to position 2,
  // both specified as signed decimal-degrees latitude and longitude.
  // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
  // Courtesy of Maarten Lamers
  float dlon = radians(long2-long1);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  float a1 = sin(dlon) * cos(lat2);
  float a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  a2 = degrees(a2);
  if (a2 < 0.0) {
    a2 += 360;
  }
  return a2;
}

float distance_between (float lat1, float long1, float lat2, float long2)
{
  // returns distance in meters between two positions, both specified
  // as signed decimal-degrees latitude and longitude. Uses great-circle
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  float delta = radians(long1-long2);
  float sdlong = sin(delta);
  float cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  float slat1 = sin(lat1);
  float clat1 = cos(lat1);
  float slat2 = sin(lat2);
  float clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = pow(delta, 2);
  delta += pow(clat2 * sdlong, 2);
  delta = sqrt(delta);
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6372795;
}


