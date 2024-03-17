# esp-status-light

## Setup

Any ESP board with WiFi and a WS2812 LED strip, matrix or ring should work.

This specific config was tested with:

- Wemos D1 Mini
- Neopixel ring (24 LEDs)
- PlatformIO CLI

Define hardware settings in `main.h`, set up the number of LEDs and segments there.

```
pio run -t upload
```

## API usage

You can either control the status light via segment numbers or add names to segments in persistent memory.

Set status of a semgent (zero-indexed), default statuses: `free`, `occupied`, `dnd`:

```
PUT /api/status

Body:
{
  "segment": 1,
  "status": "occupied"
}

Response: 
ok
```

Set status of a named segment:

```
PUT /api/status

Body:
{
  "name": "Bob",
  "status": "free"
}
Response: 
ok
```

Add name to segment:

```
PUT /api/segments

Body:
{
  "segment": 0,
  "name": "Alice"
}

Response:
saved Alice to segment 0
```

Get current status of a segment:
```
GET /api/segments?segment=<id>

Response:
{
  "status": "off"
}
```

Get current status of all segments:
```
GET /api/segments?segment=<id>

Response:
[
  {
    "name": "Alice",
    "position": 0,
    "status": "dnd"
  },
  {
    "name": "Bob",
    "position": 1,
    "status": "free"
  }
]
```

Toggle light status (keeping segment status):
```
POST /api/leds

Body:
{
  "leds": false
}

Response: 
leds set to 0
```
