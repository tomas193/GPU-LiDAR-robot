#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <SerialStream.h> 
#include <unistd.h> 

using namespace std;

vector<float> vec_lec;
vector<vector<int>> visited;


class Motor {
private:
    int baud_rate;
    string port;
    int time_out;
    vector<int> speeds;
    LibSerial::SerialStream arduino;

public:
    Motor(int baud_rate, const string& port, int time_out)
        : baud_rate(baud_rate), port(port), time_out(time_out), speeds({0, 0, 0, 0}) {
        serial_connection();
    }

    void serial_connection() {
        arduino.Open(port);
        if (!arduino.IsOpen()) {
            cerr << "Error: Could not open serial port." << endl;
            exit(1);
        }
        arduino.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_9600);
        usleep(2000000);  // Wait for 2 seconds to ensure connection
        cout << "Connection established successfully with Arduino." << endl;
    }

    std::string generate_command(const vector<int>& speeds) {
        this->speeds = speeds;
        vector<string> command_parts;
        for (size_t i = 0; i < speeds.size(); ++i) {
            int speed = speeds[i];
            string direction = (speed == 0) ? "0" : (speed > 0) ? "+" : "-";
            string formatted_speed = direction + to_string(abs(speed));
            command_parts.push_back("M" + to_string(i+1) + ":" + formatted_speed);
        }
        string command = "";
        for (const auto &part : command_parts) {
            command += part + ",";
        }
        command.pop_back(); // Remove the last comma
        command += "\n";
        return command;
    }

    string send_command(const string& command) {
        cout << "Sending command: " << command;
        arduino.write(command.c_str(), command.length());
        string feedback;
        string line;
        while (arduino.rdbuf()->in_avail() > 0) {
            getline(arduino, line);
            feedback += line;
        }
        return feedback;
    }

    void write_speeds(const std::vector<int>& speeds) {
        string feedback = send_command(generate_command(speeds));
        categorize_feedback(feedback);
    }

    void categorize_feedback(const std::string& feedback) {
        cout << "Feedback: " << feedback << endl;
        // To be determined
    }

    void close_arduino() {
        arduino.Close();
        cout << "Closed connection." << endl;
    }

    void stop_motors() {
        write_speeds({0, 0, 0, 0});
        //send_command("STOP");
    }

    void fw(){
    std::vector<int> speeds = {150, 150, 150, 150};
    write_speeds(speeds);
    usleep(1670000);
    stop_motors();
    usleep(1000000);
    }

    void back(){
    std::vector<int> speeds = {-150, -150, -150, -150};
    write_speeds(speeds);
    usleep(1670000);
    stop_motors();
    usleep(1000000);
    }

    void right(){
    std::vector<int> speeds = {-150, 150, -150, 150};
    write_speeds(speeds);
    usleep(1670000);
    stop_motors();
    usleep(1000000);
    }

    void left(){
    std::vector<int> speeds = {150, -150, 150, -150};
    write_speeds(speeds);
    usleep(1670000);
    stop_motors();
    usleep(1000000);
    }
	
};


int lectures(const vector<int>& actual) {
    cout<<"nodo actual: "<<actual[0]<<","<<actual[1]<<endl;
   
    system("python3 read.py");

    // Abre el archivo en modo de lectura
    ifstream archivo("lectures.txt");

    // Verifica si se pudo abrir el archivo correctamente
    if (!archivo.is_open()) {
        cerr << "No se pudo abrir el archivo." << endl;
        return 1;
    }
    string linea; //variable que guarda cada linea de texto
    getline(archivo,linea); //obtener linea de archivo
    stringstream ss(linea);
    string valor;

    while (getline(ss,valor,',')){
       float entero=stof(valor); //convertir string a float
       vec_lec.push_back(entero);} //insetar datos en vec_lec

   // Cierra el archivo después de leerlo
    archivo.close();

    float grados=0.01745329252;
    int frame=50; //centimetros
    int k=90; //angulo donde inicia la medicion  
    
     
   for (const auto& p:vec_lec){
      vector<int> aux; //guardar x,y
      double hip=((p*100)/frame);//convertir medicion en espacio de estados
      int x=round(hip*(cos(k*grados)));//componente x de la medicion
      int y=round(hip*(sin(k*grados)));//componente y de la medicion
      x+=actual[0];y+=actual[1];
      aux.push_back(x);aux.push_back(y);
      bool flag=false; //flag for repeated coord

      for (int i=0;i<visited.size();i++){
      if (aux==visited[i]){flag=true;break;}}//checar si una coordenada se repite
      if (flag==false && aux!=actual){visited.push_back(aux);}//insertar si no esta en la lista
      k+=1;}  

      
 // for (int i=0;i<visited.size();i++){
 //  cout<<visited[i][0]<<","<<visited[i][1]<<endl;}
 // cout<<"no permitidos: "<<visited.size()<<endl;	 

}

void vecinos(const vector<int>& actual, vector<vector<int>>& vecs){
  vecs={};
  vector<vector<int>> aux;
  aux.push_back({actual[0],(actual[1]+1)});
  aux.push_back({(actual[0]+1),(actual[1]+1)});
  aux.push_back({(actual[0]+1),(actual[1])});
  aux.push_back({(actual[0]+1),(actual[1]-1)});
  aux.push_back({actual[0],(actual[1]-1)});
  aux.push_back({(actual[0]-1),(actual[1]-1)});
  aux.push_back({(actual[0]-1),(actual[1])});
  aux.push_back({(actual[0]-1),(actual[1]+1)});

  for(int i=0;i<aux.size();i++){
     bool flag=false;
     for (int k=0;k<visited.size();k++){
        if(aux[i]==visited[k]){flag=true;break;}
     }
     if(flag==false){vecs.push_back(aux[i]);}
  }

}

void bestf(vector<int>& actual, vector<vector<int>> vecs,vector<int>& meta){
  float dist=fabs(meta[0]-(actual[0]))+fabs(meta[1]-(actual[1]));
  for(int i=0;i<vecs.size();i++){
     float aux=fabs(meta[0]-(vecs[i][0]))+fabs(meta[1]-(vecs[i][1]));
     if(aux<=dist){dist=aux;actual=vecs[i];}
  }
}

int main(){
  Motor motors(9600, "/dev/ttyUSB1", 1);

  vector<int> inicio={10,10};
  vector<int> meta={0,0};
  vector<vector<int>> vecs;

  while(inicio!=meta){
  lectures(inicio);
  vecinos(inicio,vecs);
  bestf(inicio,vecs,meta);


  cout<<inicio[0]<<","<<inicio[1]<<endl;}
  motors.close_arduino();
}

