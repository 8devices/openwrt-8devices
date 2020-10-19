#!/bin/bash
#
# Configuration of the command line tests
#
# Wireshark - Network traffic analyzer
# By Gerald Combs <gerald@wireshark.org>
# Copyright 2005 Ulf Lamping
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

# The files we want to test are in the build directory.
WIRESHARK_RUN_FROM_BUILD_DIRECTORY=1
export WIRESHARK_RUN_FROM_BUILD_DIRECTORY

# Set WS_SYSTEM to our system type, e.g. Windows, Linux, Darwin
# http://support.microsoft.com/kb/100843
if [ -n "${OS#Windows}" ] ; then
	WS_SYSTEM="Windows"
	export CYGWIN="$CYGWIN error_start=c:\cygwin\bin\dumper.exe -d %1 %2"
else
	WS_SYSTEM=`uname -s`
fi

#
#
ENDIANNESS="little"
echo -n I | od -to2 | awk '{ lastbit = substr($2,6,1); exit lastbit }'
if [ $? -eq 0 ] ; then
	ENDIANNESS="big"
fi

# Absolute path to the source tree
SOURCE_DIR="$(cd "$(dirname "$0")" && cd .. && pwd)"

# Absolute path to this test directory (for capture and config files)
TESTS_DIR="$SOURCE_DIR/test"

# Are we allowed to open interfaces or capture on this system?
SKIP_CAPTURE=${SKIP_CAPTURE:-1}

# Override the last two items if we're running Windows
if [ "$WS_SYSTEM" = "Windows" ] ; then
	WS_BIN_PATH=${WS_BIN_PATH:-$SOURCE_DIR/wireshark-gtk2}
	WS_QT_BIN_PATH=${WS_QT_BIN_PATH:-$SOURCE_DIR/wireshark-qt-release}
	SKIP_CAPTURE=0
fi

# Path to the Wireshark binaries, default to source dir if unset
WS_BIN_PATH=${WS_BIN_PATH:-$SOURCE_DIR}
WS_QT_BIN_PATH=${WS_QT_BIN_PATH:-$WS_BIN_PATH}

# Tweak the following to your liking.
WIRESHARK=$WS_QT_BIN_PATH/wireshark
WIRESHARK_GTK=$WS_BIN_PATH/wireshark-gtk
TSHARK=$WS_BIN_PATH/tshark
RAWSHARK=$WS_BIN_PATH/rawshark
CAPINFOS=$WS_BIN_PATH/capinfos
MERGECAP=$WS_BIN_PATH/mergecap
TEXT2PCAP=$WS_BIN_PATH/text2pcap
DUMPCAP=$WS_BIN_PATH/dumpcap

# interface with at least a few packets/sec traffic on it
# (e.g. start a web radio to generate some traffic :-)
# an interfaces index (1 based) should do well for recent devbuilds
if [ "$WS_SYSTEM" = "Windows" -a -z "$TRAFFIC_CAPTURE_IFACE" ] ; then
	# Try to fetch the first Ethernet interface.
	TRAFFIC_CAPTURE_IFACE=`$TSHARK -D 2>&1 | \
		egrep 'Ethernet|Network Connection|VMware|Intel|Realtek' | \
		head -1 | cut -c 1`
fi
TRAFFIC_CAPTURE_IFACE=${TRAFFIC_CAPTURE_IFACE:-1}

# time to capture some traffic (in seconds)
# (you may increase this if you get errors caused by very low traffic)
TRAFFIC_CAPTURE_DURATION=10

# the default is to not capture in promiscuous mode
# (this makes known trouble with some Windows WLAN adapters)
# if you need promiscuous mode, comment this line out
TRAFFIC_CAPTURE_PROMISC=-p

# only test capturing from a fifo if we're not on Windows
#  and we have a mkfifo. (Windows cygwin has a mkfifo but
#   Windows dumpcap & etc use Windows named pipes which
#   are different than the cygwin named pipes).
#
if [ "$WS_SYSTEM" != "Windows" ] && which mkfifo &>/dev/null ; then
	TEST_FIFO=1
fi

# Tell Wireshark to quit after capuring packets.
export WIRESHARK_QUIT_AFTER_CAPTURE="True"

CAPTURE_DIR="$TESTS_DIR/captures/"

TSHARK_VERSION=$($TSHARK -v | tr '\n' ' ')

# Figure out if we were built with lua or not so we can skip the lua tests if we
# don't have it.
echo "$TSHARK_VERSION" | grep -q "with Lua"
HAVE_LUA=$?

# Check whether we need to skip the HTTP2/HPACK decryption test.
echo "$TSHARK_VERSION" | grep -q "with nghttp2"
HAVE_NGHTTP2=$?

# Check whether we need to skip a certain decryption test.
# XXX What do we print for Nettle?
echo "$TSHARK_VERSION" | egrep -q "with MIT Kerberos|with Heimdal Kerberos"
HAVE_KERBEROS=$?

# first version is "compiled with", second is "running on" version.
GCRY_VERSION=$(echo "$TSHARK_VERSION" | grep -oE 'Gcrypt [1-9]+(\.[1-9]+)?' | sed -n '1s/Gcrypt //p')
if [ -n "$GCRY_VERSION" ] && ! echo "$GCRY_VERSION" | grep -q '1\.[456]'; then
	# Current minimum Gcrypt version is 1.4.2,
	# assume 1.7 or newer if not 1,4, 1.5 or 1.6.
	HAVE_LIBGCRYPT17=true
else
	HAVE_LIBGCRYPT17=false
fi

HAVE_ICONV="False"
hash iconv 2>/dev/null && HAVE_ICONV="True"

# Display our environment

##printf "\n ------- Info =-----------------\n"
##printf "Syms :$WS_SYSTEM: :$TRAFFIC_CAPTURE_IFACE: :$SKIP_CAPTURE: :$TEST_FIFO:\n"
##
##ls -l $WIRESHARK $TSHARK $DUMPCAP
##ls -l $(which wireshark) $(which tshark) $(which dumpcap)
##printf " ----------------------------------\n\n"

# Editor modelines
#
# Local Variables:
# sh-basic-offset: 8
# tab-width: 8
# indent-tabs-mode: t
# End:
#
# ex: set shiftwidth=8 tabstop=8 noexpandtab:
# :indentSize=8:tabSize=8:noTabs=false:
