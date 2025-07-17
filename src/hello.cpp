#include "mylib/world.h"
#include "mylib/hello.h"
#include "mylib/avlTree.h"

using namespace std;

void hello_world()
{

  cout << "good bye " << endl;
  cout << AGE << endl;

  return;
}

int main(int argc, char *argv[])
{
  // 開始一條使用 "hello_world" function 的新執行緒
  // boost::thread my_thread(&hello_world);
  // 等待執行緒完成工作
  // my_thread.join();

  // 1. Create tree and insert a set of values
  AvlTree<int> tree(10);
  tree.insert(20); // RR case
  tree.insert(30); // triggers RR rotation
  tree.insert(25); // RL case (should trigger RL rotation)
  tree.insert(5);  // LL case
  tree.insert(2);  // triggers LL rotation
  tree.insert(8);  // LR case (should trigger LR rotation)
  tree.insert(15); // No rotation needed
  tree.insert(12); // Additional inserts to mix things up

  cout << "In-order traversal after inserting (should be sorted): ";
  tree.printInOrder(tree.root); // root is private; for test, make it public or use a getter
  cout << "\n\nTree structure:\n";
  tree.printTree(tree.root);

  // 2. Search test (should find existing and not find non-existing)
  cout << "\nSearching for 15: " << (tree.search(15) ? "found" : "not found") << endl;
  cout << "Searching for 99: " << (tree.search(99) ? "found" : "not found") << endl;

  // 3. Remove a leaf
  tree.remove(2); // leaf
  cout << "\nAfter removing leaf 2:\n";
  tree.printTree(tree.root);

  // 4. Remove a node with one child
  tree.remove(8); // one child
  cout << "\nAfter removing 8 (node with one child):\n";
  tree.printTree(tree.root);

  // 5. Remove node with two children
  tree.remove(20); // has children, in-order successor/predecessor used
  cout << "\nAfter removing 20 (node with two children):\n";
  tree.printTree(tree.root);

  // 6. Remove root (possibly challenging rebalance)
  tree.remove(10);
  cout << "\nAfter removing 10 (root):\n";
  tree.printTree(tree.root);

  // 7. Stress test: Insert elements to cause deep right-left then remove
  AvlTree<int> stressTree(100);
  for (int v : {150, 120, 110, 130, 170, 160, 180, 140, 115, 125})
  {
    stressTree.insert(v);
  }
  cout << "\nStress test tree:\n";
  tree.printTree(stressTree.root);

  stressTree.remove(130);
  cout << "\nAfter removing 130:\n";
  tree.printTree(stressTree.root);

  hello_world();
  return 0;
}