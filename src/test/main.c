#include "../utils/mydns.h"

int main() {
    init_mydns("127.0.0.1", 12345);
    resolve("hello", "8080", 0, 0);
}
