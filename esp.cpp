import machine
import time
import uarray as array

# Configuration for Micro-LED Driver Interface
SPI_BUS = 1
SCK_PIN = 18
MOSI_PIN = 23
CS_PIN = 5

WIDTH = 64
HEIGHT = 64
EXPOSURE_TIME_MS = 1500  # Tailored to photoresist sensitivity

class MicroLEDLithoDriver:
    def __init__(self, spi_bus, sck, mosi, cs, width, height):
        self.width = width
        self.height = height
        self.cs = machine.Pin(cs, machine.Pin.OUT)
        self.cs.value(1)
        
        # Hardware SPI initialized at high frequency (20 MHz)
        self.spi = machine.SPI(
            spi_bus, 
            baudrate=20000000, 
            polarity=0, 
            phase=0, 
            sck=machine.Pin(sck), 
            mosi=machine.Pin(mosi)
        )

    def load_matrix_from_file(self, filepath):
        """Loads flattened uint8 binary pattern output from GenAI model."""
        with open(filepath, "rb") as f:
            buffer = bytearray(f.read())
        return buffer

    def execute_exposure(self, intensity_buffer, duration_ms):
        """Streams pattern frame to display controller and triggers UV pulse."""
        print("Projecting GenAI OPC Pattern to Micro-LED Matrix...")
        
        self.cs.value(0) # Select Micro-LED Driver IC
        # Framebuffer transfer to display registers
        self.spi.write(intensity_buffer) 
        self.cs.value(1) # Latch data

        # Precision UV Exposure Hold
        print(f"Exposing Photoresist for {duration_ms} ms...")
        time.sleep_ms(duration_ms)
        
        # Clear Display (Blanking frame)
        blank_buffer = bytearray(self.width * self.height)
        self.cs.value(0)
        self.spi.write(blank_buffer)
        self.cs.value(1)
        print("Exposure Complete.")

# ==========================================
# Execution Loop
# ==========================================
if __name__ == "__main__":
    driver = MicroLEDLithoDriver(
        spi_bus=SPI_BUS, 
        sck=SCK_PIN, 
        mosi=MOSI_PIN, 
        cs=CS_PIN, 
        width=WIDTH, 
        height=HEIGHT
    )
    
    # Load and fire exposure
    pattern = driver.load_matrix_from_file("opc_pattern_matrix.raw")
    driver.execute_exposure(pattern, EXPOSURE_TIME_MS)
