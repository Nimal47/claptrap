from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
import uvicorn
import httpx

app = FastAPI()

# Store active dashboard connections
dashboards = set()

# 1. Serve the Phone Camera Streamer Page
@app.get("/phone", response_class=HTMLResponse)
async def get_phone():
    with open("templates/phone.html", "r") as f:
        return HTMLResponse(content=f.read())

# 2. Serve the Driver Dashboard Page
@app.get("/dashboard", response_class=HTMLResponse)
async def get_dashboard():
    with open("templates/dashboard.html", "r") as f:
        return HTMLResponse(content=f.read())

# 3. WebSocket for Phone Camera Input -> Broadcast to Dashboard
@app.websocket("/stream")
async def websocket_stream(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            # Receive raw base64 JPEG frame from the phone
            data = await websocket.receive_text()
            # Broadcast it instantly to all open dashboards
            for client in dashboards:
                try:
                    await client.send_text(data)
                except Exception:
                    pass
    except WebSocketDisconnect:
        print("Phone disconnected.")

# 4. WebSocket for Dashboard Connections
@app.websocket("/dash-ws")
async def websocket_dashboard(websocket: WebSocket):
    await websocket.accept()
    dashboards.add(websocket)
    try:
        while True:
            await websocket.receive_text() # Keep connection alive
    except WebSocketDisconnect:
        dashboards.remove(websocket)

# 5. Relay Keyboard Driving Commands to the ESP32
@app.get("/drive")
async def drive(cmd: str, esp_ip: str, speed: int = 0): # <-- Added speed here
    try:
        # We now attach BOTH the cmd and the speed to the outgoing request!
        target_url = f"http://{esp_ip}/drive?cmd={cmd}&speed={speed}" 
        
        async with httpx.AsyncClient() as client:
            await client.get(target_url, timeout=2.0)
            
        return {"status": "success", "cmd": cmd, "speed": speed}
    except Exception as e:
        return {"status": "error", "details": str(e)}

if __name__ == "__main__":
    # Run server on your local network (port 8000)
    uvicorn.run(app, host="0.0.0.0", port=8000)