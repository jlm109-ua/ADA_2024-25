#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <tuple>
#include <climits>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <cstring>

using namespace std;

// Enumeración de pasos
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

// Estructura del laberinto
struct MazeStruct
{
    int n, m;
    vector<vector<int>> maze;
} mazeStruct;

// Nodo para representar cada estado
struct Node
{
    int x, y;
    int cost;
    vector<Step> path;
};

// Estadísticas
struct Stats
{
    int nv = 0;                        // Nodos visitados
    int ne = 0;                        // Nodos explorados
    int nhv = 0;                       // Nodos hoja visitados
    int nnf = 0;                       // Nodos no factibles
    int nnp = 0;                       // Nodos no prometedores
    int ppd = 0;                       // Prometedores descartados
    int msah = 0;                      // Mejor solución hoja
    int msap = 0;                      // Mejor solución pesimista
    chrono::duration<double, milli> t; // Tiempo
};

// Declaración de funciones ya proporcionadas
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

// BFS como función de búsqueda por anchura
Node bfs()
{
    int N = mazeStruct.n, M = mazeStruct.m;
    vector<vector<bool>> seen(N, vector<bool>(M, false));
    vector<vector<Step>> parentStep(N, vector<Step>(M));
    vector<vector<pair<int, int>>> parentPos(N, vector<pair<int, int>>(M));
    queue<tuple<int, int, int>> q;

    if (mazeStruct.maze[0][0] == 0)
    {
        Node fail;
        fail.cost = INT_MAX;
        return fail;
    }

    q.push({0, 0, 1});
    seen[0][0] = true;

    int di[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dj[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    while (!q.empty())
    {
        auto [i, j, d] = q.front();
        q.pop();

        if (i == N - 1 && j == M - 1)
        {
            vector<Step> path;
            int ci = i, cj = j;

            while (!(ci == 0 && cj == 0))
            {
                Step s = parentStep[ci][cj];
                path.push_back(s);
                tie(ci, cj) = parentPos[ci][cj];
            }

            reverse(path.begin(), path.end());

            Node result;
            result.x = N - 1;
            result.y = M - 1;
            result.cost = d;
            result.path = path;
            return result;
        }

        for (int dir = 0; dir < 8; dir++)
        {
            int ni = i + di[dir];
            int nj = j + dj[dir];
            if (ni >= 0 && ni < N && nj >= 0 && nj < M && mazeStruct.maze[ni][nj] == 1 && !seen[ni][nj])
            {
                seen[ni][nj] = true;
                parentStep[ni][nj] = static_cast<Step>(dir);
                parentPos[ni][nj] = {i, j};
                q.push({ni, nj, d + 1});
            }
        }
    }

    Node fail;
    fail.cost = INT_MAX;
    return fail;
}

// Funcion principal
int main(int argc, char *argv[])
{
    string file;
    bool p = false, p2D = false, debug = false;

    if (!checkArgs(argc, argv, file, p, p2D, debug))
        return 1;

    maze_parser(file, debug);

    Stats stats;
    auto start = chrono::steady_clock::now();
    Node result = bfs();
    auto end = chrono::steady_clock::now();
    stats.t = end - start;

    printStats(result, stats, debug);
    if (p2D)
        print2D(result);

    return 0;
}
