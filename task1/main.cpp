#include <iostream>
#include <thread>
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>

#include "ThreadPool.h"
#include <QThreadPool>
#include <QtConcurrent>

using namespace std;
using namespace std::chrono;


void printArr(std::vector<double>& arr)
{
    for (auto& item: arr)
    {
        cout << item << " ";
    }
    cout << endl;
}


void parallelPrefixSum(std::vector<double>& arr)
{
    size_t size = arr.size();
    unsigned int n = std::thread::hardware_concurrency();
    ThreadPool pool(n);
    std::vector< std::future<void> > results;

    size_t depth = log2 (size);
    for(size_t d = 0; d < depth; ++d) {

        size_t works = std::pow (2, depth - d - 1);

        for(size_t k = 0; k < works; ++k)
        {
            results.emplace_back(
                pool.enqueue(
                [d, k, &arr]
                {
                    int arraySize = std::pow (2, d + 1);
                    int arrayStart = k*arraySize;
                    arr[arrayStart + arraySize - 1] += arr[arrayStart + arraySize/2 - 1];
                    return;
                })
            );
        }
        for(auto && result: results)
            result.get();
        results.clear();
    }

    arr[size-1] = 0;

    for(int d = depth-1; d >= 0; --d) {

        int works = std::pow (2, depth - d - 1);

        for(int k = 0; k < works; ++k)
        {
            results.emplace_back(
                pool.enqueue(
                [d, k, &arr]
                {
                    int arraySize = std::pow (2, d + 1);
                    int arrayStart = k*arraySize;
                    int temp =  arr[arrayStart + arraySize - 1];
                    arr[arrayStart + arraySize - 1] += arr[arrayStart + arraySize/2 - 1];
                    arr[arrayStart + arraySize/2 - 1] = temp;
                    return;
                })
            );
        }
        for(auto && result: results)
            result.get();
        results.clear();
    }

}

void parallelQtPrefixSum(std::vector<double>& arr)
{
    size_t size = arr.size();
    std::vector< QFuture<void> > results;

    size_t depth = log2 (size);
    for(size_t d = 0; d < depth; ++d) {

        size_t works = std::pow (2, depth - d - 1);

        for(size_t k = 0; k < works; ++k)
        {
            results.push_back(QtConcurrent::run(QThreadPool::globalInstance(), [d, k, &arr]
            {
                int arraySize = std::pow (2, d + 1);
                int arrayStart = k*arraySize;
                arr[arrayStart + arraySize - 1] += arr[arrayStart + arraySize/2 - 1];
                return;
            }));
        }
        for(auto && result: results)
            result.waitForFinished();
        results.clear();
    }

    arr[size-1] = 0;

    for(int d = depth-1; d >= 0; --d) {

        int works = std::pow (2, depth - d - 1);

        for(int k = 0; k < works; ++k)
        {
            results.push_back(QtConcurrent::run(QThreadPool::globalInstance(), [d, k, &arr]
            {
                int arraySize = std::pow (2, d + 1);
                int arrayStart = k*arraySize;
                int temp =  arr[arrayStart + arraySize - 1];
                arr[arrayStart + arraySize - 1] += arr[arrayStart + arraySize/2 - 1];
                arr[arrayStart + arraySize/2 - 1] = temp;
                return;
            }));
        }
        for(auto && result: results)
            result.waitForFinished();
        results.clear();
    }

}

void normalPrefixSum(std::vector<double>& arr)
{
    cout << "operations count " << arr.size() << endl;
    for (size_t i = 1; i < arr.size(); ++i)
    {
        volatile double* ptr = &arr[i];
        *ptr = arr[i-1] + *ptr;
    }
}

volatile int GlobalVar = 10;

int main()
{
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);


    size_t size = std::pow (2, 25);
    std::vector<double> arr(size);
    std::generate(arr.begin(), arr.end(), [&](){return dis(gen);});

    std::vector<double> copy1(arr.begin(), arr.end());
    //printArr(copy2);
    high_resolution_clock::time_point start1 = high_resolution_clock::now();
    //std::partial_sum (arr.begin(), arr.end(), copy1.begin());
    normalPrefixSum(copy1);
    high_resolution_clock::time_point end1 = high_resolution_clock::now();
    GlobalVar = copy1[size-1];
    //printArr(copy2);
    cout << "normalPrefixSum execution time " <<
            duration_cast<milliseconds>( end1 - start1 ).count() << " milliseconds" << endl;

    std::vector<double> copy2(arr.begin(), arr.end());
    //printArr(copy2);
    high_resolution_clock::time_point start2 = high_resolution_clock::now();
    parallelPrefixSum(copy2);
    high_resolution_clock::time_point end2 = high_resolution_clock::now();
    GlobalVar = copy2[size-1];
    //printArr(copy2);
    cout << "parallelPrefixSum execution time " <<
            duration_cast<milliseconds>( end2 - start2 ).count() << " milliseconds" << endl;

    std::vector<double> copy4(arr.begin(), arr.end());
    //printArr(copy2);
    high_resolution_clock::time_point start4 = high_resolution_clock::now();
    //parallelQtPrefixSum(copy4);

    high_resolution_clock::time_point end4 = high_resolution_clock::now();
    GlobalVar = copy4[size-1];
    //printArr(copy2);
    cout << "parallelQtPrefixSum execution time " <<
            duration_cast<milliseconds>( end4 - start4 ).count() << " milliseconds" << endl;
    std::cout.flush();
}
