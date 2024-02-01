# What is this?

WSLiveEditor is a helpful tool for quick testing levels that are generated programtically by a tool, by opening a Websocket Server it allows to remove old objects and add new ones without needing to re-open the editor or restart the game.

Up on entering the editor, a sebsocket server will be opened on `127.0.0.1:1313`

## Usgae

You send **actions** in form of a json object. At the moment there are only 2 types of actions.
- The `ADD` action: adds objects, sent in form of level string
- The `REMOVE` action: removes objects that have a specific group

## Actions

Every action needs to have an `action` key (the type) and the appropiate value with it. The `ADD` action needs an `objects` key whereas the `REMOVE` action needs a `group` key. Example of an `ADD` action:
```json
{
  "action": "ADD",
  "objects": "1,1,2,60,3,90,57,1.9999"
}
```
This action will add a default object with group ids 1 and 9999 near the spawn of the player, extensive documentation for the level string format can be found in [gddocs](https://wyliemaster.github.io/gddocs/#/resources/client/level-components/level-object?id=object-string)

Example of a `REMOVE` action:
```json
{
  "action": "REMOVE",
  "group": 9999
}
```
This will remove every object with group id 9999
You should to give your objects a common remove group that is only used by your tool, this ensures you can use `REMOVE` to clean them all before adding the new ones. 

When an action is performed, the server returns a status by default. it can be success:
```json
{
  "status": "successful"
}
```
or error:
```json
{
  "status": "error",
  "error": "InvalidJson",
  "message": "'action' field is missing or is the wrong type"
}
```


Additionally actions can have the keys `id`, `response` and `close` 
- The `id` key will be sent back in the status response and it can be of any type. The following action:
- The `response`key if set to false will skip sending the status back for the specified action
- The `close` key if set to true will close the connection with the websocket client

# Example
Spawn a defalt block at a random hard coded x position. The reomve group will be 9999

```js
const WebSocket = require('ws');
const socket = new WebSocket('ws://127.0.0.1:1313');

socket.addEventListener('message', (event) => {
  console.log('Message from server:', event.data);
});

socket.addEventListener('open', (event) => {
	const del = {
		action: 'REMOVE',
		group: 9999,
	};
	socket.send(JSON.stringify(del));
	
	//few hardcoded x positions
	const array = [45, 75, 105, 135, 165];
	const randomElement = array[Math.floor(Math.random() * array.length)];
	const add = {
		action: 'ADD',
		objects: "1,1,2," + randomElement + ",3,75,57,1.2.3.4.9999",
		close: true
	};
	socket.send(JSON.stringify(add));
});
```
Run this script multiple times to see the object jumping around