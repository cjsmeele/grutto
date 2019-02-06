#include <grutto.hh>

int main() {
    for (int i = 0;; ++i) {
        print((i & 1) == 0 ? "/beep/" : "/boop/");
        yield();
    }

    return 0xdeadb012;
}
