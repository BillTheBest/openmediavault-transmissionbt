<?php

/**
 * Copyright (C) 2009-2012 Volker Theile <volker.theile@openmediavault.org>
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

require_once ("openmediavault/object.inc");
require_once ("openmediavault/error.inc");
require_once ("openmediavault/util.inc");
require_once ("openmediavault/rpc.inc");
require_once ("openmediavault/notify.inc");
require_once ("transmission/rpc.inc");

class OMVRpcServiceTransmissionBT extends OMVRpcServiceAbstract
{
    /**
     * Get the RPC service name
     */
    public function getName()
    {
        return "TransmissionBT";
    }

    /**
     * Initialize the RPC service
     */
    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("getFilesAndLocations");
        $this->registerMethod("setFilesAndLocations");
        $this->registerMethod("getBandwidth");
        $this->registerMethod("setBandwidth");
        $this->registerMethod("getQueuing");
        $this->registerMethod("setQueuing");
        $this->registerMethod("getPeer");
        $this->registerMethod("setPeer");
        $this->registerMethod("getScheduling");
        $this->registerMethod("setScheduling");
        $this->registerMethod("getStatus");
        $this->registerMethod("getList");
        $this->registerMethod("upload");
        $this->registerMethod("delete");
        $this->registerMethod("pause");
        $this->registerMethod("resume");
        $this->registerMethod("queueMove");
        $this->registerMethod("addUrl");
    }

    /**
     * Get general settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $xpath = "//services/transmissionbt";
        $object = $xmlConfig->get($xpath);
        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xpath);

        // Modify result object
        $object['enable']                           = boolval($object['enable']);
        $object['pexenabled']                       = boolval($object['pexenabled']);
        $object['dhtenabled']                       = boolval($object['dhtenabled']);
        $object['encryption']                       = intval($object['encryption']);
        $object['rpcport']                          = intval($object['rpcport']);
        $object['rpcurl']                           = trim($object['rpcurl'], '/');
        $object['rpcauthenticationrequired']        = boolval($object['rpcauthenticationrequired']);
        $object['blocklistenabled']                 = boolval($object['blocklistenabled']);
        $object['blocklistsyncenabled']             = boolval($object['blocklistsyncenabled']);
        $object['lazy-bitfield-enabled']            = boolval($object['lazy-bitfield-enabled']);
        $object['umask']                            = intval($object['umask']);
        $object['lpd-enabled']                      = boolval($object['lpd-enabled']);
        $object['scrape-paused-torrents-enabled']   = boolval($object['scrape-paused-torrents-enabled']);
        $object['utp-enabled']                      = boolval($object['utp-enabled']);
        $object['message-level']                    = intval($object['message-level']);
        $object['cache-size-mb']                    = intval($object['cache-size-mb']);
        $object['script-torrent-done-enabled']      = boolval($object['script-torrent-done-enabled']);
        unset($object['modules']);

        return $object;
    }

    /**
     * Set general settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     * @throws E_CONFIG_SET_OBJECT_FAILED
     * @throws E_CONFIG_SAVE_FAILED
     */
    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "enable":{"type":"boolean"},
                "encryption":{"type":"integer","enum":[0,1,2]},
                "message-level":{"type":"integer","enum":[0,1,2,3]},
                "lazy-bitfield-enabled":{"type":"boolean"},
                "scrape-paused-torrents-enabled":{"type":"boolean"},
                "umask":{"type":"integer"},
                "cache-size-mb":{"type":"integer"},
                "rpcport":{"type":"integer","minimum":1024,"maximum":65535},
                "rpcurl":{"type":"string","format":"regex","pattern":"/^[a-z0-9]+$/i"},
                "rpcauthenticationrequired":{"type":"boolean"},
                "rpcusername":{"type":"string"},
                "rpcpassword":{"type":"string"},
                "blocklistsyncfrequency":{"type":"string","enum":["hourly","daily","weekly","monthly"]},
                "blocklisturl":{"type":"string"},
                "script-torrent-done-filename":{"type":"string"},
                "pexenabled":{"type":"boolean"},
                "dhtenabled":{"type":"boolean"},
                "lpd-enabled":{"type":"boolean"},
                "utp-enabled":{"type":"boolean"},
                "blocklistenabled":{"type":"boolean"},
                "blocklistsyncenabled":{"type":"boolean"},
                "script-torrent-done-enabled":{"type":"boolean"}
            }
        }');

        // Get existing configuration object
        $xpath = "//services/transmissionbt";
        $oldObject = $xmlConfig->get($xpath);

        if (is_null($oldObject))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // If no directory for downloads is set
        // and the user is trying to enable the
        // service, abort the save.
        $downloadFolderRef = $oldObject['modules']
                                       ['filesandlocations']
                                       ['paths']
                                       ['download']
                                       ['sharedfolderref'];
        $msg = 'No directory for downloads set, please set one before enabling the service';

        if ($params['enable'] === true && empty($downloadFolderRef))
            throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED, $msg);

        // Prepare configuration data
        $object = array(
            "enable" => array_boolval($params, 'enable'),
            "pexenabled" => array_boolval($params, 'pexenabled'),
            "dhtenabled" => array_boolval($params, 'dhtenabled'),
            "encryption" => $params['encryption'],
            "rpcurl" => '/' . $params['rpcurl'] . '/',
            "rpcport" => $params['rpcport'],
            "rpcauthenticationrequired" => array_boolval($params, 'rpcauthenticationrequired'),
            "rpcusername" => $params['rpcusername'],
            "rpcpassword" => $params['rpcpassword'],
            "blocklistenabled" => array_boolval($params, 'blocklistenabled'),
            "blocklistsyncenabled" => array_boolval($params, 'blocklistsyncenabled'),
            "blocklistsyncfrequency" => $params['blocklistsyncfrequency'],
            "blocklisturl" => $params['blocklisturl'],
            "modules" => isset($oldObject['modules']) ? $oldObject['modules'] : array(),
            "lazy-bitfield-enabled" => array_boolval($params, 'lazy-bitfield-enabled'),
            "umask" => $params['umask'],
            "lpd-enabled" => array_boolval($params, 'lpd-enabled'),
            "scrape-paused-torrents-enabled" => array_boolval($params, 'scrape-paused-torrents-enabled'),
            "utp-enabled" => array_boolval($params, 'utp-enabled'),
            "message-level" => $params['message-level'],
            "cache-size-mb" => $params['cache-size-mb'],
            "script-torrent-done-enabled" => array_boolval($params, 'script-torrent-done-enabled'),
            "script-torrent-done-filename" => $params['script-torrent-done-filename'],
        );

        // Set configuration object
        if ($xmlConfig->replace($xpath, $object) === false)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.services.transmissionbt", $object);

        return $object;
    }

    /**
     * Get locations and files settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getFilesAndLocations($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $xpath = "//services/transmissionbt/modules/filesandlocations";
        $object = $xmlConfig->get($xpath);

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify result data
        // Locations
        $object['download-sharedfolderref'] = $object['paths']['download']['sharedfolderref'];
        $object['download-dir'] = $object['paths']['download']['dir'];

        $object['incomplete-dir-enabled'] = boolval($object['paths']['incomplete']['enabled']);
        $object['incomplete-sharedfolderref'] = $object['paths']['incomplete']['sharedfolderref'];
        $object['incomplete-dir'] = $object['paths']['incomplete']['dir'];

        $object['watch-dir-enabled'] = boolval($object['paths']['watch']['enabled']);
        $object['watch-sharedfolderref'] = $object['paths']['watch']['sharedfolderref'];
        $object['watch-dir'] = $object['paths']['watch']['dir'];

        // Files
        $object['preallocation'] = intval($object['preallocation']);
        $object['rename-partial-files'] = boolval($object['rename-partial-files']);
        $object['start-added-torrents'] = boolval($object['start-added-torrents']);
        $object['trash-original-torrent-files'] = boolval($object['trash-original-torrent-files']);

        return $object;
    }

    /**
     * Set locations and files settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setFilesAndLocations($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "download-sharedfolderref":{' . $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'] . '},
                "download-dir":{"type":"string"},
                "incomplete-dir-enabled":{"type":"boolean"},
                "incomplete-sharedfolderref":{' . $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'] . '},
                "incomplete-dir":{"type":"string"},
                "watch-dir-enabled":{"type":"boolean"},
                "watch-sharedfolderref":{' . $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'] . '},
                "watch-dir":{"type":"string"},
                "preallocation":{"type":"integer","enum":[0,1,2]},
                "rename-partial-files":{"type":"boolean"},
                "start-added-torrents":{"type":"boolean"},
                "trash-original-torrent-files":{"type":"boolean"}
            }
        }');

        // Prepare configuration data
        $object = array(
            // Locations
            'paths' => array(
                'download' => array(
                    'sharedfolderref' => $params['download-sharedfolderref'],
                    'dir' => $params['download-dir'],
                ),
                'incomplete' => array(
                    'enabled' => array_boolval($params, 'incomplete-dir-enabled'),
                    'sharedfolderref' => $params['incomplete-sharedfolderref'],
                    'dir' => $params['incomplete-dir'],
                ),
                'watch' => array(
                    'enabled' => array_boolval($params, 'watch-dir-enabled'),
                    'sharedfolderref' => $params['watch-sharedfolderref'],
                    'dir' => $params['watch-dir'],
                ),
            ),

            // Files
            "preallocation" => $params['preallocation'],
            "rename-partial-files" => array_boolval($params, 'rename-partial-files'),
            "start-added-torrents" => array_boolval($params, 'start-added-torrents'),
            "trash-original-torrent-files" => array_boolval($params, 'trash-original-torrent-files'),
        );

        // Set configuration object
        $xpath = "//services/transmissionbt/modules/filesandlocations";
        if ($xmlConfig->replace($xpath, $object) === false)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.services.transmissionbt", $object);

        return $object;
    }

    /**
     * Get bandwidth settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getBandwidth($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $xpath = "//services/transmissionbt/modules/bandwidth";
        $object = $xmlConfig->get($xpath);
        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify result data
        // Speed
        $object['speed-limit-down'] = intval($object['speed-limit-down']);
        $object['speed-limit-down-enabled'] = boolval($object['speed-limit-down-enabled']);
        $object['speed-limit-up'] = intval($object['speed-limit-up']);
        $object['speed-limit-up-enabled'] = boolval($object['speed-limit-up-enabled']);
        $object['upload-slots-per-torrent'] = intval($object['upload-slots-per-torrent']);

        // Turtle Mode
        $object['alt-speed-enabled'] = boolval($object['alt-speed-enabled']);
        $object['alt-speed-up'] = intval($object['alt-speed-up']);
        $object['alt-speed-down'] = intval($object['alt-speed-down']);

        return $object;
    }

    /**
     * Set bandwidth settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setBandwidth($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "speed-limit-down":{"type":"integer","minimum":0},
                "speed-limit-down-enabled":{"type":"boolean"},
                "speed-limit-up":{"type":"integer","minimum":0},
                "speed-limit-up-enabled":{"type":"boolean"},
                "upload-slots-per-torrent":{"type":"integer"},
                "alt-speed-enabled":{"type":"boolean"},
                "alt-speed-up":{"type":"integer","minimum":0},
                "alt-speed-down":{"type":"integer","minimum":0}
            }
        }');

        // Prepare configuration data
        $object = array(
            // Speed
            "speed-limit-down" => $params['speed-limit-down'],
            "speed-limit-down-enabled" => array_boolval($params, 'speed-limit-down-enabled'),
            "speed-limit-up" => $params['speed-limit-up'],
            "speed-limit-up-enabled" => array_boolval($params, 'speed-limit-up-enabled'),
            "upload-slots-per-torrent" => $params['upload-slots-per-torrent'],
            // Turtle Mode
            "alt-speed-enabled" => array_boolval($params, 'alt-speed-enabled'),
            "alt-speed-up" => $params['alt-speed-up'],
            "alt-speed-down" => $params['alt-speed-down'],
        );

        // Set configuration object
        $xpath = "//services/transmissionbt/modules/bandwidth";

        if ($xmlConfig->replace($xpath, $object) === false)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.services.transmissionbt", $object);

        return $object;
    }

    /**
     * Get queuing settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getQueuing($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $xpath = "//services/transmissionbt/modules/queuing";
        $object = $xmlConfig->get($xpath);

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify result data
        // General
        $object['queue-stalled-enabled'] = boolval($object['queue-stalled-enabled']);
        $object['queue-stalled-minutes'] = intval($object['queue-stalled-minutes']);

        // Download
        $object['download-queue-enabled'] = boolval($object['download-queue-enabled']);
        $object['download-queue-size'] = intval($object['download-queue-size']);

        // Seed
        $object['seed-queue-enabled'] = boolval($object['seed-queue-enabled']);
        $object['seed-queue-size'] = intval($object['seed-queue-size']);

        return $object;
    }

    /**
     * Set queueing settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setQueuing($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "queue-stalled-enabled":{"type":"boolean"},
                "queue-stalled-minutes":{"type":"integer","minimum":0},
                "download-queue-enabled":{"type":"boolean"},
                "download-queue-size":{"type":"integer","minimum":0},
                "seed-queue-enabled":{"type":"boolean"},
                "seed-queue-size":{"type":"integer","minimum":0}
            }
        }');

        // Prepare configuration data
        $object = array(
            // General
            "queue-stalled-enabled" => array_boolval($params, 'queue-stalled-enabled'),
            "queue-stalled-minutes" => $params['queue-stalled-minutes'],

            // Download
            "download-queue-enabled" => array_boolval($params, 'download-queue-enabled'),
            "download-queue-size" => $params['download-queue-size'],

            // Seed
            "seed-queue-enabled" => array_boolval($params, 'seed-queue-enabled'),
            "seed-queue-size" => $params['seed-queue-size'],
        );

        // Set configuration object
        $xpath = "//services/transmissionbt/modules/queuing";
        if ($xmlConfig->replace($xpath, $object) === false)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.services.transmissionbt", $object);

        return $object;
    }

    /**
     * Get peer settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getPeer($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $xpath = "//services/transmissionbt/modules/peer";
        $object = $xmlConfig->get($xpath);

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify result data
        // Limits
        $object['peer-limit-global'] = intval($object['peer-limit-global']);
        $object['peer-limit-per-torrent'] = intval($object['peer-limit-per-torrent']);

        // Peer ports
        $object['peer-port'] = intval($object['peer-port']);
        $object['peer-port-random-high'] = intval($object['peer-port-random-high']);
        $object['peer-port-random-low'] = intval($object['peer-port-random-low']);
        $object['peer-port-random-on-start'] = boolval($object['peer-port-random-on-start']);
        $object['port-forwarding-enabled'] = boolval($object['port-forwarding-enabled']);

        return $object;
    }

    /**
     * Set peer settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setPeer($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "bind-address-ipv4":{"type":"string","format":"regex","pattern":"/^[0-9]{1,3}\\\.[0-9]{1,3}\\\.[0-9]{1,3}\\\.[0-9]{1,3}/"},
                "bind-address-ipv6":{"type":"string"},
                "peer-limit-global":{"type":"integer","minimum":0},
                "peer-limit-per-torrent":{"type":"integer","minimum":0},
                "peer-socket-tos":{"type":"string","enum":' . '["default","lowcost","throughput","lowdelay","reliability"]},
                "peer-port":{"type":"integer","minimum":1024,"maximum":65535},
                "peer-port-random-low":{"type":"integer","minimum":1024,"maximum":65535},
                "peer-port-random-high":{"type":"integer","minimum":1024,"maximum":65535},
                "peer-port-random-on-start":{"type":"boolean"},
                "port-forwarding-enabled":{"type":"boolean"}
            }
        }');

        // Prepare configuration data
        $object = array(
            // Bindings
            "bind-address-ipv4" => $params['bind-address-ipv4'],
            "bind-address-ipv6" => $params['bind-address-ipv6'],

            // Limits
            "peer-limit-global" => $params['peer-limit-global'],
            "peer-limit-per-torrent" => $params['peer-limit-per-torrent'],
            "peer-socket-tos" => $params['peer-socket-tos'],

            // Peer ports
            "peer-port" => $params['peer-port'],
            "peer-port-random-high" => $params['peer-port-random-high'],
            "peer-port-random-low" => $params['peer-port-random-low'],
            "peer-port-random-on-start" => array_boolval($params, 'peer-port-random-on-start'),
            "port-forwarding-enabled" => array_boolval($params, 'port-forwarding-enabled'),
        );

        // Set configuration object
        $xpath = "//services/transmissionbt/modules/peer";
        if ($xmlConfig->replace($xpath, $object) === false)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.services.transmissionbt", $object);

        return $object;
    }

    /**
     * Get scheduling settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getScheduling($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $xpath = "//services/transmissionbt/modules/scheduling";
        $object = $xmlConfig->get($xpath);

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify result data
        // Scheduling
        $object['alt-speed-time-enabled'] = boolval($object['alt-speed-time-enabled']);
        $object['alt-speed-time-begin'] = intval($object['alt-speed-time-begin']);
        $object['begin-hour'] = intval($object['alt-speed-time-begin'] / 60);
        $object['begin-minute'] = ($object['alt-speed-time-begin'] % 60);
        $object['alt-speed-time-end'] = intval($object['alt-speed-time-end']);
        $object['end-hour'] = intval($object['alt-speed-time-end'] / 60);
        $object['end-minute'] = ($object['alt-speed-time-end'] % 60);

        // Days
        $object['days-sunday'] = false;
        if (($object['alt-speed-time-day'] & 1) === 1) {
            $object['days-sunday'] = true;
        }
        $object['days-monday'] = false;
        if (($object['alt-speed-time-day'] & 2) === 2) {
            $object['days-monday'] = true;
        }
        $object['days-tuesday'] = false;
        if (($object['alt-speed-time-day'] & 4) === 4) {
            $object['days-tuesday'] = true;
        }
        $object['days-wednesday'] = false;
        if (($object['alt-speed-time-day'] & 8) === 8) {
            $object['days-wednesday'] = true;
        }
        $object['days-thursday'] = false;
        if (($object['alt-speed-time-day'] & 16) === 16) {
            $object['days-thursday'] = true;
        }
        $object['days-friday'] = false;
        if (($object['alt-speed-time-day'] & 32) === 32) {
            $object['days-friday'] = true;
        }
        $object['days-saturday'] = false;
        if (($object['alt-speed-time-day'] & 64) === 64) {
            $object['days-saturday'] = true;
        }

        // Idle
        $object['idle-seeding-limit'] = intval($object['idle-seeding-limit']);
        $object['idle-seeding-limit-enabled'] = boolval($object['idle-seeding-limit-enabled']);

        // Ratio
        $object['ratio-limit'] = floatval($object['ratio-limit']);
        $object['ratio-limit-enabled'] = boolval($object['ratio-limit-enabled']);

        return $object;
    }

    /**
     * Set scheduling settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setScheduling($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "alt-speed-time-enabled":{"type":"boolean"},
                "begin-hour":{"type":"integer","minimum":0,"maximum":23},
                "begin-minute":{"type":"integer","minimum":0,"maximum":59},
                "end-hour":{"type":"integer","minimum":0,"maximum":23},
                "end-minute":{"type":"integer","minimum":0,"maximum":59},
                "days-sunday":{"type":"boolean"},
                "days-monday":{"type":"boolean"},
                "days-tuesday":{"type":"boolean"},
                "days-wednesday":{"type":"boolean"},
                "days-thursday":{"type":"boolean"},
                "days-friday":{"type":"boolean"},
                "days-saturday":{"type":"boolean"},
                "idle-seeding-limit":{"type":"integer","minimum":0},
                "idle-seeding-limit-enabled":{"type":"boolean"},
                "ratio-limit":{"type":"number","minimum":0},
                "ratio-limit-enabled":{"type":"boolean"}
            }
        }');

        $params['alt-speed-time-day'] = 0;
        if (array_boolval($params, 'days-sunday') === true) {
            $params['alt-speed-time-day'] += 1;
        }
        if (array_boolval($params, 'days-monday') === true) {
            $params['alt-speed-time-day'] += 2;
        }
        if (array_boolval($params, 'days-tuesday') === true) {
            $params['alt-speed-time-day'] += 4;
        }
        if (array_boolval($params, 'days-wednesday') === true) {
            $params['alt-speed-time-day'] += 8;
        }
        if (array_boolval($params, 'days-thursday') === true) {
            $params['alt-speed-time-day'] += 16;
        }
        if (array_boolval($params, 'days-friday') === true) {
            $params['alt-speed-time-day'] += 32;
        }
        if (array_boolval($params, 'days-saturday') === true) {
            $params['alt-speed-time-day'] += 64;
        }

        // Prepare configuration data
        $object = array(
            'alt-speed-time-enabled' => array_boolval($params, 'alt-speed-time-enabled'),
            'alt-speed-time-begin' => intval(($params['begin-hour'] * 60) + $params['begin-minute']),
            'alt-speed-time-end' => intval(($params['end-hour'] * 60) + $params['end-minute']),
            'alt-speed-time-day' => $params['alt-speed-time-day'],

            // Idle
            'idle-seeding-limit' => $params['idle-seeding-limit'],
            'idle-seeding-limit-enabled' => array_boolval($params, 'idle-seeding-limit-enabled'),

            // Ratio
            'ratio-limit' => $params['ratio-limit'],
            'ratio-limit-enabled' => array_boolval($params, 'ratio-limit-enabled'),
        );

        // Set configuration object
        $xpath = "//services/transmissionbt/modules/scheduling";
        if ($xmlConfig->replace($xpath, $object) === false)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.services.transmissionbt", $object);

        return $object;
    }

    /**
     * Get running status through the
     * TransmissionBT module
     */
    public function getStatus()
    {
        $module = new OMVModuleTransmissionBT();
        return array($module->getStatus());

        $this->getList();
    }

    /**
     * Get list of torrents.
     *
     * @param array $params An array containing the following fields:
     *   \em start The index where to start.
     *   \em limit The number of objects to process.
     *   \em sortfield The name of the column used to sort.
     *   \em sortdir The sort direction, ASC or DESC.
     *
     * @return array Returns an array containing the requested objects. The field \em total
     *   contains the total number of objects, \em data contains the object
     *   array. An exception will be thrown in case of an error.
     */
    public function getList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
        "type":"object",
            "properties":{
                "start":{"type":"integer"},
                "limit":{' . $GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'] . '},
                "sortfield":{' . $GLOBALS['OMV_JSONSCHEMA_SORTFIELD'] . '},
                "sortdir":{' . $GLOBALS['OMV_JSONSCHEMA_SORTDIR'] . '}
            }
        }');

        $transmissionbt_rpc = $this->factoryTransmission();
        $torrents = $transmissionbt_rpc->get(
            array(),
            array(
                'id',
                'name',
                'status',
                'totalSize',
                'haveValid',
                'percentDone',
                'eta',
                'peersConnected',
                'peersSendingToUs',
                'rateDownload',
                'rateUpload',
                'addedDate',
                'doneDate',
                'uploadRatio',
                'queuePosition'
            )
        );

        $torrents = $torrents['arguments']['torrents'];

        // Filter result
        return $this->applyFilter(
            $torrents,
            $params['start'],
            $params['limit'],
            $params['sortfield'],
            $params['sortdir']
        );
    }

    /**
     * Process uploaded torrent file
     */
    public function upload($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
            "filename":{"type":"string"},
            "filepath":{"type":"string"}
            }
        }');

        $transmissionbt_rpc = $this->factoryTransmission();
        $torrent = $transmissionbt_rpc->add_file($params['filepath']);

        return $torrent;
    }

    /**
     * Add torrent by url
     */
    public function addUrl($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "url":{"type":"string"},
                "start_download":{"type":"boolean"}
            }
        }');


        $transmissionbt_rpc = $this->factoryTransmission();

        $url = $params['url'];
        $paused = !array_boolval($params, 'start_download');

        $result = $transmissionbt_rpc->add($url, '', array('paused' => $paused));

        if (isset($result['result'])) {
            if ($result['result'] == 'success') {
                return 'success';
            } else {
                throw new Exception($result['result']);
            }
        }

        throw new Exception('Something went really wrong');
    }

    /**
     * Delete specific torrent
     */
    public function delete($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "id":{"type":"integer", "minimum":0},
                "deleteLocalData":{"type":"boolean"}
            }
        }');


        $transmissionbt_rpc = $this->factoryTransmission();

        $transmissionbt_rpc->remove(
            $params['id'],
            array_boolval($params, 'deleteLocalData')
        );

        return 'success';
    }

    /**
     * Resume specific torrent.
     */
    public function resume($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "id":{"type":"integer", "minimum":0}
            }
        }');

        $transmissionbt_rpc = $this->factoryTransmission();
        $transmissionbt_rpc->start($params['id']);

        return 'success';
    }

    /**
     * Pause specific torrent.
     */
    public function pause($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "id":{"type":"integer", "minimum":0}
            }
        }');

        $transmissionbt_rpc = $this->factoryTransmission();
        $transmissionbt_rpc->stop($params['id']);

        return 'success';
    }

    /**
     * Queue move specific torrent
     */
    public function queueMove($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type":"object",
            "properties":{
                "id":{"type":"integer", "minimum":0},
                "action":{"type":"string","enum":' . '["top","up","down","bottom"]}
            }
        }');

        $transmissionbt_rpc = $this->factoryTransmission();

        switch ($params['action']) {
            case 'top':
                $transmissionbt_rpc->queueMoveTop($params['id']);
                break;
            case 'up':
                $transmissionbt_rpc->queueMoveUp($params['id']);
                break;
            case 'down':
                $transmissionbt_rpc->queueMoveDown($params['id']);
                break;
            case 'bottom':
                $transmissionbt_rpc->queueMoveBottom($params['id']);
                break;
            default:
                throw new OMVException('Unknown Action: ' . $params['action']);
                break;
        }

        return 'success';
    }

    /**
     * Returns a new Transmission_RPC object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     * @return Transmission_RPC
     */
    private function factoryTransmission()
    {
        global $xmlConfig;

        $xpath = "//services/transmissionbt";
        $object = $xmlConfig->get($xpath);
        if (is_null($object)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
        }

        $transmissionbt_rpc = new Transmission_RPC();

        $transmissionbt_rpc->url = 'http://localhost:' . $object['rpcport'] . $object['rpcurl'] . 'rpc';

        $transmissionbt_rpc->return_as_array = true;
        // Set authentication when needed
        if (array_boolval($object, 'rpcauthenticationrequired') === true) {
            $transmissionbt_rpc->username = $object['rpcusername'];
            $transmissionbt_rpc->password = $object['rpcpassword'];
        }

        return $transmissionbt_rpc;
    }

}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceTransmissionBT());
