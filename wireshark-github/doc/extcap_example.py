#!/usr/bin/env python

# Copyright 2014 Roland Knall <rknall [AT] gmail.com>
#
# Wireshark - Network traffic analyzer
# By Gerald Combs <gerald@wireshark.org>
# Copyright 1998 Gerald Combs
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

"""
This is a generic example, which produces pcap packages every n seconds, and
is configurable via extcap options.

@note
{
To use this script on Windows, please generate an extcap_example.bat inside
the extcap folder, with the following content:

-------
@echo off
<Path to python interpreter> <Path to script file> %*
-------

Windows is not able to execute Python scripts directly, which also goes for all
other script-based formates beside VBScript
}

"""

from __future__ import print_function
import os
import sys
import signal
import re
import argparse
import time
import struct
import binascii
from threading import Thread

ERROR_USAGE          = 0
ERROR_ARG            = 1
ERROR_INTERFACE      = 2
ERROR_FIFO           = 3
ERROR_DELAY          = 4

CTRL_CMD_INITIALIZED = 0
CTRL_CMD_SET         = 1
CTRL_CMD_ADD         = 2
CTRL_CMD_REMOVE      = 3
CTRL_CMD_ENABLE      = 4
CTRL_CMD_DISABLE     = 5
CTRL_CMD_STATUSBAR   = 6
CTRL_CMD_INFORMATION = 7
CTRL_CMD_WARNING     = 8
CTRL_CMD_ERROR       = 9

CTRL_ARG_MESSAGE     = 0
CTRL_ARG_DELAY       = 1
CTRL_ARG_VERIFY      = 2
CTRL_ARG_BUTTON      = 3
CTRL_ARG_HELP        = 4
CTRL_ARG_RESTORE     = 5
CTRL_ARG_LOGGER      = 6
CTRL_ARG_NONE        = 255

initialized = False
message = ''
delay = 0.0
verify = False
button = False
button_disabled = False

"""
This code has been taken from http://stackoverflow.com/questions/5943249/python-argparse-and-controlling-overriding-the-exit-status-code - originally developed by Rob Cowie http://stackoverflow.com/users/46690/rob-cowie
"""
class ArgumentParser(argparse.ArgumentParser):
	def _get_action_from_name(self, name):
		"""Given a name, get the Action instance registered with this parser.
		If only it were made available in the ArgumentError object. It is
		passed as it's first arg...
		"""
		container = self._actions
		if name is None:
			return None
		for action in container:
			if '/'.join(action.option_strings) == name:
				return action
			elif action.metavar == name:
				return action
			elif action.dest == name:
				return action

	def error(self, message):
		exc = sys.exc_info()[1]
		if exc:
			exc.argument = self._get_action_from_name(exc.argument_name)
			raise exc
		super(ArgumentParser, self).error(message)

#### EXTCAP FUNCTIONALITY

