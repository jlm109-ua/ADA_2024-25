// Juan Llinares Mauri 74011239E
#include <iostream>
#include <cmath>

using namespace std;

// Recursiva. O(n)
unsigned long pow2_1(unsigned n, long long &pasos)
{
    if (n == 0)
    {
        pasos++;
        return 1;
    }
    pasos++;
    return 2 * pow2_1(n - 1, pasos);
}

// Exponenciación por cuadrados. O(log n)
unsigned long pow2_2(unsigned n, long long &pasos)
{
    if (n == 0)
    {
        pasos++;
        return 1;
    }

    if (n % 2 == 0)
    {
        pasos++;
        int mitad = pow2_2(n / 2, pasos);
        return mitad * mitad;
    }
    else
    {
        pasos++;
        return 2 * pow2_2(n - 1, pasos);
    }
}

// Iterativa. O(n^2)
unsigned long pow2_3(unsigned n, long long &pasos)
{
    pasos++;
    int result = 1;

    for (unsigned int i = 0; i < n; i++)
    {
        pasos++;
        int temp = 0;
        for (int j = 0; j < result; j++)
        {
            pasos++;
            temp += 2;
        }
        result = temp;
    }
    return result;
}

int main()
{
    long long pasos1, pasos2, pasos3 = 0; // Variables para las estadísticas.

    unsigned long result1, result2, result3 = 0; // Variables para los resultados.

    // Realizamos pruebas de n^15 a n^20
    for (int i = 1; i <= 8; i++)
    {
        pasos1 = 0;
        pasos2 = 0;
        pasos3 = 0;

        // Control de errores
        result1 = pow2_1(i, pasos1);
        if (result1 != pow(2, i))
        {
            cout << "Error en pow2_1" << endl;
            return 1;
        }

        result2 = pow2_2(i, pasos2);
        if (result2 != pow(2, i))
        {
            cout << "Error en pow2_2" << endl;
            return 1;
        }

        result3 = pow2_3(i, pasos3);
        if (result3 != pow(2, i))
        {
            cout << "Error en pow2_3" << endl;
            return 1;
        }

        cout << i << "\t" << pasos1 << "\t" << pasos2 << "\t" << pasos3 << endl;
    }

    return 0;
}