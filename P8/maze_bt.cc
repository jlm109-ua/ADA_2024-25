// Juan Llinares Mauri 74011239E
#include <iostream>
#include <fstream>
#include <map>
#include <tuple>
#include <chrono>
#include <string.h>
#include <climits>
#include <vector>
#include <algorithm>
#include <iomanip> // Para setw, si quieres alineación adicional

// Colores para la salida por consola en el modo [--debug]
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

using namespace std;

/// Enumerado para las direcciones del camino codificado (flag [-p]).
/// Las direcciones son:
/// - N: Norte
/// - NE: Noreste
/// - E: Este
/// - SE: Sureste
/// - S: Sur
/// - SW: Suroeste
/// - W: Oeste
/// - NW: Noroeste
enum Step
{
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW
};

/// Estructura para almacenar las estadísticas del algoritmo.
/// - nv: Número de nodos visitados
/// - ne: Número de nodos explorados
/// - nhv: Número de nodos hoja visitados
/// - nnf: Número de nodos no factibles
/// - nnp: Número de nodos no prometedores
/// - t: Tiempo de ejecución
struct Stats
{
    int nv = 0;                      // Número de nodos visitados
    int ne = 0;                      // Número de nodos explorados
    int nhv = 0;                     // Número de nodos hoja visitados
    int nnf = 0;                     // Número de nodos no factibles
    int nnp = 0;                     // Número de nodos no prometedores
    std::chrono::duration<double> t; // Tiempo de ejecución
};

/// Estructura para almacenar un nodo.
/// - x: Coordenada x del nodo
/// - y: Coordenada y del nodo
/// - path: Camino codificado
/// - cost: Coste del nodo
struct Node
{
    int x = 0, y = 0;  // Coordenadas del nodo
    vector<Step> path; // Camino codificado
    int cost = 0;      // Coste del nodo
};

/**
 * @brief Función que imprime el modo de uso.
 */
void printUsageError()
{
    cerr << "ERROR: Uso: maze_bt [-p] [--p2D] -f fichero_entrada --debug" << endl;
}

/**
 * @brief Función que imprime los resultados del algoritmo.
 *
 * @param mejorNodo Resultado del algoritmo
 * @param stats Estructura con las estadísticas
 * @param debug Flag para el modo debug
 */
void printStats(Node mejorNodo, Stats stats, bool debug)
{
    if (debug)
    {
        cout << "Resultado: " << mejorNodo.cost << endl;
        cout << "Nodos visitados: " << stats.nv << "\nNodos explorados: " << stats.ne << "\nNodos hoja visitados: " << stats.nhv << "\nNodos no factibles: " << stats.nnf << "\nNodos no prometedores: " << stats.nnp << endl;
        cout << "Tiempo de ejecución: " << stats.t.count() << " ms" << endl;
    }
    else
    {
        if (mejorNodo.cost == INT_MAX)
            cout << 0 << endl;
        else
            cout << mejorNodo.cost << endl;
        cout << stats.nv << " " << stats.ne << " " << stats.nhv << " " << stats.nnf << " " << stats.nnp << endl;
        cout << fixed << setprecision(3) << stats.t.count() << endl;
    }
}

/**
 * @brief Función que imprime el laberinto en 2D usando el camino codificado de la mejor solución.
 *
 * @param maze Laberinto
 * @param mejorNodo Mejor nodo encontrado
 * @return - void
 */
void print2D(vector<vector<int>> maze, Node mejorNodo)
{
    vector<vector<int>> route = maze; // Copia del laberinto para marcar el camino

    auto position = make_tuple(0, 0); // Posición inicial

    if (mejorNodo.cost != INT_MAX)
        route[get<0>(position)][get<1>(position)] = 2; // Marcamos la posición inicial
    // if (mejorNodo.path.size() != 0)
    // route[get<0>(position)][get<1>(position)] = 2; // Marcamos la posición inicial
    else
    {
        cout << "0" << endl;
        return;
    }

    for (auto it = mejorNodo.path.begin(); it != mejorNodo.path.end(); ++it)
    {
        Step move = *it; // Movimiento actual

        // Obtenemos el incremento de fila y columna
        int incx, incy;
        tie(incx, incy) = make_tuple(0, 0);

        switch (move)
        {
        case N:
            incx = -1;
            break;
        case NE:
            incx = -1;
            incy = 1;
            break;
        case E:
            incy = 1;
            break;
        case SE:
            incx = 1;
            incy = 1;
            break;
        case S:
            incx = 1;
            break;
        case SW:
            incx = 1;
            incy = -1;
            break;
        case W:
            incy = -1;
            break;
        case NW:
            incx = -1;
            incy = -1;
            break;
        }

        // Actualizamos la posición
        position = make_tuple(get<0>(position) + incx, get<1>(position) + incy);
        route[get<0>(position)][get<1>(position)] = 2; // Marcamos el camino
    }

    // Imprimimos el laberinto
    for (long unsigned int i = 0; i < route.size(); i++)
    {
        for (long unsigned int j = 0; j < route[i].size(); j++)
        {
            if (route[i][j] == 2)
                cout << "*";
            else
                cout << route[i][j];
        }
        cout << endl;
    }
}

