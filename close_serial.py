import serial.tools.list_ports
import subprocess
import os
import platform

def close_serial_port():
    target_board = "esp32"  # Sesuaikan dengan nama device Anda
    ports = serial.tools.list_ports.comports()

    for port in ports:
        if target_board in port.description.lower():
            print(f"Found device {port.description} on {port.device}")
            if platform.system() == "Windows":
                # Gunakan taskkill untuk mematikan proses di Windows
                subprocess.call(["taskkill", "/F", "/IM", "platformio.exe"], shell=True)
            elif platform.system() == "Linux" or platform.system() == "Darwin":
                # Gunakan lsof untuk Linux/Mac
                os.system(f"fuser -k {port.device}")
            print(f"Closed port {port.device}")

if __name__ == "__main__":
    close_serial_port()
