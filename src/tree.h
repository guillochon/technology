template <class Leaf> class Node {

private:
  Leaf data;

public:
  Node(Leaf data);
  Leaf get_data();
};
