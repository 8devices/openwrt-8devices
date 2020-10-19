/* iface_toolbar.h
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

#ifndef __IFACE_TOOLBAR_H__
#define __IFACE_TOOLBAR_H__

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    INTERFACE_TYPE_UNKNOWN,
    INTERFACE_TYPE_BOOLEAN,
    INTERFACE_TYPE_BUTTON,
    INTERFACE_TYPE_SELECTOR,
    INTERFACE_TYPE_STRING
} iface_toolbar_ctrl_type;

typedef enum {
    INTERFACE_ROLE_UNKNOWN,
    INTERFACE_ROLE_CONTROL,
    INTERFACE_ROLE_HELP,
    INTERFACE_ROLE_LOGGER,
    INTERFACE_ROLE_RESTORE
} iface_toolbar_ctrl_role;

typedef struct _iface_toolbar_value {
    int num;
    gchar *value;
    gchar *display;
    gboolean is_default;
} iface_toolbar_value;

typedef struct _iface_toolbar_control {
    int num;
    iface_toolbar_ctrl_type ctrl_type;
    iface_toolbar_ctrl_role ctrl_role;
    gchar *display;
    gchar *validation;
    gboolean is_required;
    gchar *tooltip;
    gchar *placeholder;
    union {
        gboolean boolean;
        gchar *string;
    } default_value;
    GList *values;
} iface_toolbar_control;

typedef struct _iface_toolbar {
    gchar *menu_title;
    gchar *help;
    GList *ifnames;
    GList *controls;
} iface_toolbar;

typedef void (*iface_toolbar_add_cb_t)(const iface_toolbar *);
typedef void (*iface_toolbar_remove_cb_t)(const gchar *);

void iface_toolbar_add(const iface_toolbar *toolbar);

void iface_toolbar_remove(const gchar *menu_title);

gboolean iface_toolbar_use(void);

void iface_toolbar_register_cb(iface_toolbar_add_cb_t, iface_toolbar_remove_cb_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __IFACE_TOOLBAR_H__ */

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
