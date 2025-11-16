#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <cstdint>

class FastRng {
public:
    explicit FastRng(std::uint64_t seed = 88172645463393265ull) : state(seed) {}

    std::uint32_t next_u32() {
        state ^= state << 7;
        state ^= state >> 9;
        return static_cast<std::uint32_t>(state);
    }

    int next_int(int l, int r) {
        std::uint32_t span = static_cast<std::uint32_t>(r - l);
        return l + static_cast<int>(next_u32() % span);
    }

private:
    std::uint64_t state;
};

class ArrayGenerator {
public:
    enum class Type { Random, Reversed, AlmostSorted };

    ArrayGenerator(int maxSize, int maxValue) : maxSize_(maxSize), rng_(123456789) {
        random_.resize(maxSize_);
        for (int i = 0; i < maxSize_; ++i) {
            random_[i] = static_cast<int>(rng_.next_u32() % (maxValue + 1));
        }

        reversed_ = random_;
        std::sort(reversed_.begin(), reversed_.end(), std::greater<int>());

        almost_ = random_;
        std::sort(almost_.begin(), almost_.end());
        int swaps = maxSize_ / 20;
        for (int i = 0; i < swaps; ++i) {
            int x = rng_.next_int(0, maxSize_);
            int y = rng_.next_int(0, maxSize_);
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
    mutable FastRng rng_;
    std::vector<int> random_;
    std::vector<int> reversed_;
    std::vector<int> almost_;
};

static const int INSERTION_THRESHOLD = 16;

int partition_quick(std::vector<int> &a, int left, int right, FastRng &rng) {
    int pivotIndex = rng.next_int(left, right);
    int pivot = a[pivotIndex];
    int i = left;
    int j = right - 1;
    while (true) {
        while (a[i] < pivot) ++i;
        while (a[j] > pivot) --j;
        if (i >= j) return j;
        std::swap(a[i], a[j]);
        ++i;
        --j;
    }
}

void quick_sort_rec(std::vector<int> &a, int left, int right, FastRng &rng) {
    while (right - left > 1) {
        int p = partition_quick(a, left, right, rng);
        int left_len = p + 1 - left;
        int right_len = right - (p + 1);
        if (left_len < right_len) {
            if (left < p + 1) {
                quick_sort_rec(a, left, p + 1, rng);
            }
            left = p + 1;
        } else {
            if (p + 1 < right) {
                quick_sort_rec(a, p + 1, right, rng);
            }
            right = p + 1;
        }
    }
}

void quick_sort(std::vector<int> &a, FastRng &rng) {
    if (!a.empty()) {
        quick_sort_rec(a, 0, static_cast<int>(a.size()), rng);
    }
}

void insertion_segment(std::vector<int> &a, int left, int right) {
    for (int i = left + 1; i < right; ++i) {
        int x = a[i];
        int j = i - 1;
        while (j >= left && a[j] > x) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = x;
    }
}

void heap_sort_segment(std::vector<int> &a, int left, int right) {
    std::make_heap(a.begin() + left, a.begin() + right);
    std::sort_heap(a.begin() + left, a.begin() + right);
}

void introsort_rec(std::vector<int> &a, int left, int right, int depth_limit, FastRng &rng) {
    while (right - left > 1) {
        int len = right - left;
        if (len < INSERTION_THRESHOLD) {
            insertion_segment(a, left, right);
            return;
        }
        if (depth_limit == 0) {
            heap_sort_segment(a, left, right);
            return;
        }
        --depth_limit;

        int p = partition_quick(a, left, right, rng);
        int left_len = p + 1 - left;
        int right_len = right - (p + 1);

        if (left_len < right_len) {
            if (left < p + 1) {
                introsort_rec(a, left, p + 1, depth_limit, rng);
            }
            left = p + 1;
        } else {
            if (p + 1 < right) {
                introsort_rec(a, p + 1, right, depth_limit, rng);
            }
            right = p + 1;
        }
    }
}

void introsort(std::vector<int> &a, FastRng &rng) {
    int n = static_cast<int>(a.size());
    if (n <= 1) return;

    int depth_limit = 0;
    int m = n;
    while (m > 1) {
        m >>= 1;
        ++depth_limit;
    }
    depth_limit *= 2;

    introsort_rec(a, 0, n, depth_limit, rng);
}

class SortTester {
public:
    explicit SortTester(const ArrayGenerator &gen) : gen_(gen) {}

    long long measure_quick(ArrayGenerator::Type type, int n, int repeats) {
        std::vector<int> base;
        gen_.fill(base, n, type);
        std::vector<int> a(n);
        long long total = 0;
        for (int rep = 0; rep < repeats; ++rep) {
            std::copy(base.begin(), base.end(), a.begin());
            FastRng rng(123456789u + static_cast<std::uint64_t>(rep));
            auto start = std::chrono::high_resolution_clock::now();
            quick_sort(a, rng);
            auto end = std::chrono::high_resolution_clock::now();
            total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }
        return total / repeats;
    }

    long long measure_intro(ArrayGenerator::Type type, int n, int repeats) {
        std::vector<int> base;
        gen_.fill(base, n, type);
        std::vector<int> a(n);
        long long total = 0;
        for (int rep = 0; rep < repeats; ++rep) {
            std::copy(base.begin(), base.end(), a.begin());
            FastRng rng(987654321u + static_cast<std::uint64_t>(rep));
            auto start = std::chrono::high_resolution_clock::now();
            introsort(a, rng);
            auto end = std::chrono::high_resolution_clock::now();
            total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
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

    ArrayGenerator generator(MAX_N, MAX_VALUE);
    SortTester tester(generator);

    std::ofstream out("../a3_results.csv");
    if (!out) {
        std::cerr << "Cannot open a3_results.csv for writing\n";
        return 1;
    }

    out << "n,array_type,algorithm,time_us\n";

    ArrayGenerator::Type types[3] = {
        ArrayGenerator::Type::Random,
        ArrayGenerator::Type::Reversed,
        ArrayGenerator::Type::AlmostSorted
    };

    for (int n = MIN_N; n <= MAX_N; n += STEP_N) {
        for (int ti = 0; ti < 3; ++ti) {
            ArrayGenerator::Type t = types[ti];
            const char *name = array_type_name(t);

            long long tq = tester.measure_quick(t, n, REPEATS);
            out << n << ',' << name << ",quick," << tq << '\n';

            long long ti_intro = tester.measure_intro(t, n, REPEATS);
            out << n << ',' << name << ",intro," << ti_intro << '\n';
        }
    }

    std::cerr << "Done. Results written to a3_results.csv\n";
    return 0;
}
