// src/a1i.cpp
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    double xc[3], yc[3], r[3], r2[3];
    for (int i = 0; i < 3; ++i) {
        std::cin >> xc[i] >> yc[i] >> r[i];
        r2[i] = r[i] * r[i];
    }

    double lx = -1e18, rx = 1e18;
    double ly = -1e18, ry = 1e18;

    for (int i = 0; i < 3; ++i) {
        lx = std::max(lx, xc[i] - r[i]);
        rx = std::min(rx, xc[i] + r[i]);
        ly = std::max(ly, yc[i] - r[i]);
        ry = std::min(ry, yc[i] + r[i]);
    }

    if (lx >= rx || ly >= ry) {
        std::cout << std::fixed << std::setprecision(10) << 0.0;
        return 0;
    }

    double w = rx - lx;
    double h = ry - ly;
    double areaRect = w * h;

    const int SAMPLES = 3000000;

    std::mt19937_64 rng(123456789);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    int inside = 0;

    for (int i = 0; i < SAMPLES; ++i) {
        double x = lx + w * dist(rng);
        double y = ly + h * dist(rng);

        bool ok = true;
        for (int j = 0; j < 3; ++j) {
            double dx = x - xc[j];
            double dy = y - yc[j];
            if (dx * dx + dy * dy > r2[j]) {
                ok = false;
                break;
            }
        }
        if (ok) ++inside;
    }

    double estimate = areaRect * static_cast<double>(inside)
                      / static_cast<double>(SAMPLES);

    std::cout << std::fixed << std::setprecision(10) << estimate;
    return 0;
}
