// Juan Llinares Mauri 74011239E
#include <iostream>
#include <fstream>
#include <string.h>
#include <climits>
#include <vector>

using namespace std;

/**
 * @brief Función que comprueba los argumentos de entrada.
 */
bool checkArgs(int argc, char *argv[], string &file, bool &p2D, bool &debug)
{
    if (argc < 3)
    {
        cerr << "ERROR: Número de argumentos incorrecto." << endl;
        cerr << "Uso: maze_greedy [--p2D] -f fichero_entrada --debug" << endl;
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
        cerr << "Uso: maze_greedy [--p2D] -f fichero_entrada --debug" << endl;
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
        else if (strcmp(argv[i], "--p2D") == 0)
        {
            p2D = true;
        }
        else if (strcmp(argv[i], "--debug") == 0)
        {
            debug = true;
        }
        else
        {
            cerr << "ERROR: Argumento no reconocido." << endl;
            cerr << "Uso: maze_greedy [--p2D] -f fichero_entrada --debug" << endl;
            return false;
        }
    }

    return true;
}

/**
 * @brief Función que lee el laberinto de un fichero.
 */
void maze_parser(string file, int &n, int &m, int **&maze, bool debug)
{
    ifstream f(file);
    if (!f)
    {
        cerr << "ERROR: No se ha podido abrir el fichero de entrada: " << file << endl;
        cerr << "Uso: maze_greedy [--p2D] -f fichero_entrada --debug" << endl;
        exit(1);
    }

    f >> n >> m;

    if (debug)
    {
        cout << "DEBUG - DIMENSIONES" << endl;
        cout << "DEBUG - \tFilas: " << n << endl;
        cout << "DEBUG - \tColumnas: " << m << endl;
    }

    maze = new int *[n];
    for (int i = 0; i < n; i++)
        maze[i] = new int[m];

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
 * @brief Construye el camino seguido por el algoritmo voraz.
 * @param maze Laberinto
 * @param n Filas
 * @param m Columnas
 * @return Vector con las coordenadas del camino (parcial si no hay solución completa)
 */
vector<pair<int, int>> reconstruct_path(int **maze, int n, int m)
{
    vector<pair<int, int>> path;

    // Comprobamos si la entrada es inaccesible
    if (maze[0][0] == 0)
        return path;

    int i = 0, j = 0;        // Coordenadas para iterar en el laberinto.
    path.emplace_back(i, j); // Añadir entrada

    while (i != n - 1 || j != m - 1)
    {
        bool moved = false; // Comprobamos si ha habido movimiento

        // Diagonal abajo-derecha
        if (i + 1 < n && j + 1 < m && maze[i + 1][j + 1] == 1)
        {
            i++;
            j++;
            moved = true;
        }
        // Abajo
        else if (i + 1 < n && maze[i + 1][j] == 1)
        {
            i++;
            moved = true;
        }
        // Derecha
        else if (j + 1 < m && maze[i][j + 1] == 1)
        {
            j++;
            moved = true;
        }

        // Si no se pudo hacer ningún movimiento más, terminamos
        if (!moved)
            break;

        path.emplace_back(i, j);
    }

    return path; // Devolvemos el camino recorrido
}

/**
 * @brief Muestra el laberinto con el camino marcado con '*'
 * @param maze Laberinto original
 * @param path Vector con las coordenadas del camino
 * @param n Filas
 * @param m Columnas
 */
void print_maze_with_path(int **maze, const vector<pair<int, int>> &path, int n, int m)
{
    // Creamos una matriz de caracteres para la salida
    char **output = new char *[n];

    for (int i = 0; i < n; i++)
    {
        output[i] = new char[m];
        for (int j = 0; j < m; j++)
            output[i][j] = maze[i][j] + '0'; // Convertir a carácter
    }

    // Marcamos el camino con *
    for (const auto &p : path)
        output[p.first][p.second] = '*';

    // Imprimimos la matriz
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
            cout << output[i][j];
        cout << endl;
    }

    // Liberamos memoria
    for (int i = 0; i < n; i++)
        delete[] output[i];
    delete[] output;
}

/**
 * @brief Función que implementa el algoritmo voraz para resolver el laberinto.
 * @param maze Laberinto
 * @param n Filas
 * @param m Columnas
 */
int maze_greedy(int **maze, int n, int m)
{
    // Si la entrada no es accesible devolvemos 0 directamente
    if (maze[0][0] == 0)
        return 0;

    int pasos = 1;    // Contamos con la posición inicial ya recorrida.
    int i = 0, j = 0; // Coordenadas para iterar en el laberinto.

    while (i != n - 1 || j != m - 1)
    {
        // Diagonal abajo-derecha
        if (i + 1 < n && j + 1 < m && maze[i + 1][j + 1] == 1)
        {
            i++;
            j++;
        }
        // Abajo
        else if (i + 1 < n && maze[i + 1][j] == 1)
            i++;
        // Derecha
        else if (j + 1 < m && maze[i][j + 1] == 1)
            j++;
        // Si no hay movimientos posibles, no encontramos solución
        else
            return 0;

        pasos++;
    }

    return pasos;
}

int main(int argc, char *argv[])
{
    string file;
    bool p2D = false, debug = false;
    int n, m;                   // Dimensiones del laberinto.
    int **maze = nullptr;       // Laberinto
    int **greedyPath = nullptr; // Camino que sigue el algoritmo voraz
    int resultGreedy = 0;       // Resultado del algoritmo voraz

    if (!checkArgs(argc, argv, file, p2D, debug))
        return 1;
    else
        maze_parser(file, n, m, maze, debug);

    if (debug)
    {
        cout << "DEBUG - ARGUMENTOS" << endl;
        cout << "DEBUG - \tFichero de entrada: " << file << endl;
        cout << "DEBUG - \tp2D: " << p2D << endl;
        cout << "DEBUG - \tdebug: " << debug << endl;
    }

    resultGreedy = maze_greedy(maze, n, m);

    cout << resultGreedy << endl;

    if (p2D)
    {
        vector<pair<int, int>> path = reconstruct_path(maze, n, m);
        print_maze_with_path(maze, path, n, m);
    }

    // Liberamos memoria
    for (int i = 0; i < n; i++)
    {
        delete[] maze[i];
        if (greedyPath)
            delete[] greedyPath[i];
    }
    delete[] maze;

    if (greedyPath)
        delete[] greedyPath;

    return 0;
}