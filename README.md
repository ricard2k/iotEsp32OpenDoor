# iotEsp32OpenDoor
Open Door device with ESP32 chip 


## Background
Few month ago, we're moved our office to a new place, prevously this plave was a shop, and has a roller shutter closure and an automatic sliding door, both electronicaly driven, but with different systems.
Now we have to carry several remotes to access the office every morning.

Hey! We're a tech firm: We can integrate this into a mobile App

The first goal of this project is the security. Is not acceptable that somebody can open the door over internet.

Second goal: The scope of the app must be local, we cant open the door from home, we need to be phisically near to open the door. 

To meet the first goal, we use our own authorization system based in oauth2. 
The user log-in in the SSO web, and it generates a token that can be used to identify him by the other services.
To ensure the security, the service receiving the token, must check its validity against the SSO. 
All the communications must be encrypted, to ensure that the valid tokens can't been intercepted. 

We could have used a internet conected device that register in a centralized node, and an App accesing this node  (The model used by AWS IoT, Google Cloud IoT or other MQTT based services), but instead we are going to implement a web service locally in the IoT device, that only will be accesible at the local network.

Our mobile App must be connected to the local WiFi to open the door. This solve the phisical location problem of the second goal, because the open mecanism isn't accesible from internet.
Anyway, the device still needs internet access to verify the token against SSO.

## Solution
The selected development environment is platformio, for simplicity, we will use Arduino as development framework.

The wifi configuration process will be implemented with the [tzapu/WiFiManager](https://github.com/tzapu/WiFiManager) library modified for ESP32.

The Secure communication will be assured via [fhessel/esp32_https_server](https://github.com/fhessel/esp32_https_server) library.
