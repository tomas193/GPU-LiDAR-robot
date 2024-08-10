import serial
import time
import atexit

class Motor:
    def __init__(self, baud_rate, port, time_out):
        self.baud_rate = baud_rate
        self.port = port
        self.time_out = time_out
        self.speeds = [None, None, None, None]
        self.__Arduino = None
        self.__serial_connection()
        #atexit.register(self.close_arduino())

    def __serial_connection(self):
        """
        This function create the serial connection with arduino. It is executed once Motor instance is created
        """
        try:
            self.__Arduino = serial.Serial(self.port, self.baud_rate, timeout=self.time_out)
            time.sleep(2)  # Espera para asegurar la conexión
            if self.__Arduino.isOpen():
                  print('Conexión establecida correctamente con Arduino.')
            else:
                  print('No se pudo establecer la conexión con Arduino.')
        except serial.SerialException as e:
            print(f'Error al abrir el puerto serie: {e}')

        '''self.__Arduino = serial.Serial(self.port, self.baud_rate, timeout = self.time_out)
        print('bien')
        time.sleep(2)'''

    def __generate_command(self, speeds):
        """
        Generate a command for the arduino, based on speeds
        """
        self.speeds = speeds            #Save the last speeds values
        command_parts = []
        for i , speed  in enumerate(speeds):
            if speed == 0:
                direction = '0'
                formatted_speed = f"{direction}{speed}"
            else:
                direction = '+' if speed > 0 else '-'
                formatted_speed = f"{direction}{abs(speed)}"
            command_parts.append(f"M{i+1}:{formatted_speed}")
        command = ",".join(command_parts) + "\n"
        return command

    def __send_command(self,command):
        """
        Send a command to Arduino and waits for feedback
        """
        print(f"Sending command: {command}")

        self.__Arduino.write(command.encode())
        end_time = time.time() + 0.2   #Max wait a second for the feedback
        feedback = ""
        while time.time() < end_time:
            if self.__Arduino.in_waiting > 0:
                feedback = self.__Arduino.readline().decode().strip()
                break
            time.sleep(0.05)
        return feedback
    def write_speeds(self, speeds):
        """
        Executes motor actions according to the speeds, also call categorize feedback, to check the feedback
        """
        self.__categorize_feedback(self.__send_command(self.__generate_command(speeds)))

    def __categorize_feedback(self, feedback):
        print("Feedback: ")
        print(feedback)
        #To be determined

    def close_arduino(self):
        """
        Execut at the end of the program, close serial comunication
        """
        #self.__Arduino.close()
        print("Closed connection")

    def stop_motors(self):
        self.write_speeds([0,0,0,0])#Set pwms to 0
        #To diable de motors
        self.__send_command_stop()

    def __send_command_stop(self):
        """
        Send a command to Arduino and waits for feedback
        """
        print(f"Sending command: STOP")

        self.__Arduino.write("STOP".encode())
        end_time = time.time() + 0.2   #Max wait a second for the feedback
        feedback = ""
        while time.time() < end_time:
            if self.__Arduino.in_waiting > 0:
                feedback = self.__Arduino.readline().decode().strip()
                break
            time.sleep(0.05)
        return feedback

