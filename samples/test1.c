int print_primes(int limit) {
    int count = 0;
    int n = 2;

    while (n <= limit) {
        int is_prime = 1;
        int d = 2;

        while (d < n) {
            if (n % d == 0) {
                is_prime = 0;
            }
            d = d + 1;
        }

        if (is_prime == 1) {
            print(n);
            count = count + 1;
        }

        n = n + 1;
    }

    return count;
}

int total = print_primes(100);
print("count:");
print(total);