# 1. FIBONACCI ===================================================

import numpy as np
import matplotlib.pyplot as plt

def fibonacci_pair(n):
    """
    Returns (F_n, F_{n+1}) using matrix exponentiation.
    """
    A = np.array([[1, 1],
                  [1, 0]], dtype=object)

    base = np.array([[1],   # F1
                     [0]],  # F0
                    dtype=object)

    result = np.linalg.matrix_power(A, n) @ base

    Fn1 = result[0, 0]   # F_{n+1}
    Fn  = result[1, 0]   # F_n
    return Fn, Fn1


# Example: plot Fibonacci numbers for n = 0..15
n_values = np.arange(0, 16)
fib_n = []
fib_n1 = []

for n in n_values:
    Fn, Fn1 = fibonacci_pair(n)
    fib_n.append(Fn)
    fib_n1.append(Fn1)

plt.figure(figsize=(8, 5))
plt.plot(n_values, fib_n, marker='o', label='F_n')
plt.plot(n_values, fib_n1, marker='s', label='F_{n+1}')
plt.title('Fibonacci numbers')
plt.xlabel('n')
plt.ylabel('Value')
plt.legend()
plt.grid(True)
plt.show()

# 2. BASKETBALL ===================================================

import random

def simulate_one_game():
    """
    Simulates one game.
    Returns 'Bob' or 'John'.
    """
    while True:
        # Bob's turn
        if random.random() < 1/4:
            return 'Bob'

        # John's turn
        if random.random() < 1/3:
            return 'John'


def simulate_many_games(num_games=10**6):
    bob_wins = 0
    john_wins = 0

    for _ in range(num_games):
        winner = simulate_one_game()
        if winner == 'Bob':
            bob_wins += 1
        else:
            john_wins += 1

    return bob_wins, john_wins


num_games = 10**6
bob_wins, john_wins = simulate_many_games(num_games)

print("Bob wins:", bob_wins)
print("John wins:", john_wins)
print("P(Bob wins) ≈", bob_wins / num_games)
print("P(John wins) ≈", john_wins / num_games)

# 3. Mandelbrot set ===================================================

# a) Define the complex plane
x = np.arange(-2, 1, 0.1)
y = np.arange(-1.5, 1.5, 0.1)

# b) Generate coordinate matrices
X, Y = np.meshgrid(x, y)
C = X + 1j * Y

# c) Initialize Z
Z = np.zeros_like(C, dtype=complex)

# d) Mandelbrot recurrence
for k in range(100):
    Z = Z**2 + C

# e) Create mask M: True where |Z| < 2
M = np.abs(Z) < 2

# f) Plot
plt.figure(figsize=(8, 6))
plt.imshow(M, extent=[x.min(), x.max(), y.min(), y.max()], origin='lower')
plt.title('Mandelbrot Set')
plt.xlabel('Re(c)')
plt.ylabel('Im(c)')
plt.show()

# 4. Matrix operations ===================================================

import math
import time

# Generate data
A = np.random.rand(1000, 1000)
B = np.random.rand(1000, 1000)
c = np.random.rand(1000)

# Memory usage
print("A.nbytes =", A.nbytes, "bytes")
print("B.nbytes =", B.nbytes, "bytes")
print("c.nbytes =", c.nbytes, "bytes")
print("Total =", A.nbytes + B.nbytes + c.nbytes, "bytes")

# Largest square matrix fitting in 32 GB RAM
ram_bytes = 32 * 1024**3   # 32 GiB
bytes_per_element = 8      # float64
max_n = int(math.sqrt(ram_bytes / bytes_per_element))

print("Largest square matrix dimension fitting in 32 GB:", max_n)
print("That matrix would use about", max_n**2 * 8 / 1024**3, "GiB")

# Time multiplication: d = A @ B @ c
start = time.time()
d = A @ B @ c
end = time.time()

print("Execution time:", end - start, "seconds")
print("Result vector shape:", d.shape)