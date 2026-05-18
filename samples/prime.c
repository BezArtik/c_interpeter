int is_prime(int n) {
    if (n < 2) {
        return 0;
    }
    int d = 2;
    while (d < n) {
        if (n % d == 0) {
            return 0;
        }
        d = d + 1;
    }
    return 1;
}

int count = 0;
int n = 2;
while (n <= 100) {
    if (is_prime(n) == 1) {
        print(n);
        count = count + 1;
    }
    n = n + 1;
}
print("total:");
print(count);