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
// require("js/omv/module/admin/service/transmissionbt/manage/TorrentList.js")

OMV.WorkspaceManager.registerPanel({
    id        : "manage",
    path      : "/service/transmissionbt",
    text      : _("Jobs"),
    position  : 10,
    className : "OMV.module.admin.service.transmissionbt.manage.TorrentList"
});
