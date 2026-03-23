# Backend API Contract

## REST

### GET /api/devices

```json
{
  "items": [
    {
      "id": "eqp01",
      "name": "EQP-01 Etcher",
      "ip": "10.10.20.15",
      "port": 5000,
      "mode": "active",
      "status": "connected",
      "selected": true
    }
  ]
}
```

### GET /api/devices/{id}/messages

```json
[
  {
    "id": "m1",
    "timestamp": "2026-03-23T10:00:01+08:00",
    "direction": "recv",
    "stream": 6,
    "function": 11,
    "note": "Event Report"
  }
]
```

## WebSocket

### WS /ws/devices/{id}

```json
{
  "type": "secs_message",
  "deviceId": "eqp01",
  "timestamp": "2026-03-23T10:00:01+08:00",
  "payload": {
    "direction": "recv",
    "stream": 6,
    "function": 11,
    "systemBytes": 1001,
    "note": "Event Report"
  }
}
```
