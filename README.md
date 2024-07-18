# $\text{\color{gray}DataStoreX   }$
Private data store server for roblox studio. 

Download the [latest server release](https://github.com/semachkin/DataStoreX/releases/download/Server/DataStoreX.7z) or make sure that the [GNU Make](https://www.gnu.org/software/make/) utility is installed and build it yourself by running build.bat\
Before starting, configure config.ini as you need
```ini
[Server]
Port = ---              ; port in which the server opens. 1922 by default
LogLevel = ---          ; level of logs. INFO, WARN, FAIL or NULL. INFO by default
LogFile = ---           ; log file directory. console.log by default
```

To connect to the server you need use roblox studio user agent. Download the [latest client release](https://github.com/semachkin/DataStoreX/releases/download/Client/DataStoreX.rbxmx)

Require DataStoreX module from ServerScript. And configurate ConnectionConfig attributes:\
```
_ip          - ip address of server (192.168.x.x if you use private network etc.)\
_port        - port of the server from config.ini\
storage_name - name of the storage from where you will write/read key values
```

### $\text{\color{lightgreen}Using}$
```lua
local datastore = require(path.to.DataStoreX)
local key = 'key'

datastore:SetAsync(key, '习近平')
datastore:GetAsync(key) -- 习近平
datastore:DeleteKey(key)
```
#### $\text{\color{yellow}!! Notation !!}$
Functions can throw an error if the connection is unsuccessful, so you need to wrap calls in pcall
### $\text{\color{lightgreen}DataStoreX API}$
| Method      | Arguments                          | Return values  | Description |
| --------    | -------                            | -------        | -------     |
| SetAsync    | **key**: string, **value**: any    | ()             | Sets the value of the **key** to **value** and returns void |
| GetAsync    | **key**: string                    | any            | Gets the value of **key** and returns its contents |
#### $\text{\color{Grey}Other API}$
| Method      | Arguments                          | Return values  | Description |
| --------    | -------                            | -------        | -------     |
| HandShake   | ()                                 | boolean        | Performs a handshake with the server |
| DeleteKey   | **key**: string                    | boolean        | Deletes the **key** |

### $\text{\color{red}for support contant semachka3000 on Discord}$
