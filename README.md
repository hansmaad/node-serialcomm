# node-serialcomm
NodeJS module that lists serial ports from HKLM\HARDWARE\DEVICEMAP\SERIALCOMM.
Non Windows platforms will return [].

## Usage
```
npm i serialcom
```

```
const SerialPort = require('serialport');
const serialcomm = require('serialcomm');

const list = await SerialPort.list();
const additionalPorts = await serialcomm.list();
for (port of additionalPorts) {
  if (!list.find(p => p.path === port)) {
    list.push({ path: port });
  }
}
```
