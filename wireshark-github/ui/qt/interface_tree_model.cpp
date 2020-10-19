/* interface_tree_model.cpp
 * Model for the interface data for display in the interface frame
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

#include "interface_tree_model.h"

#ifdef HAVE_LIBPCAP
#include "ui/capture.h"
#include "caputils/capture-pcap-util.h"
#include "capture_opts.h"
#include "ui/capture_ui_utils.h"
#include "ui/capture_globals.h"
#endif

#include "wsutil/filesystem.h"

#include "qt_ui_utils.h"
#include "stock_icon.h"
#include "wireshark_application.h"

/* Needed for the meta type declaration of QList<int>* */
#include "sparkline_delegate.h"

#ifdef HAVE_EXTCAP
#include "extcap.h"
#endif

const QString InterfaceTreeModel::DefaultNumericValue = QObject::tr("default");

/**
 * This is the data model for interface trees. It implies, that the index within
 * global_capture_opts.all_ifaces is identical to the row. This is always the case, even
 * when interfaces are hidden by the proxy model. But for this to work, every access
 * to the index from within the view, has to be filtered through the proxy model.
 */
InterfaceTreeModel::InterfaceTreeModel(QObject *parent) :
    QAbstractTableModel(parent)
#ifdef HAVE_LIBPCAP
    ,stat_cache_(NULL)
#endif
{
    connect(wsApp, SIGNAL(appInitialized()), this, SLOT(interfaceListChanged()));
    connect(wsApp, SIGNAL(localInterfaceListChanged()), this, SLOT(interfaceListChanged()));
}

InterfaceTreeModel::~InterfaceTreeModel(void)
{
#ifdef HAVE_LIBPCAP
    if (stat_cache_) {
        capture_stat_stop(stat_cache_);
        stat_cache_ = NULL;
    }
#endif // HAVE_LIBPCAP
}

QString InterfaceTreeModel::interfaceError()
{
    QString errorText;
    if ( rowCount() == 0 )
    {
        errorText = tr("No Interfaces found.");
    }
#ifdef HAVE_LIBPCAP
    else if ( global_capture_opts.ifaces_err != 0 )
    {
        errorText = tr(global_capture_opts.ifaces_err_info);
    }
#endif

    return errorText;
}

int InterfaceTreeModel::rowCount(const QModelIndex & ) const
{
#ifdef HAVE_LIBPCAP
    return (global_capture_opts.all_ifaces ? global_capture_opts.all_ifaces->len : 0);
#else
    /* Currently no interfaces available for libpcap-less builds */
    return 0;
#endif
}

int InterfaceTreeModel::columnCount(const QModelIndex & ) const
{
    /* IFTREE_COL_MAX is not being displayed, it is the definition for the maximum numbers of columns */
    return ((int) IFTREE_COL_MAX);
}

