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
		objects: "1,1,2," + randomElement + ",3,75,57,1.9999",
		close: true
	};
	socket.send(JSON.stringify(add));
});