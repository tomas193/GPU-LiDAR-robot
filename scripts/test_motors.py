# This is a sample Python script.
import time
from Motor import Motor




# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    motors = Motor(9600, "/dev/ttyUSB1", 1)
    #speeds = [10, 10 , 10, 10]
    #motors.write_speeds(speeds)
    #time.sleep(3)
    speeds = [50, 50 , 50, 50]
    motors.write_speeds(speeds)
    time.sleep(1)

    speeds = [-100, -100 , -100, -100]
    motors.write_speeds(speeds)
    time.sleep(2)

    speeds = [-100, 100 , -100, 100]
    motors.write_speeds(speeds)
    time.sleep(3)

    speeds = [100, -100 , 100, -100]
    motors.write_speeds(speeds)
    time.sleep(3)


    motors.stop_motors()

