/* decode_as.c
 * Routines for dissector Decode As handlers
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

#include "config.h"

#include <glib.h>

#include "decode_as.h"
#include "packet.h"
#include "prefs.h"
#include "prefs-int.h"
#include "wsutil/file_util.h"
#include "wsutil/filesystem.h"
#include "epan/dissectors/packet-dcerpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

GList *decode_as_list = NULL;

void register_decode_as(decode_as_t* reg)
{
    dissector_table_t decode_table;

    /* Ensure valid functions */
    DISSECTOR_ASSERT(reg->populate_list);
    DISSECTOR_ASSERT(reg->reset_value);
    DISSECTOR_ASSERT(reg->change_value);

    decode_table = find_dissector_table(reg->table_name);
    if (decode_table != NULL)
    {
        dissector_table_allow_decode_as(decode_table);
    }

    decode_as_list = g_list_append(decode_as_list, reg);
}


struct decode_as_default_populate
{
    decode_as_add_to_list_func add_to_list;
    gpointer ui_element;
};

static void
decode_proto_add_to_list (const gchar *table_name, gpointer value, gpointer user_data)
{
    struct decode_as_default_populate* populate = (struct decode_as_default_populate*)user_data;
    const gchar     *proto_name;
    gint       i;
    dissector_handle_t handle;


    handle = (dissector_handle_t)value;
    proto_name = dissector_handle_get_short_name(handle);

    i = dissector_handle_get_protocol_index(handle);
    if (i >= 0 && !proto_is_protocol_enabled(find_protocol_by_id(i)))
        return;

    populate->add_to_list(table_name, proto_name, value, populate->ui_element);
}

void decode_as_default_populate_list(const gchar *table_name, decode_as_add_to_list_func add_to_list, gpointer ui_element)
{
    struct decode_as_default_populate populate;

    populate.add_to_list = add_to_list;
    populate.ui_element = ui_element;

    dissector_table_foreach_handle(table_name, decode_proto_add_to_list, &populate);
}

gboolean decode_as_default_reset(const gchar *name, gconstpointer pattern)
{
    switch (get_dissector_table_selector_type(name)) {
    case FT_UINT8:
    case FT_UINT16:
    case FT_UINT24:
    case FT_UINT32:
        dissector_reset_uint(name, GPOINTER_TO_UINT(pattern));
        return TRUE;
    case FT_STRING:
    case FT_STRINGZ:
    case FT_UINT_STRING:
    case FT_STRINGZPAD:
        dissector_reset_string(name, (!pattern)?"":(gchar *) pattern);
        return TRUE;
    default:
        return FALSE;
    };

    return TRUE;
}

gboolean decode_as_default_change(const gchar *name, gconstpointer pattern, gpointer handle, gchar *list_name _U_)
{
    dissector_handle_t* dissector = (dissector_handle_t*)handle;
    if (dissector != NULL) {
        switch (get_dissector_table_selector_type(name)) {
        case FT_UINT8:
        case FT_UINT16:
        case FT_UINT24:
        case FT_UINT32:
            dissector_change_uint(name, GPOINTER_TO_UINT(pattern), *dissector);
            return TRUE;
        case FT_STRING:
        case FT_STRINGZ:
        case FT_UINT_STRING:
        case FT_STRINGZPAD:
            dissector_change_string(name, (!pattern)?"":(gchar *) pattern, *dissector);
            return TRUE;
        default:
            return FALSE;
        };

        return FALSE;
    }

    return TRUE;
}

/* Some useful utilities for Decode As */

/*
 * A list of dissectors that need to be reset.
 */
static GSList *dissector_reset_list = NULL;

/*
 * A callback function to parse each "decode as" entry in the file and apply the change
 */
