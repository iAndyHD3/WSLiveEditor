const WebSocket = require('ws');
const socket = new WebSocket('ws://127.0.0.1:1313');
const fs = require('fs');
const path = require('path');

socket.addEventListener('message', (event) => {
    console.log('Message from server:', event.data);
});

socket.addEventListener('close', (event) => {
    console.log("Disconnecting from the server");
});

socket.addEventListener('open', (event) => {
    console.log("Connected to the server");
    
    const del = {
        action: 'REMOVE_OBJECTS',
        group: 9999,
    };
    socket.send(JSON.stringify(del));

    const filePath = path.join(__dirname, 'adaydreamjourney.txt');

    const fileContent = fs.readFileSync(filePath, 'utf8');

     const add = {
         action: 'ADD_OBJECTS',
         objects: fileContent,
         close: true
     };
    
     socket.send(JSON.stringify(add));

});
