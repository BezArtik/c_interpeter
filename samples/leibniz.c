// Leibniz formula for π: π = 4 * (1 - 1/3 + 1/5 - 1/7 + ...)
// This code calculates an approximation of π using the Leibniz formula.

double leibniz_pi(int terms) {
    double pi = 0.0;
    int i = 0;
    double sign = 1.0;

    while (i < terms) {
        double denom = 2.0 * i + 1.0;
        pi = pi + sign * 4.0 / denom;
        sign = 0.0 - sign;
        i = i + 1;
    }

    return pi;
}

double result = leibniz_pi(100000);
print(result);