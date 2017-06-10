//
// Created by Jacek Dziubinski on 2017-06-09.
//

#ifndef PROJECT_BYTES_CONVERTER_H
#define PROJECT_BYTES_CONVERTER_H

class bytes_converter {
public:
    static int get_int(const char *buffer)
    {
        return int((unsigned char) (buffer[3]) << 24 |
                   (unsigned char) (buffer[2]) << 16 |
                   (unsigned char) (buffer[1]) << 8 |
                   (unsigned char) (buffer[0]));
    }

    static unsigned int get_uint(const char *buffer)
    {
        return unsigned((unsigned char) (buffer[3]) << 24 |
                        (unsigned char) (buffer[2]) << 16 |
                        (unsigned char) (buffer[1]) << 8 |
                        (unsigned char) (buffer[0]));
    }

    static std::vector<char> get_bytes_int(int obj)
    {
        std::vector<char> v(sizeof(int));
        for (unsigned i = 0; i < sizeof(int); ++i) {
            v[i] = obj & 0xFF;
            obj >>= 8;
        }
        return v;
    }
};

#endif //PROJECT_BYTES_CONVERTER_H
