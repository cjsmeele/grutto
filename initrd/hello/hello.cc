#include <grutto.hh>

int main() {
    print("greetings from user space :-)\n");

    int pid = getpid();

    while (true)
        print(static_cast<char>('A' + pid - 1));

    return 0xdeadb011;
}
