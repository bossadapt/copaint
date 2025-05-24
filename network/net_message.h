#pragma once
#include "net_common.h"
#include <type_traits>
namespace olc {
namespace net {
template <typename T> struct message_header {
  T id{};
  uint32_t size = 0;
};

template <typename T> struct message {
  message_header<T> header{};
  std::vector<uint8_t> body;

  // returns size of entrie message packet in bytes
  size_t size() const { return sizeof(message_header<T>) + body.size(); }
  // Overide for std::cout
  friend std::ostream &operator<<(std::ostream &os, const message<T> &msg) {
    os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
    return os;
  }
  // Pushes any  POD-like data insto the message buffer
  template <typename DataType>
  friend message<T> &operator<<(message<T> &msg, const DataType &data) {
    static_assert(std::is_standard_layout<DataType>::value,
                  "Data is too complex to be pushed int vector");
    // cache current size of vector, as this will be point we instert the data
    size_t i = msg.body.size();
    msg.body.resize(msg.body.size() + sizeof(DataType));
    // copy data insto allocated vector space
    std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

    // recalculate the message size
    msg.header.size = msg.size();

    return msg;
  }
  // Pulls My POD-like data form the message buffer
  template <typename DataType>
  friend message<T> &operator>>(message<T> &msg, DataType &data) {
    // Check that the type of the data being pushed is trivially copyable
    static_assert(std::is_standard_layout<DataType>::value,
                  "Data is too complex to be pulled from vector");

    // Cache the location towards the end of the vector where the pulled data
    // starts
    size_t i = msg.body.size() - sizeof(DataType);

    // Physically copy the data from the vector into the user variable
    std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

    // Shrink the vector to remove read bytes, and reset end position
    msg.body.resize(i);

    // Recalculate the message size
    msg.header.size = msg.size();

    // Return the target message so it can be "chained"
    return msg;
  }
};

// Forward declare the connection
template <typename T> class connection;
template <typename T> struct owned_message {
  std::shared_ptr<connection<T>> remote = nullptr;
  message<T> msg;

  friend std::ostream &operator<<(std::ostream &os,
                                  const owned_message<T> &msg) {
    os << msg.msg;
    return os;
  }
};

} // namespace net

} // namespace olc