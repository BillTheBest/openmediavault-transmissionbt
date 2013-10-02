/**
 * Copyright (C) 2013 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/tab/Panel.js")
// require("js/omv/module/admin/service/transmissionbt/configure/Settings.js")
// require("js/omv/module/admin/service/transmissionbt/configure/FilesAndLocations.js")
// require("js/omv/module/admin/service/transmissionbt/configure/Peer.js")
// require("js/omv/module/admin/service/transmissionbt/configure/Bandwidth.js")
// require("js/omv/module/admin/service/transmissionbt/configure/Queuing.js")
// require("js/omv/module/admin/service/transmissionbt/configure/Scheduling.js")

Ext.define("OMV.module.admin.service.transmissionbt.Configure", {
    extend   : "OMV.workspace.tab.Panel",
    requires : [
        "OMV.module.admin.service.transmissionbt.configure.Settings"
    ],

    initComponent : function() {
        var me = this;

        Ext.apply(me, {
            items : [
                Ext.create("OMV.module.admin.service.transmissionbt.configure.Settings", {
                    title: _("Settings")
                }),
                Ext.create("OMV.module.admin.service.transmissionbt.configure.FilesAndLocations", {
                    title: _("Files and locations")
                }),
                Ext.create("OMV.module.admin.service.transmissionbt.configure.Peer", {
                    title: _("Peer")
                }),
                Ext.create("OMV.module.admin.service.transmissionbt.configure.Bandwidth", {
                    title: _("Bandwidth")
                }),
                Ext.create("OMV.module.admin.service.transmissionbt.configure.Queuing", {
                    title: _("Queuing")
                }),
                Ext.create("OMV.module.admin.service.transmissionbt.configure.Scheduling", {
                    title: _("Scheduling")
                })
            ]
        });

        me.callParent(arguments);
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "configure",
    path      : "/service/transmissionbt",
    text      : _("Server"),
    position  : 20,
    className : "OMV.module.admin.service.transmissionbt.Configure"
});
