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
bool checkArgs(int argc, char *argv[], string &file, bool &ignore_naive, bool &p2D, bool &t, bool &debug)
{
    if (argc < 3)
    {
        cerr << "ERROR: Número de argumentos incorrecto." << endl;
        cerr << "Uso: maze [-t] [--p2D] [--ignore-naive] -f fichero_entrada --debug" << endl;
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
        cerr << "Uso: maze [-t] [--p2D] [--ignore-naive] -f fichero_entrada --debug" << endl;
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
        else if (strcmp(argv[i], "--ignore-naive") == 0)
        {
            ignore_naive = true;
        }
        else if (strcmp(argv[i], "--p2D") == 0)
        {
            p2D = true;
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            t = true;
        }
        else if (strcmp(argv[i], "--debug") == 0)
        {
            debug = true;
        }
        else
        {
            cerr << "ERROR: Argumento no reconocido." << endl;
            cerr << "Uso: maze [-t] [--p2D] [--ignore-naive] -f fichero_entrada --debug" << endl;
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
        cerr << "Uso: maze [-t] [--p2D] [--ignore-naive] -f fichero_entrada --debug" << endl;
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
 * @brief Función que reconstruye el camino desde el origen al destino usando la matriz de memoización.
 * @param maze Laberinto
 * @param n Filas
 * @param m Columnas
 * @param memo Matriz de memoización
 * @return Vector con las coordenadas del camino
 */
vector<pair<int, int>> reconstruct_path(int **maze, int n, int m, int **memo)
{
    // Vector de coordenadas del camino
    vector<pair<int, int>> path;

    if (memo[0][0] == -1)
        return path; // No hay camino

    int i = 0, j = 0;
    path.emplace_back(i, j); // Añadimos el origen al vector de coordenadas

    while (i != n - 1 || j != m - 1)
    {
        int current = memo[i][j] - 1;

        // Probamos movimientos posibles en orden preferente
        if (j + 1 < m && maze[i][j + 1] == 1 && memo[i][j + 1] == current)
            j++; // Derecha
        else if (i + 1 < n && maze[i + 1][j] == 1 && memo[i + 1][j] == current)
            i++; // Abajo
        else if (i + 1 < n && j + 1 < m && maze[i + 1][j + 1] == 1 && memo[i + 1][j + 1] == current)
        {
            // Diagonal
            i++;
            j++;
        }

        path.emplace_back(i, j); // Añadimos la coordenada al vector
    }

    return path;
}

/**
 * @brief Función que muestra el laberinto con el camino marcado con *
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
 * @brief Función que imprime la tabla de memoización
 * @param memo Matriz de memoización
 * @param n Filas
 * @param m Columnas
 */
void print_memoization_table(int **memo, int n, int m)
{
    cout << "Memoization table:" << endl;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            if (memo[i][j] == -2) // No se ha calculado
                cout << "-";
            else if (memo[i][j] == -1) // Inaccesible
                cout << "X";
            else // Valor almacenado durante la resolución
                cout << memo[i][j];

            if (j < m - 1)
                cout << " ";
        }
        cout << endl;
    }
}

/**
 * @brief Función que imprime la tabla iterativa
 * @param dp Tabla de programación dinámica
 * @param n Filas
 * @param m Columnas
 */
void print_iterative_table(int **dp, int n, int m)
{
    cout << "Iterative table:" << endl;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            if (dp[i][j] == -2) // No se ha calculado
                cout << "-";
            else if (dp[i][j] == -1) // Inaccesible
                cout << "X";
            else // Valor almacenado durante la resolución
                cout << dp[i][j];

            if (j < m - 1)
                cout << " ";
        }
        cout << endl;
    }
}

/**
 * @brief Función recursiva naive.
 * @param maze Laberinto
 * @param n Filas
 * @param m Columnas
 * @param i Fila actual
 * @param j Columna actual
 * @return Longitud del camino más corto desde (i,j) hasta (n-1,m-1), o -1 si no hay camino
 */
int maze_naive_helper(int **maze, int n, int m, int i, int j)
{
    // Si estamos fuera de los límites o en una celda bloqueada
    if (i >= n || j >= m || maze[i][j] == 0)
        return -1;

    // Si hemos llegado al final
    if (i == n - 1 && j == m - 1)
        return 1;

    // Intentamos movimientos posibles:
    int right = maze_naive_helper(maze, n, m, i, j + 1);        // derecha
    int down = maze_naive_helper(maze, n, m, i + 1, j);         // abajo
    int diagonal = maze_naive_helper(maze, n, m, i + 1, j + 1); // diagonal

    // Encontramos el mínimo camino válido entre los posibles
    int min_path = -1;

    if (right != -1) // Si el camino a la derecha es válido, lo tomamos como mínimo
        min_path = right;

    if (down != -1)                            // Si el camino hacia abajo es válido, lo tomamos como mínimo
        if (min_path == -1 || down < min_path) // Si no hay camino o el camino hacia abajo es más corto, lo tomamos como mínimo
            min_path = down;

    if (diagonal != -1)                            // Si el camino en diagonal es válido, lo tomamos como mínimo
        if (min_path == -1 || diagonal < min_path) // Si no hay camino o el camino en diagonal es más corto, lo tomamos como mínimo
            min_path = diagonal;

    // Si al menos un camino es válido, sumamos 1 al resultado
    if (min_path != -1)
        return 1 + min_path;

    return -1;
}

