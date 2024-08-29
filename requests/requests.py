import requests

class WaterPumpController:
    def __init__(self, esp32_ip):
        self.base_url = f"http://{esp32_ip}"

    def turn_on_pump(self, relay):
        response = requests.get(f"{self.base_url}/on{relay}")
        if response.status_code == 200:
            print(f"Pump {relay} turned on successfully.")
        else:
            print(f"Failed to turn on pump {relay}. Status code: {response.status_code}")

    def turn_off_pump(self, relay):
        response = requests.get(f"{self.base_url}/off{relay}")
        if response.status_code == 200:
            print(f"Pump {relay} turned off successfully.")
        else:
            print(f"Failed to turn off pump {relay}. Status code: {response.status_code}")

    def set_timer(self, relay, duration, interval):
        payload = f"{relay}&{duration}&{interval}"
        headers = {'Content-Type': 'application/x-www-form-urlencoded'}
        response = requests.post(f"{self.base_url}/timer", data=payload, headers=headers)
        if response.status_code == 200:
            print(f"Timer set for pump {relay} with duration {duration} ms and interval {interval} ms.")
        else:
            print(f"Failed to set timer for pump {relay}. Status code: {response.status_code}")

if __name__ == "__main__":
    esp32_ip = "192.168.1.100"  # Replace with your ESP32's IP address

    controller = WaterPumpController(esp32_ip)

    # Example usage:
    controller.turn_on_pump(1)  # Turn on relay 1
    controller.turn_off_pump(1
