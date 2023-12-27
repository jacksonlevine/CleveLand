#include <memory>

struct IntTup {
public:
    int x;
    int y;
    int z;
    IntTup(int x, int y, int z);
    IntTup(int x, int z);
    IntTup();
    void set(int x, int y, int z);
    void set(int x, int z);
    bool operator==(const IntTup& other) const;
    bool operator!=(const IntTup& other) const;
    IntTup& operator=(const IntTup& other);
    IntTup& operator+=(const IntTup& other);
};

IntTup operator+(IntTup first, const IntTup& second);

struct IntTupHash {
    std::size_t operator()(const IntTup& tup) const;
};