static prefs_set_pref_e
read_set_decode_as_entries(gchar *key, const gchar *value,
                           void *user_data,
                           gboolean return_range_errors _U_)
{
    gchar *values[4] = {NULL, NULL, NULL, NULL};
    gchar delimiter[4] = {',', ',', ',','\0'};
    gchar *pch;
    guint i, j;
    GHashTable* processed_entries = (GHashTable*)user_data;
    dissector_table_t sub_dissectors;
    prefs_set_pref_e retval = PREFS_SET_OK;
    gboolean is_valid = FALSE;

    if (strcmp(key, DECODE_AS_ENTRY) == 0) {
        /* Parse csv into table, selector, initial, current */
        for (i = 0; i < 4; i++) {
            pch = strchr(value, delimiter[i]);
            if (pch == NULL) {
                for (j = 0; j < i; j++) {
                    g_free(values[j]);
                }
                return PREFS_SET_SYNTAX_ERR;
            }
            values[i] = g_strndup(value, pch - value);
            value = pch + 1;
        }
        sub_dissectors = find_dissector_table(values[0]);
        if (sub_dissectors != NULL) {
            dissector_handle_t handle;
            ftenum_t selector_type;
            pref_t* pref_value;
            module_t *module;
            const char* proto_name;

            selector_type = dissector_table_get_type(sub_dissectors);

            handle = dissector_table_get_dissector_handle(sub_dissectors, values[3]);
            if (handle != NULL || g_ascii_strcasecmp(values[3], DECODE_AS_NONE) == 0) {
                is_valid = TRUE;
            }

            if (is_valid) {
                if (IS_FT_STRING(selector_type)) {
                    dissector_change_string(values[0], values[1], handle);
                } else {
                    char *p;
                    long long_value;

                    long_value = strtol(values[1], &p, 0);
                    if (p == values[0] || *p != '\0' || long_value < 0 ||
                          (unsigned long)long_value > UINT_MAX) {
                        retval = PREFS_SET_SYNTAX_ERR;
                        is_valid = FALSE;
                    } else {
                        dissector_change_uint(values[0], (guint)long_value, handle);
                    }

                    /* Now apply the value data back to dissector table preference */
                    proto_name = proto_get_protocol_filter_name(dissector_handle_get_protocol_index(handle));
                    module = prefs_find_module(proto_name);
                    pref_value = prefs_find_preference(module, values[0]);
                    if (pref_value != NULL) {
                        gboolean replace = FALSE;
                        if (g_hash_table_lookup(processed_entries, proto_name) == NULL) {
                            /* First decode as entry for this protocol, ranges may be replaced */
                            replace = TRUE;

                            /* Remember we've processed this protocol */
                            g_hash_table_insert(processed_entries, (gpointer)proto_name, (gpointer)proto_name);
                        }

                        prefs_add_decode_as_value(pref_value, (guint)long_value, replace);
                        module->prefs_changed = TRUE;
                    }

                }
            }
            if (is_valid) {
                decode_build_reset_list(values[0], selector_type, values[1], NULL, NULL);
            }
        } else {
            retval = PREFS_SET_SYNTAX_ERR;
        }

    } else {
        retval = PREFS_SET_NO_SUCH_PREF;
    }

    for (i = 0; i < 4; i++) {
        g_free(values[i]);
    }
    return retval;
}

void
load_decode_as_entries(void)
{
    char   *daf_path;
    FILE   *daf;

    if (dissector_reset_list) {
        decode_clear_all();
    }

    daf_path = get_persconffile_path(DECODE_AS_ENTRIES_FILE_NAME, TRUE);
    if ((daf = ws_fopen(daf_path, "r")) != NULL) {
        /* Store saved entries for better range processing */
        GHashTable* processed_entries = g_hash_table_new(g_str_hash, g_str_equal);
        read_prefs_file(daf_path, daf, read_set_decode_as_entries, processed_entries);
        g_hash_table_destroy(processed_entries);
        fclose(daf);
    }
    g_free(daf_path);
}

static void
decode_as_write_entry (const gchar *table_name, ftenum_t selector_type,
                       gpointer key, gpointer value, gpointer user_data)
{
    FILE *da_file = (FILE *)user_data;
    dissector_handle_t current, initial;
    const gchar *current_proto_name, *initial_proto_name;

    current = dtbl_entry_get_handle((dtbl_entry_t *)value);
    if (current == NULL)
        current_proto_name = DECODE_AS_NONE;
    else
        current_proto_name = dissector_handle_get_short_name(current);
    initial = dtbl_entry_get_initial_handle((dtbl_entry_t *)value);
    if (initial == NULL)
        initial_proto_name = DECODE_AS_NONE;
    else
        initial_proto_name = dissector_handle_get_short_name(initial);

    switch (selector_type) {

    case FT_UINT8:
    case FT_UINT16:
    case FT_UINT24:
    case FT_UINT32:
        /*
         * XXX - write these in decimal, regardless of the base of
         * the dissector table's selector, as older versions of
         * Wireshark used atoi() when reading this file, and
         * failed to handle hex or octal numbers.
         *
         * That will be fixed in future 1.10 and 1.12 releases,
         * but pre-1.10 releases are at end-of-life and won't
         * be fixed.
         */
        fprintf (da_file,
                 DECODE_AS_ENTRY ": %s,%u,%s,%s\n",
                 table_name, GPOINTER_TO_UINT(key), initial_proto_name,
                 current_proto_name);
        break;

    case FT_STRING:
    case FT_STRINGZ:
    case FT_UINT_STRING:
    case FT_STRINGZPAD:
        fprintf (da_file,
                 DECODE_AS_ENTRY ": %s,%s,%s,%s\n",
                 table_name, (gchar *)key, initial_proto_name,
                 current_proto_name);
        break;

    default:
        g_assert_not_reached();
        break;
    }
}

