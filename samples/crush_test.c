// This sample code is designed to test the handling of 
// various return statements in different contexts, including recursion, 
// nested function calls, conditional returns, and loops. 
// It includes a simple implementation of the Fibonacci function, 
// as well as functions to check for even numbers and divisors.


int fibonacci(int n) {
    if (n <= 1) { return n; }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

double square(double x) {
    return x * x;
}

double sum_of_squares(double a, double b) {
    return square(a) + square(b);
}

bool is_even(int n) {
    if (n % 2 == 0) { return true; }
    return false;
}

string even_or_odd(int n) {
    if (is_even(n)) { return "even"; }
    return "odd";
}

bool has_divisor(int n, int max) {
    for (int i = 2; i <= max; ++i) {
        if (n % i == 0) { return true; }
    }
    return false;
}

void print_fib_up_to(int limit) {
    for (int i = 0; i <= limit; ++i) {
        int f = fibonacci(i);
        if (f > limit) { return; }
        print(f);
    }
}

print("Fibonacci(30):");
print(fibonacci(30));

print("Sum of squares 3 and 4:");
print(sum_of_squares(3.0, 4.0));

print("Sum of squares 5 and 12 under the root:");
print(sqrt(sum_of_squares(5.0, 12.0)));

print("7 is:");
print(even_or_odd(7));

print("Has divisor 17 up to 4:");
print(has_divisor(17, 4));

print("Has divisor 15 up to 4:");
print(has_divisor(15, 4));

print("Fibonacci up to 50:");
print_fib_up_to(50);

