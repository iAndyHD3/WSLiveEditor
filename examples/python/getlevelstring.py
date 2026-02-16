import asyncio
import websockets
import json
from concurrent.futures import ThreadPoolExecutor

executor = ThreadPoolExecutor(max_workers=1)

async def communicate():
    uri = "ws://127.0.0.1:1313"
    
    # Increase max_size to 100MB just to be safe
    # Increase max_queue to allow more messages to buffer in memory
    async with websockets.connect(
        uri, 
        max_size=100 * 1024 * 1024, 
        ping_interval=None,   # Disables automatic pings to prevent timeout during heavy CPU usage
        ping_timeout=None,    # Or set these to very high values (e.g., 60)
        max_queue=100         # Allows more messages to sit in the buffer
    ) as websocket:
        
        payload = {"action": "GET_LEVEL_STRING", "close": False}
        await websocket.send(json.dumps(payload))

        async for message in websocket:
            print("Message received, starting parse...")
            
            # Offload to thread so the loop doesn't die
            loop = asyncio.get_running_loop()
            try:
                resp = await loop.run_in_executor(executor, json.loads, message)
                
                if resp.get("status") == "successful":
                    # Use a slice if you just want to verify it worked without 
                    # flooding your terminal with 40MB of text
                    data_sample = str(resp.get("response"))[:100]
                    print(f"Success! Sample: {data_sample}...")
            except Exception as e:
                print(f"Parsing failed: {e}")

try:
    asyncio.run(communicate())
except Exception as e:
    print(f"Final Error: {e}")