/**
 * @brief Función wrapper recursiva para la versión naive.
 * @return Longitud del camino más corto o -1 si no hay camino.
 */
int maze_naive(int **maze, int n, int m)
{
    // Comprobamos si el origen o el final son inaccesibles antes de hacer nada.
    if (maze[0][0] == 0 || maze[n - 1][m - 1] == 0)
        return -1;

    return maze_naive_helper(maze, n, m, 0, 0);
}

/**
 * @brief Función wrapper recursiva con almacén (memoización).
 * @param maze Laberinto
 * @param n Filas
 * @param m Columnas
 * @param i Fila actual
 * @param j Columna actual
 * @param memo Matriz de memoización
 * @return Longitud del camino más corto desde (i,j) hasta (n-1,m-1), o -1 si no hay camino
 */
int maze_memo_helper(int **maze, int n, int m, int i, int j, int **memo)
{
    // Si estamos fuera de los límites o en una celda bloqueada
    if (i >= n || j >= m || maze[i][j] == 0)
        return -1;

    if (memo[i][j] != -2)
        return memo[i][j];

    // Si hemos llegado al final
    if (i == n - 1 && j == m - 1)
    {
        memo[i][j] = 1;
        return 1;
    }

    // Intentamos movimientos posibles:
    int right = maze_memo_helper(maze, n, m, i, j + 1, memo);        // derecha
    int down = maze_memo_helper(maze, n, m, i + 1, j, memo);         // abajo
    int diagonal = maze_memo_helper(maze, n, m, i + 1, j + 1, memo); // diagonal

    // Encontramos el mínimo camino válido entre los posibles
    int min_path = -1;

    if (right != -1) // Si el camino a la derecha es válido, lo tomamos como mínimo
        min_path = right;

    if (down != -1)                            // Si el camino hacia abajo es válido, lo tomamos como mínimo
        if (min_path == -1 || down < min_path) // Si no hay camino o el camino hacia abajo es más corto, lo tomamos como mínimo
            min_path = down;

    if (diagonal != -1)                            // Si el camino en diagonal es válido, lo tomamos como mínimo
        if (min_path == -1 || diagonal < min_path) // Si no hay camino o el camino en diagonal es más corto, lo tomamos como mínimo
            min_path = diagonal;

    // Si al menos un camino es válido, sumamos 1 al resultado
    if (min_path != -1)
        memo[i][j] = 1 + min_path;
    else
        memo[i][j] = -1;

    return memo[i][j];
}

/**
 * @brief Función recursiva con almacén (memoización).
 */
int maze_memo(int **maze, int n, int m, int **&memo)
{
    // Comprobamos si el origen o el final son inaccesibles antes de hacer nada.
    if (maze[0][0] == 0 || maze[n - 1][m - 1] == 0)
        return -1;

    // Inicializamos la matriz de memoización
    memo = new int *[n];
    for (int i = 0; i < n; i++)
    {
        memo[i] = new int[m];
        for (int j = 0; j < m; j++)
            memo[i][j] = -2;
    }

    return maze_memo_helper(maze, n, m, 0, 0, memo);
}

/**
 * @brief Función iterativa con programación dinámica para resolver el laberinto
 * @param maze Laberinto
 * @param n Filas
 * @param m Columnas
 * @param dp Tabla de programación dinámica
 * @return Longitud del camino más corto o -1 si no hay solución
 */
int maze_it_matrix(int **maze, int n, int m, int **&dp)
{
    // Comprobamos si el origen o el final son inaccesibles antes de hacer nada.
    if (maze[0][0] == 0 || maze[n - 1][m - 1] == 0)
    {
        return -1;
    }

    // Creamos la tabla de programación dinámica
    dp = new int *[n];
    for (int i = 0; i < n; i++)
    {
        dp[i] = new int[m];
        for (int j = 0; j < m; j++)
        {
            dp[i][j] = -1; // Inicializamos a -1 (no accesible)
        }
    }

    // El destino tiene distancia 1 (contamos las celdas)
    dp[n - 1][m - 1] = 1;

    // Llenamos la tabla desde abajo hacia arriba
    for (int i = n - 1; i >= 0; i--)
    {
        for (int j = m - 1; j >= 0; j--)
        {
            // Si es la celda destino o está bloqueada, ya está procesada
            if ((i == n - 1 && j == m - 1) || maze[i][j] == 0)
            {
                continue;
            }

            // Posibles movimientos
            int right = (j + 1 < m) ? dp[i][j + 1] : -1;
            int down = (i + 1 < n) ? dp[i + 1][j] : -1;
            int diag = (i + 1 < n && j + 1 < m) ? dp[i + 1][j + 1] : -1;

            // Encontramos el mínimo válido
            int min_path = -1;
            if (right != -1)
                min_path = right;
            if (down != -1 && (min_path == -1 || down < min_path))
                min_path = down;
            if (diag != -1 && (min_path == -1 || diag < min_path))
                min_path = diag;

            // Si hay al menos un camino válido
            if (min_path != -1)
            {
                dp[i][j] = 1 + min_path;
            }
        }
    }

    int result = dp[0][0];

    return result;
}

