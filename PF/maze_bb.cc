// Juan Llinares Mauri 74011239E
#include <iostream>
#include <fstream>
#include <map>
#include <tuple>
#include <chrono>
#include <string.h>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <cmath>

// Colores para la salida por consola en el modo [--debug]
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"

using namespace std;

/// Estructura para almacenar el laberinto.
/// - n: Número de filas
/// - m: Número de columnas
/// - maze: Laberinto (vector de vectores de enteros)
/// - - 0: Pared
/// - - 1: Camino
struct Maze
{
    int n = 0;                     // Número de filas
    int m = 0;                     // Número de columnas
    vector<vector<int>> maze = {}; // Laberinto
};

Maze mazeStruct; // Variable global para el laberinto

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
    int ppd = 0;                     // Número de nodos prometedores pero descartados
    int msah = 0;                    // Número de veces que la mejor solución se ha actualizado desde un nodo hoja
    int msap = 0;                    // Número de veces que la mejor solución se ha actualizado a partir de la cota pesimista de un nodo sin completar
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
 * @brief Función que calcula la heurística de Chebyshev.
 * La heurística de Chebyshev es la distancia máxima entre las coordenadas x e y del nodo actual y el objetivo (el final del laberinto).
 * @param node Nodo actual
 * @param n Número de filas
 * @param m Número de columnas
 * @return int: Heurística de Chebyshev
 */
int chebyshev(const Node &node)
{
    return max(abs(node.x - (mazeStruct.n - 1)), abs(node.y - (mazeStruct.m - 1)));
}

/**
 * @brief Función que calcula la heurística de Manhattan.
 * La heurística de Manhattan es la suma de las diferencias absolutas entre las coordenadas del nodo actual y el final del laberinto.
 * @param node Nodo actual
 * @return int: Heurística de Manhattan
 */
int euclidean(const Node &node)
{
    return sqrt(pow(node.x - (mazeStruct.n - 1), 2) + pow(node.y - (mazeStruct.m - 1), 2));
}

vector<vector<int>> dist; // Distancias desde el final del laberinto a cada celda

/**
 * @brief Función que precomputa las distancias desde el final del laberinto a cada celda.
 * Utiliza una búsqueda en anchura para calcular las distancias.
 */
