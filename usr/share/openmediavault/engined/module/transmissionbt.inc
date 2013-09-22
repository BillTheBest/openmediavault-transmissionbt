<?php

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

require_once ("openmediavault/module.inc");
require_once ("openmediavault/config.inc");
require_once ("openmediavault/error.inc");
require_once ("openmediavault/util.inc");
require_once ("openmediavault/initscript.inc");

class OMVModuleTransmissionBT extends OMVModuleServiceAbstract implements OMVINotifyListener, OMVIModuleServiceStatus
{

    /**
     * Get the module name.
     */
    public function getName()
    {
        return "transmissionbt";
    }

    /**
    * Generate the configuration.
    * 
    * @throws E_EXEC_FAILED
    */ 
    public function applyConfig()
    {
        // Currently transmission overwrites the settings.json configuration
        // file on exit. This means transmission must be terminated before
        // writing the new settings.json file.
        $initd = new OMVSysVInitScript("transmission-daemon", false);
        $initd->stop();
        
        $cmd = "omv-mkconf transmissionbt 2>&1";
        
        if (0 !== $this->exec($cmd, $output))
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
                $cmd, implode("\n", $output));
    }

    /**
    * Stop the managed service.
    *
    * @throws E_CONFIG_GET_OBJECT_FAILED
    */ 
    public function stopService()
    {
        global $xmlConfig;
        
        // Get the configuration object
        $xpath = "//services/transmissionbt";
        $object = $xmlConfig->get($xpath);
        
        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, 
                $xpath);
        
        if (boolval($object['enable']) === false) {
            $initd = new OMVSysVInitScript("transmission-daemon", boolval($object['enable']));
            $initd->stop();
        }
    }
    
    /**
    * Start the managed service.
    * 
    * @throws E_CONFIG_GET_OBJECT_FAILED
    */ 
    public function startService()
    {
        global $xmlConfig;
        
        // Get the configuration object
        $xpath = "//services/transmissionbt";
        $object = $xmlConfig->get($xpath);
        
        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, 
                $xpath);
        
        if (boolval($object['enable']) === true) {
            $initd = new OMVSysVInitScript("transmission-daemon", true);
            $initd->exec();
        }
    }
    
    /**
    * Get the status of the managed service.
    * 
    * @throws E_CONFIG_GET_OBJECT_FAILED
    */ 
    public function getStatus()
    {
        global $xmlConfig;
        
        // Get the configuration object
        $xpath = "//services/transmissionbt";
        $object = $xmlConfig->get($xpath);
        
        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, 
                $xpath);
        
        // Check if process is running. Init script does not support 'status'.
        $cmd = "ps --no-heading -C transmission-daemon";
        $this->exec($cmd, $output);
        
        return array(
            "name" => $this->getName(), 
            "title" => gettext("BitTorrent"), 
            "enabled" => boolval($object['enable']), 
            "running" => !empty($output[0])
        );
    }

    /**
     * Bind listeners.
     */
    function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.transmissionbt",
            array($this, "setDirty"));

        $moduleMgr = &OMVModuleMgr::getInstance();
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.transmissionbt",
            array($moduleMgr->getModule("zeroconf"), "setDirty")); 
    }

}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleTransmissionBT());
