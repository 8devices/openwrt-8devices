/* extcap_parser.c
 *
 * Routines for extcap external capture
 * Copyright 2013, Mike Ryan <mikeryan@lacklustre.net>
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

#include <config.h>

#include <stdio.h>
#include <glib.h>
#include <string.h>

#include "ui/iface_toolbar.h"
#include "wsutil/strtoi.h"

#include "extcap.h"
#include "extcap_parser.h"

void extcap_printf_complex(extcap_complex *comp) {
    gchar *ret = extcap_get_complex_as_string(comp);
    printf("%s", ret);
    g_free(ret);
}

gchar *extcap_get_complex_as_string(extcap_complex *comp) {
    return (comp ? g_strdup(comp->_val) : NULL);
}

extcap_complex *extcap_parse_complex(extcap_arg_type complex_type,
                                     const gchar *data) {

    extcap_complex *rc = g_new0(extcap_complex, 1);

    rc->_val = g_strdup(data);
    rc->complex_type = complex_type;

    return rc;
}

gboolean extcap_compare_is_default(extcap_arg *element, extcap_complex *test) {
    if (element == NULL || element->default_complex == NULL || test == NULL)
        return FALSE;

    if (g_strcmp0(element->default_complex->_val, test->_val) == 0)
        return TRUE;

    return FALSE;
}

void extcap_free_complex(extcap_complex *comp) {
    if (comp)
        g_free(comp->_val);
    g_free(comp);
}

gint extcap_complex_get_int(extcap_complex *comp) {
    if (comp == NULL || comp->_val == NULL || comp->complex_type != EXTCAP_ARG_INTEGER)
        return (gint)0;

    return (gint) g_ascii_strtoll(comp->_val, NULL, 10);
}

guint extcap_complex_get_uint(extcap_complex *comp) {
    if (comp == NULL || comp->_val == NULL || comp->complex_type != EXTCAP_ARG_UNSIGNED)
        return (guint)0;
    return (guint) g_ascii_strtoull(comp->_val, NULL, 10);
}

gint64 extcap_complex_get_long(extcap_complex *comp) {
    if (comp == NULL || comp->_val == NULL || comp->complex_type != EXTCAP_ARG_LONG)
        return (gint64)0;
    return g_ascii_strtoll(comp->_val, NULL, 10);
}

gdouble extcap_complex_get_double(extcap_complex *comp) {
    if (comp == NULL || comp->_val == NULL || comp->complex_type != EXTCAP_ARG_DOUBLE)
        return (gdouble)0;
    return g_strtod(comp->_val, NULL);
}

gboolean extcap_complex_get_bool(extcap_complex *comp) {
    if (comp == NULL || comp->_val == NULL)
        return FALSE;

    if (comp->complex_type != EXTCAP_ARG_BOOLEAN && comp->complex_type != EXTCAP_ARG_BOOLFLAG)
        return FALSE;

    return g_regex_match_simple(EXTCAP_BOOLEAN_REGEX, comp->_val, G_REGEX_CASELESS, (GRegexMatchFlags)0);
}

gchar *extcap_complex_get_string(extcap_complex *comp) {
    /* Not checking for argument type, to use this method as fallback if only strings are needed */
    return comp != NULL ? comp->_val : NULL;
}

