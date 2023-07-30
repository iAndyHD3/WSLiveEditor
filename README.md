# WSLiveEditor

## Usage

Connect to localhost at `ws://127.0.0.1:8080`, if it connecton was successful server will send a `hello there` message

### Data

json array is sent as follows

```
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

The actions are executed in ascending order using the `order` key:
- remove all objects that have group 5
- add the object string

for now only these 2 types are supported, for `remove_objects` only `group` type is supported

### Response

json object where `ok` is a boolean and an optional `error` if there is a reason
```
{
  "ok": true
}
```

