#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <cmath>
#include <algorithm>

struct Circle {
    double x;
    double y;
    double r;
    double r2;
};

double exact_area() {
    const double pi = std::acos(-1.0);
    return 0.25 * pi + 1.25 * std::asin(0.8) - 1.0;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Circle c[3];
    c[0].x = 1.0;  c[0].y = 1.0;  c[0].r = 1.0;
    c[1].x = 1.5;  c[1].y = 2.0;  c[1].r = std::sqrt(5.0) / 2.0;
    c[2].x = 2.0;  c[2].y = 1.5;  c[2].r = std::sqrt(5.0) / 2.0;
    for (int i = 0; i < 3; ++i) {
        c[i].r2 = c[i].r * c[i].r;
    }

    double S_exact = exact_area();

    double lx_w = 1e18, rx_w = -1e18;
    double ly_w = 1e18, ry_w = -1e18;
    for (int i = 0; i < 3; ++i) {
        lx_w = std::min(lx_w, c[i].x - c[i].r);
        rx_w = std::max(rx_w, c[i].x + c[i].r);
        ly_w = std::min(ly_w, c[i].y - c[i].r);
        ry_w = std::max(ry_w, c[i].y + c[i].r);
    }

    double lx_n = -1e18, rx_n = 1e18;
    double ly_n = -1e18, ry_n = 1e18;
    for (int i = 0; i < 3; ++i) {
        lx_n = std::max(lx_n, c[i].x - c[i].r);
        rx_n = std::min(rx_n, c[i].x + c[i].r);
        ly_n = std::max(ly_n, c[i].y - c[i].r);
        ry_n = std::min(ry_n, c[i].y + c[i].r);
    }

    std::mt19937_64 rng(987654321);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::ofstream out("../results.csv");
    if (!out) {
        std::cerr << "Cannot open ../results.csv for writing\n";
        return 1;
    }

    out << std::fixed << std::setprecision(10);
    out << "N,type,estimate,abs_error\n";

    for (int N = 100; N <= 10000; N += 500) {
        for (int rectType = 0; rectType < 2; ++rectType) {
            double lx = rectType == 0 ? lx_w : lx_n;
            double rx = rectType == 0 ? rx_w : rx_n;
            double ly = rectType == 0 ? ly_w : ly_n;
            double ry = rectType == 0 ? ry_w : ry_n;

            double w = rx - lx;
            double h = ry - ly;
            double areaRect = w * h;

            int inside = 0;
            for (int i = 0; i < N; ++i) {
                double x = lx + w * dist(rng);
                double y = ly + h * dist(rng);

                bool ok = true;
                for (int j = 0; j < 3; ++j) {
                    double dx = x - c[j].x;
                    double dy = y - c[j].y;
                    if (dx * dx + dy * dy > c[j].r2) {
                        ok = false;
                        break;
                    }
                }
                if (ok) {
                    ++inside;
                }
            }

            double estimate =
                areaRect * static_cast<double>(inside) / static_cast<double>(N);
            double err = std::fabs(estimate - S_exact);

            out << N << ","
                << (rectType == 0 ? "wide" : "narrow") << ","
                << estimate << "," << err << "\n";
        }
    }

    std::cerr << "Done. Results written to ../results.csv\n";
    return 0;
}
