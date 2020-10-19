/* Combine dump files, either by appending or by merging by timestamp
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Mergecap written by Scott Renfro <scott@renfro.org> based on
 * editcap by Richard Sharpe and Guy Harris
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <string.h>

#include <wiretap/wtap.h>

#ifndef HAVE_GETOPT_LONG
#include <wsutil/wsgetopt.h>
#endif

#include <wsutil/clopts_common.h>
#include <wsutil/cmdarg_err.h>
#include <wsutil/crash_info.h>
#include <wsutil/filesystem.h>
#include <wsutil/file_util.h>
#include <wsutil/privileges.h>
#include <wsutil/strnatcmp.h>
#include <ws_version_info.h>

#ifdef HAVE_PLUGINS
#include <wsutil/plugins.h>
#endif

#include <wsutil/report_message.h>

#include <wiretap/merge.h>

#ifdef _WIN32
#include <wsutil/unicode-utils.h>
#endif /* _WIN32 */

#include "ui/failure_message.h"

/*
 * Show the usage
 */
static void
print_usage(FILE *output)
{
  fprintf(output, "\n");
  fprintf(output, "Usage: mergecap [options] -w <outfile>|- <infile> [<infile> ...]\n");
  fprintf(output, "\n");
  fprintf(output, "Output:\n");
  fprintf(output, "  -a                concatenate rather than merge files.\n");
  fprintf(output, "                    default is to merge based on frame timestamps.\n");
  fprintf(output, "  -s <snaplen>      truncate packets to <snaplen> bytes of data.\n");
  fprintf(output, "  -w <outfile>|-    set the output filename to <outfile> or '-' for stdout.\n");
  fprintf(output, "  -F <capture type> set the output file type; default is pcapng.\n");
  fprintf(output, "                    an empty \"-F\" option will list the file types.\n");
  fprintf(output, "  -I <IDB merge mode> set the merge mode for Interface Description Blocks; default is 'all'.\n");
  fprintf(output, "                    an empty \"-I\" option will list the merge modes.\n");
  fprintf(output, "\n");
  fprintf(output, "Miscellaneous:\n");
  fprintf(output, "  -h                display this help and exit.\n");
  fprintf(output, "  -v                verbose output.\n");
}

/*
 * Report an error in command-line arguments.
 */