/**
 * @brief Función que comprueba los argumentos de entrada.
 *
 * @param argc Número de argumentos
 * @param argv Vector de argumentos
 * @param file Nombre del fichero
 * @param p Flag para el camino codificado
 * @param p2D Flag para el camino en 2D
 * @param debug Flag para el modo debug
 */
bool checkArgs(int argc, char *argv[], string &file, bool &p, bool &p2D, bool &debug)
{
    if (argc < 3)
    {
        cerr << "ERROR: Número de argumentos incorrecto." << endl;
        printUsageError();
        return false;
    }

    bool file_flag = false;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0)
            file_flag = true;
    }

    if (!file_flag)
    {
        cerr << "ERROR: Fichero de entrada no especificado." << endl;
        printUsageError();
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "ERROR: Fichero de entrada no especificado." << endl;
                return false;
            }

            file = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-p") == 0)
            p = true;
        else if (strcmp(argv[i], "--p2D") == 0)
            p2D = true;
        else if (strcmp(argv[i], "--debug") == 0)
            debug = true;
        else
        {
            cerr << "ERROR: Argumento no reconocido." << endl;
            printUsageError();
            return false;
        }
    }

    return true;
}

/**
 * @brief Función que lee el laberinto de un fichero.
 *
 * @param file Nombre del fichero
 * @param n Número de filas
 * @param m Número de columnas
 * @param maze Laberinto
 * @param debug Flag para el modo debug
 */
void maze_parser(string file, int &n, int &m, vector<vector<int>> &maze, bool debug)
{
    ifstream f(file);
    if (!f)
    {
        cerr << "ERROR: No se ha podido abrir el fichero de entrada: " << file << endl;
        printUsageError();
        exit(1);
    }

    f >> n >> m;

    if (debug)
    {
        cout << "DEBUG - DIMENSIONES" << endl;
        cout << "DEBUG - \tFilas: " << n << endl;
        cout << "DEBUG - \tColumnas: " << m << endl;
    }

    maze.resize(n, vector<int>(m));

    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            f >> maze[i][j];

    if (debug)
    {
        cout << "DEBUG - LABERINTO" << endl;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
                cout << maze[i][j] << " ";
            cout << endl;
        }
    }

    f.close();
}

/**
 * @brief Función que calcula la heurística de Chebyshev.
 * La heurística de Chebyshev es la distancia máxima entre las coordenadas x e y del nodo actual y el objetivo (el final del laberinto).
 * @param node Nodo actual
 * @param n Número de filas
 * @param m Número de columnas
 * @return int: Heurística de Chebyshev
 */
int chebyshev(const Node &node, int n, int m)
{
    return max(abs(node.x - (n - 1)), abs(node.y - (m - 1)));
}

/**
 * @brief Comprueba si el nodo es hoja.
 * Un nodo será hoja si es el último nodo del laberinto, es decir, si está en la posición (n-1, m-1).
 * @param node Nodo a comprobar
 * @param n Número de filas
 * @param m Número de columnas
 * @return - true si es hoja
 * @return - false si no es hoja
 */
bool isLeaf(const Node &node, int n, int m)
{
    return node.x == n - 1 && node.y == m - 1;
}

/**
 * @brief Devuelve la solución del nodo actual.
 *
 * @param node Nodo actual
 * @return int: Coste del nodo
 */
int solution(const Node &node)
{
    return node.cost; // Retorna el coste del nodo
}

