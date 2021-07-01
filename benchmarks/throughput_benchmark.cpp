#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <chrono>
#include <vector>

// void print(const char *str)
// {
//     write(STDOUT_FILENO, str, strlen(str));
// }

void isIntSumValid(int num, int sum)
{
    if (sum == ((num * (num - 1)) / 2))
    {
        std::cout << "SUM is valid" << std::endl;
    }
    else
    {
        std::cout << "SUM is invalid" << std::endl;
    }
}

void isDoubleSumValid(double num, double sum)
{
    if (sum == ((num * (num - 1)) / 2))
    {
        std::cout << "SUM is valid" << std::endl;
    }
    else
    {
        std::cout << "SUM is invalid" << std::endl;
    }
}

// Function to return the next random number
int getNum(std::vector<int> &v)
{

    // Size of the vector
    int n = v.size();

    // Generate a random number
    srand(time(NULL));

    // Make sure the number is within
    // the index range
    int index = rand() % n;

    // Get random number from the vector
    int num = v[index];

    // Remove the number from the vector
    std::swap(v[index], v[n - 1]);
    v.pop_back();

    // Return the removed number
    return num;
}

// Function to generate n non-repeating random numbers
std::vector<int> getIndexArray(int n)
{
    std::vector<int> v(n);

    // Fill the vector with the values
    // 1, 2, 3, ..., n
    for (int i = 0; i < n; i++)
        v[i] = i;

    return v;
}

void bench_for_int(int num)
{
    int n = num * sizeof(int);
    int sum = 0;

    int *p = (int *)malloc(n);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < (n / sizeof(int)); i++)
    {
        // printf("%d\n", i);
        p[i] = i;
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < (n / sizeof(int)); i++)
    {
        sum += p[i];
    }
    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> total_time = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t1);
    std::chrono::duration<double> read_time = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    std::chrono::duration<double> write_time = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

    std::cout << "--------------INT SEQ " << num << " ---------------" << std::endl;

    // write time
    std::cout << "Write Time " << write_time.count() << " seconds." << std::endl;
    // read time
    std::cout << "Read Time " << read_time.count() << " seconds." << std::endl;
    // total time
    std::cout << "Total Time " << total_time.count() << " seconds." << std::endl;
    // Avg read time / element
    std::cout << "Avg Read Time " << read_time.count() / num << " seconds." << std::endl;
    // Avg write time / element
    std::cout << "Avg Write Time " << write_time.count() / num << " seconds." << std::endl;
    // Avg total time / element
    std::cout << "Avg Total Time " << total_time.count() / num << " seconds." << std::endl;

    // Sum
    std::cout << "Sum " << sum << std::endl;

    isIntSumValid(num, sum);
}

void bench_for_double(int num)
{
    int n = num * sizeof(double);
    double sum = 0;
    double j = 0;

    double *p = (double *)malloc(n);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < (n / sizeof(double)); i++)
    {
        // printf("%d\n", i);
        p[i] = j;
        j++;
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < (n / sizeof(double)); i++)
    {
        sum += p[i];
    }
    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> total_time = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t1);
    std::chrono::duration<double> read_time = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    std::chrono::duration<double> write_time = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

    std::cout << "--------------DOUBLE SEQ " << num << " ---------------" << std::endl;

    // write time
    std::cout << "Write Time " << write_time.count() << " seconds." << std::endl;
    // read time
    std::cout << "Read Time " << read_time.count() << " seconds." << std::endl;
    // total time
    std::cout << "Total Time " << total_time.count() << " seconds." << std::endl;
    // Avg read time / element
    std::cout << "Avg Read Time " << read_time.count() / num << " seconds." << std::endl;
    // Avg write time / element
    std::cout << "Avg Write Time " << write_time.count() / num << " seconds." << std::endl;
    // Avg total time / element
    std::cout << "Avg Total Time " << total_time.count() / num << " seconds." << std::endl;

    // Sum
    std::cout << "Sum " << sum << std::endl;
    isDoubleSumValid((double)num, sum);
}

void bench_for_int_random(int num)
{
    int n = num * sizeof(int);
    int sum = 0;
    int i = 0;

    int *p = (int *)malloc(n);
    std::vector<int> idx1 = getIndexArray(num);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    while (idx1.size())
    {
        i = getNum(idx1);
        p[i] = i;
    }
    std::vector<int> idx2 = getIndexArray(num);

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    while (idx2.size())
    {
        i = getNum(idx2);
        sum += p[i];
    }
    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> total_time = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t1);
    std::chrono::duration<double> read_time = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    std::chrono::duration<double> write_time = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

    std::cout << "--------------INT RAND " << num << " ---------------" << std::endl;

    // write time
    std::cout << "Write Time " << write_time.count() << " seconds." << std::endl;
    // read time
    std::cout << "Read Time " << read_time.count() << " seconds." << std::endl;
    // total time
    std::cout << "Total Time " << total_time.count() << " seconds." << std::endl;
    // Avg read time / element
    std::cout << "Avg Read Time " << read_time.count() / num << " seconds." << std::endl;
    // Avg write time / element
    std::cout << "Avg Write Time " << write_time.count() / num << " seconds." << std::endl;
    // Avg total time / element
    std::cout << "Avg Total Time " << total_time.count() / num << " seconds." << std::endl;

    // Sum
    std::cout << "Sum " << sum << std::endl;
    isIntSumValid(num, sum);
}

void bench_for_double_random(int num)
{
    int n = num * sizeof(double);
    int i = 0;
    double sum = 0;
    double j = 0;

    double *p = (double *)malloc(n);
    std::vector<int> idx1 = getIndexArray(num);
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    while (idx1.size())
    {
        i = getNum(idx1);
        p[i] = i;
    }
    std::vector<int> idx2 = getIndexArray(num);

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    while (idx2.size())
    {
        i = getNum(idx2);
        sum += p[i];
    }
    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> total_time = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t1);
    std::chrono::duration<double> read_time = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t2);
    std::chrono::duration<double> write_time = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

    std::cout << "--------------DOUBLE RAND " << num << " ---------------" << std::endl;

    // write time
    std::cout << "Write Time " << write_time.count() << " seconds." << std::endl;
    // read time
    std::cout << "Read Time " << read_time.count() << " seconds." << std::endl;
    // total time
    std::cout << "Total Time " << total_time.count() << " seconds." << std::endl;
    // Avg read time / element
    std::cout << "Avg Read Time " << read_time.count() / num << " seconds." << std::endl;
    // Avg write time / element
    std::cout << "Avg Write Time " << write_time.count() / num << " seconds." << std::endl;
    // Avg total time / element
    std::cout << "Avg Total Time " << total_time.count() / num << " seconds." << std::endl;

    // Sum
    std::cout << "Sum " << sum << std::endl;
    isDoubleSumValid((double)num, sum);
}

int main(int argc, char *argv[])
{
    // bench_for_int(1024);
    // bench_for_int(10240);
    // bench_for_int(102400);
    // bench_for_int(1024000);

    // bench_for_double(1024);
    // bench_for_double(10240);
    // bench_for_double(102400);
    // bench_for_double(1024000);

    bench_for_int_random(1024);
    bench_for_int_random(10240);
    bench_for_int_random(102400);
    bench_for_int_random(1024000);

    bench_for_double_random(1024);
    bench_for_double_random(10240);
    bench_for_double_random(102400);
    bench_for_double_random(1024000);
}