# Water Pump Controller

This project is an ESP32-based water pump controller that allows you to control up to three water pumps via relays. The controller can be operated remotely using HTTP APIs, enabling you to turn the pumps on and off, and set timers to control how long and how often the pumps should be turned on.

## Features

- **Control Relays Remotely:** Turn on/off the pumps connected to the ESP32 using HTTP GET requests.
- **Set Timers:** Configure timers for the relays to automatically turn the pumps on and off at specified intervals.
- **Multi-Relay Support:** Supports up to three relays connected to the ESP32.

## Project Structure

```plaintext
water_pump_controller/
├── main/
│   ├── main.c                # Main application code for the ESP32
│   └── CMakeLists.txt        # CMake configuration for the project
├── requests/
│   ├── requests.py           # Python script to interact with the ESP32 API
└── CMakeLists.txt            # Top-level CMake configuration
```

## Components Overview

### 1. main.c
The main.c file contains the core logic for the ESP32. It includes:

	- Wi-Fi Setup: Configures the ESP32 to connect to your Wi-Fi network.
	- Relay Control: Sets up three GPIO pins to control the relays connected to the water pumps.
	- HTTP Server: Implements an HTTP server to handle API requests for controlling the relays and setting timers.

API Endpoints

1. Turn On Relay

	 - URL: /onX where X is the relay number (1, 2, or 3).
	Method: GET
	Example: http://<ESP32_IP>/on1 turns on relay 1.

2. Turn Off Relay

	URL: /offX where X is the relay number (1, 2, or 3).
	Method: GET
	Example: http://<ESP32_IP>/off1 turns off relay 1.
	

3. Set Timer
	URL: /timer
	Method: POST
	Payload: relay_number&duration&interval
		relay_number: Relay to control (1, 2, or 3)
		duration: Time in milliseconds the relay should stay on.
		interval: Time in milliseconds between activations.


	Example: 1&1000&5000 sets relay 1 to turn on for 1000 ms every 5000 ms.

### 2. requests.py

The requests.py file provides a Python interface to interact with the ESP32's API. It allows you to:

	- Turn on a relay: Use the turn_on_pump() method.
	- Turn off a relay: Use the turn_off_pump() method.
	- Set a timer: Use the set_timer() method.

### Example Usage

```bash
if __name__ == "__main__":
    esp32_ip = "192.168.1.100"  # Replace with your ESP32's IP address

    controller = WaterPumpController(esp32_ip)

    # Example usage:
    controller.turn_on_pump(1)  # Turn on relay 1
    controller.turn_off_pump(1)  # Turn off relay 1
    controller.set_timer(1, 1000, 5000)  # Set a timer for relay 1: on for 1000 ms, every 5000 ms
```

## Getting Started

### Prerequisites
	ESP32 Development Board
	Relay Module(s)
	Python 3.x (for running requests.py)

### Hardware Setup
	1. Connect the Relay Module(s) to the ESP32:
		Relay 1: Connect to GPIO 23.
		Relay 2: Connect to GPIO 22.
		Relay 3: Connect to GPIO 21.
	2. Connect Power: Ensure that your ESP32 and the relay module(s) are powered correctly.

### Software Setup
1. Clone the Repository:

```bash
git clone https://github.com/your-repository/water_pump_controller.git
cd water_pump_controller
```

2. Set Up ESP-IDF:

	Install ESP-IDF as per the official ESP-IDF documentation.
	Ensure your environment is set up by sourcing the export.sh script.

3. Build and Flash the Code:

```bash
cd main
idf.py build
idf.py flash
idf.py monitor
```

4 Run the Python Script

```bash
cd requests
python3 requests.py
```

Modify the requests.py script to match your use case or directly interact with the ESP32 via HTTP requests.

###Usage
 - Control the Relays:

	Use a web browser, curl, or the provided requests.py script to interact with the relays.
 - Set Timers:
	Use the /timer API to schedule automatic on/off cycles for the pumps.


## Future Enhancements
 - Security: Add authentication to the API.
 - Status Monitoring: Implement real-time monitoring of the relay states.
 - Web Interface: Develop a web-based GUI for easier control.


## License
This project is licensed under the MIT License - see the LICENSE file for details.

```bash

### How to Use the `README.md`

1. **Replace Placeholders:** Ensure you replace placeholders like `"https://github.com/your-repository/water_pump_controller.git"` with the actual URL of your repository.

2. **Customize:** Add any additional details specific to your setup, such as more specific instructions for setting up the hardware or software.

3. **Include in Your Repository:** Place this `README.md` file in the root directory of your `water_pump_controller` project, so it's visible on the project's main page when viewed on GitHub or any other repository hosting service.

This `README.md` provides a comprehensive overview of your project, guiding users through setup, usage, and future development possibilities.
```
