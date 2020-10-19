/* packet-ieee80211-radio.h
 * Routines for pseudo 802.11 header dissection and radio packet timing calculation
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * Copyright 2012 Parc Inc and Samsung Electronics
 * Copyright 2015, 2016 & 2017 Cisco Inc
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __WLAN_RADIO_H__
#define __WLAN_RADIO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct aggregate {
  guint phy;
  union ieee_802_11_phy_info phy_info;
  gint8 rssi; /* sometimes only available on the last frame */
  guint duration; /* total duration of data in microseconds (without preamble) */
};

struct wlan_radio {
  struct aggregate *aggregate; /* if this frame is part of an aggregate, point to it, otherwise NULL */
  guint prior_aggregate_data; /* length of all prior data in this aggregate
                                 used for calculating duration of this subframe */
  guint64 start_tsf;
  guint64 end_tsf;

  gint64 ifs; /* inter frame space in microseconds */

  guint16 nav;
  gint8 rssi;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __WLAN_RADIO_H__ */