static extcap_token_sentence *extcap_tokenize_sentence(const gchar *s) {
    GRegex *regex = NULL;
    GMatchInfo *match_info = NULL;
    GError *error = NULL;
    gchar *param_value = NULL;
    guint param_type = EXTCAP_PARAM_UNKNOWN;

    extcap_token_sentence *rs = g_new0(extcap_token_sentence, 1);

    rs->sentence = NULL;

    /* Regex for catching just the allowed values for sentences */
    if ((regex = g_regex_new("^[\\t| ]*(arg|value|interface|extcap|dlt|control)(?=[\\t| ]+\\{)",
                             (GRegexCompileFlags) G_REGEX_CASELESS, (GRegexMatchFlags) 0, NULL)) != NULL) {
        g_regex_match(regex, s, (GRegexMatchFlags) 0, &match_info);

        if (g_match_info_matches(match_info))
            rs->sentence = g_match_info_fetch(match_info, 0);

        g_match_info_free(match_info);
        g_regex_unref(regex);
    }
    /* No valid sentence found, exiting here */
    if (rs->sentence == NULL) {
        g_free(rs);
        return NULL;
    }

    rs->param_list = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);

    /* Capture the argument and the value of the list. This will ensure,
     * that regex patterns given to {validation=} are parsed correctly,
     * as long as }{ does not occur within the pattern */
    regex = g_regex_new("\\{([a-zA-Z_-]*?)\\=(.*?)\\}(?=\\{|$|\\s)",
                        (GRegexCompileFlags) G_REGEX_CASELESS, (GRegexMatchFlags) 0, NULL);
    if (regex != NULL) {
        g_regex_match_full(regex, s, -1, 0, (GRegexMatchFlags) 0, &match_info, &error);
        while (g_match_info_matches(match_info)) {
            gchar *arg = g_match_info_fetch(match_info, 1);

            if (arg == NULL)
                break;

            param_value = g_match_info_fetch(match_info, 2);

            if (g_ascii_strcasecmp(arg, "number") == 0) {
                param_type = EXTCAP_PARAM_ARGNUM;
            } else if (g_ascii_strcasecmp(arg, "call") == 0) {
                param_type = EXTCAP_PARAM_CALL;
            } else if (g_ascii_strcasecmp(arg, "display") == 0) {
                param_type = EXTCAP_PARAM_DISPLAY;
            } else if (g_ascii_strcasecmp(arg, "type") == 0) {
                param_type = EXTCAP_PARAM_TYPE;
            } else if (g_ascii_strcasecmp(arg, "arg") == 0) {
                param_type = EXTCAP_PARAM_ARG;
            } else if (g_ascii_strcasecmp(arg, "default") == 0) {
                param_type = EXTCAP_PARAM_DEFAULT;
            } else if (g_ascii_strcasecmp(arg, "value") == 0) {
                param_type = EXTCAP_PARAM_VALUE;
            } else if (g_ascii_strcasecmp(arg, "range") == 0) {
                param_type = EXTCAP_PARAM_RANGE;
            } else if (g_ascii_strcasecmp(arg, "tooltip") == 0) {
                param_type = EXTCAP_PARAM_TOOLTIP;
            } else if (g_ascii_strcasecmp(arg, "placeholder") == 0) {
                param_type = EXTCAP_PARAM_PLACEHOLDER;
            } else if (g_ascii_strcasecmp(arg, "mustexist") == 0) {
                param_type = EXTCAP_PARAM_FILE_MUSTEXIST;
            } else if (g_ascii_strcasecmp(arg, "fileext") == 0) {
                param_type = EXTCAP_PARAM_FILE_EXTENSION;
            } else if (g_ascii_strcasecmp(arg, "name") == 0) {
                param_type = EXTCAP_PARAM_NAME;
            } else if (g_ascii_strcasecmp(arg, "enabled") == 0) {
                param_type = EXTCAP_PARAM_ENABLED;
            } else if (g_ascii_strcasecmp(arg, "parent") == 0) {
                param_type = EXTCAP_PARAM_PARENT;
            } else if (g_ascii_strcasecmp(arg, "required") == 0) {
                param_type = EXTCAP_PARAM_REQUIRED;
            } else if (g_ascii_strcasecmp(arg, "save") == 0) {
                param_type = EXTCAP_PARAM_SAVE;
            } else if (g_ascii_strcasecmp(arg, "validation") == 0) {
                param_type = EXTCAP_PARAM_VALIDATION;
            } else if (g_ascii_strcasecmp(arg, "version") == 0) {
                param_type = EXTCAP_PARAM_VERSION;
            } else if (g_ascii_strcasecmp(arg, "help") == 0) {
                param_type = EXTCAP_PARAM_HELP;
            } else if (g_ascii_strcasecmp(arg, "control") == 0) {
                param_type = EXTCAP_PARAM_CONTROL;
            } else if (g_ascii_strcasecmp(arg, "role") == 0) {
                param_type = EXTCAP_PARAM_ROLE;
            } else {
                param_type = EXTCAP_PARAM_UNKNOWN;
            }

            g_hash_table_insert(rs->param_list, ENUM_KEY(param_type), param_value);

            g_match_info_next(match_info, &error);
            g_free(arg);
        }
        g_match_info_free(match_info);
        g_regex_unref(regex);
    }

    return rs;
}