void precomputeDistances()
{
    int N = mazeStruct.n, M = mazeStruct.m;

    dist.assign(N, vector<int>(M, INT_MAX)); // Inicializamos la matriz de distancias a "infinito"
    queue<pair<int, int>> q;                 // Cola para la búsqueda en anchura

    if (mazeStruct.maze[N - 1][M - 1] == 1) // Si la posición final es accesible
    {
        dist[N - 1][M - 1] = 0; // La distancia a sí mismo es 0
        q.push({N - 1, M - 1}); // Añadimos la posición final a la cola
    }

    // Direcciones posibles
    int di[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dj[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    while (!q.empty())
    {
        auto [i, j] = q.front(); // Obtenemos la posición actual
        q.pop();

        // Expandimos el nodo actual
        for (int k = 0; k < 8; ++k)
        {
            int ni = i + di[k], nj = j + dj[k];
            if (ni >= 0 && ni < N && nj >= 0 && nj < M && mazeStruct.maze[ni][nj] == 1 && dist[ni][nj] > dist[i][j] + 1)
            {
                dist[ni][nj] = dist[i][j] + 1; // Actualizamos la distancia
                q.push({ni, nj});              // Añadimos la nueva posición
            }
        }
    }
}

/**
 * @brief Función que calcula la cota optimista de un nodo.
 *
 * @param node Nodo actual
 * @param maze Laberinto
 * @return int: Valor de la cota optimista
 */
int optimist(const Node &node)
{
    // return chebyshev(node); // <-- tarda mucho para laberintos grandes
    // return euclidean(node); // <-- tarda mucho para laberintos grandes
    return dist[node.x][node.y]; // Devolvemos la distancia precalculada desde el nodo actual hasta el final del laberinto
}

/**
 * @brief Sobrecarga del operador < para comparar nodos.
 * Se sobrecarga el operador < para poder usar la cola de prioridad con los nodos, ordenando por coste.
 * @param a Nodo a
 * @param b Nodo b
 */
bool operator<(const Node &a, const Node &b)
{
    return (a.cost + optimist(a)) > (b.cost + optimist(b)); // Ordenamos los nodos por coste (menor coste primero)
}

/**
 * @brief Estructura para comparar nodos en la cola de prioridad.
 */
struct compareNodes
{
    /**
     * * @brief Sobrecarga del operador () para comparar nodos.
     *
     * @param a Nodo a
     * @param b Nodo b
     * @return - true si el coste de a es mayor que el de b
     * @return - false si el coste de a es menor o igual que el de b
     */
    bool operator()(const Node &a, const Node &b)
    {
        return a.cost > b.cost; // Ordenamos los nodos por coste (menor coste primero)
    }
};

/**
 * @brief Función que imprime el modo de uso.
 */
void printUsageError()
{
    cerr << "ERROR: Uso: maze_bb [-p] [--p2D] -f fichero_entrada --debug" << endl;
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
        cout << "Nodos visitados: " << stats.nv << "\nNodos explorados: " << stats.ne << "\nNodos hoja visitados: " << stats.nhv << "\nNodos no factibles: " << stats.nnf << "\nNodos no prometedores: " << stats.nnp << "\nNodos prometedores descartados: " << stats.ppd << "\nMejor solución actualizada (hoja): " << stats.msah << "\nMejor solución actualizada (pesimista): " << stats.msap << endl;
        cout << "Tiempo de ejecución: " << stats.t.count() << " ms" << endl;
    }
    else
    {
        if (mejorNodo.cost == INT_MAX)
            cout << 0 << endl;
        else
            cout << mejorNodo.cost << endl;
        cout << stats.nv << " " << stats.ne << " " << stats.nhv << " " << stats.nnf << " " << stats.nnp << " " << stats.ppd << " " << stats.msah << " " << stats.msap << endl;
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
void print2D(Node mejorNodo)
{
    vector<vector<int>> route = mazeStruct.maze; // Copia del laberinto para marcar el camino

    auto position = make_tuple(0, 0); // Posición inicial

    if (mejorNodo.cost != INT_MAX)
        route[get<0>(position)][get<1>(position)] = 2; // Marcamos la posición inicial
    // if (mejorNodo.path.size() != 0) // <-- Esta condición no funciona porque para el laberinto 01 sólo hay una celda y no se podría marcar como visitada pues mejorNodo.path estaría vacío aunque hubiese solución
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
void maze_parser(string file, bool debug)
{
    ifstream f(file);
    if (!f)
    {
        cerr << "ERROR: No se ha podido abrir el fichero de entrada: " << file << endl;
        printUsageError();
        exit(1);
    }

    f >> mazeStruct.n >> mazeStruct.m;

    if (debug)
    {
        cout << "DEBUG - DIMENSIONES" << endl;
        cout << "DEBUG - \tFilas: " << mazeStruct.n << endl;
        cout << "DEBUG - \tColumnas: " << mazeStruct.m << endl;
    }

    mazeStruct.maze.resize(mazeStruct.n, vector<int>(mazeStruct.m));

    for (int i = 0; i < mazeStruct.n; i++)
        for (int j = 0; j < mazeStruct.m; j++)
            f >> mazeStruct.maze[i][j];

    if (debug)
    {
        cout << "DEBUG - LABERINTO" << endl;
        for (int i = 0; i < mazeStruct.n; i++)
        {
            for (int j = 0; j < mazeStruct.m; j++)
                cout << mazeStruct.maze[i][j] << " ";
            cout << endl;
        }
    }

    f.close();
}

/**
 * @brief Comprueba si el nodo es hoja.
 * Un nodo será hoja si es el último nodo del laberinto, es decir, si está en la posición (n-1, m-1).
 * @param node Nodo a comprobar
 * @return - true si es hoja
 * @return - false si no es hoja
 */
bool isLeaf(const Node &node)
{
    return node.x == mazeStruct.n - 1 && node.y == mazeStruct.m - 1;
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
 * @brief Función que expande el nodo actual en sus hijos.
 *
 * @param node Nodo actual
 * @param maze Laberinto
 * @return vector<Node> Hijos del nodo actual
 */
vector<Node> expand(const Node &node)
{
    int n = mazeStruct.n;  // Número de filas
    int m = mazeStruct.m;  // Número de columnas
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
        int fA = a.cost + chebyshev(a);
        int fB = b.cost + chebyshev(b);
        return fA < fB; });

    return children;
}

/**
 * @brief Comprueba si la solución del nodo actual es mejor que la mejor solución encontrada hasta el momento.
 *
 * @param sol Solución del nodo actual
 * @param currentBest Mejor solución encontrada hasta el momento
 * @return - true si la solución es mejor
 * @return - false si no es mejor
 */
bool isBetter(int sol, int currentBest)
{
    return sol < currentBest;
}

/**
 * @brief Función que comprueba si el nodo es factible.
 * Un nodo es factible si está dentro de los límites del laberinto y no es una pared.
 * @param node Nodo actual
 * @return - true si está dentro de los límites del laberinto y es un camino
 * @return - false si está fuera de los límites del laberinto o es una pared
 */
bool isFeasible(const Node &node)
{
    return node.x >= 0 && node.x < mazeStruct.n &&
           node.y >= 0 && node.y < mazeStruct.m &&
           mazeStruct.maze[node.x][node.y] == 1;
}

/**
 * @brief Función que comprueba si el nodo es prometedor.
 * Un nodo es prometedor si su coste más la solución optimista es menor que el de la mejor solución encontrada hasta el momento.
 * @param node Nodo actual
 * @param currentBest Mejor solución encontrada hasta el momento
 * @return - true si el coste del nodo es menor que la mejor solución
 * @return - false si no es menor
 */
bool isPromising(const Node &node, const Node &currentBest)
{
    return (node.cost + optimist(node)) < currentBest.cost;
}

/**
 * @brief Función que inicializa el nodo inicial.
 * Inicializa el nodo inicial con la posición (0,0), coste 1 y camino vacío.
 * @return Node: Nodo inicial
 */
Node initNode()
{
    Node initial = Node();         // Inicializamos el nodo inicial
    initial.x = 0;                 // Inicializamos la posición x
    initial.y = 0;                 // Inicializamos la posición y
    initial.cost = 0;              // Inicializamos el coste a 0
    initial.path = vector<Step>(); // Inicializamos el camino vacío

    return initial; // Retornamos el nodo inicial
}

/**
 * @brief Función que implementa la búsqueda en anchura para resolver el laberinto.
 * La búsqueda en anchura es un algoritmo de búsqueda que explora todos los nodos a la misma profundidad antes de pasar a la siguiente.
 * @return Node: Nodo que contiene la solución del laberinto o un nodo vacío si no hay solución.
 */
Node bfs()
{
    int N = mazeStruct.n, M = mazeStruct.m;
    vector<vector<bool>> seen(N, vector<bool>(M, false));                   // Matriz para marcar los nodos visitados
    vector<vector<Step>> parentStep(N, vector<Step>(M));                    // Camino codificado
    vector<vector<pair<int, int>>> parentPos(N, vector<pair<int, int>>(M)); // Posición padre
    queue<tuple<int, int, int>> q;                                          // x, y y coste

    if (mazeStruct.maze[0][0] == 0) // Si la posición inicial no es accesible
    {
        Node fail;
        fail.cost = INT_MAX;
        return fail;
    }

    q.push({0, 0, 1}); // Añadimos el nodo inicial a la cola
    seen[0][0] = true; // Marcamos el nodo inicial como visitado

    // Direcciones posibles
    int di[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dj[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    while (!q.empty())
    {
        auto [i, j, d] = q.front();
        q.pop();

        if (i == N - 1 && j == M - 1) // Si hemos llegado al final del laberinto
        {
            // Reconstruimos el camino
            vector<Step> path;
            int ci = i, cj = j;

            while (!(ci == 0 && cj == 0))
            {
                Step s = parentStep[ci][cj];
                path.push_back(s);
                auto [pi, pj] = parentPos[ci][cj];
                ci = pi;
                cj = pj;
            }

            reverse(path.begin(), path.end()); // Invertimos el camino para que esté en el orden correcto

            // Creamos el nodo resultado
            Node result;
            result.x = N - 1;
            result.y = M - 1;
            result.cost = d;
            result.path = move(path);

            return result;
        }

        // Expandimos el nodo actual
        for (int k = 0; k < 8; ++k)
        {
            int ni = i + di[k], nj = j + dj[k];
            if (ni >= 0 && ni < N && nj >= 0 && nj < M && mazeStruct.maze[ni][nj] == 1 && !seen[ni][nj])
            {
                seen[ni][nj] = true;
                parentStep[ni][nj] = Step(k);
                parentPos[ni][nj] = {i, j};
                q.push({ni, nj, d + 1});
            }
        }
    }

    // Si no se ha encontrado solución
    Node fail;
    fail.cost = INT_MAX;
    return fail;
}

/**
 * @brief Función que calcula la cota pesimista de un nodo.
 * La cota pesimista es el coste mínimo que se puede alcanzar desde el nodo actual hasta el final del laberinto.
 * @param node Nodo actual
 * @return Node: Nodo pesimista
 */
Node pessimist(const Node &node, bool debug)
{
    Node pesimista = node;    // Inicializamos el nodo pesimista
    pesimista.cost = INT_MAX; // Inicializamos el coste a "infinito" porque no sabemos el coste de la mejor solución

    Node bfsNode = bfs();

    if (bfsNode.cost < 0)
    {
        if (debug)
            cout << "DEBUG - " << RED << "Búsqueda en Anchura NO ha enontrado solución" << RESET << endl;
    }
    else
    {
        pesimista = bfsNode; // Actualizamos el nodo pesimista con la solución de la búsqueda en anchura
        if (debug)
            cout << "DEBUG - " << GREEN << "Búsqueda en Anchura ha encontrado solución con coste: " << bfsNode.cost << RESET << endl;
    }

    return pesimista; // Devolvemos el nodo pesimista
}

/**
 * @brief Función que implementa el algoritmo de búsqueda en profundidad con poda por cotas.
 * El algoritmo busca la mejor solución al laberinto utilizando una cola de prioridad para explorar los nodos.
 * @param currentBest Mejor solución encontrada hasta el momento
 * @param stats Estructura con las estadísticas
 * @param debug Flag para el modo debug
 * @return Node: Mejor solución encontrada
 */
void maze_bb(Node &currentBest, Stats &stats, bool debug)
{

    if (debug)
        cout << "DEBUG - INICIALIZANDO DISTANCIAS" << endl;

    precomputeDistances();                              // Precomputamos las distancias desde el final del laberinto a cada celda
    Node initial = initNode();                          // Inicializamos el nodo inicial
    currentBest = pessimist(initial, debug);            // Inicializamos la mejor solución (pesimista)
    priority_queue<Node, vector<Node>, compareNodes> q; // Cola de prioridad para almacenar los nodos a explorar

    q.push(initial); // Añadimos el nodo inicial a la cola
    stats.nv++;      // Contamos el nodo inicial como visitado

    if (mazeStruct.maze[0][0] == 0) // Si la posición inicial no es accesible
    {
        currentBest.cost = INT_MAX;
        return;
    }

    while (!q.empty())
    {
        Node n = q.top(); // Obtenemos el nodo con menor coste
        q.pop();          // Lo eliminamos de la cola

        string indent(n.cost * 2, ' '); // Sangría proporcional al coste

        if (debug)
            cout << indent << CYAN << "🔍 Evaluando nodo (" << n.x << "," << n.y << ") con coste " << n.cost << RESET << endl;

        if (!isPromising(n, currentBest))
        {
            stats.ppd++;
            if (debug)
                cout << indent << RED << "⛔ Nodo no prometedor, se descarta" << RESET << endl;
            continue;
        }

        if (isLeaf(n))
        {
            stats.nhv++;
            if (debug)
                cout << indent << GREEN << "🌿 Nodo hoja alcanzado en (" << n.x << "," << n.y << ") con solución " << solution(n) << RESET << endl;

            if (isBetter(solution(n), solution(currentBest)))
            {
                currentBest = n;
                stats.msah++;
                if (debug)
                    cout << indent << BOLD << GREEN << "✅ ¡Nueva mejor solución encontrada desde hoja!: " << solution(n) << RESET << endl;
            }
            continue;
        }

        stats.nv++;
        if (debug)
            cout << indent << YELLOW << "📂 Expandiendo nodo (" << n.x << "," << n.y << ")" << RESET << endl;

        for (Node a : expand(n))
        {
            string childIndent(a.cost * 2, ' '); // Sangría para el hijo

            if (debug)
                cout << childIndent << "↳ Hijo (" << a.x << "," << a.y << ") con coste " << a.cost << endl;

            if (isFeasible(a))
            {
                if (debug)
                    cout << childIndent << GREEN << "✔️  Hijo factible" << RESET << endl;

                if (isBetter(solution(pessimist(a, debug)), solution(currentBest)))
                {
                    currentBest = a;
                    stats.msap++;
                    if (debug)
                        cout << childIndent << BOLD << GREEN << "🌟 Mejor solución actualizada por cota pesimista: " << solution(pessimist(a, debug)) << RESET << endl;
                }

                if (isPromising(a, currentBest))
                {
                    q.push(a);
                    stats.ne++;
                    if (debug)
                        cout << childIndent << CYAN << "➡️  Hijo prometedor, se añade a la cola" << RESET << endl;
                }
                else
                {
                    stats.nnp++;
                    if (debug)
                        cout << childIndent << RED << "⛔ Hijo no prometedor, descartado" << RESET << endl;
                }
            }
            else
            {
                stats.nnf++;
                if (debug)
                    cout << childIndent << RED << "❌ Hijo no factible, descartado" << RESET << endl;
            }
        }
    }

    if (debug)
        cout << BOLD << GREEN << "🎯 Mejor solución final: " << solution(currentBest) << RESET << endl;
}

int main(int argc, char *argv[])
{
    string file;
    bool p = false, p2D = false, debug = false; // Flags para la salida

    Stats stats; // Estructura para almacenar las estadísticas

    Node mejorNodo;           // Nodo que almacena la mejor solución encontrada
    mejorNodo.cost = INT_MAX; // Inicializamos el coste a "infinito" porque no sabemos el coste de la mejor solución

    if (!checkArgs(argc, argv, file, p, p2D, debug))
        return 1;
    else
        maze_parser(file, debug);

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

    maze_bb(mejorNodo, stats, debug);

    if (mejorNodo.cost != INT_MAX)
        mejorNodo.cost += 1; // Añadimos el coste del primer movimiento al coste total

    auto end = std::chrono::high_resolution_clock::now(); // Paramos el cronómetro

    if (debug)
        cout << "DEBUG - FIN DEL ALGORITMO" << endl;

    stats.t = (end - start) * 1000; // Tiempo en milisegundos

    printStats(mejorNodo, stats, debug); // Imprimimos los resultados

    if (p2D)
    {
        print2D(mejorNodo);
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