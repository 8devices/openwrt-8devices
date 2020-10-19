/* pluginifdemo.c
 * Routines for plugin_if demo capability
 * Author: Roland Knall <rknall@gmail.com>
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

#include <stdio.h>

#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/tap.h>
#include <epan/plugin_if.h>
#include "pluginifdemo.h"

#include <ui/uihandler.h>

void proto_register_pluginifdemo(void);
void proto_reg_handoff_pluginifdemo(void);

static int proto_pluginifdemo = -1;

void toolbar_cb(gpointer object, gpointer item_data, gpointer user_data);

void
menu_cb(ext_menubar_gui_type gui_type, gpointer gui_data, gpointer user_data _U_)
{
    pluginifdemo_ui_main(gui_type, gui_data);
}

void
about_cb(ext_menubar_gui_type gui_type _U_, gpointer gui_data _U_, gpointer user_data _U_)
{
    pluginifdemo_ui_about(gui_type, gui_data);
}

void
proto_register_pluginifdemo(void)
{
    static hf_register_info hf[] = {
    };

    static gint *ett[] = {
    };

#if 0
    module_t *pluginif_module = NULL;
#endif
    ext_menu_t * ext_menu = NULL;

    proto_pluginifdemo = proto_register_protocol("Plugin IF Demo Protocol", "Pluginifdemo", "pluginifdemo");

    proto_register_field_array(proto_pluginifdemo, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));

    ext_menu = ext_menubar_register_menu ( proto_pluginifdemo, "Plugin IF Demonstration", TRUE );
    ext_menubar_set_parentmenu (ext_menu, "Tools");

    ext_menubar_add_entry(ext_menu, "Toolbar Action Demonstrator", "Action demonstrator for the plugin toolbar", menu_cb, NULL);
    ext_menubar_add_separator(ext_menu);
    ext_menubar_add_website(ext_menu, "Wireshark Development", "See Wireshark Development", "https://www.wireshark.org/develop.html");
    ext_menubar_add_separator(ext_menu);
    ext_menubar_add_entry(ext_menu, "&About Plugin IF Demonstration", "Further information", about_cb, NULL);
#if 0
    pluginif_module = prefs_register_protocol(proto_pluginifdemo, NULL);
#endif


    ext_toolbar_t * tb = ext_toolbar_register_toolbar("Plugin Interface Demo Toolbar");

    ext_toolbar_add_entry(tb, EXT_TOOLBAR_BUTTON, "Button 1", 0, "Button 1 to press", FALSE, 0, FALSE, 0, toolbar_cb, 0);
    ext_toolbar_add_entry(tb, EXT_TOOLBAR_BUTTON, "Button 2", 0, "Button 2 to press", TRUE, 0, FALSE, 0, toolbar_cb, 0);
    ext_toolbar_add_entry(tb, EXT_TOOLBAR_BOOLEAN, "Checkbox", 0, "Checkbox to Select", FALSE, 0, FALSE, 0, toolbar_cb, 0);
    ext_toolbar_add_entry(tb, EXT_TOOLBAR_STRING, "String 1", "Default String", "String without validation", FALSE, 0, TRUE, 0, toolbar_cb, 0);
    ext_toolbar_add_entry(tb, EXT_TOOLBAR_STRING, "String 2", "ABC", "String with validation", FALSE, 0, FALSE, "^[A-Z]+", toolbar_cb, 0);
    GList * entries = 0;
    entries = ext_toolbar_add_val( entries, "1", "ABCD", FALSE );
    entries = ext_toolbar_add_val(entries, "2", "EFG", FALSE );
    entries = ext_toolbar_add_val(entries, "3", "HIJ", TRUE );
    entries = ext_toolbar_add_val(entries, "4", "KLM", FALSE );
    entries = ext_toolbar_add_val(entries, "5", "NOP", FALSE );
    entries = ext_toolbar_add_val(entries, "6", "QRS", FALSE );
    entries = ext_toolbar_add_val(entries, "7", "TUVW", FALSE );
    entries = ext_toolbar_add_val(entries, "8", "XYZ", FALSE );
    ext_toolbar_add_entry(tb, EXT_TOOLBAR_SELECTOR, "Selector", 0, "Selector to choose from", FALSE, entries, FALSE, 0, toolbar_cb, 0);

    pluginifdemo_toolbar_register(tb);
}

void toolbar_cb(gpointer toolbar_item, gpointer item_data, gpointer user_data _U_)
{
    if ( ! toolbar_item )
        return;

    gchar * message = 0;
    ext_toolbar_t * entry = (ext_toolbar_t *)toolbar_item;

    if ( entry->item_type == EXT_TOOLBAR_BUTTON )
        pluginifdemo_toolbar_log ( "Button pressed at toolbar" );
    else if ( entry->item_type == EXT_TOOLBAR_BOOLEAN )
    {
        gboolean data = *((gboolean *)item_data);
        message = g_strdup_printf( "Checkbox selected value: %d", (int) (data) );
        pluginifdemo_toolbar_log(message);
    }
    else if ( entry->item_type == EXT_TOOLBAR_STRING )
    {
        gchar * data = (gchar *)item_data;
        message = g_strdup_printf( "String entered in toolbar: %s", data );
        pluginifdemo_toolbar_log(message);
    }
    else if ( entry->item_type == EXT_TOOLBAR_SELECTOR )
    {
        ext_toolbar_value_t * data = (ext_toolbar_value_t *)item_data;
        message = g_strdup_printf( "Value from toolbar: %s", data->value );
        pluginifdemo_toolbar_log(message);
    }

    g_free(message);
}

void
proto_reg_handoff_pluginifdemo(void)
{

}

/*
 * Editor modelines  -  http://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
