const WebSocket = require("ws");
const socket = new WebSocket("ws://127.0.0.1:1313");

// Increase max payload size if necessary (server-side usually, but good to know)
socket.on("open", () => {
    console.log("Connected to server");
    
    const request = {
        action: "GET_LEVEL_STRING",
        // Try setting close to false first to see if data flows better
        close: true
    };
    
    socket.send(JSON.stringify(request));
});

socket.on("message", (data) => {
    console.log(`Received chunk of size: ${data.length || data.byteLength}`);
    data = data.toString();
    console.log(data);
    try {
        const resp = JSON.parse(data);
        
        if (resp.status === "successful") {
            console.log("Data received successfully!");
            // Process your large string here
        }
        else {
            console.log("Data recieved but error");
        }
    } catch (e) {
        // If JSON.parse fails, it might be the raw string or a partial chunk
        console.log("Raw data length:", data.length);
    }
});

socket.on("error", (err) => {
    console.error("Socket Error:", err.message);
});

socket.on("close", () => {
    console.log("Connection closed");
});