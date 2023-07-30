# WSLiveEditor

## Usage

Load the mod using any modloader, when the main menu starts server will try to open on localhost `ws://127.0.0.1:8080`, server will send `hello there` to every new connection

### Data

json array is sent as follows

```json
[
  {
    "type": "add_objects_string",
    "value": "1,1,2,150,3,150;1,1,2,150,3,180",
    "order": 1
  },
  {
    "type": "remove_objects",
    "filter": "group",
    "value": 5,
    "order": 0
  }
]
```

### GZIP

WSLiveEditor supports gziped json, if your object string is long minify the json and send it gzipped.
Note: the gziped string must start with `H4sIAAAAAAAA`, the above example would be:
```
H4sIAAAAAAAACouuViqpLEhVslJKTEmJz0/KSk0uKY4vLinKzEtX0lEqS8wpBUka6hjqGOkYmhroGINIaxS+hQFQZX5RSmqRkpVhrQ7cxKLU3PyyVJihQDVpmTklIEVK6UX5pQUI403h2g1qYwHGUY+okgAAAA==
```

The actions are executed in ascending order using the `order` key:
- remove all objects that have group 5
- add the object string

for now only these 2 types are supported, for `remove_objects` only `group` filter is supported

### Response

json object where `ok` is a boolean and an optional `error` if there is a reason
```json
{
  "ok": true
}
```

```json
{
  "ok": false,
  "error": "User is not in the editor"
}
```

## Testing

The [Simple WebSocket Client](https://chrome.google.com/webstore/detail/simple-websocket-client/pfdhoblngboilpfeibdedpjgfnlcodoo) chrome extension works great