/**
 * @brief Función iterativa con mejor complejidad espacial: usa 2 vectores en lugar de una matriz
 * @param maze Laberinto
 * @param n Filas
 * @param m Columnas
 * @return Longitud del camino más corto o -1 si no hay solución
 */
int maze_it_vector(int **maze, int n, int m)
{
    // Comprobamos si el origen o el final son inaccesibles antes de hacer nada.
    if (maze[0][0] == 0 || maze[n - 1][m - 1] == 0)
        return -1;

    // Usamos solo dos vectores en lugar de una matriz
    vector<int> current_row(m, -1); // Fila actual
    vector<int> next_row(m, -1);    // Fila siguiente

    // Inicializamos la última fila (caso base)
    next_row[m - 1] = 1;

    // Llenamos la tabla desde abajo hacia arriba
    for (int i = n - 1; i >= 0; i--)
    {
        for (int j = m - 1; j >= 0; j--)
        {
            // La celda destino ya está inicializada
            if (i == n - 1 && j == m - 1)
            {
                current_row[j] = 1;
                continue;
            }

            // Si la celda actual está bloqueada
            if (maze[i][j] == 0)
            {
                current_row[j] = -1;
                continue;
            }

            // Posibles movimientos (derecha usa current_row, abajo/diagonal usan next_row)
            int right = (j + 1 < m) ? current_row[j + 1] : -1;
            int down = (i + 1 < n) ? next_row[j] : -1;
            int diag = (i + 1 < n && j + 1 < m) ? next_row[j + 1] : -1;

            // Encontramos el mínimo válido
            int min_path = -1;
            if (right != -1)
                min_path = right;
            if (down != -1 && (min_path == -1 || down < min_path))
                min_path = down;
            if (diag != -1 && (min_path == -1 || diag < min_path))
                min_path = diag;

            current_row[j] = (min_path != -1) ? 1 + min_path : -1;
        }

        // Actualizamos next_row para la siguiente iteración
        if (i != 0)
        {
            next_row = current_row;
            fill(current_row.begin(), current_row.end(), -1);
        }
    }

    return current_row[0];
}

int main(int argc, char *argv[])
{
    string file;
    bool ignore_naive, p2D, debug, t = false;
    int n, m;               // Dimensiones del laberinto.
    int **maze = nullptr;   // Laberinto.
    int **memo = nullptr;   // Matriz de memoización.
    int **dp = nullptr;     // Tabla de programación dinámica.
    int resultNaive = 0;    // Resultado del algoritmo naive.
    int resultMemo = 0;     // Resultado del algoritmo con memoización.
    int resultIt = 0;       // Resultado del algoritmo iterativo.
    int resultItVector = 0; // Resultados de la versión iterativa con vector.

    if (!checkArgs(argc, argv, file, ignore_naive, p2D, t, debug))
        return 1;
    else
        maze_parser(file, n, m, maze, debug);

    if (debug)
    {
        cout << "DEBUG - ARGUMENTOS" << endl;
        cout << "DEBUG - \tFichero de entrada: " << file << endl;
        cout << "DEBUG - \tignore-naive: " << ignore_naive << endl;
        cout << "DEBUG - \tp2D: " << p2D << endl;
        cout << "DEBUG - \tt: " << t << endl;
        cout << "DEBUG - \tdebug: " << debug << endl;
    }

    if (!ignore_naive) // Si --ignore-naive, no ejecutamos el algoritmo naive.
    {
        resultNaive = maze_naive(maze, n, m);

        if (resultNaive < 0)
            resultNaive = 0;
    }

    resultMemo = maze_memo(maze, n, m, memo);

    if (resultMemo < 0)
        resultMemo = 0;

    resultIt = maze_it_matrix(maze, n, m, dp);

    if (resultIt < 0)
        resultIt = 0;

    resultItVector = maze_it_vector(maze, n, m);

    if (resultItVector < 0)
        resultItVector = 0;

    if (ignore_naive)
        cout << "- " << resultMemo << " " << resultIt << " " << resultItVector << endl;
    else
        cout << resultNaive << " " << resultMemo << " " << resultIt << " " << resultItVector << endl;

    if (p2D && resultMemo > 0)
    {
        vector<pair<int, int>> path = reconstruct_path(maze, n, m, memo);
        print_maze_with_path(maze, path, n, m);
    }
    else if (p2D && resultMemo == 0)
    {
        cout << "0" << endl;
    }

    if (t)
    {
        print_memoization_table(memo, n, m);
        print_iterative_table(dp, n, m);
    }

    // Liberamos memoria
    for (int i = 0; i < n; i++)
    {
        delete[] maze[i];
        if (memo)
            delete[] memo[i];
    }
    delete[] maze;

    if (memo)
        delete[] memo;

    if (dp)
        delete[] dp;

    return 0;
}