QVariant InterfaceTreeModel::data(const QModelIndex &index, int role) const
{
#ifdef HAVE_LIBPCAP
    bool interfacesLoaded = true;
    if ( ! global_capture_opts.all_ifaces || global_capture_opts.all_ifaces->len == 0 )
        interfacesLoaded = false;

    if ( !index.isValid() )
        return QVariant();

    int row = index.row();
    InterfaceTreeColumns col = (InterfaceTreeColumns) index.column();

    if ( interfacesLoaded )
    {
        interface_t device = g_array_index(global_capture_opts.all_ifaces, interface_t, row);

        /* Data for display in cell */
        if ( role == Qt::DisplayRole )
        {
            /* Only the name is being displayed */
            if ( col == IFTREE_COL_NAME )
            {
                return QString(device.display_name);
            }
            else if ( col == IFTREE_COL_INTERFACE_NAME )
            {
                return QString(device.name);
            }
            else if ( col == IFTREE_COL_PIPE_PATH )
            {
                return QString(device.if_info.name);
            }
            else if ( col == IFTREE_COL_CAPTURE_FILTER )
            {
                if ( device.cfilter && strlen(device.cfilter) > 0 )
                    return html_escape(QString(device.cfilter));
            }
#ifdef HAVE_EXTCAP
            else if ( col == IFTREE_COL_EXTCAP_PATH )
            {
                return QString(device.if_info.extcap);
            }
#endif
            else if ( col == IFTREE_COL_SNAPLEN )
            {
                return device.has_snaplen ? QString::number(device.snaplen) : DefaultNumericValue;
            }
#ifdef CAN_SET_CAPTURE_BUFFER_SIZE
            else if ( col == IFTREE_COL_BUFFERLEN )
            {
                return QString::number(device.buffer);
            }
#endif
            else if ( col == IFTREE_COL_TYPE )
            {
                return QVariant::fromValue((int)device.if_info.type);
            }
            else if ( col == IFTREE_COL_INTERFACE_COMMENT )
            {
                QString comment = gchar_free_to_qstring(capture_dev_user_descr_find(device.name));
                if ( comment.length() > 0 )
                    return comment;
                else
                    return QString(device.if_info.vendor_description);
            }
            else if ( col == IFTREE_COL_DLT )
            {
                QString linkname = QObject::tr("DLT %1").arg(device.active_dlt);
                for (GList *list = device.links; list != NULL; list = g_list_next(list)) {
                    link_row *linkr = (link_row*)(list->data);
                    if (linkr->dlt != -1 && linkr->dlt == device.active_dlt) {
                        linkname = linkr->name;
                        break;
                    }
                }

                return linkname;
            }
            else
            {
                /* Return empty string for every other DisplayRole */
                return QVariant();
            }
        }
        else if ( role == Qt::CheckStateRole )
        {
            if ( col == IFTREE_COL_HIDDEN )
            {
                /* Hidden is a de-selection, therefore inverted logic here */
                return device.hidden ? Qt::Unchecked : Qt::Checked;
            }
            else if ( col == IFTREE_COL_PROMISCUOUSMODE )
            {
                return device.pmode ? Qt::Checked : Qt::Unchecked;
            }
#ifdef HAVE_PCAP_CREATE
            else if ( col == IFTREE_COL_MONITOR_MODE )
            {
                return device.monitor_mode_enabled ? Qt::Checked : Qt::Unchecked;
            }
#endif
        }
        /* Used by SparkLineDelegate for loading the data for the statistics line */
        else if ( role == Qt::UserRole )
        {
            if ( col == IFTREE_COL_STATS )
            {
                if ( points.contains(device.name) )
                    return qVariantFromValue(points[device.name]);
            }
            else if ( col == IFTREE_COL_HIDDEN )
            {
                return QVariant::fromValue((bool)device.hidden);
            }
        }
#ifdef HAVE_EXTCAP
        /* Displays the configuration icon for extcap interfaces */
        else if ( role == Qt::DecorationRole )
        {
            if ( col == IFTREE_COL_EXTCAP )
            {
                if ( device.if_info.type == IF_EXTCAP )
                    return QIcon(StockIcon("x-capture-options"));
            }
        }
        else if ( role == Qt::TextAlignmentRole )
        {
            if ( col == IFTREE_COL_EXTCAP )
            {
                return Qt::AlignRight;
            }
        }
#endif
        /* Displays the tooltip for each row */
        else if ( role == Qt::ToolTipRole )
        {
            return toolTipForInterface(row);
        }
    }
#else
    Q_UNUSED(index)
    Q_UNUSED(role)
#endif

    return QVariant();
}

