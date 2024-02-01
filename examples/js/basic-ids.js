const WebSocket = require('ws');
const socket = new WebSocket('ws://127.0.0.1:1313');

socket.addEventListener('message', (event) => {
	console.log('Message from server:', event.data);
});

socket.addEventListener('close', (event) => {
	console.log("Disconnecting from the server");
});

socket.addEventListener('open', (event) => {
	console.log("Connected to the server");
	
	const del = {
		action: 'REMOVE',
		group: 9999,
		id: 1
	};
	socket.send(JSON.stringify(del));
	
	const add = {
		action: 'ADD',
		objects: "1,1,2,60,3,90,57,1.9999",
		id: 2,
		close: true
	};
	socket.send(JSON.stringify(add));
});