/**
 * @brief Comprueba si la solución del nodo actual es mejor que la mejor solución encontrada hasta el momento.
 *
 * @param sol Solución del nodo actual
 * @param currentBest Mejor solución encontrada hasta el momento
 * @return - true si la solución es mejor
 * @return - false si no es mejor
 */
bool isBest(int sol, int currentBest)
{
    return sol < currentBest;
}

/**
 * @brief Función que expande el nodo actual en sus hijos.
 *
 * @param node Nodo actual
 * @param maze Laberinto
 * @param n Número de filas
 * @param m Número de columnas
 * @return vector<Node> Hijos del nodo actual
 */
vector<Node> expand(const Node &node, int n, int m)
{
    vector<Node> children; // Vector para almacenar los hijos del nodo actual

    // Direcciones posibles
    map<Step, tuple<int, int>> steps_inc = {
        {N, make_tuple(-1, 0)},
        {NE, make_tuple(-1, 1)},
        {E, make_tuple(0, 1)},
        {SE, make_tuple(1, 1)},
        {S, make_tuple(1, 0)},
        {SW, make_tuple(1, -1)},
        {W, make_tuple(0, -1)},
        {NW, make_tuple(-1, -1)}};

    for (auto it = steps_inc.begin(); it != steps_inc.end(); ++it) // Para cada dirección...
    {
        // Obtenemos la nueva posición
        Step move = it->first;

        // Obtenemos el incremento de fila y columna
        int incx, incy;
        tie(incx, incy) = it->second;

        // Calculamos la nueva posición
        int newX = node.x + incx;
        int newY = node.y + incy;

        // Creamos al nuevo hijo
        Node child;
        child.x = newX;
        child.y = newY;
        child.path = node.path;
        child.path.push_back(move); // Añadimos el movimiento al camino
        child.cost = node.cost + 1;

        // Lo añadimos al vector de hijos
        children.push_back(child);
    }

    // Ordenamos los hijos por heurística f(n) = cost + h
    std::sort(children.begin(), children.end(), [n, m](const Node &a, const Node &b)
              {
        int fA = a.cost + chebyshev(a, n, m);
        int fB = b.cost + chebyshev(b, n, m);
        return fA < fB; });

    return children;
}

/**
 * @brief Función que comprueba si el nodo es factible.
 * Un nodo es factible si está dentro de los límites del laberinto y no es una pared.
 * @param node Nodo actual
 * @param maze Laberinto
 * @param n Número de filas
 * @param m Número de columnas
 * @return - true si está dentro de los límites del laberinto y es un camino
 * @return - false si está fuera de los límites del laberinto o es una pared
 */
bool isFeasible(const Node &node, const vector<vector<int>> &maze, int n, int m)
{
    return node.x >= 0 && node.x < n &&
           node.y >= 0 && node.y < m &&
           maze[node.x][node.y] == 1;
}

/**
 * @brief Función que comprueba si el nodo es prometedor.
 * Un nodo es prometedor si su coste es menor que el de la mejor solución encontrada hasta el momento.
 * @param node Nodo actual
 * @param currentBest Mejor solución encontrada hasta el momento
 * @return - true si el coste del nodo es menor que la mejor solución
 * @return - false si no es menor
 */
bool isPromising(const Node &node, const Node &currentBest, int n, int m)
{
    // return node.cost < currentBest.cost; <-- Con esta heurística obtenemos casi 150k nodos visitados en el laberinto 09.
    return node.cost + chebyshev(node, n, m) < currentBest.cost; // <-- Con esta heurística (de Chebyshev) obtenemos apenas 220 nodos visitados en el laberinto 09.
}

/**
 * @brief Función que implementa el algoritmo recursivo de vuelta atrás para resolver el laberinto.
 *
 * @param maze Laberinto
 * @param n Filas
 * @param m Columnas
 * @param node Nodo actual
 * @param currentBest Mejor solución encontrada hasta el momento
 * @param stats Estructura para almacenar las estadísticas
 * @param debug Flag para el modo debug (puede afectar al rendimiento)
 * @return - void
 */
