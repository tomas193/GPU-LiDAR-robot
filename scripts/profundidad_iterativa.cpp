#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
using namespace std;

class Nodo {
public:
    vector<int> valor;
    vector<vector<int>> vecinos;
    bool endf;

    Nodo(vector<int> v) : valor(v), endf(false) {}
};

class Grafo {
public:
    vector<int> inicio;
    vector<int> final;
    vector<int> cuadricula;
    vector<vector<int>> n_p;
    vector<vector<int>> path;
    vector<vector<int>> pila;
    vector<vector<int>> visitados;
    int profundidad;
    vector<int> actual;

    Grafo(vector<int> inicio) : inicio(inicio), final({10, 10}), cuadricula({10, 10}), n_p({{3, 3}, {1, 2}, {1, 1}}), profundidad(0), actual(inicio) {
        visitados.push_back(inicio);
    }

    vector<vector<int>> calculo_vecinos(Nodo &n_a) {
        for (int i = 0; i < 4; i++) {
            for (int k = 0; k < 2; k++) {
                vector<int> aux1 = n_a.valor;
                int x = 1;
                if (i == 0) {
                    aux1[x - k] = aux1[x - k] + 1;
                } else if (i == 1) {
                    aux1[x - k] = aux1[x - k] - 1;
                } else if (i == 2) {
                    aux1[0] = aux1[0] + 1;
                    aux1[1] = aux1[1] - 1 + (k * 2);
                } else if (i == 3) {
                    aux1[0] = aux1[0] - 1;
                    aux1[1] = aux1[1] - 1 + (k * 2);
                }

                if (check(pila, aux1) && check(visitados, aux1) && check(n_p, aux1)) {
                    n_a.vecinos.push_back(aux1);
                    pila.push_back(aux1);
                }

                if (aux1 == final) {
                    n_a.endf = true;
                    return n_a.vecinos;
                }
            }
        }
        return n_a.vecinos;
    }

    bool check(vector<vector<int>>& n_p, vector<int>& aux) {
        if (find(n_p.begin(), n_p.end(), aux) != n_p.end()) {
            return false;
        }
        if (aux[0] == (cuadricula[0] + 1) || aux[1] == (cuadricula[1] + 1)) {
            return false;
        }
        if (aux[0] < 0 || aux[1] < 0) {
            return false;
        }
        return true;
    }

    void visited() {
        vector<int> aux = pila.back();
        pila.pop_back();
        visitados.push_back(aux);
    }

    void main(vector<int> actual) {
        Nodo nodo_actual(actual);
        if (inicio != final) {
            path.push_back(nodo_actual.valor);
            if (nodo_actual.valor != final) {
                if (calculo_vecinos(nodo_actual).empty()) {
                    visited();
                    if (!pila.empty()) {
                        nodo_actual = Nodo(pila.back());
                        main(nodo_actual.valor);
                    } else {
                        cout << "pila vacía" << endl;
                    }
                } else if (nodo_actual.endf) {
                    path.push_back(nodo_actual.vecinos.back());
                    nodo_actual.vecinos.pop_back();
                    cout << "\ninicio: (" << inicio[0] << ", " << inicio[1] << ") ---> final: (" << final[0] << ", " << final[1] << ")" << endl;
                    cout << "path: ";
                    for (const auto& p : path) {
                        cout << "(" << p[0] << ", " << p[1] << ") ";
                    }
                    cout << "\ncosto: " << path.size() << "\n" << endl;
                    return;
                } else {
                    int rand_index = nodo_actual.vecinos.size()-1;
                    nodo_actual = Nodo(nodo_actual.vecinos[rand_index]);
                    profundidad++;
                    main(nodo_actual.valor);
                }
            }
        }
    }
};

int main() {
    srand(time(0));
    vector<int> inicio = {2, 2};
    Grafo grafo(inicio);
    grafo.main(inicio);
    return 0;
}