QVariant InterfaceTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal )
    {
        if ( role == Qt::DisplayRole )
        {
            if ( section == IFTREE_COL_HIDDEN )
            {
                return tr("Show");
            }
            else if ( section == IFTREE_COL_INTERFACE_NAME )
            {
                return tr("Friendly Name");
            }
            else if ( section == IFTREE_COL_NAME )
            {
                return tr("Interface Name");
            }
            else if ( section == IFTREE_COL_PIPE_PATH )
            {
                return tr("Local Pipe Path");
            }
            else if ( section == IFTREE_COL_INTERFACE_COMMENT )
            {
                return tr("Comment");
            }
            else if ( section == IFTREE_COL_DLT )
            {
                return tr("Link-Layer Header");
            }
            else if ( section == IFTREE_COL_PROMISCUOUSMODE )
            {
                return tr("Promiscuous");
            }
            else if ( section == IFTREE_COL_SNAPLEN )
            {
                return tr("Snaplen (B)");
            }
#ifdef CAN_SET_CAPTURE_BUFFER_SIZE
            else if ( section == IFTREE_COL_BUFFERLEN )
            {
                return tr("Buffer (MB)");
            }
#endif
#ifdef HAVE_PCAP_CREATE
            else if ( section == IFTREE_COL_MONITOR_MODE )
            {
                return tr("Monitor Mode");
            }
#endif
            else if ( section == IFTREE_COL_CAPTURE_FILTER )
            {
                return tr("Capture Filter");
            }
        }
    }

    return QVariant();
}

QVariant InterfaceTreeModel::getColumnContent(int idx, int col, int role)
{
    return InterfaceTreeModel::data(index(idx, col), role);
}

#ifdef HAVE_PCAP_REMOTE
bool InterfaceTreeModel::isRemote(int idx)
{
    interface_t device = g_array_index(global_capture_opts.all_ifaces, interface_t, idx);
    if ( device.remote_opts.src_type == CAPTURE_IFREMOTE )
        return true;
    return false;
}
#endif

/**
 * The interface list has changed. global_capture_opts.all_ifaces may have been reloaded
 * or changed with current data. beginResetModel() and endResetModel() will signalize the
 * proxy model and the view, that the data has changed and the view has to reload
 */
void InterfaceTreeModel::interfaceListChanged()
{
    emit beginResetModel();

    points.clear();

    emit endResetModel();
}

/*
 * Displays the tooltip code for the given device index.
 */
QVariant InterfaceTreeModel::toolTipForInterface(int idx) const
{
#ifdef HAVE_LIBPCAP
    if ( ! global_capture_opts.all_ifaces || global_capture_opts.all_ifaces->len <= (guint) idx)
        return QVariant();

    interface_t device = g_array_index(global_capture_opts.all_ifaces, interface_t, idx);

    QString tt_str = "<p>";
    if ( device.no_addresses > 0 )
    {
        tt_str += QString("%1: %2")
                .arg(device.no_addresses > 1 ? tr("Addresses") : tr("Address"))
                .arg(html_escape(device.addresses))
                .replace('\n', ", ");
    }
#ifdef HAVE_EXTCAP
    else if ( device.if_info.type == IF_EXTCAP )
    {
        tt_str = QString(tr("Extcap interface: %1")).arg(get_basename(device.if_info.extcap));
    }
#endif
    else
    {
        tt_str = tr("No addresses");
    }
    tt_str += "<br/>";

    QString cfilter = device.cfilter;
    if ( cfilter.isEmpty() )
    {
        tt_str += tr("No capture filter");
    }
    else
    {
        tt_str += QString("%1: %2")
                .arg(tr("Capture filter"))
                .arg(html_escape(cfilter));
    }
    tt_str += "</p>";

    return tt_str;
#else
    Q_UNUSED(idx)

    return QVariant();
#endif
}

#ifdef HAVE_LIBPCAP
void InterfaceTreeModel::stopStatistic()
{
    if ( stat_cache_ )
    {
        capture_stat_stop(stat_cache_);
        stat_cache_ = NULL;
    }
}
#endif

