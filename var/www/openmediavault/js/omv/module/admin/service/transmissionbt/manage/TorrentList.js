/**
 * Copyright (C) 2011-2012 Marcel Beck <marcel.beck@mbeck.org>
 * Copyright (C)      2013 OpenMediaVault Plugin Developers
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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/window/Upload.js")
// require("js/omv/module/admin/service/transmissionbt/util/Format.js")
// require("js/omv/module/admin/service/transmissionbt/manage/dialog/AddUrl.js")
// require("js/omv/module/admin/service/transmissionbt/manage/dialog/Delete.js")

Ext.define("OMV.module.admin.service.transmissionbt.manage.TorrentList", {
    extend      : "OMV.workspace.grid.Panel",
    requires    : [
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc"
    ],
    uses        : [
        "OMV.module.admin.service.transmissionbt.manage.dialog.AddUrl",
        "OMV.module.admin.service.transmissionbt.manage.dialog.Delete"
    ],

    autoReload              : true,
    rememberSelected        : true,
    hidePagingToolbar       : false,
    hideAddButton           : true,
    hideEditButton          : true,
    pauseWaitMsg            : _("Pausing selected item(s)"),
    resumeWaitMsg           : _("Resuming selected item(s)"),
    deletionConfirmRequired :  true,
    deletionWaitMsg         : _("Deleting selected item(s)"),
    queueMoveWaitMsg        : _("Queue moving selcted item(s)"),
    stateful                : true,
    stateId                 : "cb44cbf3-b1cb-b6ba-13548ab0dc7c246c",

    columns: [{
        header    : _("ID"),
        sortable  : true,
        dataIndex : "id"
    },{
        header    : _("Name"),
        sortable  : true,
        dataIndex : "name"
    },{
        header    : _("Status"),
        sortable  : true,
        dataIndex : "status",
        renderer  : OMV.module.services.transmissionbt.util.Format.statusRenderer
    },{
        header    : _("Done"),
        sortable  : true,
        dataIndex : "percentDone",
        renderer  : OMV.module.services.transmissionbt.util.Format.doneRenderer
    },{
        header    : _("ETA"),
        sortable  : true,
        dataIndex : "eta",
        renderer  : OMV.module.services.transmissionbt.util.Format.etaRenderer
    },{
        header    : _("Peers"),
        sortable  : true,
        dataIndex : "peers",
        renderer  : OMV.module.services.transmissionbt.util.Format.peersRenderer
    },{
        header    : _("DL-rate"),
        sortable  : true,
        dataIndex : "rateDownload",
        renderer  : OMV.module.services.transmissionbt.util.Format.rateRenderer
    },{
        header    : _("UL-rate"),
        sortable  : true,
        dataIndex : "rateUpload",
        renderer  : OMV.module.services.transmissionbt.util.Format.rateRenderer
    },{
        header    : _("Date added"),
        sortable  : true,
        dataIndex : "addedDate",
        renderer  : OMV.module.services.transmissionbt.util.Format.timestampRenderer
    },{
        header    : _("Date done"),
        sortable  : true,
        dataIndex : "doneDate",
        renderer  : OMV.module.services.transmissionbt.util.Format.timestampRenderer
    },{
        header    : _("Ratio"),
        sortable  : true,
        dataIndex : "uploadRatio",
        renderer  : OMV.module.services.transmissionbt.util.Format.ratioRenderer
    },{
        header    : _("Queue"),
        sortable  : true,
        dataIndex : "queuePosition"
    }],

    initComponent : function() {
        var me = this;

        Ext.apply(me, {
            store: Ext.create("OMV.data.Store", {
                autoload : false,
                model    : OMV.data.Model.createImplicit({
                    idProperty    : "id",
                    totalProperty : "total",
                    fields        : [
                        { name :  "id" },
                        { name : "name" },
                        { name : "status" },
                        { name : "totalSize" },
                        { name : "haveValid" },
                        { name : "percentDone" },
                        { name : "eta" },
                        { name : "peersConnected" },
                        { name : "peersSendingToUs" },
                        { name : "rateDownload" },
                        { name : "rateUpload" },
                        { name : "addedDate" },
                        { name : "doneDate" },
                        { name : "uploadRatio" },
                        { name : "queuePosition" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        "service" : "TransmissionBT",
                        "method"  : "getList"
                    }
                }
            })
        });

        me.menu = me.getMenu();
        me.callParent(arguments);

        me.on({
            itemcontextmenu : me.onItemContextMenu
        });
    },

    onReload : function(id, success, response) {
        var me = this;

        if (success) {
            if (!response[0].running)  {
                me.disableReloadAndButtons();
            } else {
                if (me.store !== null)
                    me.store.reload();

                me.toggleAddTorrentButtons(true);
                me.onSelectionChange();
            }
        } else {
            OMV.MessageBox.error(null, response);
            me.disableReloadAndButtons();
        }

    },

    disableReloadAndButtons : function() {
        var me = this;

        me.toggleAddTorrentButtons(false);
        me.store.removeAll();
    },

    doReload : function() {
        var me = this;

        // Run a rpc request to see if we
        // should enable automatic reload
        OMV.Rpc.request({
            scope    : me,
            callback : me.onReload,
            rpcData  : {
                service : "TransmissionBT",
                method  : "getStatus"
            }
        });
    },

    getTopToolbarItems : function() {
        var me = this;
        var items = me.callParent(arguments);

        Ext.Array.insert(items, 1, [{
            id      : me.getId() + "-reload",
            xtype   : "button",
            text    : _("Reload"),
            icon    : "images/refresh.png",
            iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler : Ext.Function.bind(me.doReload, me, [ me ]),
            scope   : me
        },{
            id      : me.getId() + "-upload",
            xtype   : "button",
            text    : _("Upload"),
            icon    : "images/upload.png",
            iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler : Ext.Function.bind(me.onUploadButton, me, [ me ]),
            scope   : me
        },{
            id      : me.getId() + "-add-url",
            xtype   : "button",
            text    : _("Add URL"),
            icon    : "images/add.png",
            iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler : Ext.Function.bind(me.onAddUrlButton, me, [ me ]),
            scope   : me
        },{
            id      : me.getId() + "-resume",
            xtype   : "button",
            text    : _("Resume"),
            icon    : "images/transmissionbt-resume.png",
            iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler : Ext.Function.bind(me.onResumeButton, me, [ me ]),
            scope   : me,
            disabled: true
        },{
            id      : me.getId() + "-pause",
            xtype   : "button",
            text    : _("Pause"),
            icon    : "images/transmissionbt-pause.png",
            iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler : Ext.Function.bind(me.onPauseButton, me, [ me ]),
            scope   : me,
            disabled: true
        }]);

        return items;
    },

    getMenu : function() {
        var me = this;

        return Ext.create("Ext.menu.Menu", {
            items: [{
                id      : me.getId() + "-menu-resume",
                text    : _("Resume"),
                icon    : "images/transmissionbt-resume.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : me.onResumeButton,
                scope   : me
            },{
                id      : me.getId() + "-menu-pause",
                text    : _("Pause"),
                icon    : "images/transmissionbt-pause.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : me.onPauseButton,
                scope   : me
            },{
                id      : me.getId() + "-menu-delete",
                text    : _("Delete"),
                icon    : "images/delete.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : me.onDeleteButton,
                scope   : me
            },{
                id      : me.getId() + "-menu-queue-top",
                text    : _("Queue Move Top"),
                icon    : "images/arrow-up.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : me.onQueueMoveMenu,
                scope   : me,
                action  : 'top'
            },{
                id      : me.getId() + "-menu-queue-up",
                text    : _("Queue Move Up"),
                icon    : "images/arrow-up.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : me.onQueueMoveMenu,
                scope   : me,
                action  : 'up'
            },{
                id      : me.getId() + "-menu-queue-down",
                text    : _("Queue Move Down"),
                icon    : "images/arrow-down.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : me.onQueueMoveMenu,
                scope   : me,
                action  : 'down'
            },{
                id      : me.getId() + "-menu-queue-bottom",
                text    : _("Queue Move Bottom"),
                icon    : "images/arrow-down.png",
                iconCls : Ext.baseCSSPrefix + "btn-icon-16x16",
                handler : me.onQueueMoveMenu,
                scope   : me,
                action  : 'bottom'
            }]
        });
    },

    onDestroy : function() {
        var me = this;

        me.menu.destroy();
        me.callParent(arguments);
    },

    onItemContextMenu : function (view, record, item, index, e) {
        var me = this;

        e.stopEvent();
        me.menu.showAt(e.getXY());
    },

    onSelectionChange : function(model, records) {
        var me = this;

        // Don't pass records since we use pop
        // in these methods and arrays are
        // passed by reference
        me.toggleTopToolbarButtons();
        me.toggleMenuButtons();
    },

    toggleAddTorrentButtons : function(enable) {
        var me = this;
        addUrlButton = me.queryById(me.getId() + "-add-url");
        uploadButton = me.queryById(me.getId() + "-upload");

        if (enable) {
            addUrlButton.enable();
            uploadButton.enable();
        } else {
            addUrlButton.disable();
            uploadButton.disable();
        }
    },

    toggleTopToolbarButtons : function(records) {
        var me = this;

        if (!records)
            records = me.getSelection();

        var tbarBtnName = [
            "resume",
            "pause",
            "delete"
        ];

        var tbarBtnDisabled = {
            "resume" : false,
            "pause"  : false,
            "delete" : false
        };

        // Set button states on resume, pause and delete depending
        // selected row/s
        if (records.length > 1) {
            tbarBtnDisabled = {
                "resume" : true,
                "pause"  : true,
                "delete" : true
            };
        } else if (records.length == 1) {
            var record = records.pop();
            var status = parseInt(record.get("status"), 10);

            /* 0: Torrent is stopped    */
            /* 1: Queued to check files */
            /* 2: Checking files        */
            /* 3: Queued to download    */
            /* 4: Downloading           */
            /* 5: Queued to seed        */
            /* 6: Seeding               */
            switch (status) {
                case 0:
                    tbarBtnDisabled = {
                        "resume" : true,
                        "pause"  : false,
                        "delete" : true
                    };
                    break;
                default:
                    tbarBtnDisabled = {
                        "resume" : false,
                        "pause"  : true,
                        "delete" : true
                    };
                    break;
            }
        }

        for (var i = 0, j = tbarBtnName.length; i < j; i++) {
            var tbarBtnCtrl = me.queryById(me.getId() + "-" +
                tbarBtnName[i]);

            if (!Ext.isEmpty(tbarBtnCtrl)) {
                if (tbarBtnDisabled[tbarBtnName[i]] === false) {
                    tbarBtnCtrl.disable();
                } else {
                    tbarBtnCtrl.enable();
                }
            }
        }
    },

    toggleMenuButtons : function(records) {
        var me = this;

        if (!records)
            records = me.getSelection();

        var menuBtnName = [
            "menu-resume",
            "menu-pause",
            "menu-delete",
            "menu-queue-top",
            "menu-queue-up",
            "menu-queue-down",
            "menu-queue-bottom"
        ];

        var menuBtnDisabled = {
            "menu-resume" : false,
            "menu-pause"  : false,
            "menu-delete" : false,

            "menu-queue-top"    : false,
            "menu-queue-up"     : false,
            "menu-queue-down"   : false,
            "menu-queue-bottom" : false
        };

        if (records.length > 1) {
            menuBtnDisabled = {
                "menu-resume" : true,
                "menu-pause"  : true,
                "menu-delete" : true,

                "menu-queue-top"    : true,
                "menu-queue-up"     : true,
                "menu-queue-down"   : true,
                "menu-queue-bottom" : true
            };
        } else if (records.length == 1) {
            var record = records.pop();
            var status = parseInt(record.get("status"), 10);

            /* 0: Torrent is stopped    */
            /* 1: Queued to check files */
            /* 2: Checking files        */
            /* 3: Queued to download    */
            /* 4: Downloading           */
            /* 5: Queued to seed        */
            /* 6: Seeding               */
            switch (status) {
                case 0:
                    menuBtnDisabled = {
                        "menu-resume" : true,
                        "menu-pause"  : false,
                        "menu-delete" : true,

                        "menu-queue-top"    : true,
                        "menu-queue-up"     : true,
                        "menu-queue-down"   : true,
                        "menu-queue-bottom" : true
                    };
                    break;
                default:
                    menuBtnDisabled = {
                        "menu-resume" : false,
                        "menu-pause"  : true,
                        "menu-delete" : true,

                        "menu-queue-top"    : true,
                        "menu-queue-up"     : true,
                        "menu-queue-down"   : true,
                        "menu-queue-bottom" : true
                    };
                    break;
            }
        }

        for (var i = 0, j = menuBtnName.length; i < j; i++) {
            var menuBtnCtrl = me.menu.queryById(me.getId() + "-" +
                menuBtnName[i]);

            if (!Ext.isEmpty(menuBtnCtrl)) {
                if (menuBtnDisabled[menuBtnName[i]] === false) {
                    menuBtnCtrl.disable();
                } else {
                    menuBtnCtrl.enable();
                }
            }
        }
    },

    /* Upload handlers */
    onUploadButton : function() {
        var me = this;

        Ext.create("OMV.window.Upload", {
            title     : _("Upload torrent"),
            service   : "TransmissionBT",
            method    : "upload",
            listeners : {
                success : function () {
                    me.doReload();
                },
                scope : me
            }
        }).show();
    },

    /* AddUrl handlers */
    onAddUrlButton : function() {
        var me = this;

        Ext.create("OMV.module.admin.service.transmissionbt.manage.dialog.AddUrl", {
            listeners : {
                scope  : me,
                submit : function() {
                    me.doReload();
                }
            }
        }).show();
    },

    /* Delete handlers */
    onDeleteButton : function() {
        var me = this;
        var records = me.getSelection();

        Ext.create("OMV.module.admin.service.transmissionbt.manage.dialog.Delete", {
            listeners : {
                scope  : me,
                submit : function(id, values) {
                    me.startDeletion(records, values.delete_local_data);
                }
            }
        }).show();
    },

    startDeletion : function(records, delete_local_data) {
        var me = this;

        if (records.length <= 0)
            return;

        me.deleteActionInfo = {
            records           : records,
            count             : records.length,
            delete_local_data : delete_local_data
        };

        var record = records.pop();

        // Display progress dialog
        OMV.MessageBox.progress("", me.deletionWaitMsg, "");
        me.updateProgress(
            me.deleteActionInfo.count,
            me.deleteActionInfo.records.length);

        me.doDeletion(record, delete_local_data);
    },

    doDeletion : function(record, delete_local_data) {
        var me = this;

        OMV.Rpc.request({
            scope    : me,
            callback : me.onDeletion,
            rpcData  : {
                service : "TransmissionBT",
                method  : "delete",
                params  : {
                    id              : record.get("id"),
                    deleteLocalData : delete_local_data
                }
            }
        });
    },

    onDeletion : function(id, success, response) {
        var me = this;

        if (success) {
            if (me.deleteActionInfo.records.length > 0) {
                var record = me.deleteActionInfo.records.pop();

                // Update progress dialog
                me.updateProgress(
                    me.deleteActionInfo.count,
                    me.deleteActionInfo.records.length);

                // Execute deletion function
                me.doDeletion(record, me.deleteActionInfo.delete_local_data);
            } else {
                // Remove temporary local variables
                delete me.deleteActionInfo;

                // Update and hide progress dialog
                OMV.MessageBox.updateProgress(1, "100% completed ...");
                OMV.MessageBox.hide();
                me.doReload();
            }
        } else {
            // Remove temporary local variables
            delete me.deleteActionInfo;

            // Hide progress dialog
            OMV.MessageBox.hide();

            // Display error message
            OMV.MessageBox.error(null, response);
        }
    },

    /* Pause handlers */
    onPauseButton : function() {
        var me = this;
        var records = me.getSelection();

        me.startPause(records);
    },

    startPause : function (records) {
        var me = this;

        if (records.length <= 0)
            return;

        // Store selected records in a local variable
        me.pauseActionInfo = {
            records : records,
            count   : records.length
        };

        var record = records.pop();

        // Display progress dialog
        OMV.MessageBox.progress("", me.pauseWaitMsg, "");
        me.updateProgress(
            me.pauseActionInfo.count,
            me.pauseActionInfo.records.length);

        me.doPause(record);
    },

    doPause : function (record) {
        var me = this;

        OMV.Rpc.request({
            scope    : me,
            callback : me.onPause,
            rpcData  : {
                service : "TransmissionBT",
                method  : "pause",
                params  : {
                    id : record.get("id")
                }
            }
        });
    },

    onPause : function (id, success, response) {
        var me = this;

        if (success) {
            if (me.pauseActionInfo.records.length > 0) {
                var record = me.pauseActionInfo.records.pop();

                // Update progress dialog
                me.updateProgress(
                    me.pauseActionInfo.count,
                    me.pauseActionInfo.records.length);

                // Execute pause function
                me.doPause(record);
            } else {
                // Remove temporary local variables
                delete me.pauseActionInfo;

                // Update and hide progress dialog
                OMV.MessageBox.updateProgress(1, "100% completed ...");
                OMV.MessageBox.hide();
                me.doReload();
            }
        } else {
            // Remove temporary local variables
            delete me.pauseActionInfo;

            // Hide progress dialog
            OMV.MessageBox.hide();

            // Display error message
            OMV.MessageBox.error(null, response);
        }
    },

    /* Resume handlers */
    onResumeButton : function() {
        var me = this;
        var records = me.getSelection();

        me.startResume(records);
    },

    startResume : function (records) {
        var me = this;

        if (records.length <= 0)
            return;

        // Store selected records in a local variable
        me.resumeActionInfo = {
            records : records,
            count   : records.length
        };

        var record = records.pop();

        // Display progress dialog
        OMV.MessageBox.progress("", me.resumeWaitMsg, "");
        me.updateProgress(
            me.resumeActionInfo.count,
            me.resumeActionInfo.records.length);

        me.doResume(record);
    },

    doResume : function (record) {
        var me = this;

        OMV.Rpc.request({
            scope    : me,
            callback : me.onResume,
            rpcData  : {
                service : "TransmissionBT",
                method  : "resume",
                params  : {
                    id : record.get("id")
                }
            }
        });
    },

    onResume : function (id, success, response) {
        var me = this;

        if (success) {
            if (me.resumeActionInfo.records.length > 0) {
                var record = me.resumeActionInfo.records.pop();

                // Update progress dialog
                me.updateProgress(
                    me.resumeActionInfo.count,
                    me.resumeActionInfo.records.length);

                // Execute resume function
                me.doResume(record);
            } else {
                // Remove temporary local variables
                delete me.resumeActionInfo;

                // Update and hide progress dialog
                OMV.MessageBox.updateProgress(1, "100% completed ...");
                OMV.MessageBox.hide();

                me.doReload();
            }
        } else {
            // Remove temporary local variables
            delete me.resumeActionInfo;

            // Hide progress dialog
            OMV.MessageBox.hide();

            // Display error message
            OMV.MessageBox.error(null, response);
        }
    },

    /* Queue move handlers */
    onQueueMoveMenu : function(item, event) {
        var me = this;
        var records = me.getSelection();

        me.startQueueMove(records, item.action);
    },

    startQueueMove : function(records, action) {
        var me = this;

        if (records.length <= 0)
            return;

        // Store selected records in a local variable
        me.queueMoveActionInfo = {
            records : records,
            count   : records.length,
            action  : action
        };

        // Get first record to be moved
        var record = records.pop();

        // Display progress dialog
        OMV.MessageBox.progress("", me.queueMoveWaitMsg, "");
        me.updateProgress(
            me.queueMoveActionInfo.count,
            me.queueMoveActionInfo.records.length);

        // Execute move function
        me.doQueueMove(record, action);
    },

    doQueueMove : function(record, action) {
        var me = this;

        OMV.Rpc.request({
            scope    : me,
            callback : me.onQueueMove,
            rpcData  : {
                service : "TransmissionBT",
                method  : "queueMove",
                params  : {
                    id     : record.get("id"),
                    action : action
                }
            }
        });
    },

    onQueueMove : function (id, success, response) {
        var me = this;

        if (success) {
            if (me.queueMoveActionInfo.records.length > 0) {
                var record = me.queueMoveActionInfo.records.pop();
                var action = me.queueMoveActionInfo.action;

                // Update progress dialog
                me.updateProgress(
                    me.queueMoveActionInfo.count,
                    me.queueMoveActionInfo.records.length);

                // Execute move function
                me.doQueueMove(record, action);
            } else {
                // Remove temporary local variables
                delete me.queueMoveActionInfo;

                // Update and hide progress dialog
                OMV.MessageBox.updateProgress(1, "100% completed ...");
                OMV.MessageBox.hide();
                me.doReload();
            }
        } else {
            // Remove temporary local variables
            delete me.queueMoveActionInfo;

            // Hide progress dialog
            OMV.MessageBox.hide();

            // Display error message
            OMV.MessageBox.error(null, response);
        }
    },

    /* Update progress handler */
    updateProgress : function(totalCount, doneCount) {
        var me = this;

        // Calculate percentage
        var p = (totalCount - doneCount) / totalCount;

        // Create message text
        var text = Math.round(100 * p) + "% completed ...";

        // Update progress dialog
        OMV.MessageBox.updateProgress(p, text);
    }

});
