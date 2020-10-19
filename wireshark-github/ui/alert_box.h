/* alert_box.h
 * Routines to put up various "standard" alert boxes used in multiple
 * places
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __ALERT_BOX_H__
#define __ALERT_BOX_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Alert box for general errors.
 */
extern void failure_alert_box(const char *msg_format, ...) G_GNUC_PRINTF(1, 2);
extern void vfailure_alert_box(const char *msg_format, va_list ap);

/*
 * Alert box for general warnings.
 */
extern void vwarning_alert_box(const char *msg_format, va_list ap);

/*
 * Alert box for a failed attempt to open a capture file for reading.
 * "filename" is the name of the file being opened; "err" is assumed
 * to be a UNIX-style errno or a WTAP_ERR_ value; "err_info" is assumed
 * to be a string giving further information for some WTAP_ERR_ values..
 */
extern void cfile_open_failure_alert_box(const char *filename, int err,
                                         gchar *err_info);

/*
 * Alert box for a failed attempt to open a capture file for writing.
 * "filename" is the name of the file being opened; "err" is assumed
 * to be a UNIX-style errno or a WTAP_ERR_ value; "file_type_subtype"
 * is a WTAP_FILE_TYPE_SUBTYPE_ value for the type and subtype of file
 * being opened.
 */
extern void cfile_dump_open_failure_alert_box(const char *filename, int err,
                                              int file_type_subtype);

/*
 * Alert box for a failed attempt to read from a capture file.
 * "err" is assumed to be a UNIX-style errno or a WTAP_ERR_ value;
 * "err_info" is assumed to be a string giving further information for
 * some WTAP_ERR_ values.
 */
extern void cfile_read_failure_alert_box(const char *filename, int err,
                                         gchar *err_info);

/*
 * Alert box for a failed attempt to write to a capture file.
 * "in_filename" is the name of the file from which the record being
 * written came; "out_filename" is the name of the file to which we're
 * writing; "err" is assumed "err" is assumed to be a UNIX-style errno
 * or a WTAP_ERR_ value; "err_info" is assumed to be a string giving
 * further information for some WTAP_ERR_ values; "framenum" is the frame
 * number of the record on which the error occurred; "file_type_subtype"
 * is a WTAP_FILE_TYPE_SUBTYPE_ value for the type and subtype of file
 * being written.
 */
extern void cfile_write_failure_alert_box(const char *in_filename,
                                          const char *out_filename,
                                          int err, gchar *err_info,
                                          guint32 framenum,
                                          int file_type_subtype);

/*
 * Alert box for a failed attempt to close a capture file.
 * "err" is assumed to be a UNIX-style errno or a WTAP_ERR_ value.
 *
 * When closing a capture file:
 *
 *    some information in the file that can't be determined until
 *    all packets have been written might be written to the file
 *    (such as a table of the file offsets of all packets);
 *
 *    data buffered in the low-level file writing code might be
 *    flushed to the file;
 *
 *    for remote file systems, data written to the file but not
 *    yet sent to the server might be sent to the server or, if
 *    that data was sent asynchronously, "out of space", "disk
 *    quota exceeded", or "I/O error" indications might have
 *    been received but not yet delivered, and the close operation
 *    could deliver them;
 *
 * so we have to check for write errors here.
 */
extern void cfile_close_failure_alert_box(const char *filename, int err);

/*
 * Alert box for a failed attempt to open or create a file.
 * "err" is assumed to be a UNIX-style errno; "for_writing" is TRUE if
 * the file is being opened for writing and FALSE if it's being opened
 * for reading.
 */
extern void open_failure_alert_box(const char *filename, int err,
                                   gboolean for_writing);

/*
 * Alert box for a failed attempt to read a file.
 * "err" is assumed to be a UNIX-style errno.
 */
extern void read_failure_alert_box(const char *filename, int err);

/*
 * Alert box for a failed attempt to write to a file.
 * "err" is assumed to be a UNIX-style errno.
 */
extern void write_failure_alert_box(const char *filename, int err);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ALERT_BOX_H__ */

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