static GList *extcap_tokenize_sentences(const gchar *s) {

    GList *sentences = NULL;
    extcap_token_sentence *item = NULL;
    gchar **list, **list_iter;

    list_iter = list = g_strsplit(s, "\n", 0);
    while (*list_iter != NULL) {
        item = extcap_tokenize_sentence(*list_iter);
        if (item)
            sentences = g_list_append(sentences, item);
        list_iter++;
    }

    g_strfreev(list);

    return sentences;
}

static void extcap_free_value(extcap_value *v) {
    if (v == NULL)
        return;

    g_free(v->call);
    g_free(v->display);

    g_free(v);
}

static void extcap_free_valuelist(gpointer data, gpointer user_data _U_) {
    extcap_free_value((extcap_value *) data);
}

void extcap_free_arg(extcap_arg *a) {

    if (a == NULL)
        return;

    g_free(a->call);
    g_free(a->display);
    g_free(a->tooltip);
    g_free(a->placeholder);
    g_free(a->fileextension);
    g_free(a->regexp);
    g_free(a->device_name);

    if (a->range_start != NULL)
        extcap_free_complex(a->range_start);

    if (a->range_end != NULL)
        extcap_free_complex(a->range_end);

    if (a->default_complex != NULL)
        extcap_free_complex(a->default_complex);

    g_list_foreach(a->values, (GFunc) extcap_free_valuelist, NULL);
    g_list_free(a->values);
    g_free(a);
}

static void extcap_free_toolbar_value(iface_toolbar_value *v) {
    if (v == NULL)
        return;

    g_free(v->value);
    g_free(v->display);
}

static void extcap_free_toolbar_control(iface_toolbar_control *c) {
    if (c == NULL)
        return;

    g_free(c->display);
    g_free(c->validation);
    g_free(c->tooltip);
    g_free(c->placeholder);
}

void extcap_free_arg_list(GList *a) {
    g_list_foreach(a, (GFunc)extcap_free_arg, NULL);
    g_list_free(a);
}

static gint glist_find_numbered_arg(gconstpointer listelem, gconstpointer needle) {
    if (((const extcap_arg *) listelem)->arg_num == *((const int *) needle))
        return 0;
    return 1;
}

static gint glist_find_numbered_control(gconstpointer listelem, gconstpointer needle) {
    if (((const iface_toolbar_control *) listelem)->num == *((const int *) needle))
        return 0;
    return 1;
}

static void extcap_free_tokenized_sentence(gpointer s, gpointer user_data _U_) {
    extcap_token_sentence *t = (extcap_token_sentence *)s;

    if (t == NULL)
        return;

    g_free(t->sentence);
    g_hash_table_destroy(t->param_list);
    g_free(t);
}

static void extcap_free_tokenized_sentences(GList *sentences) {
    if (sentences == NULL)
        return;

    g_list_foreach(sentences, extcap_free_tokenized_sentence, NULL);
    g_list_free(sentences);
}

