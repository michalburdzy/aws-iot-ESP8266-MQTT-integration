# NodeMCU smart sensor

## Features:

- integrated with AWS IoT
- hosting a static Website
- hosting a ElegantOTA updater at path /update

## MQTT messages:

### Trigger measurement and sending MQTT message

```
{
  "message": "measure"
}
```

### Retrieve current config

```
{
  "message": "getConfig"
}
```

### Update config / Fine-tune sensor reads

```
{
  "message": "updateConfig",
  "key": "temperatureShift",
  "value": 4
}
```

```
{
  "message": "updateConfig",
  "key": "pressureShift",
  "value": 3
}
```

```
{
  "message": "updateConfig",
  "key": "altitudeShift",
  "value": 100
}
```

```
{
  "message": "updateConfig",
  "key": "timeout",
  "value": 1000
}
```
