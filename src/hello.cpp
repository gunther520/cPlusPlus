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

void showMenu()
{
  cout << "\n==== AVL Tree Menu ====\n";
  cout << "1: Insert\n";
  cout << "2: Delete\n";
  cout << "3: Search\n";
  cout << "4: Print In-Order\n";
  cout << "5: Print Tree Structure\n";
  cout << "6: Exit\n";
  cout << "Select option: ";
}

int main(int argc, char *argv[])
{
  // 開始一條使用 "hello_world" function 的新執行緒
  // boost::thread my_thread(&hello_world);
  // 等待執行緒完成工作
  // my_thread.join();
  AvlTree<int> tree(0); // or prompt user for initial value
  int option, value;

  while (true)
  {
    showMenu();
    cin >> option;
    cout << '\n';
    switch (option)
    {
    case 1:
      cout << "Value to insert: ";
      cin >> value;
      tree.insert(value);
      break;
    case 2:
      cout << "Value to delete: ";
      cin >> value;
      tree.remove(value);
      break;
    case 3:
      cout << "Value to search: ";
      cin >> value;
      cout << (tree.search(value) ? "Found" : "Not found") << endl;
      break;
    case 4:
      cout << "In-order: ";
      tree.printInOrder(tree.root);
      cout << endl;
      break;
    case 5:
      cout << "Tree structure:\n";
      tree.printTree(tree.root);
      break;
    case 6:

      hello_world();
      return 0;
    }
  }

  return 0;
}