static extcap_arg *extcap_parse_arg_sentence(GList *args, extcap_token_sentence *s) {
    gchar *param_value = NULL;

    extcap_arg *target_arg = NULL;
    extcap_value *value = NULL;
    GList *entry = NULL;
    int tint;
    extcap_sentence_type sent = EXTCAP_SENTENCE_UNKNOWN;

    if (s == NULL)
        return target_arg;

    if (g_ascii_strcasecmp(s->sentence, "arg") == 0) {
        sent = EXTCAP_SENTENCE_ARG;
        /* printf("ARG sentence\n"); */
    } else if (g_ascii_strcasecmp(s->sentence, "value") == 0) {
        sent = EXTCAP_SENTENCE_VALUE;
        /* printf("VALUE sentence\n"); */
    }

    if (sent == EXTCAP_SENTENCE_ARG) {
        target_arg = g_new0(extcap_arg, 1);
        target_arg->arg_type = EXTCAP_ARG_UNKNOWN;
        target_arg->save = TRUE;


        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_ARGNUM))) == NULL) {
            extcap_free_arg(target_arg);
            return NULL;
        }

        if (sscanf(param_value, "%d", &(target_arg->arg_num)) != 1) {
            extcap_free_arg(target_arg);
            return NULL;
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_CALL))) == NULL) {
            extcap_free_arg(target_arg);
            return NULL;
        }
        target_arg->call = g_strdup(param_value);

        /* No value only parameters allowed */
        if (strlen(target_arg->call) == 0) {
            extcap_free_arg(target_arg);
            return NULL;
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DISPLAY))) == NULL) {
            extcap_free_arg(target_arg);
            return NULL;
        }
        target_arg->display = g_strdup(param_value);

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_TOOLTIP)))
                != NULL) {
            target_arg->tooltip = g_strdup(param_value);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_PLACEHOLDER)))
                != NULL) {
            target_arg->placeholder = g_strdup(param_value);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_FILE_MUSTEXIST)))
                != NULL) {
            target_arg->fileexists = g_regex_match_simple(EXTCAP_BOOLEAN_REGEX, param_value, G_REGEX_CASELESS, (GRegexMatchFlags)0);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_FILE_EXTENSION)))
                != NULL) {
            target_arg->fileextension = g_strdup(param_value);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_VALIDATION)))
                != NULL) {
            target_arg->regexp = g_strdup(param_value);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_REQUIRED)))
                != NULL) {
            target_arg->is_required = g_regex_match_simple(EXTCAP_BOOLEAN_REGEX, param_value, G_REGEX_CASELESS, (GRegexMatchFlags)0);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_TYPE)))
                == NULL) {
            /* printf("no type in ARG sentence\n"); */
            extcap_free_arg(target_arg);
            return NULL;
        }

        if (g_ascii_strcasecmp(param_value, "integer") == 0) {
            target_arg->arg_type = EXTCAP_ARG_INTEGER;
        } else if (g_ascii_strcasecmp(param_value, "unsigned") == 0) {
            target_arg->arg_type = EXTCAP_ARG_UNSIGNED;
        } else if (g_ascii_strcasecmp(param_value, "long") == 0) {
            target_arg->arg_type = EXTCAP_ARG_LONG;
        } else if (g_ascii_strcasecmp(param_value, "double") == 0) {
            target_arg->arg_type = EXTCAP_ARG_DOUBLE;
        } else if (g_ascii_strcasecmp(param_value, "boolean") == 0) {
            target_arg->arg_type = EXTCAP_ARG_BOOLEAN;
        } else if (g_ascii_strcasecmp(param_value, "boolflag") == 0) {
            target_arg->arg_type = EXTCAP_ARG_BOOLFLAG;
        } else if (g_ascii_strcasecmp(param_value, "selector") == 0) {
            target_arg->arg_type = EXTCAP_ARG_SELECTOR;
        } else if (g_ascii_strcasecmp(param_value, "radio") == 0) {
            target_arg->arg_type = EXTCAP_ARG_RADIO;
        } else if (g_ascii_strcasecmp(param_value, "string") == 0) {
            target_arg->arg_type = EXTCAP_ARG_STRING;
        } else if (g_ascii_strcasecmp(param_value, "password") == 0) {
            target_arg->arg_type = EXTCAP_ARG_PASSWORD;
            /* default setting is to not save passwords */
            target_arg->save = FALSE;
        } else if (g_ascii_strcasecmp(param_value, "fileselect") == 0) {
            target_arg->arg_type = EXTCAP_ARG_FILESELECT;
        } else if (g_ascii_strcasecmp(param_value, "multicheck") == 0) {
            target_arg->arg_type = EXTCAP_ARG_MULTICHECK;
        } else if (g_ascii_strcasecmp(param_value, "timestamp") == 0) {
            target_arg->arg_type = EXTCAP_ARG_TIMESTAMP;
        } else {
            printf("invalid type %s in ARG sentence\n", param_value);
            extcap_free_arg(target_arg);
            return NULL;
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_SAVE)))
                != NULL) {
            target_arg->save = g_regex_match_simple(EXTCAP_BOOLEAN_REGEX, param_value, G_REGEX_CASELESS, (GRegexMatchFlags)0);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_RANGE)))
                != NULL) {
            gchar *cp = g_strstr_len(param_value, -1, ",");

            if (cp == NULL) {
                printf("invalid range, expected value,value got %s\n",
                       param_value);
                extcap_free_arg(target_arg);
                return NULL;
            }

            if ((target_arg->range_start = extcap_parse_complex(
                                               target_arg->arg_type, param_value)) == NULL) {
                printf("invalid range, expected value,value got %s\n",
                       param_value);
                extcap_free_arg(target_arg);
                return NULL;
            }

            if ((target_arg->range_end = extcap_parse_complex(
                                             target_arg->arg_type, cp + 1)) == NULL) {
                printf("invalid range, expected value,value got %s\n",
                       param_value);
                extcap_free_arg(target_arg);
                return NULL;
            }
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DEFAULT)))
                != NULL) {
            if (target_arg->arg_type != EXTCAP_ARG_MULTICHECK && target_arg->arg_type != EXTCAP_ARG_SELECTOR)
            {
                if ((target_arg->default_complex = extcap_parse_complex(
                                                       target_arg->arg_type, param_value)) == NULL) {
                    printf("invalid default, couldn't parse %s\n", param_value);
                }
            }
        }

    } else if (sent == EXTCAP_SENTENCE_VALUE) {
        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_ARG)))
                == NULL) {
            printf("no arg in VALUE sentence\n");
            return NULL;
        }

        if (sscanf(param_value, "%d", &tint) != 1) {
            printf("invalid arg in VALUE sentence\n");
            return NULL;
        }

        if ((entry = g_list_find_custom(args, &tint, glist_find_numbered_arg))
                == NULL) {
            printf("couldn't find arg %d in list for VALUE sentence\n", tint);
            return NULL;
        }

        value = g_new0(extcap_value, 1);
        value->arg_num = tint;

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_VALUE)))
                == NULL) {
            /* printf("no value in VALUE sentence\n"); */
            extcap_free_value(value);
            return NULL;
        }
        value->call = g_strdup(param_value);

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DISPLAY)))
                == NULL) {
            /* printf("no display in VALUE sentence\n"); */
            extcap_free_value(value);
            return NULL;
        }
        value->display = g_strdup(param_value);

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_PARENT)))
                != NULL) {
            value->parent = g_strdup(param_value);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DEFAULT)))
                != NULL) {
            /* printf("found default value\n"); */
            value->is_default = g_regex_match_simple(EXTCAP_BOOLEAN_REGEX, param_value, G_REGEX_CASELESS, (GRegexMatchFlags)0);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_ENABLED)))
                != NULL) {
            value->enabled = g_regex_match_simple(EXTCAP_BOOLEAN_REGEX, param_value, G_REGEX_CASELESS, (GRegexMatchFlags)0);
        }

        ((extcap_arg *) entry->data)->values = g_list_append(
                ((extcap_arg *) entry->data)->values, value);

        return NULL;
    }

    return target_arg;
}