int
save_decode_as_entries(gchar** err)
{
    char *pf_dir_path;
    char *daf_path;
    FILE *da_file;

    if (create_persconffile_dir(&pf_dir_path) == -1) {
        *err = g_strdup_printf("Can't create directory\n\"%s\"\nfor recent file: %s.",
                                pf_dir_path, g_strerror(errno));
        g_free(pf_dir_path);
        return -1;
    }

    daf_path = get_persconffile_path(DECODE_AS_ENTRIES_FILE_NAME, TRUE);
    if ((da_file = ws_fopen(daf_path, "w")) == NULL) {
        *err = g_strdup_printf("Can't open decode_as_entries file\n\"%s\": %s.",
                                daf_path, g_strerror(errno));
        g_free(daf_path);
        return -1;
    }

    fputs("# \"Decode As\" entries file for Wireshark " VERSION ".\n"
        "#\n"
        "# This file is regenerated each time \"Decode As\" preferences\n"
        "# are saved within Wireshark. Making manual changes should be safe,\n"
        "# however.\n", da_file);

    dissector_all_tables_foreach_changed(decode_as_write_entry, da_file);
    fclose(da_file);
    g_free(daf_path);
    return 0;
}

/*
 * Data structure for tracking which dissector need to be reset.  This
 * structure is necessary as a hash table entry cannot be removed
 * while a g_hash_table_foreach walk is in progress.
 */
typedef struct dissector_delete_item {
    /* The name of the dissector table */
    gchar *ddi_table_name;
    /* The type of the selector in that dissector table */
    ftenum_t ddi_selector_type;
    /* The selector in the dissector table */
    union {
        guint   sel_uint;
        char    *sel_string;
    } ddi_selector;
} dissector_delete_item_t;

void
decode_build_reset_list (const gchar *table_name, ftenum_t selector_type,
                         gpointer key, gpointer value _U_,
                         gpointer user_data _U_)
{
    dissector_delete_item_t *item;

    item = g_new(dissector_delete_item_t,1);
    item->ddi_table_name = g_strdup(table_name);
    item->ddi_selector_type = selector_type;
    switch (selector_type) {

    case FT_UINT8:
    case FT_UINT16:
    case FT_UINT24:
    case FT_UINT32:
        item->ddi_selector.sel_uint = GPOINTER_TO_UINT(key);
        break;

    case FT_STRING:
    case FT_STRINGZ:
    case FT_UINT_STRING:
    case FT_STRINGZPAD:
        item->ddi_selector.sel_string = g_strdup((char *)key);
        break;

    default:
        g_assert_not_reached();
    }
    dissector_reset_list = g_slist_prepend(dissector_reset_list, item);
}

/* clear all settings */
void
decode_clear_all(void)
{
    dissector_delete_item_t *item;
    GSList *tmp;

    dissector_all_tables_foreach_changed(decode_build_reset_list, NULL);

    for (tmp = dissector_reset_list; tmp; tmp = g_slist_next(tmp)) {
        item = (dissector_delete_item_t *)tmp->data;
        switch (item->ddi_selector_type) {

        case FT_UINT8:
        case FT_UINT16:
        case FT_UINT24:
        case FT_UINT32:
            dissector_reset_uint(item->ddi_table_name,
                                 item->ddi_selector.sel_uint);
            break;

        case FT_STRING:
        case FT_STRINGZ:
        case FT_UINT_STRING:
        case FT_STRINGZPAD:
            dissector_reset_string(item->ddi_table_name,
                                   item->ddi_selector.sel_string);
            g_free(item->ddi_selector.sel_string);
            break;

        default:
            g_assert_not_reached();
        }
        g_free(item->ddi_table_name);
        g_free(item);
    }
    g_slist_free(dissector_reset_list);
    dissector_reset_list = NULL;

    g_list_free(decode_as_list);
    decode_as_list = NULL;

    decode_dcerpc_reset_all();
}

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
