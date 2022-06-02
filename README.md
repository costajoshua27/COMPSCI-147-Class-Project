# COMPSCI-147-Class-Project

1. The code in `/src` is ran on our ESP32 dev board.
2. The code in `/server` is ran on an AWS EC2 instance
  - The ESP32 consumes the `<EC2_INSTANCE_IP>/?id=<ID>&charge=<CHARGE_LEVEL>&timestamp=<TIMESTAMP>` endpoint to publish the battery data to the server
3. The webpage and JavaScript code in `/graph` can be ran locally on any browser
  - On the browser, go to `/path/to/graph/index.html?id=<CHARGE_ID>`
  - This webpage consumes the `<EC2_INSTANCE_IP>/data?id=<ID>` endpoint to retrieve the charge data for the currently charging battery
  - You can get `CHARGE_ID` from the OLED display of the ESP32 when it is charging a battery
