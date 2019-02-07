#include <grutto.hh>

void thread(size_t n) { while (true) print((int)n); }

char thread_stack[1024];

int main() {
    print("greetings from user space :-)\n");

    //int pid = getpid();

    int i = 1;
    make_thread(thread, thread_stack+i*64, sizeof(thread_stack)-i*64, i), i++;
    make_thread(thread, thread_stack+i*64, sizeof(thread_stack)-i*64, i), i++;
    make_thread(thread, thread_stack+i*64, sizeof(thread_stack)-i*64, i), i++;
    make_thread(thread, thread_stack+i*64, sizeof(thread_stack)-i*64, i), i++;
    make_thread(thread, thread_stack+i*64, sizeof(thread_stack)-i*64, i), i++;
    make_thread(thread, thread_stack+i*64, sizeof(thread_stack)-i*64, i), i++;

    while (true) yield();

    return 0xdeadb011;
}
