/**
 * Copyright (C) 2013 OpenMediaVault Plugin Developers
 * Copyright (c) 2012 Marcel Beck
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
// require("js/omv/workspace/window/Form.js") 

Ext.define("OMV.module.admin.service.transmissionbt.manage.dialog.AddUrl", {
    extend : "OMV.workspace.window.Form",
    
    rpcService: "TransmissionBT",
    rpcSetMethod    : "addUrl",
    hideResetButton : true,
    width           : 500,
    title           : _("Add Torrent by URL"),
    okButtonText    : _("OK"),
    submitMsg       : _("Adding Torrent ..."),

	getFormItems : function () {
		return [{
					xtype      : "textfield",
					name       : "url",
					id         : "url",
					fieldLabel : _("URL"),
					allowBlank : false
				},
				{
					xtype      : "checkbox",
					name       : "start_download",
					id         : "start_download",
					fieldLabel : _("Start download"),
					checked    : true
		}];
	},
	
	onOkButton : function() {
        var me = this;

        if (!me.isValid()) {

        } else {
            me.doSubmit();
        }
    }
});
