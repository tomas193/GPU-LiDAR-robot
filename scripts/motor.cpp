#include <iostream>
#include <string>
#include <vector>
#include <SerialStream.h>  // Replace with appropriate serial library for your C++ environment
#include <unistd.h>       // For sleep function
#include <cstdlib>        // For exit function

class Motor {
private:
    int baud_rate;
    std::string port;
    int time_out;
    std::vector<int> speeds;
    LibSerial::SerialStream arduino;

public:
    Motor(int baud_rate, const std::string& port, int time_out)
        : baud_rate(baud_rate), port(port), time_out(time_out), speeds({0, 0, 0, 0}) {
        serial_connection();
    }

    void serial_connection() {
        arduino.Open(port);
        if (!arduino.IsOpen()) {
            std::cerr << "Error: Could not open serial port." << std::endl;
            exit(1);
        }
        arduino.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_9600);
        usleep(2000000);  // Wait for 2 seconds to ensure connection
        std::cout << "Connection established successfully with Arduino." << std::endl;
    }

    std::string generate_command(const std::vector<int>& speeds) {
        this->speeds = speeds;
        std::vector<std::string> command_parts;
        for (size_t i = 0; i < speeds.size(); ++i) {
            int speed = speeds[i];
            std::string direction = (speed == 0) ? "0" : (speed > 0) ? "+" : "-";
            std::string formatted_speed = direction + std::to_string(std::abs(speed));
            command_parts.push_back("M" + std::to_string(i+1) + ":" + formatted_speed);
        }
        std::string command = "";
        for (const auto &part : command_parts) {
            command += part + ",";
        }
        command.pop_back(); // Remove the last comma
        command += "\n";
        return command;
    }

    std::string send_command(const std::string& command) {
        std::cout << "Sending command: " << command;
        arduino.write(command.c_str(), command.length());
        std::string feedback;
        std::string line;
        while (arduino.rdbuf()->in_avail() > 0) {
            std::getline(arduino, line);
            feedback += line;
        }
        return feedback;
    }

    void write_speeds(const std::vector<int>& speeds) {
        std::string feedback = send_command(generate_command(speeds));
        categorize_feedback(feedback);
    }

    void categorize_feedback(const std::string& feedback) {
        std::cout << "Feedback: " << feedback << std::endl;
        // To be determined
    }

    void close_arduino() {
        arduino.Close();
        std::cout << "Closed connection." << std::endl;
    }

    void stop_motors() {
        write_speeds({0,0,0, 0});
        //send_command("STOP");
    }

    void fw(){
    std::vector<int> speeds = {150, 150, 150, 150};
    write_speeds(speeds);
    usleep(1500000);
    stop_motors();
    usleep(1000000);
    }

    void back(){
    std::vector<int> speeds = {-150, -150, -150, -150};
    write_speeds(speeds);
    usleep(1690000);
    stop_motors();
    usleep(1000000);
    }

    void right(){
    std::vector<int> speeds = {-70, 70, -255,255};
    write_speeds(speeds);
    usleep(1670000);
    stop_motors();
    usleep(1000000);
    }

    void left(){
    std::vector<int> speeds = {255, -255, -255, 255};
    write_speeds(speeds);
    usleep(1670000);
    stop_motors();
    usleep(1000000);
    }


    ~Motor(){close_arduino();}	
};

int main(){
    Motor motors(9600, "/dev/ttyUSB0", 1);

    //motors.fw();
    //usleep(1000000);
    //motors.back();
    //motors.left();
    motors.right();
    motors.close_arduino();

    return 0;
}

