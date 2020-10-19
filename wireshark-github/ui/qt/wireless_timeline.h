/* wireless_timeline.h
 * GUI to show an 802.11 wireless timeline of packets
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

#include <QScrollArea>

#ifndef WIRELESSTIMELINE_H
#define WIRELESSTIMELINE_H

#include <stdio.h>

#include <config.h>

#include <glib.h>

#include "file.h"

#include "ui/ui_util.h"

#include <epan/prefs.h>
#include <epan/plugin_if.h>
#include <epan/timestamp.h>

#include <epan/dissectors/packet-ieee80211-radio.h>

#include <QScrollArea>

#include "cfile.h"

/* pixels height for rendered timeline */
#define TIMELINE_HEIGHT 64

/* Maximum zoom levels for the timeline */
#define TIMELINE_MAX_ZOOM 25.0

class WirelessTimeline;
class PacketList;

class WirelessTimeline : public QWidget
{
    Q_OBJECT

public:
    explicit WirelessTimeline(QWidget *parent);
    void setPacketList(PacketList *packet_list);
    void captureFileReadStarted(capture_file *cf);
    void captureFileReadFinished();

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent (QMouseEvent *event);
    void mouseMoveEvent (QMouseEvent *event);
    void mouseReleaseEvent (QMouseEvent *event);
    bool event(QEvent *event);
    void wheelEvent(QWheelEvent *event);

public slots:
    void bgColorizationProgress(int first, int last);
    void packetSelectionChanged();
    void appInitialized();

protected:
    static void tap_timeline_reset(void* tapdata);
    static gboolean tap_timeline_packet(void *tapdata, packet_info* pinfo, epan_dissect_t* edt, const void *data);

    struct wlan_radio* get_wlan_radio(guint32 packet_num);

    void clip_tsf();
    int position(guint64 tsf, float ratio);
    int find_packet_tsf(guint64 tsf);
    void doToolTip(struct wlan_radio *wr, QPoint pos, int x);
    void zoom(double x_fraction);
    double zoom_level;
    qreal start_x, last_x;
    PacketList *packet_list;
    guint find_packet(qreal x);
    float rgb[TIMELINE_HEIGHT][3];

    guint64 start_tsf;
    guint64 end_tsf;
    int first_packet; /* first packet displayed */
    struct wlan_radio *first, *last;
    capture_file *capfile;

    GHashTable* radio_packet_list;
};

#endif // WIRELESS_TIMELINE_H

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
