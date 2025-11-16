#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <algorithm>

class ArrayGenerator {
public:
    enum class Type { Random, Reversed, AlmostSorted };

    ArrayGenerator(int maxSize, int maxValue) : maxSize_(maxSize), rng_(123456789) {
        std::uniform_int_distribution<int> dist(0, maxValue);

        random_.resize(maxSize_);
        for (int i = 0; i < maxSize_; ++i) random_[i] = dist(rng_);

        reversed_ = random_;
        std::sort(reversed_.begin(), reversed_.end(), std::greater<int>());

        almost_.resize(maxSize_);
        std::sort(random_.begin(), random_.end());
        almost_ = random_;
        int swaps = maxSize_ / 20;
        std::uniform_int_distribution<int> pos(0, maxSize_ - 1);
        for (int i = 0; i < swaps; ++i) {
            int x = pos(rng_);
            int y = pos(rng_);
            std::swap(almost_[x], almost_[y]);
        }
    }

    void fill(std::vector<int> &dest, int n, Type type) const {
        dest.resize(n);
        const std::vector<int> *src = nullptr;
        if (type == Type::Random) src = &random_;
        else if (type == Type::Reversed) src = &reversed_;
        else src = &almost_;
        std::copy(src->begin(), src->begin() + n, dest.begin());
    }

private:
    int maxSize_;
    mutable std::mt19937 rng_;
    std::vector<int> random_;
    std::vector<int> reversed_;
    std::vector<int> almost_;
};

void insertion_sort_segment(std::vector<int> &a, int l, int r) {
    for (int i = l + 1; i < r; ++i) {
        int x = a[i];
        int j = i - 1;
        while (j >= l && a[j] > x) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = x;
    }
}

void merge_sort_std_rec(std::vector<int> &a, std::vector<int> &tmp, int l, int r) {
    if (r - l <= 1) return;
    int m = l + (r - l) / 2;
    merge_sort_std_rec(a, tmp, l, m);
    merge_sort_std_rec(a, tmp, m, r);
    int i = l;
    int j = m;
    int k = l;
    while (i < m && j < r) {
        if (a[i] <= a[j]) tmp[k++] = a[i++];
        else tmp[k++] = a[j++];
    }
    while (i < m) tmp[k++] = a[i++];
    while (j < r) tmp[k++] = a[j++];
    for (int t = l; t < r; ++t) a[t] = tmp[t];
}

void merge_sort_hybrid_rec(std::vector<int> &a, std::vector<int> &tmp,
                           int l, int r, int threshold) {
    int len = r - l;
    if (len <= threshold) {
        insertion_sort_segment(a, l, r);
        return;
    }
    int m = l + (r - l) / 2;
    merge_sort_hybrid_rec(a, tmp, l, m, threshold);
    merge_sort_hybrid_rec(a, tmp, m, r, threshold);
    int i = l;
    int j = m;
    int k = l;
    while (i < m && j < r) {
        if (a[i] <= a[j]) tmp[k++] = a[i++];
        else tmp[k++] = a[j++];
    }
    while (i < m) tmp[k++] = a[i++];
    while (j < r) tmp[k++] = a[j++];
    for (int t = l; t < r; ++t) a[t] = tmp[t];
}

class SortTester {
public:
    explicit SortTester(const ArrayGenerator &gen) : gen_(gen) {}

    long long measure_merge(ArrayGenerator::Type type, int n, int repeats) {
        std::vector<int> base;
        gen_.fill(base, n, type);
        std::vector<int> a(n);
        std::vector<int> tmp(n);
        long long total = 0;
        for (int rep = 0; rep < repeats; ++rep) {
            std::copy(base.begin(), base.end(), a.begin());
            auto start = std::chrono::high_resolution_clock::now();
            merge_sort_std_rec(a, tmp, 0, n);
            auto end = std::chrono::high_resolution_clock::now();
            total += std::chrono::duration_cast<std::chrono::microseconds>(
                         end - start).count();
        }
        return total / repeats;
    }

    long long measure_hybrid(ArrayGenerator::Type type, int n,
                             int threshold, int repeats) {
        std::vector<int> base;
        gen_.fill(base, n, type);
        std::vector<int> a(n);
        std::vector<int> tmp(n);
        long long total = 0;
        for (int rep = 0; rep < repeats; ++rep) {
            std::copy(base.begin(), base.end(), a.begin());
            auto start = std::chrono::high_resolution_clock::now();
            merge_sort_hybrid_rec(a, tmp, 0, n, threshold);
            auto end = std::chrono::high_resolution_clock::now();
            total += std::chrono::duration_cast<std::chrono::microseconds>(
                         end - start).count();
        }
        return total / repeats;
    }

private:
    const ArrayGenerator &gen_;
};

const char *array_type_name(ArrayGenerator::Type t) {
    if (t == ArrayGenerator::Type::Random) return "random";
    if (t == ArrayGenerator::Type::Reversed) return "reversed";
    return "almost";
}

int main() {
    const int MAX_N = 100000;
    const int MIN_N = 500;
    const int STEP_N = 1000;
    const int MAX_VALUE = 6000;
    const int REPEATS = 5;
    const int thresholds[] = {5, 10, 20, 30, 50};
    const int thresholdsCount = sizeof(thresholds) / sizeof(thresholds[0]);

    ArrayGenerator generator(MAX_N, MAX_VALUE);
    SortTester tester(generator);

    std::ofstream out("a2_results.csv");
    if (!out) {
        std::cerr << "Cannot open a2_results.csv for writing\n";
        return 1;
    }

    out << "n,array_type,algorithm,threshold,time_us\n";

    ArrayGenerator::Type types[3] = {
        ArrayGenerator::Type::Random,
        ArrayGenerator::Type::Reversed,
        ArrayGenerator::Type::AlmostSorted
    };

    for (int n = MIN_N; n <= MAX_N; n += STEP_N) {
        for (int ti = 0; ti < 3; ++ti) {
            ArrayGenerator::Type t = types[ti];
            const char *tname = array_type_name(t);

            long long t_merge = tester.measure_merge(t, n, REPEATS);
            out << n << ',' << tname << ",merge,0," << t_merge << '\n';

            for (int k = 0; k < thresholdsCount; ++k) {
                int thr = thresholds[k];
                long long t_h = tester.measure_hybrid(t, n, thr, REPEATS);
                out << n << ',' << tname << ",hybrid," << thr << ','
                    << t_h << '\n';
            }
        }
    }

    std::cerr << "Done. Results written to a2_results.csv\n";
    return 0;
}
