#include "technology.h"
#include "tree.h"

template <class Leaf> Node<Leaf>::Node(Leaf data) : data(data) {}

template <class Leaf> Leaf Node<Leaf>::get_data() { return data; }

// Instantiate the particular template variants we want.
template class Node<Technology>;
