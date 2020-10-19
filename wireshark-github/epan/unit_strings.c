/* unit_strings.c
 * Units to append to field values
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "config.h"

#include <wsutil/utf8_entities.h>
#include <wsutil/str_util.h>
#include "unit_strings.h"

char* unit_name_string_get_value(guint32 value, unit_name_string* units)
{
    if (units->plural == NULL)
        return units->singular;

    return plurality(value, units->singular, units->plural);
}

char* unit_name_string_get_value64(guint64 value, unit_name_string* units)
{
    if (units->plural == NULL)
        return units->singular;

    return plurality(value, units->singular, units->plural);
}

char* unit_name_string_get_double(double value, unit_name_string* units)
{
    if (units->plural == NULL)
        return units->singular;

    return plurality(value, units->singular, units->plural);
}

/*
 * A default set of unit strings that dissectors can use for
 * header fields.  Some units intentionally have a space
 * character in them for spacing between unit and value
 */
const unit_name_string units_foot_feet = { " foot", " feet" };
const unit_name_string units_bit_bits = { " bit", " bits" };
const unit_name_string units_byte_bytes = { " byte", " bytes" };
const unit_name_string units_octet_octets = { " octet", " octets" };
const unit_name_string units_word_words = { " word", " words" };
const unit_name_string units_tick_ticks = { " tick", " ticks" };
const unit_name_string units_meters = { "m", NULL };
const unit_name_string units_meter_meters = { " meter", " meters" };
const unit_name_string units_week_weeks = { " week", " weeks" };
const unit_name_string units_day_days = { " day", " days" };
const unit_name_string units_hour_hours = { " hour", " hours" };
const unit_name_string units_hours = { "h", NULL };
const unit_name_string units_minute_minutes = { " minute", " minutes" };
const unit_name_string units_minutes = { "min", NULL };
const unit_name_string units_second_seconds = { " second", " seconds" };
const unit_name_string units_seconds = { "s", NULL };
const unit_name_string units_millisecond_milliseconds = { " millisecond", " milliseconds" };
const unit_name_string units_milliseconds = { "ms", NULL };
const unit_name_string units_microsecond_microseconds = { " microsecond", " microseconds" };
const unit_name_string units_microseconds = { UTF8_MICRO_SIGN "s", NULL };
const unit_name_string units_nanosecond_nanoseconds = { " nanosecond", " nanoseconds" };
const unit_name_string units_nanoseconds = { "ns", NULL };
const unit_name_string units_nanometers = { "nm", NULL };
const unit_name_string units_degree_degrees = { " degree", " degrees" };
const unit_name_string units_degree_celsius = { UTF8_DEGREE_SIGN "C", NULL };
const unit_name_string units_decibels = { "dB", NULL };
const unit_name_string units_dbm = { "dBm", NULL };
const unit_name_string units_dbi = { "dBi", NULL };
const unit_name_string units_mbm = { "mBm", NULL };
const unit_name_string units_percent = { "%", NULL };
const unit_name_string units_khz = { "kHz", NULL };
const unit_name_string units_mhz = { "MHz", NULL };
const unit_name_string units_ghz = { "GHz", NULL };
const unit_name_string units_hz = { "Hz", NULL };
const unit_name_string units_hz_s = { "Hz/s", NULL };
const unit_name_string units_kbit = { "kbit", NULL };
const unit_name_string units_kbps = { "Kbps", NULL };
const unit_name_string units_kibps = { "KiB/s", NULL };
const unit_name_string units_km = { "km", NULL };
const unit_name_string units_kmh = { "km/h", NULL };
const unit_name_string units_bit_sec = { "bits/s", NULL };
const unit_name_string units_milliamps = { "mA", NULL };
const unit_name_string units_microwatts = { UTF8_MICRO_SIGN "W", NULL };
const unit_name_string units_volt = { "V", NULL };
const unit_name_string units_grams_per_second = { "g/s", NULL };
const unit_name_string units_meter_sec = { "m/s", NULL };
const unit_name_string units_meter_sec_squared = { "m/s" UTF8_SUPERSCRIPT_TWO , NULL };
const unit_name_string units_segment_remaining = { " segment remaining", " segments remaining" };
const unit_name_string units_frame_frames = { " frame", " frames" };
const unit_name_string units_revolutions_per_minute = { "rpm", NULL };
const unit_name_string units_kilopascal = { "kPa", NULL };
const unit_name_string units_newton_metre = { "Nm", NULL };
const unit_name_string units_liter_per_hour = { "L/h", NULL };


/*
 * Editor modelines
 *
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
