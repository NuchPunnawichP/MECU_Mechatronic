import machine
from machine import Pin, UART
import time

# ============================================================
#   SMART STEPPER MOTOR CLASS (IMPROVED WITH STATUS REPORTING)
# ============================================================

class StepperMotor:
    def __init__(self, name, pin_a, pin_b, pin_c, pin_d, ir_pin, seq_cw, seq_ccw):
        self.name = name
        
        # Motor pins
        self.pins = [
            Pin(pin_a, Pin.OUT),
            Pin(pin_b, Pin.OUT),
            Pin(pin_c, Pin.OUT),
            Pin(pin_d, Pin.OUT)
        ]
        
        # IR sensor
        self.ir_sensor = Pin(ir_pin, Pin.IN, Pin.PULL_UP)

        # Sequences
        self.seq_cw = seq_cw
        self.seq_ccw = seq_ccw

        # State variables
        self.is_running = False
        self.direction = 'stop'
        self.step_index = 0
        self.target_count = 0
        self.current_count = 0

        # IR state tracking
        self.sensor_last_state = 1
        self.low_start = 0

        # Speed timer (non-blocking stepping)
        self.last_step_time = time.ticks_ms()

        self.stop_power()

    # ---------------------------------------------------------
    def apply_step(self, step_pattern):
        for p, val in zip(self.pins, step_pattern):
            p.value(val)

    # ---------------------------------------------------------
    def stop_power(self):
        for p in self.pins:
            p.value(0)

    # ---------------------------------------------------------
    def stop(self):
        self.is_running = False
        self.direction = 'stop'
        self.target_count = 0
        self.current_count = 0
        self.stop_power()

    # ---------------------------------------------------------
    def set_task(self, count, direction):
        self.target_count = int(count)
        self.current_count = 0
        self.direction = direction
        self.sensor_last_state = 1

        if self.target_count > 0:
            self.is_running = True
            print(f"{self.name}: Starting task (Count={self.target_count}, Dir={self.direction})")
        else:
            self.stop()

    # ---------------------------------------------------------
    #      MAIN MOTOR UPDATE (CALLED EVERY LOOP)
    # ---------------------------------------------------------
    def update(self, motor_speed_ms):
        
        # -----------------------------------------------------
        #  IR SENSOR READ (DEBOUNCED & FILTERED)
        # -----------------------------------------------------
        if self.is_running:
            raw_val = self.ir_sensor.value()

            # Detect falling edge (transition from HIGH to LOW)
            if raw_val == 0 and self.sensor_last_state == 1:
                # Confirm low: filter out noise
                time.sleep_us(500)
                if self.ir_sensor.value() == 0:
                    # Count on falling edge
                    self.current_count += 1
                    print(f"{self.name}: Detected {self.current_count}/{self.target_count}")

            self.sensor_last_state = raw_val

        # -----------------------------------------------------
        #  END TASK CHECK
        # -----------------------------------------------------
        if not self.is_running:
            self.stop_power()
            return

        if self.current_count >= self.target_count:
            print(f"{self.name}: Task finished.")
            self.stop()
            return

        # -----------------------------------------------------
        #  NON-BLOCKING STEPPER TIMING
        # -----------------------------------------------------
        now = time.ticks_ms()
        if time.ticks_diff(now, self.last_step_time) < motor_speed_ms:
            return  # Skip until next allowed step

        self.last_step_time = now

        # Choose sequence
        if self.direction == 'cw':
            pattern = self.seq_cw[self.step_index]
        else:
            pattern = self.seq_ccw[self.step_index]

        self.apply_step(pattern)
        self.step_index = (self.step_index + 1) % 8


# ============================================================
#   8-STEP HALF-STEP SEQUENCES
# ============================================================

ccw_seq = [
    [1,0,0,0], [1,1,0,0], [0,1,0,0], [0,1,1,0],
    [0,0,1,0], [0,0,1,1], [0,0,0,1], [1,0,0,1]
]
cw_seq = list(reversed(ccw_seq))


# ============================================================
#   CREATE MOTORS (YOUR PIN MAPPING)
# ============================================================

motor1 = StepperMotor("Motor 1", 15, 28, 7, 27, 5, cw_seq, ccw_seq)
motor2 = StepperMotor("Motor 2", 26, 6, 17, 16, 4, cw_seq, ccw_seq)
motor3 = StepperMotor("Motor 3", 2, 14, 13, 12, 3, cw_seq, ccw_seq)


# Speed per step (non-blocking)
motor_speed_ms = 1


# ============================================================
#   UART SETUP (Communication with ESP32)
# ============================================================

# UART0: TX=GPIO0, RX=GPIO1
uart = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1))

# Status reporting timer
last_status_time = time.ticks_ms()
status_interval_ms = 500  # Send status every 500ms


# ============================================================
#   HELPER FUNCTIONS
# ============================================================

def send_status():
    """Send motor status to ESP32"""
    status = "STATUS:M1={},M2={},M3={}\n".format(
        1 if motor1.is_running else 0,
        1 if motor2.is_running else 0,
        1 if motor3.is_running else 0
    )
    uart.write(status)


def process_command(command):
    """Process command received from ESP32"""
    command = command.strip()
    print(f"Received command: {command}")
    
    if len(command) == 3 and command.isdigit():
        motor1.set_task(command[0], 'cw')
        motor2.set_task(command[1], 'cw')
        motor3.set_task(command[2], 'cw')
        
    elif command == "0":
        print("Stopping all motors.")
        motor1.stop()
        motor2.stop()
        motor3.stop()


# ============================================================
#   MAIN LOOP
# ============================================================

print("RP2040 Motor Control with UART Ready.")
print("Waiting for commands from ESP32...")

serial_buffer = ""

while True:
    # -----------------------------------------------
    #   READ UART INPUT FROM ESP32
    # -----------------------------------------------
    if uart.any():
        char = uart.read(1).decode('utf-8', 'ignore')
        
        if char in ['\n', '\r']:
            if serial_buffer:
                process_command(serial_buffer)
                serial_buffer = ""
        else:
            serial_buffer += char

    # -----------------------------------------------
    #   MOTOR UPDATES
    # -----------------------------------------------
    motor1.update(motor_speed_ms)
    motor2.update(motor_speed_ms)
    motor3.update(motor_speed_ms)

    # -----------------------------------------------
    #   SEND STATUS TO ESP32 PERIODICALLY
    # -----------------------------------------------
    now = time.ticks_ms()
    if time.ticks_diff(now, last_status_time) >= status_interval_ms:
        send_status()
        last_status_time = now
