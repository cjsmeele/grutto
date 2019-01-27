using u32 = unsigned int;

// Note: We're not actually running in user mode yet.

void syscallish(u32 a = 0,
                u32 b = 0,
                u32 c = 0,
                u32 d = 0) {
    asm volatile("int $0x80"
                 :: "a" (a),
                    "b" (b),
                    "c" (c),
                    "d" (d));
}

void print(const char *s) {
    syscallish(0xbeeeeeef, (u32)s);
}
void print(int x) {
    syscallish(0xbeeeeef, x);
}

int main() {
    syscallish(0xba551e);

    for (int i = 0; i < 10; ++i) {
        print("greetings from fake-userspace :D (x");
        print(i+1);
        print(")\n");
    }

    return 0xdeadb011;
}