GList *extcap_parse_args(gchar *output) {
    GList *result = NULL;
    GList *walker = NULL;
    GList *temp = NULL;

    walker = extcap_tokenize_sentences(output);
    temp = walker;

    while (walker) {
        extcap_arg *ra = NULL;
        extcap_token_sentence *sentence = (extcap_token_sentence *)walker->data;

        if ((ra = extcap_parse_arg_sentence(result, sentence)) != NULL)
            result = g_list_append(result, (gpointer) ra);

        walker = g_list_next(walker);
    }

    extcap_free_tokenized_sentences(temp);

    return result;
}

static extcap_interface *extcap_parse_interface_sentence(extcap_token_sentence *s) {
    extcap_sentence_type sent = EXTCAP_SENTENCE_UNKNOWN;
    gchar *param_value = NULL;
    extcap_interface *ri = NULL;

    if (s == NULL)
        return NULL;

    if (g_ascii_strcasecmp(s->sentence, "interface") == 0) {
        sent = EXTCAP_SENTENCE_INTERFACE;
    } else if (g_ascii_strcasecmp(s->sentence, "extcap") == 0) {
        sent = EXTCAP_SENTENCE_EXTCAP;
    }

    if (sent == EXTCAP_SENTENCE_UNKNOWN)
        return NULL;

    ri = g_new0(extcap_interface, 1);

    ri->if_type = sent;

    if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_VALUE)))
            == NULL && sent == EXTCAP_SENTENCE_INTERFACE) {
        printf("No value in INTERFACE sentence\n");
        g_free(ri);
        return NULL;
    }
    ri->call = g_strdup(param_value);

    if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DISPLAY)))
            == NULL && sent == EXTCAP_SENTENCE_INTERFACE) {
        printf("No display in INTERFACE sentence\n");
        g_free(ri->call);
        g_free(ri);
        return NULL;
    }
    ri->display = g_strdup(param_value);

    if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_VERSION)))
            != NULL) {
        ri->version = g_strdup(param_value);
    }

    if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_HELP)))
            != NULL) {
        ri->help = g_strdup(param_value);
    }

    return ri;
}