"""@brief Extcap configuration
This method prints the extcap configuration, which will be picked up by the
interface in Wireshark to present a interface specific configuration for
this extcap plugin
"""
def extcap_config(interface):
	args = []
	values = []

	args.append ( (0, '--delay', 'Time delay', 'Time delay between packages', 'integer', '{range=1,15}{default=5}') )
	args.append ( (1, '--message', 'Message', 'Package message content', 'string', '{required=true}{placeholder=Please enter a message here ...}') )
	args.append ( (2, '--verify', 'Verify', 'Verify package content', 'boolflag', '{default=yes}') )
	args.append ( (3, '--remote', 'Remote Channel', 'Remote Channel Selector', 'selector', ''))
	args.append ( (4, '--fake_ip', 'Fake IP Address', 'Use this ip address as sender', 'string', '{save=false}{validation=\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b}'))
	args.append ( (5, '--ltest', 'Long Test', 'Long Test Value', 'long', '{default=123123123123123123}'))
	args.append ( (6, '--d1test', 'Double 1 Test', 'Long Test Value', 'double', '{default=123.456}'))
	args.append ( (7, '--d2test', 'Double 2 Test', 'Long Test Value', 'double', '{default= 123,456}'))
	args.append ( (8, '--password', 'Password', 'Package message password', 'password', '') )
	args.append ( (9, '--ts', 'Start Time', 'Capture start time', 'timestamp', '') )
	args.append ( (10, '--logfile', 'Log File Test', 'The Log File Test', 'fileselect', '') )
	args.append ( (11, '--radio', 'Radio Test', 'Radio Test Value', 'radio', '') )
	args.append ( (12, '--multi', 'MultiCheck Test', 'MultiCheck Test Value', 'multicheck', '') )

	values.append ( (3, "if1", "Remote1", "true" ) )
	values.append ( (3, "if2", "Remote2", "false" ) )

	values.append ( (11, "r1", "Radio1", "false" ) )
	values.append ( (11, "r2", "Radio2", "true" ) )

	values.append ( (12, "m1", "MultiCheck1", "false" ) )
	values.append ( (12, "m2", "MultiCheck2", "false" ) )

	for arg in args:
		print ("arg {number=%d}{call=%s}{display=%s}{tooltip=%s}{type=%s}%s" % arg)

	for value in values:
		print ("value {arg=%d}{value=%s}{display=%s}{default=%s}" % value)


def extcap_interfaces():
	print ("extcap {version=1.0}{help=http://www.wireshark.org}{display=Example extcap interface}")
	print ("interface {value=example1}{display=Example interface 1 for extcap}")
	print ("interface {value=example2}{display=Example interface 2 for extcap}")
	print ("control {number=%d}{type=string}{display=Message}{tooltip=Package message content. Must start with a capital letter.}{placeholder=Enter package message content here ...}{validation=^[A-Z]+}" % CTRL_ARG_MESSAGE)
	print ("control {number=%d}{type=selector}{display=Time delay}{tooltip=Time delay between packages}" % CTRL_ARG_DELAY)
	print ("control {number=%d}{type=boolean}{display=Verify}{default=true}{tooltip=Verify package content}" % CTRL_ARG_VERIFY)
	print ("control {number=%d}{type=button}{display=Turn on}{tooltip=Turn on or off}" % CTRL_ARG_BUTTON)
	print ("control {number=%d}{type=button}{role=help}{display=Help}{tooltip=Show help}" % CTRL_ARG_HELP)
	print ("control {number=%d}{type=button}{role=restore}{display=Restore}{tooltip=Restore default values}" % CTRL_ARG_RESTORE)
	print ("control {number=%d}{type=button}{role=logger}{display=Log}{tooltip=Show capture log}" % CTRL_ARG_LOGGER)
	print ("value {control=%d}{value=1}{display=1}" % CTRL_ARG_DELAY)
	print ("value {control=%d}{value=2}{display=2}" % CTRL_ARG_DELAY)
	print ("value {control=%d}{value=3}{display=3}" % CTRL_ARG_DELAY)
	print ("value {control=%d}{value=4}{display=4}" % CTRL_ARG_DELAY)
	print ("value {control=%d}{value=5}{display=5}{default=true}" % CTRL_ARG_DELAY)
	print ("value {control=%d}{value=60}{display=60}" % CTRL_ARG_DELAY)


def extcap_dlts(interface):
	if ( interface == '1' ):
		print ("dlt {number=147}{name=USER0}{display=Demo Implementation for Extcap}")
	elif ( interface == '2' ):
		print ("dlt {number=148}{name=USER1}{display=Demo Implementation for Extcap}")

def validate_capture_filter(capture_filter):
	if capture_filter != "filter" and capture_filter != "valid":
		print("Illegal capture filter")

"""

### FAKE DATA GENERATOR

Extcap capture routine
 This routine simulates a capture by any kind of user defined device. The parameters
 are user specified and must be handled by the extcap.

 The data captured inside this routine is fake, so change this routine to present
 your own input data, or call your own capture program via Popen for example. See

 for more details.

"""
def unsigned(n):
	return int(n) & 0xFFFFFFFF