static void
mergecap_cmdarg_err(const char *fmt, va_list ap)
{
  fprintf(stderr, "mergecap: ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

/*
 * Report additional information for an error in command-line arguments.
 */
static void
mergecap_cmdarg_err_cont(const char *fmt, va_list ap)
{
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

struct string_elem {
  const char *sstr;     /* The short string */
  const char *lstr;     /* The long string */
};

static gint
string_compare(gconstpointer a, gconstpointer b)
{
  return strcmp(((const struct string_elem *)a)->sstr,
                ((const struct string_elem *)b)->sstr);
}

static void
string_elem_print(gpointer data, gpointer not_used _U_)
{
  fprintf(stderr, "    %s - %s\n", ((struct string_elem *)data)->sstr,
          ((struct string_elem *)data)->lstr);
}

#ifdef HAVE_PLUGINS
/*
 * General errors and warnings are reported with an console message
 * in mergecap.
 */
static void
failure_warning_message(const char *msg_format, va_list ap)
{
  fprintf(stderr, "mergecap: ");
  vfprintf(stderr, msg_format, ap);
  fprintf(stderr, "\n");
}
#endif

static void
list_capture_types(void) {
  int i;
  struct string_elem *captypes;
  GSList *list = NULL;

  captypes = g_new(struct string_elem,WTAP_NUM_FILE_TYPES_SUBTYPES);

  fprintf(stderr, "mergecap: The available capture file types for the \"-F\" flag are:\n");
  for (i = 0; i < WTAP_NUM_FILE_TYPES_SUBTYPES; i++) {
    if (wtap_dump_can_open(i)) {
      captypes[i].sstr = wtap_file_type_subtype_short_string(i);
      captypes[i].lstr = wtap_file_type_subtype_string(i);
      list = g_slist_insert_sorted(list, &captypes[i], string_compare);
    }
  }
  g_slist_foreach(list, string_elem_print, NULL);
  g_slist_free(list);
  g_free(captypes);
}

static void
list_idb_merge_modes(void) {
  int i;

  fprintf(stderr, "mergecap: The available IDB merge modes for the \"-I\" flag are:\n");
  for (i = 0; i < IDB_MERGE_MODE_MAX; i++) {
    fprintf(stderr, "    %s\n", merge_idb_merge_mode_to_string(i));
  }
}

static gboolean
merge_callback(merge_event event, int num,
               const merge_in_file_t in_files[], const guint in_file_count,
               void *data _U_)
{
  guint i;

  switch (event) {

    case MERGE_EVENT_INPUT_FILES_OPENED:
      for (i = 0; i < in_file_count; i++) {
        fprintf(stderr, "mergecap: %s is type %s.\n", in_files[i].filename,
                wtap_file_type_subtype_string(wtap_file_type_subtype(in_files[i].wth)));
      }
      break;

    case MERGE_EVENT_FRAME_TYPE_SELECTED:
      /* for this event, num = frame_type */
      if (num == WTAP_ENCAP_PER_PACKET) {
        /*
         * Find out why we had to choose WTAP_ENCAP_PER_PACKET.
         */
        int first_frame_type, this_frame_type;

        first_frame_type = wtap_file_encap(in_files[0].wth);
        for (i = 1; i < in_file_count; i++) {
          this_frame_type = wtap_file_encap(in_files[i].wth);
          if (first_frame_type != this_frame_type) {
            fprintf(stderr, "mergecap: multiple frame encapsulation types detected\n");
            fprintf(stderr, "          defaulting to WTAP_ENCAP_PER_PACKET\n");
            fprintf(stderr, "          %s had type %s (%s)\n",
                    in_files[0].filename,
                    wtap_encap_string(first_frame_type),
                    wtap_encap_short_string(first_frame_type));
            fprintf(stderr, "          %s had type %s (%s)\n",
                    in_files[i].filename,
                    wtap_encap_string(this_frame_type),
                    wtap_encap_short_string(this_frame_type));
            break;
          }
        }
      }
      fprintf(stderr, "mergecap: selected frame_type %s (%s)\n",
              wtap_encap_string(num),
              wtap_encap_short_string(num));
      break;

    case MERGE_EVENT_READY_TO_MERGE:
      fprintf(stderr, "mergecap: ready to merge records\n");
      break;

    case MERGE_EVENT_PACKET_WAS_READ:
      /* for this event, num = count */
      fprintf(stderr, "Record: %d\n", num);
      break;

    case MERGE_EVENT_DONE:
      fprintf(stderr, "mergecap: merging complete\n");
      break;
  }

  /* false = do not stop merging */
  return FALSE;
}


int
main(int argc, char *argv[])
{
  GString            *comp_info_str;
  GString            *runtime_info_str;
  char               *init_progfile_dir_error;
  int                 opt;
  static const struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"version", no_argument, NULL, 'V'},
      {0, 0, 0, 0 }
  };
  gboolean            do_append          = FALSE;
  gboolean            verbose            = FALSE;
  int                 in_file_count      = 0;
  guint32             snaplen            = 0;
#ifdef PCAP_NG_DEFAULT
  int                 file_type          = WTAP_FILE_TYPE_SUBTYPE_PCAPNG; /* default to pcap format */
#else
  int                 file_type          = WTAP_FILE_TYPE_SUBTYPE_PCAP; /* default to pcapng format */
#endif
  int                 err                = 0;
  gchar              *err_info           = NULL;
  int                 err_fileno;
  guint32             err_framenum;
  char               *out_filename       = NULL;
  merge_result        status             = MERGE_OK;
  idb_merge_mode      mode               = IDB_MERGE_MODE_MAX;
  merge_progress_callback_t cb;

  cmdarg_err_init(mergecap_cmdarg_err, mergecap_cmdarg_err_cont);

#ifdef _WIN32
  arg_list_utf_16to8(argc, argv);
  create_app_running_mutex();
#endif /* _WIN32 */

  /* Get the compile-time version information string */
  comp_info_str = get_compiled_version_info(NULL, NULL);

  /* Get the run-time version information string */
  runtime_info_str = get_runtime_version_info(NULL);

  /* Add it to the information to be reported on a crash. */
  ws_add_crash_info("Mergecap (Wireshark) %s\n"
       "\n"
       "%s"
       "\n"
       "%s",
    get_ws_vcs_version_info(), comp_info_str->str, runtime_info_str->str);
  g_string_free(comp_info_str, TRUE);
  g_string_free(runtime_info_str, TRUE);

  /*
   * Get credential information for later use.
   */
  init_process_policies();

  /*
   * Attempt to get the pathname of the directory containing the
   * executable file.
   */
  init_progfile_dir_error = init_progfile_dir(argv[0], main);
  if (init_progfile_dir_error != NULL) {
    fprintf(stderr,
            "mergecap: Can't get pathname of directory containing the mergecap program: %s.\n",
            init_progfile_dir_error);
    g_free(init_progfile_dir_error);
  }

  wtap_init();

#ifdef HAVE_PLUGINS
  init_report_message(failure_warning_message, failure_warning_message,
                      NULL, NULL, NULL);

  /* Scan for plugins.  This does *not* call their registration routines;
     that's done later.

     Don't report failures to load plugins because most (non-wiretap)
     plugins *should* fail to load (because we're not linked against
     libwireshark and dissector plugins need libwireshark).*/
  scan_plugins(DONT_REPORT_LOAD_FAILURE);

  /* Register all libwiretap plugin modules. */
  register_all_wiretap_modules();
#endif

  /* Process the options first */
  while ((opt = getopt_long(argc, argv, "aF:hI:s:vVw:", long_options, NULL)) != -1) {

    switch (opt) {
    case 'a':
      do_append = !do_append;
      break;

    case 'F':
      file_type = wtap_short_string_to_file_type_subtype(optarg);
      if (file_type < 0) {
        fprintf(stderr, "mergecap: \"%s\" isn't a valid capture file type\n",
                optarg);
        list_capture_types();
        status = MERGE_ERR_INVALID_OPTION;
        goto clean_exit;
      }
      break;

    case 'h':
      printf("Mergecap (Wireshark) %s\n"
             "Merge two or more capture files into one.\n"
             "See https://www.wireshark.org for more information.\n",
             get_ws_vcs_version_info());
      print_usage(stdout);
      goto clean_exit;
      break;

    case 'I':
      mode = merge_string_to_idb_merge_mode(optarg);
      if (mode == IDB_MERGE_MODE_MAX) {
        fprintf(stderr, "mergecap: \"%s\" isn't a valid IDB merge mode\n",
                optarg);
        list_idb_merge_modes();
        status = MERGE_ERR_INVALID_OPTION;
        goto clean_exit;
      }
      break;

    case 's':
      snaplen = get_nonzero_guint32(optarg, "snapshot length");
      break;

    case 'v':
      verbose = TRUE;
      break;

    case 'V':
      comp_info_str = get_compiled_version_info(NULL, NULL);
      runtime_info_str = get_runtime_version_info(NULL);
      show_version("Mergecap (Wireshark)", comp_info_str, runtime_info_str);
      g_string_free(comp_info_str, TRUE);
      g_string_free(runtime_info_str, TRUE);
      goto clean_exit;
      break;

    case 'w':
      out_filename = optarg;
      break;

    case '?':              /* Bad options if GNU getopt */
      switch(optopt) {
      case'F':
        list_capture_types();
        break;
      case'I':
        list_idb_merge_modes();
        break;
      default:
        print_usage(stderr);
      }
      status = MERGE_ERR_INVALID_OPTION;
      goto clean_exit;
      break;
    }
  }

  cb.callback_func = merge_callback;
  cb.data = NULL;

  /* check for proper args; at a minimum, must have an output
   * filename and one input file
   */
  in_file_count = argc - optind;
  if (!out_filename) {
    fprintf(stderr, "mergecap: an output filename must be set with -w\n");
    fprintf(stderr, "          run with -h for help\n");
    status = MERGE_ERR_INVALID_OPTION;
    goto clean_exit;
  }
  if (in_file_count < 1) {
    fprintf(stderr, "mergecap: No input files were specified\n");
    return 1;
  }

  /* setting IDB merge mode must use PCAPNG output */
  if (mode != IDB_MERGE_MODE_MAX && file_type != WTAP_FILE_TYPE_SUBTYPE_PCAPNG) {
    fprintf(stderr, "The IDB merge mode can only be used with PCAPNG output format\n");
    status = MERGE_ERR_INVALID_OPTION;
    goto clean_exit;
  }

  /* if they didn't set IDB merge mode, set it to our default */
  if (mode == IDB_MERGE_MODE_MAX) {
    mode = IDB_MERGE_MODE_ALL_SAME;
  }

  /* open the outfile */
  if (strcmp(out_filename, "-") == 0) {
    /* merge the files to the standard output */
    status = merge_files_to_stdout(file_type,
                                   (const char *const *) &argv[optind],
                                   in_file_count, do_append, mode, snaplen,
                                   "mergecap", verbose ? &cb : NULL,
                                   &err, &err_info, &err_fileno, &err_framenum);
  } else {
    /* merge the files to the outfile */
    status = merge_files(out_filename, file_type,
                         (const char *const *) &argv[optind], in_file_count,
                         do_append, mode, snaplen, "mergecap", verbose ? &cb : NULL,
                         &err, &err_info, &err_fileno, &err_framenum);
  }

  switch (status) {
    case MERGE_OK:
      break;

    case MERGE_USER_ABORTED:
      /* we don't catch SIGINT/SIGTERM (yet?), so we couldn't have aborted */
      g_assert(FALSE);
      break;

    case MERGE_ERR_CANT_OPEN_INFILE:
      cfile_open_failure_message("mergecap", argv[optind + err_fileno],
                                 err, err_info);
      break;

    case MERGE_ERR_CANT_OPEN_OUTFILE:
      cfile_dump_open_failure_message("mergecap", out_filename, err, file_type);
      break;

    case MERGE_ERR_CANT_READ_INFILE:
      cfile_read_failure_message("mergecap", argv[optind + err_fileno],
                                 err, err_info);
      break;

    case MERGE_ERR_BAD_PHDR_INTERFACE_ID:
      cmdarg_err("Record %u of \"%s\" has an interface ID that does not match any IDB in its file.",
                 err_framenum, argv[optind + err_fileno]);
      break;

    case MERGE_ERR_CANT_WRITE_OUTFILE:
       cfile_write_failure_message("mergecap", argv[optind + err_fileno],
                                   out_filename, err, err_info, err_framenum,
                                   file_type);
       break;

    case MERGE_ERR_CANT_CLOSE_OUTFILE:
        cfile_close_failure_message(out_filename, err);
        break;

    default:
      cmdarg_err("Unknown merge_files error %d", status);
      break;
  }

clean_exit:
  wtap_cleanup();
  free_progdirs();
#ifdef HAVE_PLUGINS
  plugins_cleanup();
#endif
  return (status == MERGE_OK) ? 0 : 2;
}

/*
 * Editor modelines  -  http://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 2
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=2 tabstop=8 expandtab:
 * :indentSize=2:tabSize=8:noTabs=true:
 */

