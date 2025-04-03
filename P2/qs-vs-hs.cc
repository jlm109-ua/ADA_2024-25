// JUAN LLINARES MAURI 74011239E

/*
ADA. 2024-25
Practice 2: "Empirical analysis by means of program-steps account of two sorting algorithms: Middle-Quicksort and Heapsort."
*/
#include <unistd.h>
#include <iostream>
#include <math.h>
#include <iomanip> // Lo uso para setprecision en el último cout

using namespace std;

int REPETICIONES = 30;

//--------------------------------------------------------------
// Middle-Quicksort:
// The algorithm selects the middle element of the array as the "pivot".
// In a process called "partitioning", it rearranges the elements so that
// all elements smaller than the pivot are placed to its left, and
// all elements greater than the pivot are placed to its right.
// The process is then repeated recursively on the two resulting
// subarrays (left and right of the pivot).
//--------------------------------------------------------------

void middle_QuickSort(int *v, long left, long right, long long &pasos)
{
    long i, j;
    int pivot;
    pasos++;
    if (left < right)
    {
        pasos++;
        i = left;
        j = right;
        pivot = v[(i + j) / 2];
        // pivot based partitioning:
        do
        {
            while (v[i] < pivot)
            {
                pasos++;
                i++;
            }
            while (v[j] > pivot)
            {
                j--;
                pasos++;
            }
            if (i <= j)
            {
                pasos++;
                swap(v[i], v[j]);
                i++;
                j--;
            }
        } while (i <= j);
        // Repeat for each non-empty subarray:
        if (left < j)
            middle_QuickSort(v, left, j, pasos);
        if (i < right)
            middle_QuickSort(v, i, right, pasos);
    }
}

//--------------------------------------------------------------
// Heapsort:
// The algorithm works by repeatedly selecting the largest remaining element
// and placing it at the end of the vector in its correct position.
//
// To efficiently select the largest element, it builds a max-heap.
//
// The sink procedure is used for heap construction (or reconstruction).
//--------------------------------------------------------------

void sink(int *v, size_t n, size_t i, long long &pasos)
// Sink an element (indexed by i) in a tree to maintain the heap property.
// n is the size of the heap.
{
    size_t largest;
    size_t l, r; // indices of left and right childs

    do
    {
        pasos++;
        largest = i;   // Initialize largest as root
        l = 2 * i + 1; // left = 2*i + 1
        r = 2 * i + 2; // right = 2*i + 2

        // If the left child exists and is larger than the root
        if (l < n && v[l] > v[largest])
        {
            largest = l;
            pasos++;
        }

        // If the right child exists and is larger than the largest so far
        if (r < n && v[r] > v[largest])
        {
            largest = r;
            pasos++;
        }

        // If the largest is still the root, the process is done
        if (largest == i)
        {
            pasos++;
            break;
        }

        // Otherwise, swap the new largest with the current node i and repeat the process with the children
        swap(v[i], v[largest]);
        i = largest;
    } while (true);
}

//--------------------------------------------------------------
// Heapsort algorithm (ascending sorting)
void heapSort(int *v, size_t n, long long &pasos)
{
    // Build a max-heap with the input array ("heapify"):
    // Starting from the last non-leaf node (right to left), sink each element to construct the heap.
    for (size_t i = n / 2 - 1; true; i--)
    {
        pasos++;
        sink(v, n, i, pasos);
        if (i == 0)
            break; // As size_t is an unsigned type
    }

    // At this point, we have a max-heap. Now, sort the array:
    // Repeatedly swap the root (largest element) with the last element and rebuild the heap.
    for (size_t i = n - 1; i > 0; i--)
    {
        pasos++;
        // Move the root (largest element) to the end by swapping it with the last element.
        swap(v[0], v[i]);
        // Rebuild the heap by sinking the new root element.
        // Note that the heap size is reduced by one in each iteration (so the element moved to the end stays there)
        sink(v, i, 0, pasos);
        // The process ends when the heap has only one element, which is the smallest and remains at the beginning of the array.
    }
}

int main()
{
    long long pasosHS_RA = 0;  // Número de pasos para Heapsort con Arrays Aleatorios
    long long pasosQS_RA = 0;  // Número de pasos para Quicksort con Arrays Aleatorios
    long long pasosHS_SA = 0;  // Número de pasos para Heapsort con Arrays Ordenados
    long long pasosQS_SA = 0;  // Número de pasos para Quicksort con Arrays Ordenados
    long long pasosHS_RSA = 0; // Número de pasos para Heapsort con Arrays Ordenados Inversos
    long long pasosQS_RSA = 0; // Número de pasos para Quicksort con Arrays Ordenados Inversos

    cout << "# QUICKSORT VERSUS HEAPSORT" << endl;
    cout << "# Average processing Msteps (millions of steps)" << endl;
    cout << "# Number of samples (arrays of integer): 30" << endl;
    cout << "#\t\t\t\tRANDOM\t\tSORTED ARRAYS\tREVERSE SORTED ARRAYS" << endl;
    cout << "#\t\t\t----------------------\t----------------------\t----------------------" << endl;
    cout << "#\tSize\tQuicksort\tHeapsort\tQuicksort\tHeapsort\tQuicksort\tHeapsort" << endl;
    cout << "#======================================================================================================" << endl;

    for (int i = 15; i <= 20; i++)
    {
        int n = 1 << i; // 2^i

        pasosHS_RA = pasosQS_RA = pasosHS_SA = pasosQS_SA = pasosHS_RSA = pasosQS_RSA = 0; // Reiniciamos los pasos

        // Probamos los algoritmos con un vector ALEATORIO de tamaño 2^15 a 2^20

        int *vRA = new int[n];

        // Repetimos 30 veces sólo para el vector aleatorio
        for (int k = 0; k < REPETICIONES; k++)
        {
            for (int j = 0; j < n; j++)
                vRA[j] = rand() % 1000;

            // Middle-Quicksort
            middle_QuickSort(vRA, 0, n - 1, pasosQS_RA);

            // Heapsort
            heapSort(vRA, n, pasosHS_RA);
        }

        delete[] vRA;

        // Probamos los algoritmos con un vector ORDENADO de tamaño 2^15 a 2^20

        int *vSA = new int[n];
        for (int j = 0; j < n; j++)
            vSA[j] = j;

        // Middle-Quicksort
        middle_QuickSort(vSA, 0, n - 1, pasosQS_SA);

        // Heapsort
        heapSort(vSA, n, pasosHS_SA);

        delete[] vSA;

        // Probamos los algoritmos con un vector DESCENDENTE de tamaño 2^15 a 2^20

        int *vRSA = new int[n];

        for (int j = 0; j < n; j++)
            vRSA[j] = n - j;

        // Middle-Quicksort
        middle_QuickSort(vRSA, 0, n - 1, pasosQS_RSA);

        // Heapsort
        heapSort(vRSA, n, pasosHS_RSA);

        delete[] vRSA;

        double factor = 1.0 / (30 * 1000000);

        cout << fixed << setprecision(6) // Fijamos a 3 decimales
             << "\t" << n << "\t"
             << pasosQS_RA * factor << "\t"
             << pasosHS_RA * factor << "\t"
             << pasosQS_SA / 1000000.0 << "\t"
             << pasosHS_SA / 1000000.0 << "\t"
             << pasosQS_RSA / 1000000.0 << "\t"
             << pasosHS_RSA / 1000000.0 << endl;
    }

    return 0;
}