def pcap_fake_header():

	header = bytearray()
	header += struct.pack('<L', int ('a1b2c3d4', 16 ))
	header += struct.pack('<H', unsigned(2) ) # Pcap Major Version
	header += struct.pack('<H', unsigned(4) ) # Pcap Minor Version
	header += struct.pack('<I', int(0)) # Timezone
	header += struct.pack('<I', int(0)) # Accurancy of timestamps
	header += struct.pack('<L', int ('0000ffff', 16 )) # Max Length of capture frame
	header += struct.pack('<L', unsigned(1)) # Ethernet
	return header

# Calculates and returns the IP checksum based on the given IP Header
def ip_checksum(iph):
	#split into bytes
	words = splitN(''.join(iph.split()),4)
	csum = 0;
	for word in words:
		csum += int(word, base=16)
	csum += (csum >> 16)
	csum = csum & 0xFFFF ^ 0xFFFF
	return csum

def pcap_fake_package ( message, fake_ip ):

	pcap = bytearray()
	#length = 14 bytes [ eth ] + 20 bytes [ ip ] + messagelength

	caplength = len(message) + 14 + 20
	timestamp = int(time.time())

	pcap += struct.pack('<L', unsigned(timestamp ) ) # timestamp seconds
	pcap += struct.pack('<L', 0x00  ) # timestamp nanoseconds
	pcap += struct.pack('<L', unsigned(caplength ) ) # length captured
	pcap += struct.pack('<L', unsigned(caplength ) ) # length in frame

# ETH
	pcap += struct.pack('h', 0 ) # source mac
	pcap += struct.pack('h', 0 ) # source mac
	pcap += struct.pack('h', 0 ) # source mac
	pcap += struct.pack('h', 0 ) # dest mac
	pcap += struct.pack('h', 0 ) # dest mac
	pcap += struct.pack('h', 0 ) # dest mac
	pcap += struct.pack('<h', unsigned(8 )) # protocol (ip)

# IP
	pcap += struct.pack('b', int ( '45', 16 )) # IP version
	pcap += struct.pack('b', int ( '0', 16 )) #
	pcap += struct.pack('>H', unsigned(len(message)+20) ) # length of data + payload
	pcap += struct.pack('<H', int ( '0', 16 )) # Identification
	pcap += struct.pack('b', int ( '40', 16 )) # Don't fragment
	pcap += struct.pack('b', int ( '0', 16 )) # Fragment Offset
	pcap += struct.pack('b', int ( '40', 16 ))
	pcap += struct.pack('B', 0xFE ) # Protocol (2 = unspecified)
	pcap += struct.pack('<H', int ( '0000', 16 )) # Checksum

	parts = fake_ip.split('.')
	ipadr = (int(parts[0]) << 24) + (int(parts[1]) << 16) + (int(parts[2]) << 8) + int(parts[3])
	pcap += struct.pack('>L', ipadr ) # Source IP
	pcap += struct.pack('>L', int ( '7F000001', 16 )) # Dest IP

	pcap += message
	return pcap

def control_read(fn):
	try:
		header = fn.read(6)
		sp, _, length, arg, typ = struct.unpack('>sBHBB', header)
		if length > 2:
			payload = fn.read(length - 2)
		else:
			payload = ''
		return arg, typ, payload
	except:
		return None, None, None

