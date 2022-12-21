// Get references to UI elements
let connectButton = document.getElementById('connect');
let disconnectButton = document.getElementById('disconnect');
let terminalContainer = document.getElementById('terminal');
let sendForm = document.getElementById('send-form');
let lockButton = document.getElementById('lock')
let inputField = document.getElementById('input');

// Connect to the device on Connect button click
connectButton.addEventListener('click', function() {
  connect();
});

// Disconnect from the device on Disconnect button click
disconnectButton.addEventListener('click', function() {
  disconnect();
});

lockButton.addEventListener('click', function() {
  lock();
});


// Handle form submit event
sendForm.addEventListener('submit', function(event) {
  event.preventDefault(); // Prevent form sending
  send(inputField.value); // Send text field contents
  inputField.value = '';  // Zero text field
  inputField.focus();     // Focus on text field
});


// Selected device object cache
let deviceCache = null;
let lockState = 0;

// Launch Bluetooth device chooser and connect to the selected
function connect() {
    return (deviceCache ? Promise.resolve(deviceCache) :
        requestBluetoothDevice()).
        then(device => connectDeviceAndCacheCharacteristic(device)).
        //then(characteristic => startNotifications(characteristic)).
        catch(error => log(error));
}

function requestBluetoothDevice() {

    log('Requesting bluetooth device...');

    return navigator.bluetooth.requestDevice({
      filters: [{services: [0x180F]}],
    }).
        then(device => {
          log('"' + device.name + '" bluetooth device selected');
          deviceCache = device;

          deviceCache.addEventListener('gattserverdisconnected',
            handleDisconnection);
  
          return deviceCache;
        });
}

function handleDisconnection(event) {
  let device = event.target;

  log('"' + device.name +
      '" bluetooth device disconnected, trying to reconnect...');

  connectDeviceAndCacheCharacteristic(device).
      then(characteristic => startNotifications(characteristic)).
      catch(error => log(error));
}

// Characteristic object cache
let characteristicCache = null;
let characteristicCache2 = null;

// Connect to the device specified, get service and characteristic
function connectDeviceAndCacheCharacteristic(device) {
  if (device.gatt.connected && characteristicCache) {
    return Promise.resolve(characteristicCache);
  }

  log('Connecting to GATT server...');

  return device.gatt.connect().
      then(server => {
        log('GATT server connected, getting service...');

        return server.getPrimaryService(0x180F);
      }).
      then(service => {
        log('Service found, getting characteristic...');

         service.getCharacteristic(0x2A57).then(characteristic => {
          log ('Characteristic1 found');
          characteristicCache = characteristic;
        
        });

        service.getCharacteristic(0x2A58).then(characteristic => {
          log ('Characteristic2 found');
          characteristicCache2 = characteristic;
        });

       
      })

      
      
      //then(characteristic => {
        //log('Characteristic found');
        //characteristicCache = characteristic;

         /* characteristic.readValue().then(value => {
          log(`Led state is ${value.getUint8(0)}`);
        })*/
        

       // return characteristicCache;
      //});
}

// Enable the characteristic changes notification
function startNotifications(characteristic) {
  log('Starting notifications...');

  return characteristic.startNotifications().
      then(() => {
        log('Notifications started');

        characteristic.addEventListener('characteristicvaluechanged',
            handleCharacteristicValueChanged);

      });
}


// Output to terminal
function log(data, type = '') {
    terminalContainer.insertAdjacentHTML('beforeend',
        '<div' + (type ? ' class="' + type + '"' : '') + '>' + data + '</div>');
  }


// Disconnect from the connected device
function disconnect() {
  if (deviceCache) {
    log('Disconnecting from "' + deviceCache.name + '" bluetooth device...');
    deviceCache.removeEventListener('gattserverdisconnected',
        handleDisconnection);

    if (deviceCache.gatt.connected) {
      deviceCache.gatt.disconnect();
      log('"' + deviceCache.name + '" bluetooth device disconnected');
    }
    else {
      log('"' + deviceCache.name +
          '" bluetooth device is already disconnected');
    }
    
  }

   // Added condition
   if (characteristicCache) {
    characteristicCache.removeEventListener('characteristicvaluechanged',
        handleCharacteristicValueChanged);
    characteristicCache = null;
  }

  deviceCache = null;
  
}

// Data receiving
function handleCharacteristicValueChanged(event) {
  let value = new TextDecoder().decode(event.target.value);
  log(value, 'in');
}

function send(data) {

  if (!data || !characteristicCache) {
    return;
  }

  writeToCharacteristic(characteristicCache, data);
  log(data, 'out');
}

function lock(data) {

  if(lockState == 0){
    characteristicCache2.writeValue(Uint8Array.of(1));
    lockState = 1;
    lockButton.textContent = 'Lock';
  }
  else{
    characteristicCache2.writeValue(Uint8Array.of(0));
    lockState = 0;
    lockButton.textContent = 'Unlock';
  }
}

function writeToCharacteristic(characteristic, data) {
  //characteristic.writeValue(new TextEncoder().encode(data));
  characteristic.writeValue(Uint8Array.of(data));
}