static iface_toolbar_control *extcap_parse_control_sentence(GList *control_items, extcap_token_sentence *s)
{
    extcap_sentence_type sent = EXTCAP_SENTENCE_UNKNOWN;
    gchar *param_value = NULL;
    iface_toolbar_control *control = NULL;
    iface_toolbar_value *value = NULL;
    GList *entry = NULL;
    guint32 num = 0;

    if (s == NULL)
        return NULL;

    if (g_ascii_strcasecmp(s->sentence, "control") == 0) {
        sent = EXTCAP_SENTENCE_CONTROL;
    } else if (g_ascii_strcasecmp(s->sentence, "value") == 0) {
        sent = EXTCAP_SENTENCE_VALUE;
    }

    if (sent == EXTCAP_SENTENCE_UNKNOWN)
        return NULL;

    if (sent == EXTCAP_SENTENCE_CONTROL) {
        control = g_new0(iface_toolbar_control, 1);
        control->ctrl_type = INTERFACE_TYPE_UNKNOWN;

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_ARGNUM));
        if (param_value == NULL) {
            extcap_free_toolbar_control(control);
            return NULL;
        }

        if (!ws_strtou32(param_value, NULL, &num)) {
            extcap_free_toolbar_control(control);
            return NULL;
        }
        control->num = (int)num;

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DISPLAY));
        if (param_value == NULL) {
            extcap_free_toolbar_control(control);
            return NULL;
        }
        control->display = g_strdup(param_value);

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_VALIDATION)))
            != NULL) {
            control->validation = g_strdup(param_value);
        }

        if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_REQUIRED)))
            != NULL) {
            control->is_required = g_regex_match_simple(EXTCAP_BOOLEAN_REGEX, param_value, G_REGEX_CASELESS, (GRegexMatchFlags)0);
        }

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_TOOLTIP));
        if (param_value != NULL) {
            control->tooltip = g_strdup(param_value);
        }

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_PLACEHOLDER));
        if (param_value != NULL) {
            control->placeholder = g_strdup(param_value);
        }

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_TYPE));
        if (param_value == NULL) {
            extcap_free_toolbar_control(control);
            return NULL;
        }

        extcap_arg_type arg_type = EXTCAP_ARG_UNKNOWN;
        if (g_ascii_strcasecmp(param_value, "boolean") == 0) {
            control->ctrl_type = INTERFACE_TYPE_BOOLEAN;
            arg_type = EXTCAP_ARG_BOOLEAN;
        } else if (g_ascii_strcasecmp(param_value, "button") == 0) {
            control->ctrl_type = INTERFACE_TYPE_BUTTON;
        } else if (g_ascii_strcasecmp(param_value, "selector") == 0) {
            control->ctrl_type = INTERFACE_TYPE_SELECTOR;
        } else if (g_ascii_strcasecmp(param_value, "string") == 0) {
            control->ctrl_type = INTERFACE_TYPE_STRING;
            arg_type = EXTCAP_ARG_STRING;
        } else {
            printf("invalid type %s in CONTROL sentence\n", param_value);
            extcap_free_toolbar_control(control);
            return NULL;
        }

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_ROLE));
        if (param_value != NULL) {
            if (g_ascii_strcasecmp(param_value, "control") == 0) {
                control->ctrl_role = INTERFACE_ROLE_CONTROL;
            } else if (g_ascii_strcasecmp(param_value, "help") == 0) {
                control->ctrl_role = INTERFACE_ROLE_HELP;
            } else if (g_ascii_strcasecmp(param_value, "logger") == 0) {
                control->ctrl_role = INTERFACE_ROLE_LOGGER;
            } else if (g_ascii_strcasecmp(param_value, "restore") == 0) {
                control->ctrl_role = INTERFACE_ROLE_RESTORE;
            } else {
                printf("invalid role %s in CONTROL sentence\n", param_value);
                control->ctrl_role = INTERFACE_ROLE_UNKNOWN;
            }
        } else {
            /* Default role */
            control->ctrl_role = INTERFACE_ROLE_CONTROL;
        }

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DEFAULT));
        if (param_value != NULL) {
            if (arg_type != EXTCAP_ARG_UNKNOWN) {
                extcap_complex *complex = extcap_parse_complex(arg_type, param_value);
                if (complex != NULL) {
                    if (arg_type == EXTCAP_ARG_BOOLEAN) {
                        control->default_value.boolean = extcap_complex_get_bool(complex);
                    } else if (arg_type == EXTCAP_ARG_STRING) {
                        control->default_value.string = g_strdup(complex->_val);
                    }
                    extcap_free_complex(complex);
                } else {
                    printf("invalid default, couldn't parse %s\n", param_value);
                }
            }
        }

    } else if (sent == EXTCAP_SENTENCE_VALUE) {
        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_CONTROL));
        if (param_value == NULL) {
            printf("no control in VALUE sentence\n");
            return NULL;
        }

        if (!ws_strtou32(param_value, NULL, &num)) {
            extcap_free_toolbar_control(control);
            return NULL;
        }

        entry = g_list_find_custom(control_items, &num, glist_find_numbered_control);
        if (entry == NULL) {
            printf("couldn't find control %u in list for VALUE sentence\n", num);
            return NULL;
        }

        value = g_new0(iface_toolbar_value, 1);
        value->num = (int)num;

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_VALUE));
        if (param_value == NULL) {
            extcap_free_toolbar_value(value);
            return NULL;
        }
        value->value = g_strdup(param_value);

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DISPLAY));
        if (param_value == NULL) {
            extcap_free_toolbar_value(value);
            return NULL;
        }
        value->display = g_strdup(param_value);

        param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DEFAULT));
        if (param_value != NULL) {
            value->is_default = g_regex_match_simple(EXTCAP_BOOLEAN_REGEX, param_value, G_REGEX_CASELESS, (GRegexMatchFlags)0);
        }

        control = (iface_toolbar_control *)entry->data;
        control->values = g_list_append(control->values, value);

        return NULL;
    }

    return control;
}

