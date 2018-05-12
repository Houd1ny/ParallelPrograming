#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>
#include <future>
#include <unistd.h>
#include <chrono>

using namespace std::chrono;
using namespace std;

struct Point
{
    double x, y;
    friend ostream& operator<<(ostream& os, const Point& p);
};

ostream& operator<<(ostream& os, const Point& p)
{
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

using Iter = vector<Point>::const_iterator;

double dist(const Point& p1, const Point& p2)
{
    return sqrt( (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y) );
}

void print(const std::vector<Point>& arr)
{
    for (auto point : arr)
    {
        cout << point << " ";
    }
    cout << endl;
}

double bruteForce(Iter begin, Iter end)
{
    double min = numeric_limits<double>::max();
    for (auto it_i= begin; it_i != end ; ++it_i)
    {
        for (auto it_j = it_i + 1; it_j != end; ++it_j)
        {
            double dist_i_j = dist(*it_i, *it_j);
            if (dist_i_j < min)
            {
                min = dist_i_j;
            }
        }
    }
    return min;
}

double closest_pair_merge(Iter begin, Iter end, Iter mid, double d)
{
    std::vector<Point> strip;
    copy_if(begin, end, back_inserter(strip), [=](const Point& p) {
        return std::abs(p.x - mid->x) < d;
    });

    double min = numeric_limits<double>::max();
    for (auto it_i = strip.begin(); it_i != strip.end() ; ++it_i)
    {
        for (auto it_j = it_i + 1;  it_j != it_i + 8 && it_j != strip.end(); ++it_j)
        {
            double dist_i_j = dist(*it_i, *it_j);
            if (dist_i_j < min)
            {
                min = dist_i_j;
            }
        }
    }
    return min;
}

double closest_pair_rec(Iter beginPx, Iter endPx, Iter beginPy, Iter endPy, int depth)
{
    size_t size = endPx - beginPx;
    if (size <= 3)
    {
        return bruteForce(beginPx, endPx);
    }

    Iter mid = beginPx + size / 2;

    std::vector<Point> Pyl;
    std::vector<Point> Pyr;

    for (auto it = beginPy; it != endPy; ++it)
    {
      if (it->x <= mid->x)
         Pyl.push_back(*it);
      else
         Pyr.push_back(*it);
    }

    double d = 0;
    if (depth >= 0 )
    {
        auto dl = std::async(closest_pair_rec, beginPx, mid, Pyl.begin(), Pyl.end(), --depth);
        auto dr = std::async(closest_pair_rec, mid, endPx, Pyr.begin(), Pyr.end(), --depth);

        d = std::min(dl.get(), dr.get());
    }
    else
    {

        double dl = closest_pair_rec(beginPx, mid, Pyl.begin(), Pyl.end(), -1);
        double dr = closest_pair_rec(mid, endPx, Pyr.begin(), Pyr.end(), -1);

        d = std::min(dl, dr);
    }

    return std::min(d, closest_pair_merge(beginPy, endPy, mid, d));
}

double closest_pair(const std::vector<Point>& arr, int threads)
{
    std::vector<Point> Px(arr.begin(), arr.end());
    std::vector<Point> Py(arr.begin(), arr.end());

    sort(Px.begin(), Px.end(), [](const Point& l, const Point& r) { return l.x < r.x;});
    sort(Py.begin(), Py.end(), [](const Point& l, const Point& r) { return l.y < r.y;});

    int depth = log2 (threads);

    return closest_pair_rec(Px.begin(), Px.end(), Py.begin(), Py.end(), depth);
}


int main()
{

    unsigned int n = thread::hardware_concurrency();
    std::cout << n << " threads are supported.\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 100000000);

    size_t size = std::pow (2, 3);

    double d1 = 0;
    double d2 = 0;

    std::vector<Point> arr(100000);
    std::generate(arr.begin(), arr.end(), [&](){return Point{dis(gen), dis(gen)};});


    high_resolution_clock::time_point start = high_resolution_clock::now();
    d1 = closest_pair(arr, 0);
    high_resolution_clock::time_point end = high_resolution_clock::now();

    size_t milisecs_brute = duration_cast<milliseconds>( end - start ).count();
    cout << "bruteforce " << milisecs_brute << " milliseconds" << endl;

    start = high_resolution_clock::now();
    d2 = closest_pair(arr, 4);
    end = high_resolution_clock::now();

    size_t milisecs_smart = duration_cast<milliseconds>( end - start ).count();
    cout << "smart time " << milisecs_smart << " milliseconds" << endl;

    cout << "speedup " << static_cast<double>(milisecs_brute) / milisecs_smart << endl;

    return 0;
}
