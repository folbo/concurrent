//
// Created by folbo on 2017-02-05.
//

#ifndef PROJECT_FRAME_H
#define PROJECT_FRAME_H

enum class CommandType {
    Hello = 1,
    Please = 2,
    DotProduct = 3,
    DotProductChunked = 4
};

template <typename T>
struct frame {
    const static int header_length = 5;

    char type;
    unsigned int data_length;

    std::vector<char> get_bytes_int(int obj) const {
        std::vector<char> v(sizeof(int));
        for (unsigned i = 0; i < sizeof(int); ++i) {
            v[i] = obj & 0xFF;
            obj >>= 8;
        }
        return v;
    }

    unsigned int get_uint(const char *buffer) {
        return unsigned((unsigned char) (buffer[3]) << 24 |
                        (unsigned char) (buffer[2]) << 16 |
                        (unsigned char) (buffer[1]) << 8 |
                        (unsigned char) (buffer[0]));
    }
};


#endif //PROJECT_FRAME_H
