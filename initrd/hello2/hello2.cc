volatile int y = 42;
volatile int z;

int main() {
    volatile int x = y;

    z = x + 5;
    while (true)
        asm volatile("int $0x80");
    return z;
}
