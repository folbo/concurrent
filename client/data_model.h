#include <vector>
#include <iostream>

#ifndef DATA_MODEL_H
#define	DATA_MODEL_H

enum class CommandType {
  Hello = 1,
  Please = 2,
  DotProduct = 3
};


class data_model {
public:

    data_model() {
        std::cout << "default constructor" << std::endl;
    }
    
    data_model(CommandType frame_type, std::vector<char> data) :
    frame_type(frame_type),
    data(data) {
        data_length = data.size();
    }
    
    data_model(const data_model& d) {
        data = d.data;
        data_length = d.data_length;
        std::cout << "copy constructor" << std::endl;
    }
    
    std::vector<char> serialize_frame() {
         std::vector<char> frame;
         frame.reserve(5 + data_length);
         
         /// HEADER
         // type
         frame.push_back((char)frame_type);
         // data len
         auto length = get_bytes_int(data_length);
         frame.insert(frame.end(), length.begin(), length.end());
         
         /// DATA
         frame.insert(frame.end(), data.begin(), data.end());
         
         return frame;
    }
    
private:
    
    CommandType frame_type;
    int data_length;
    std::vector<char> data;
    
    
private:
    
  int get_int(const char* buffer)
  {
    return int((unsigned char)(buffer[3]) << 24 |
      (unsigned char)(buffer[2]) << 16 |
      (unsigned char)(buffer[1]) << 8 |
      (unsigned char)(buffer[0]));
  }

  std::vector<char> get_bytes_int(int obj)
  {
    std::vector<char> v(sizeof(int));
    for(unsigned i = 0; i < sizeof(int); ++i) {
      v[i] = obj & 0xFF;
      obj >>= 8;
    }
    return v;
  }

};

#endif	/* DATA_MODEL_H */
