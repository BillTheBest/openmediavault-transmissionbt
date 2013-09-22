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
// require("js/omv/workspace/form/Panel.js")

Ext.define("OMV.module.admin.service.transmissionbt.configure.Settings", {
    extend       : "OMV.workspace.form.Panel",
    
    rpcService   :"TransmissionBT",
    rpcGetMethod :"getSettings",
    rpcSetMethod :"setSettings",
    
    plugins: [{
        ptype          : "linkedfields",
        correlations   : [{
            name: [
                "blocklistsyncfrequency",
                "blocklisturl"
            ],
            conditions : [{
                name  : "blocklistsyncenabled",
                value : true
            }],
            properties : [
                "!readOnly",
                "!allowBlank"
            ]
        },{
            name       : [
                "rpcusername",
                "rpcpassword"
            ],
            conditions : [{
                name  : "rpcauthenticationrequired",
                value : true
            }],
            properties : [
                "!readOnly",
                "!allowBlank"
            ]
        }]
    }],
    
    getFormItems : function() {
        return [{
            xtype    : "fieldset",
            title    : _("General settings"),
            defaults : {
                labelSeparator : ""
            },
            items   : [{
                xtype      : "checkbox",
                name       : "enable",
                fieldLabel : _("Enable"),
                checked    : false
            },{
                xtype      : "checkbox",
                name       : "pexenabled",
                fieldLabel : _("Peer exchange (PEX)"),
                checked    : true,
                boxLabel   : _("Enable PEX.")
            },{
                xtype      : "checkbox",
                name       : "dhtenabled",
                fieldLabel : _("Distributed hash table (DHT)."),
                checked    : true,
                boxLabel   : _("Enable DHT.")
            },{
                xtype      : "checkbox",
                name       : "lpd-enabled",
                fieldLabel : _("Local Peer Discovery (LPD)."),
                checked    : false,
                boxLabel   : _("Enable LPD.")
            },{
                xtype      : "checkbox",
                name       : "utp-enabled",
                fieldLabel : _("Micro Transport Protocol (&micro;TP)."),
                checked    : true,
                boxLabel   : _("Enable &micro;TP.")
            },{
                xtype      : "combo",
                name       : "encryption",
                fieldLabel : _("Encryption"),
                queryMode  : "local",
                store      : Ext.create("Ext.data.ArrayStore", {
                    fields : [ 
                        "value", 
                        "text" 
                    ],
                    data   : [
                        [ 0, _("Off") ],
                        [ 1, _("Preferred") ],
                        [ 2, _("Forced") ]
                    ]
                }),
                displayField  : "text",
                valueField    : "value",
                allowBlank    : false,
                editable      : false,
                triggerAction : "all",
                value         : 1,
                plugins       : [{
                    ptype : "fieldinfo",
                    text  : _("The peer connection encryption mode.")
                }]
            },{
                xtype      : "combo",
                name       : "message-level",
                fieldLabel : _("Message Level"),
                queryMode  : "local",
                store      : Ext.create("Ext.data.ArrayStore", {
                    fields : [ 
                        "value", 
                        "text" 
                    ],
                    data   : [
                        [ 0, _("None") ],
                        [ 1, _("Error") ],
                        [ 2, _("Info") ],
                        [ 3, _("Debug") ]
                    ]
                }),
                displayField  : "text",
                valueField    : "value",
                allowBlank    : false,
                editable      : false,
                triggerAction : "all",
                value         : 2,
                plugins       : [{
                    ptype : "fieldinfo",
                    text  : _("Set verbosity of transmission messages.")
                }]
            },{
                xtype      : "checkbox",
                name       : "lazy-bitfield-enabled",
                fieldLabel : _("Lazy Bitfield"),
                checked    : true,
                boxLabel   : _("May help get around some ISP filtering.")
            },{
                xtype      : "checkbox",
                name       : "scrape-paused-torrents-enabled",
                fieldLabel : _("Scrape paused torrents."),
                checked    : true,
                boxLabel   : _("Enable paused torrent scraping.")
            },{
                xtype         : "numberfield",
                name          : "umask",
                fieldLabel    : _("Umask"),
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 18,
                plugins       : [{
                    ptype : "fieldinfo",
                    text  : _("Sets transmission's file mode creation mask.")
                }]
            },{
                xtype         : "numberfield",
                name          : "cache-size-mb",
                fieldLabel    : _("Cache Size"),
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 4,
                plugins       : [{
                    ptype : "fieldinfo",
                    text  : _("Cache size (in Mb) to reduce the number of disk reads and writes.")
                }]
            }]
        },{
            xtype    : "fieldset",
            title    : _("RPC/WebUI Settings"),
            defaults : {
                labelSeparator : ""
            },
            items   : [{
                xtype         : "numberfield",
                name          : "rpcport",
                fieldLabel    : _("Port"),
                vtype         : "port",
                minValue      : 1024,
                maxValue      : 65535,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 9091,
                plugins       : [{
                    ptype : "fieldinfo",
                    text  : _("Port to open and listen for RPC/Web requests on.")
                }]
            },{
                xtype      : "textfield",
                name       : "rpcurl",
                fieldLabel : _("Uri"),
                vtype      : "transmissionbturi",
                allowBlank : false,
                value      : 'transmission',
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Url to access RPC (http://localhost/&lt;URI&gt;/(rpc|web).")
                }]
            },{
                xtype      : "checkbox",
                name       : "rpcauthenticationrequired",
                fieldLabel : _("Authentication"),
                checked    : true,
                boxLabel   : _("Require clients to authenticate themselves.")
            },{
                xtype      : "textfield",
                name       : "rpcusername",
                fieldLabel : _("Username"),
                allowBlank : false,
                vtype      : "username",
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Used for client authentication.")
                }]
            },{
                xtype      : "passwordfield",
                name       : "rpcpassword",
                fieldLabel : _("Password"),
                allowBlank : false,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Used for client authentication.")
                }]
            }]
        },{
            xtype    : "fieldset",
            title    : _("Blocklists"),
            defaults : {
                labelSeparator : ""
            },
            items   : [{
                xtype      : "checkbox",
                name       : "blocklistenabled",
                fieldLabel : _("Enable"),
                checked    : false,
                boxLabel   : _("Use blocklists.")
            },{
                xtype      : "checkbox",
                name       : "blocklistsyncenabled",
                fieldLabel : _("Auto sync"),
                checked    : false,
                boxLabel   : _("Update blocklists automatically.")
            },{
                xtype      : "combo",
                name       : "blocklistsyncfrequency",
                fieldLabel : _("Sync frequency"),
                queryMode  : "local",
                store      : Ext.create("Ext.data.SimpleStore", {
                    fields : [ 
                        "value", 
                        "text" 
                    ],
                    data   : [
                        [ "hourly", _("Hourly") ],
                        [ "daily", _("Daily") ],
                        [ "weekly", _("Weekly") ],
                        [ "monthly", _("Monthly") ]
                    ]
                }),
                displayField  : "text",
                valueField    : "value",
                allowBlank    : false,
                editable      : false,
                triggerAction : "all",
                value         : "daily"
            },{
                xtype      : "textfield",
                name       : "blocklisturl",
                fieldLabel : _("URL"),
                allowBlank : true,
                width      : 300,
                value      : "http://update.transmissionbt.com/level1.gz",
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("The URL of the blocklist.")
                }]
            }]
        },{
            xtype    : "fieldset",
            title    : _("Script to Proccess After Torrent Finishes"),
            defaults : {
                labelSeparator : ""
            },
            items   : [{
                xtype      : "checkbox",
                name       : "script-torrent-done-enabled",
                fieldLabel : _("Enable"),
                checked    : false,
                boxLabel   : _("Run a script at torrent completion.")
            },{
                xtype      : "textfield",
                name       : "script-torrent-done-filename",
                fieldLabel : _("Script"),
                allowBlank : true,
                value      : "",
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Enter path to script.")
                }]
            }]
        }];
    }
});

Ext.apply(Ext.form.VTypes, {

    transmissionbturi : function(v) {
        return (/^[a-z0-9]+$/i).test(v);
    },
    transmissionbturiText : _("Invalid Uri."),
    transmissionbturiMask : /[a-z0-9\-_]/i

});