void InterfaceTreeModel::updateStatistic(unsigned int idx)
{
#ifdef HAVE_LIBPCAP
    guint diff;
    if ( ! global_capture_opts.all_ifaces || global_capture_opts.all_ifaces->len <= (guint) idx )
        return;

    interface_t device = g_array_index(global_capture_opts.all_ifaces, interface_t, idx);

    if ( device.if_info.type == IF_PIPE )
        return;

    if ( !stat_cache_ )
    {
        // Start gathering statistics using dumpcap
        // We crash (on macOS at least) if we try to do this from ::showEvent.
        stat_cache_ = capture_stat_start(&global_capture_opts);
    }
    if ( !stat_cache_ )
        return;

    struct pcap_stat stats;

    diff = 0;
    if ( capture_stats(stat_cache_, device.name, &stats) )
    {
        if ( (int)(stats.ps_recv - device.last_packets) >= 0 )
        {
            diff = stats.ps_recv - device.last_packets;
            device.packet_diff = diff;
        }
        device.last_packets = stats.ps_recv;

        global_capture_opts.all_ifaces = g_array_remove_index(global_capture_opts.all_ifaces, idx);
        g_array_insert_val(global_capture_opts.all_ifaces, idx, device);
    }

    points[device.name].append(diff);
    emit dataChanged(index(idx, IFTREE_COL_STATS), index(idx, IFTREE_COL_STATS));
#else
    Q_UNUSED(idx)
#endif
}

void InterfaceTreeModel::getPoints(int idx, PointList *pts)
{
#ifdef HAVE_LIBPCAP
    if ( ! global_capture_opts.all_ifaces || global_capture_opts.all_ifaces->len <= (guint) idx )
        return;

    interface_t device = g_array_index(global_capture_opts.all_ifaces, interface_t, idx);
    if ( points.contains(device.name) )
        pts->append(points[device.name]);
#else
    Q_UNUSED(idx)
    Q_UNUSED(pts)
#endif
}

QItemSelection InterfaceTreeModel::selectedDevices()
{
    QItemSelection mySelection;
#ifdef HAVE_LIBPCAP
    for( int idx = 0; idx < rowCount(); idx++ )
    {
        interface_t device = g_array_index(global_capture_opts.all_ifaces, interface_t, idx);

        if ( device.selected )
        {
            QModelIndex selectIndex = index(idx, 0);
            mySelection.merge(
                    QItemSelection( selectIndex, index(selectIndex.row(), columnCount() - 1) ),
                    QItemSelectionModel::SelectCurrent
                    );
        }
    }
#endif
    return mySelection;
}

bool InterfaceTreeModel::updateSelectedDevices(QItemSelection sourceSelection)
{
    bool selectionHasChanged = false;
#ifdef HAVE_LIBPCAP
    QList<int> selectedIndices;

    QItemSelection::const_iterator it = sourceSelection.constBegin();
    while(it != sourceSelection.constEnd())
    {
        QModelIndexList indeces = ((QItemSelectionRange) (*it)).indexes();

        QModelIndexList::const_iterator cit = indeces.constBegin();
        while(cit != indeces.constEnd())
        {
            QModelIndex index = (QModelIndex) (*cit);
            if ( ! selectedIndices.contains(index.row()) )
            {
                selectedIndices.append(index.row());
            }
            ++cit;
        }
        ++it;
    }

    global_capture_opts.num_selected = 0;

    for ( unsigned int idx = 0; idx < global_capture_opts.all_ifaces->len; idx++ )
    {
        interface_t device = g_array_index(global_capture_opts.all_ifaces, interface_t, idx);
        if ( !device.locked )
        {
            if ( selectedIndices.contains(idx) )
            {
                if (! device.selected )
                    selectionHasChanged = true;
                device.selected = TRUE;
                global_capture_opts.num_selected++;
            } else {
                if ( device.selected )
                    selectionHasChanged = true;
                device.selected = FALSE;
            }
            device.locked = TRUE;
            global_capture_opts.all_ifaces = g_array_remove_index(global_capture_opts.all_ifaces, idx);
            g_array_insert_val(global_capture_opts.all_ifaces, idx, device);

            device.locked = FALSE;
            global_capture_opts.all_ifaces = g_array_remove_index(global_capture_opts.all_ifaces, idx);
            g_array_insert_val(global_capture_opts.all_ifaces, idx, device);
        }
    }
#else
    Q_UNUSED(sourceSelection)
#endif
    return selectionHasChanged;
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