void maze_bt(vector<vector<int>> maze, int n, int m, const Node &node, Node &currentBest, Stats &stats, bool debug)
{
    string indent(node.cost * 2, ' '); // Para el modo [--debug]: Sangría proporcional al coste para identificar profundidad

    if (debug)
        cout << indent << CYAN << "🔍 Visitando (" << node.x << "," << node.y << ") con coste " << node.cost << RESET << endl;

    stats.nv++;

    if (maze[0][0] == 0)
        return;

    if (isLeaf(node, n, m))
    {
        if (debug)
            cout << indent << GREEN << "🌿 Hoja encontrada en (" << node.x << "," << node.y << ") con coste " << node.cost << RESET << endl;

        stats.nhv++;
        stats.ne++;

        if (isBest(solution(node), solution(currentBest)))
        {
            if (debug)
                cout << indent << BOLD << GREEN << "✅ Mejor solución encontrada: " << solution(node) << RESET << endl;
            currentBest.cost = solution(node); // Actualizamos el coste
            currentBest.path = node.path;      // Actualizamos el camino
        }
        return;
    }

    if (debug)
        cout << indent << YELLOW << "📂 Expandiendo nodo (" << node.x << "," << node.y << ") con coste " << node.cost << RESET << endl;

    for (Node child : expand(node, n, m))
    {
        if (debug)
            cout << indent << "↳ " << "Hijo (" << child.x << "," << child.y << ") con coste " << child.cost << endl;

        if (isFeasible(child, maze, n, m))
        {
            if (debug)
                cout << indent << "   " << GREEN << "✔️  Hijo factible" << RESET << endl;

            if (isPromising(child, currentBest, n, m))
            {
                if (debug)
                    cout << indent << "   " << CYAN << "➡️  Hijo prometedor, hacemos llamada recursiva" << RESET << endl;
                stats.ne++;

                maze[child.x][child.y] = -1; // Marcamos como visitado

                maze_bt(maze, n, m, child, currentBest, stats, debug);

                maze[child.x][child.y] = 1; // Restauramos el laberinto
            }
            else
            {
                if (debug)
                    cout << indent << "   " << RED << "⛔ Hijo no prometedor, cortamos" << RESET << endl;
                stats.nnp++;
            }
        }
        else
        {
            if (debug)
                cout << indent << "   " << RED << "❌ Hijo no factible, cortamos" << RESET << endl;
            stats.nnf++;
        }
    }
}

int main(int argc, char *argv[])
{
    string file;
    bool p = false, p2D = false, debug = false; // Flags para la salida
    int n, m;                                   // Dimensiones del laberinto.
    vector<vector<int>> maze;                   // Laberinto

    Stats stats; // Estructura para almacenar las estadísticas

    Node nodoInicial; // Nodo inicial
    nodoInicial.x = 0;
    nodoInicial.y = 0;
    nodoInicial.cost = 1;              // Inicializamos el coste a 1 porque el primer movimiento es el que se hace al entrar en el laberinto
    nodoInicial.path = vector<Step>(); // Inicializamos el camino vacío

    Node mejorNodo;           // Nodo que almacena la mejor solución encontrada
    mejorNodo.cost = INT_MAX; // Inicializamos el coste a "infinito" porque no sabemos el coste de la mejor solución

    if (!checkArgs(argc, argv, file, p, p2D, debug))
        return 1;
    else
        maze_parser(file, n, m, maze, debug);

    if (debug)
    {
        cout << "DEBUG - ARGUMENTOS" << endl;
        cout << "DEBUG - \tFichero de entrada: " << file << endl;
        cout << "DEBUG - \tp: " << p << endl;
        cout << "DEBUG - \tp2D: " << p2D << endl;
        cout << "DEBUG - \tdebug: " << debug << endl;
    }

    if (debug)
        cout << "DEBUG - INICIO DEL ALGORITMO" << endl;

    auto start = std::chrono::high_resolution_clock::now(); // Iniciamos el cronómetro
    maze_bt(maze, n, m, nodoInicial, mejorNodo, stats, debug);
    auto end = std::chrono::high_resolution_clock::now(); // Paramos el cronómetro

    if (debug)
        cout << "DEBUG - FIN DEL ALGORITMO" << endl;

    stats.t = end - start; // Tiempo en milisegundos

    printStats(mejorNodo, stats, debug); // Imprimimos los resultados

    if (p2D)
    {
        print2D(maze, mejorNodo);
    }

    if (p && mejorNodo.cost == INT_MAX)
    {
        cout << "<0>" << endl;
    }
    else if (p)
    {
        cout << "<";
        for (auto it = mejorNodo.path.begin(); it != mejorNodo.path.end(); ++it)
        {
            cout << *it + 1;
        }
        cout << ">" << endl;
    }

    return 0;
}