def control_read_thread(control_in, fn_out):
	global initialized, message, delay, verify, button, button_disabled
	with open(control_in, 'rb', 0 ) as fn:
	        arg = 0
		while arg != None:
			arg, typ, payload = control_read(fn)
			log = ''
			if typ == CTRL_CMD_INITIALIZED:
				initialized = True
			elif arg == CTRL_ARG_MESSAGE:
				message = payload
				log = "Message = " + payload
			elif arg == CTRL_ARG_DELAY:
				delay = float(payload)
				log = "Time delay = " + payload
			elif arg == CTRL_ARG_VERIFY:
				# Only read this after initialized
				if initialized:
					verify = (payload[0] != '\0')
					log = "Verify = " + str(verify)
					control_write(fn_out, CTRL_ARG_NONE, CTRL_CMD_STATUSBAR, "Verify changed")
			elif arg == CTRL_ARG_BUTTON:
				control_write(fn_out, CTRL_ARG_BUTTON, CTRL_CMD_DISABLE, "")
				button_disabled = True
				if button == True:
					control_write(fn_out, CTRL_ARG_BUTTON, CTRL_CMD_SET, "Turn on")
					button = False
					log = "Button turned off"
				else:
					control_write(fn_out, CTRL_ARG_BUTTON, CTRL_CMD_SET, "Turn off")
					button = True
					log = "Button turned on"

			if len(log) > 0:
				control_write(fn_out, CTRL_ARG_LOGGER, CTRL_CMD_ADD, log + "\n")

def control_write(fn, arg, typ, payload):
	packet = bytearray()
	packet += struct.pack('>sBHBB', 'T', 0, len(payload) + 2, arg, typ)
	packet += payload
	fn.write(packet)

def control_write_defaults(fn_out):
	global initialized, message, delay, verify

	while not initialized:
		time.sleep(.1)  # Wait for initial control values

	# Write startup configuration to Toolbar controls
	control_write(fn_out, CTRL_ARG_MESSAGE, CTRL_CMD_SET, message)
	control_write(fn_out, CTRL_ARG_DELAY, CTRL_CMD_SET, str(int(delay)))
	control_write(fn_out, CTRL_ARG_VERIFY, CTRL_CMD_SET, struct.pack('B', verify))

	for i in range(1,16):
		item = bytearray()
		item += str(i) + struct.pack('B', 0) + str(i) + " sec"
		control_write(fn_out, CTRL_ARG_DELAY, CTRL_CMD_ADD, item)

	control_write(fn_out, CTRL_ARG_DELAY, CTRL_CMD_REMOVE, str(60))

def extcap_capture(interface, fifo, control_in, control_out, in_delay, in_verify, in_message, remote, fake_ip):
	global message, delay, verify, button_disabled
	delay = in_delay if in_delay != 0 else 5
	message = in_message
	verify = in_verify
	counter = 1
	fn_out = None

	with open(fifo, 'wb', 0 ) as fh:
		fh.write (pcap_fake_header())

		if control_out != None:
			fn_out = open(control_out, 'wb', 0)
			control_write(fn_out, CTRL_ARG_LOGGER, CTRL_CMD_SET, "Log started at " + time.strftime("%c") + "\n")

		if control_in != None:
			# Start reading thread
			thread = Thread(target = control_read_thread, args = (control_in, fn_out))
			thread.start()

		if fn_out != None:
			control_write_defaults(fn_out)

		while True:
			if fn_out != None:
				log = "Received packet #" + str(counter) + "\n"
				control_write(fn_out, CTRL_ARG_LOGGER, CTRL_CMD_ADD, log)
				counter = counter + 1

				if button_disabled == True:
					control_write(fn_out, CTRL_ARG_BUTTON, CTRL_CMD_ENABLE, "")
					control_write(fn_out, CTRL_ARG_NONE, CTRL_CMD_INFORMATION, "Turn action finished.")
					button_disabled = False

			out = ("%s|%04X%s|%s" % ( remote.strip(), len(message), message, verify )).encode("utf8")
			fh.write (pcap_fake_package(out, fake_ip))
			time.sleep(delay)

	thread.join()
	if fn_out != None:
	        fn_out.close()

def extcap_close_fifo(fifo):
	# This is apparently needed to workaround an issue on Windows/macOS
	# where the message cannot be read. (really?)
	fh = open(fifo, 'wb', 0 )
	fh.close()

####

def usage():
	print ( "Usage: %s <--extcap-interfaces | --extcap-dlts | --extcap-interface | --extcap-config | --capture | --extcap-capture-filter | --fifo>" % sys.argv[0] )