GList *extcap_parse_interfaces(gchar *output, GList **control_items) {

    GList *result = NULL;
    GList *tokens = NULL;
    GList *walker = extcap_tokenize_sentences(output);
    tokens = walker;

    while (walker) {
        extcap_interface *ri = NULL;
        iface_toolbar_control *ti = NULL;
        extcap_token_sentence *if_sentence = (extcap_token_sentence *) walker->data;

        if (if_sentence) {
            if ((g_ascii_strcasecmp(if_sentence->sentence, "interface") == 0) ||
                (g_ascii_strcasecmp(if_sentence->sentence, "extcap") == 0))
            {
                if ((ri = extcap_parse_interface_sentence(if_sentence))) {
                    result = g_list_append(result, ri);
                }
            } else if (control_items &&
                       ((g_ascii_strcasecmp(if_sentence->sentence, "control") == 0) ||
                        (g_ascii_strcasecmp(if_sentence->sentence, "value") == 0)))
            {
                if ((ti = extcap_parse_control_sentence(*control_items, if_sentence))) {
                    *control_items = g_list_append(*control_items, ti);
                }
            }
        }

        walker = g_list_next(walker);
    }

    extcap_free_tokenized_sentences(tokens);

    return result;
}

/* Parse a tokenized set of sentences and validate, looking for DLT definitions */
static extcap_dlt *extcap_parse_dlt_sentence(extcap_token_sentence *s) {
    gchar *param_value = NULL;
    extcap_sentence_type sent = EXTCAP_SENTENCE_UNKNOWN;
    extcap_dlt *result = NULL;

    if (s == NULL)
        return result;

    if (g_ascii_strcasecmp(s->sentence, "dlt") == 0) {
        sent = EXTCAP_SENTENCE_DLT;
    }

    if (sent == EXTCAP_SENTENCE_UNKNOWN)
        return result;

    result = g_new0(extcap_dlt, 1);

    result->number = -1;
    result->name = NULL;
    result->display = NULL;

    if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_ARGNUM)))
            == NULL) {
        printf("No number in DLT sentence\n");
        g_free(result);
        return NULL;
    }
    if (sscanf(param_value, "%d", &(result->number)) != 1) {
        printf("Invalid number in DLT sentence\n");
        g_free(result);
        return NULL;
    }

    if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_NAME)))
            == NULL) {
        printf("No name in DLT sentence\n");
        g_free(result);
        return NULL;
    }
    result->name = g_strdup(param_value);

    if ((param_value = (gchar *)g_hash_table_lookup(s->param_list, ENUM_KEY(EXTCAP_PARAM_DISPLAY)))
            == NULL) {
        printf("No display in DLT sentence\n");
        g_free(result->name);
        g_free(result);
        return NULL;
    }
    result->display = g_strdup(param_value);

    return result;
}

GList *extcap_parse_dlts(gchar *output) {

    GList *walker = NULL;
    GList *temp = NULL;
    GList *result = NULL;

    walker = extcap_tokenize_sentences(output);

    temp = walker;

    while (walker) {
        extcap_dlt *data = NULL;

        if ((data = extcap_parse_dlt_sentence((extcap_token_sentence *)walker->data)) != NULL)
            result = g_list_append(result, data);

        walker = g_list_next(walker);
    }

    extcap_free_tokenized_sentences(temp);

    return result;
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
