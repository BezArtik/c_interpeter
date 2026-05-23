bool is_prime(int n) {
    if (n <= 1) { return false; }
    if (n <= 3) { return true; }
    if (n % 2 == 0 || n % 3 == 0) { return false; }
    for (int i = 5; i * i <= n; i = i + 6) {
        if (n % i == 0 || n % (i + 2) == 0) { return false; }
    }
    return true;
}

int count = 0;
for (int n = 2; n <= 100; n++) {
	bool prime = is_prime(n);
    if (prime) {
		print(n);
		++count;
	}
}
print("total:");
print(count);