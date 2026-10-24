#pragma once
#include <array>
#include <cstddef>

template<int N, int I = 1>
struct Divisors {
    static constexpr int value = (N % I == 0 ? 1 : 0) + Divisors<N, I + 1>::value;
};

template<int N>
struct Divisors<N, N> {
    static constexpr int value = 1;
};

template<int N, int I = 1, int IDX = 0>
struct FillDivisors {
    template<typename Arr>
    static void fill(Arr& arr) {
        FillDivisors<N, I + 1, IDX + (N % I == 0 ? 1 : 0)>::fill(arr);
        if (N % I == 0) arr[IDX] = I;
    }
};

template<int N, int IDX>
struct FillDivisors<N, N, IDX> {
    template<typename Arr>
    static void fill(Arr& arr) {
        arr[IDX] = N;
    }
};

// ---------- Integer to String ----------

template<int N, int IDX = 0>
struct WriteInt {
    template<typename Arr>
    static void apply(Arr& buf, int& pos) {
        WriteInt<N / 10, IDX>::apply(buf, pos);
        buf[pos++] = '0' + (N % 10);
    }
};

template<int IDX>
struct WriteInt<0, IDX> {
    template<typename Arr>
    static void apply(Arr& buf, int& pos) {
        buf[pos++] = '0';
    }
};

// ---------- ConstStr ----------

template<size_t N>
struct ConstStr {
    std::array<char, N> data{};
    size_t pos = 0;

    void push(char c) {
        data[pos++] = c;
    }

    void push_str(const char* s) {
        while (*s)
            data[pos++] = *s++;
    }

    void push_int(int value) {
        // simple recursive integer write
        char buf[20]{};
        int p = 0;
        int tmp = value;
        char tmpbuf[20]{};
        int idx = 0;

        // convert int to string
        int div = 1;
        while (tmp / div >= 10) div *= 10;
        while (div > 0) {
            tmpbuf[idx++] = '0' + tmp / div;
            tmp %= div;
            div /= 10;
        }
        for (int i = 0; i < idx; ++i)
            push(tmpbuf[i]);
    }
};

// ---------- MakeJSON ----------

template<int N>
struct MakeJSON {
    static constexpr size_t max_size = 1024;

    static std::array<char, max_size> make() {
        ConstStr<max_size> s{};
        s.push_str("{ \"groups\": [\n");

        constexpr int count = Divisors<N>::value;
        std::array<int, count> divs{};
        FillDivisors<N>::fill(divs);

        for (int i = 0; i < count; ++i) {
            s.push_str("  { \"name\": \"group by ");
            s.push_int(divs[i]);
            s.push_str("\", \"settings\": [ { \"num_groups\": ");
            s.push_int(N / divs[i]);
            s.push_str(" } ] }");
            if (i + 1 < count)
                s.push_str(",\n");
        }

        s.push_str("\n] }\0");
        return s.data;
    }
};