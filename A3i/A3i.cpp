#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

static const int INSERTION_THRESHOLD = 16;

static std::uint64_t rng_state = 88172645463393265ull;

inline std::uint32_t next_rand() {
    rng_state ^= rng_state << 7;
    rng_state ^= rng_state >> 9;
    return static_cast<std::uint32_t>(rng_state);
}

void insertion_sort_segment(std::vector<long long> &a, int left, int right) {
    for (int i = left + 1; i < right; ++i) {
        long long x = a[i];
        int j = i - 1;
        while (j >= left && a[j] > x) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = x;
    }
}

void heap_sort_segment(std::vector<long long> &a, int left, int right) {
    std::make_heap(a.begin() + left, a.begin() + right);
    std::sort_heap(a.begin() + left, a.begin() + right);
}

int partition_random(std::vector<long long> &a, int left, int right) {
    int len = right - left;
    int pivot_index = left + static_cast<int>(next_rand() % static_cast<std::uint32_t>(len));
    long long pivot = a[pivot_index];

    int i = left;
    int j = right - 1;
    while (true) {
        while (a[i] < pivot) {
            ++i;
        }
        while (a[j] > pivot) {
            --j;
        }
        if (i >= j) {
            return j;
        }
        std::swap(a[i], a[j]);
        ++i;
        --j;
    }
}

void introsort_rec(std::vector<long long> &a, int left, int right, int depth_limit) {
    while (right - left > 1) {
        int len = right - left;
        if (len < INSERTION_THRESHOLD) {
            insertion_sort_segment(a, left, right);
            return;
        }
        if (depth_limit == 0) {
            heap_sort_segment(a, left, right);
            return;
        }
        --depth_limit;

        int p = partition_random(a, left, right);

        int left_len = p + 1 - left;
        int right_len = right - (p + 1);

        if (left_len < right_len) {
            if (left < p + 1) {
                introsort_rec(a, left, p + 1, depth_limit);
            }
            left = p + 1;
        } else {
            if (p + 1 < right) {
                introsort_rec(a, p + 1, right, depth_limit);
            }
            right = p + 1;
        }
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    if (!(std::cin >> n)) {
        return 0;
    }

    std::vector<long long> a(n);
    for (int i = 0; i < n; ++i) {
        std::cin >> a[i];
    }

    if (n > 1) {
        int depth_limit = 0;
        int m = n;
        while (m > 1) {
            m >>= 1;
            ++depth_limit;
        }
        depth_limit *= 2;
        introsort_rec(a, 0, n, depth_limit);
    }

    for (int i = 0; i < n; ++i) {
        if (i) {
            std::cout << ' ';
        }
        std::cout << a[i];
    }

    return 0;
}