if __name__ == '__main__':
	interface = ""

	# Capture options
	delay = 0
	message = ""
	fake_ip = ""
	ts = 0

	parser = ArgumentParser(
		prog="Extcap Example",
		description="Extcap example program for python"
		)

	# Extcap Arguments
	parser.add_argument("--capture", help="Start the capture routine", action="store_true" )
	parser.add_argument("--extcap-interfaces", help="Provide a list of interfaces to capture from", action="store_true")
	parser.add_argument("--extcap-interface", help="Provide the interface to capture from")
	parser.add_argument("--extcap-dlts", help="Provide a list of dlts for the given interface", action="store_true")
	parser.add_argument("--extcap-config", help="Provide a list of configurations for the given interface", action="store_true")
	parser.add_argument("--extcap-capture-filter", help="Used together with capture to provide a capture filter")
	parser.add_argument("--fifo", help="Use together with capture to provide the fifo to dump data to")
	parser.add_argument("--extcap-control-in", help="Used to get control messages from toolbar")
	parser.add_argument("--extcap-control-out", help="Used to send control messages to toolbar")

	# Interface Arguments
	parser.add_argument("--verify", help="Demonstrates a verification bool flag", action="store_true" )
	parser.add_argument("--delay", help="Demonstrates an integer variable", type=int, default=0, choices=[0, 1, 2, 3, 4, 5, 6] )
	parser.add_argument("--remote", help="Demonstrates a selector choice", default="if1", choices=["if1", "if2"] )
	parser.add_argument("--message", help="Demonstrates string variable", nargs='?', default="" )
	parser.add_argument("--fake_ip", help="Add a fake sender IP adress", nargs='?', default="127.0.0.1" )
	parser.add_argument("--ts", help="Capture start time", action="store_true" )

	try:
		args, unknown = parser.parse_known_args()
	except argparse.ArgumentError as exc:
		print( "%s: %s" % ( exc.argument.dest, exc.message ), file=sys.stderr)
		fifo_found = 0
		fifo = ""
		for arg in sys.argv:
			if (arg == "--fifo" or arg == "--extcap-fifo") :
				fifo_found = 1
			elif ( fifo_found == 1 ):
				fifo = arg
				break
		extcap_close_fifo(fifo)
		sys.exit(ERROR_ARG)

	if ( len(sys.argv) <= 1 ):
		parser.exit("No arguments given!")

	if ( args.extcap_interfaces == False and args.extcap_interface == None ):
		parser.exit("An interface must be provided or the selection must be displayed")
	if ( args.extcap_capture_filter and not args.capture ):
		validate_capture_filter(args.extcap_capture_filter)
		sys.exit(0)

	if ( args.extcap_interfaces == True or args.extcap_interface == None ):
		extcap_interfaces()
		sys.exit(0)

	if ( len(unknown) > 1 ):
		print("Extcap Example %d unknown arguments given" % len(unknown) )

	m = re.match ( 'example(\d+)', args.extcap_interface )
	if not m:
		sys.exit(ERROR_INTERFACE)
	interface = m.group(1)

	message = args.message
	if ( args.message == None or len(args.message) == 0 ):
		message = "Extcap Test"

	fake_ip = args.fake_ip
	if ( args.fake_ip == None or len(args.fake_ip) < 7 or len(args.fake_ip.split('.')) != 4 ):
		fake_ip = "127.0.0.1"

	ts = args.ts

	if args.extcap_config:
		extcap_config(interface)
	elif args.extcap_dlts:
		extcap_dlts(interface)
	elif args.capture:
		if args.fifo is None:
			sys.exit(ERROR_FIFO)
		# The following code demonstrates error management with extcap
		if args.delay > 5:
			print("Value for delay [%d] too high" % args.delay, file=sys.stderr)
			extcap_close_fifo(args.fifo)
			sys.exit(ERROR_DELAY)

		try:
			extcap_capture(interface, args.fifo, args.extcap_control_in, args.extcap_control_out, args.delay, args.verify, message, args.remote, fake_ip)
		except KeyboardInterrupt:
			pass
	else:
		usage()
		sys.exit(ERROR_USAGE)
