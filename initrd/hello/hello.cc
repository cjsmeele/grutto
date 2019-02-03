#include <grutto.hh>

int main() {
    print("hi! user-mode works :-)\n");

    for (volatile int i = 0; i < 10; ++i) {
        print("greetings from actual userspace! :D (x");
        print(i+1);
        print(")\n");
    }

    print("\nuser-mode will now try to exec a privileged insn (this should raise a GPF)\n");

    asm volatile("cli");

    return 0xdeadb011;
}
