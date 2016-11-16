#include <dequeue>

namespace ellis {

ellis_array_node() {
  /* TODO: allow the the user hint the size of the array */
}

ellis_node& operator[](size_t index) {
  return m_data[index];
}

const ellis_node& operator[](size_t index) const {
  return m_data[index];
}

void append(const ellis_node &node) {
  /* TODO */
}

void append(ellis_node &&node) {
  /* TODO */
}

void extend(const ellis_array_node &other) {
/* TODO */
}

void extend(ellis_array_node &&other) {
/* TODO */
}

void insert(size_t position, const ellis_node &) {
/* TODO */
}

void insert(size_t position, ellis_node &&) {
/* TODO */
}

void erase(size_t position) {
/* TODO */
}

void reserve(size_t n) {
/* TODO */
}

void foreach(std::function<void(ellis_node &)> fn) {
/* TODO */
}

void foreach(std::function<void(const ellis_node &)> fn) const {
/* TODO */
}

ellis_array_node & filter(std::function<bool(const ellis_node &)> fn) const {
/* TODO */
}

size_t length() const {
/* TODO */
}

bool empty() const {
/* TODO */
}

void clear() {
/* TODO */
}

}
