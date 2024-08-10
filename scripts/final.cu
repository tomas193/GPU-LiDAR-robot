
//Paralelo 

#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;

vector<float> vec_lec;
vector<vector<int>> visited;

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
    int frame=30; //centimetros
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

      
  for (int i=0;i<visited.size();i++){
   cout<<visited[i][0]<<","<<visited[i][1]<<endl;}
   cout<<"no permitidos: "<<visited.size()<<endl;	 

}

const int n = 501;     //10001; 

const int inicio_x = 10;
const int inicio_y = 10;
const int goal_x = 0; //10000;
const int goal_y = 0; //10000;

static void checking(cudaError_t err){
    if (err != cudaSuccess){
        printf("%s in %s at line %d\n", cudaGetErrorString(err), __FILE__, __LINE__);
        exit(EXIT_FAILURE);}}

__host__ __device__ 
int get_index(int x, int y) {
    if(x <= n && y <n) return y * n + x;

    else return -0;}

__host__ __device__ 
void get_coordinates(int index, int* x, int* y) {
    *y = index / n;
    *x = index % n;}

void construir_mundo(int* mundo) {
    // Inicializar todo el mundo a 0
    for (int i = 0; i < n*n; i++) {
        mundo[i] = 0;}

    // Marcar el estado inicial como visitado
    int index = get_index(inicio_x, inicio_y);
    mundo[index] = 1;
    
   //mundo[get_index(goal_x, goal_y)] = 2;
}

__device__
int movimientos[8][2] = {
    {1, 0}, // Derecha
    {-1, 0}, // Izquierda
    {0, 1}, // Arriba
    {0, -1}, // Abajo
    {1, 1}, // Arriba-Derecha
    {-1, 1}, // Arriba-Izquierda
    {-1, -1}, // Abajo-Izquierda
    {1, -1} // Abajo-Derecha
};

__global__
void kernel(int* ruta, int inicio, int final, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx != inicio && idx < n){
      int x, y;
      get_coordinates(idx, &x, &y);

      int inicio_x, inicio_y;
      get_coordinates(inicio, &inicio_x, &inicio_y);

      int min_dist = INT_MAX;
      int nearest_idx = -1;

      for (int i = 0; i < 8; i++) {
          int vecino_x = x + movimientos[i][0];
          int vecino_y = y + movimientos[i][1];
          
          if (vecino_x >= 0 && vecino_x < n && vecino_y >= 0 && vecino_y < n) {
              int vecino_idx = get_index(vecino_x, vecino_y);
              int dist = abs(inicio_x - vecino_x) + abs(inicio_y - vecino_y);

              if (dist < min_dist) {
                  min_dist = dist;
                  nearest_idx = vecino_idx;
              }
          }
      }
         ruta[idx] = nearest_idx;
      }
    }
  


int main() {
    int size = n*n*sizeof(int);
    unsigned t0, t1;
    int inicio = get_index(inicio_x, inicio_y);
    int final =  get_index(goal_x, goal_y);
    int *h_mundo; int* d_mundo;
    int* h_ruta; int* d_ruta;

    vector<int> actual={inicio_x,inicio_y};
  
    lectures(actual);
    
    cudaError_t err;
    err = cudaMalloc((void **) &d_mundo, size);
    checking(err);
    err = cudaMalloc((void **) &d_ruta, size); 
    checking(err);

    h_mundo = (int*) malloc(size);
    h_ruta = (int*) malloc(size);

    construir_mundo(h_mundo); 
    
    h_ruta[inicio] = -1; // El nodo inicial no tiene un nodo anterior

    cudaMemcpy(d_mundo, h_mundo, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_ruta, h_ruta, size, cudaMemcpyHostToDevice);

    int threadsPerBlock = 1024;
    int numBlocks = (n*n + threadsPerBlock - 1) / threadsPerBlock;
    
    t0 = clock();
    kernel<<<numBlocks, threadsPerBlock>>>(d_ruta, inicio, final, n*n);
    cudaDeviceSynchronize();
    t1 = clock();

    cudaMemcpy(h_ruta, d_ruta, size, cudaMemcpyDeviceToHost);

    // Inicializar el nodo actual al nodo final
    int nodo_actual = final;

    // Mientras no hayamos llegado al nodo inicial
    vector<std::pair<int, int>> trayectoria;

    while (nodo_actual != inicio) {
        
      int x, y;
      get_coordinates(nodo_actual, &x, &y);
      //printf("Nodo: (%d, %d)\n", x, y);

      trayectoria.push_back(std::make_pair(x, y));

      // Mueve al nodo actual hacia atrás a lo largo de la ruta
      nodo_actual = h_ruta[nodo_actual];}
      
      // Finalmente, imprimir el nodo inicial
      reverse(trayectoria.begin(), trayectoria.end());

    for (const auto& p: trayectoria) {
    cout << "(" << p.first << ", " << p.second << ") ";}
    cout << std::endl;
    double time = (double(t1-t0)/CLOCKS_PER_SEC);
    cout << "Execution Time: " << time << endl;
}
    
