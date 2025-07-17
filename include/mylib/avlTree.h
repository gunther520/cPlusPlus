#include <iostream>

template <typename T>
class AvlNode
{
public:
    AvlNode(T val);
    ~AvlNode();
    T val;
    AvlNode *left;
    AvlNode *right;
    int height;
    int bf;
    void updateMetrics();
};

template <typename T>
class AvlTree
{
private:
    bool insertion(AvlNode<T> *&root, T val);
    bool removal(AvlNode<T> *&root, T val);
    bool searchNow(AvlNode<T> *root, T val);
    AvlNode<T> *leftRotation(AvlNode<T> *root);
    AvlNode<T> *rightRotation(AvlNode<T> *root);

    void clear(AvlNode<T> *node)
    {
        if (node != nullptr)
        {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

public:
    void printInOrder(AvlNode<T> *root);
    void printTree(AvlNode<T> *root, int d = 0);
    AvlNode<T> *root;
    AvlTree(T val);
    ~AvlTree();
    bool search(T val);
    void insert(T val);
    void remove(T val);
};

template <typename T>
void AvlTree<T>::printInOrder(AvlNode<T> *rt)
{
    if (!rt)
        return;
    printInOrder(rt->left);
    std::cout << rt->val << " ";
    printInOrder(rt->right);
}

template <typename T>
void AvlTree<T>::printTree(AvlNode<T> *rt, int depth)
{
    if (!rt)
        return;
    for (int i = 0; i < depth; ++i)
        std::cout << "  ";
    std::cout << rt->val << " (h: " << rt->height << ", bf: " << rt->bf << ")\n";
    printTree(rt->left, depth + 1);
    printTree(rt->right, depth + 1);
}

template <typename T>
AvlNode<T>::AvlNode(T val) : val(val), left(nullptr), right(nullptr), height(1), bf(0)
{
}

template <typename T>
AvlNode<T>::~AvlNode()
{
}

template <typename T>
AvlTree<T>::AvlTree(T val)
{
    root = new AvlNode(val);
}

template <typename T>
AvlTree<T>::~AvlTree()
{
    clear(root);
}

template <typename T>
bool AvlTree<T>::searchNow(AvlNode<T> *rt, T v)
{
    if (rt == nullptr)
    {
        return false;
    }

    if (rt->val == v)
    {
        return true;
    }
    else if (v < rt->val)
    {
        return this->searchNow(rt->left, v);
    }
    return this->searchNow(rt->right, v);
}

template <typename T>
bool AvlTree<T>::search(T v)
{
    return this->searchNow(this->root, v);
}

template <typename T>
bool AvlTree<T>::insertion(AvlNode<T> *&rt, T v)
{
    if (v < rt->val)
    {
        if (rt->left == nullptr)
        {
            rt->left = new AvlNode(v);
        }
        else
        {
            if (!insertion(rt->left, v))
            {
                return false;
            }
        }
        rt->updateMetrics();
        if (rt->bf > 1)
        {
            // rotate
            if (rt->left->bf <= -1)
            { // LR
                rt->left = leftRotation(rt->left);
            }
            rt = rightRotation(rt);
        }
    }
    else if (rt->val < v)
    {
        if (rt->right == nullptr)
        {
            rt->right = new AvlNode(v);
        }
        else
        {
            if (!insertion(rt->right, v))
            {
                return false;
            };
        }
        rt->updateMetrics();

        if (rt->bf < -1)
        {
            if (rt->right->bf >= 1)
            {
                rt->right = rightRotation(rt->right);
            }
            rt = leftRotation(rt);
        }
    }
    else
    {
        return false;
    }
    return true;
}

template <typename T>
void AvlTree<T>::insert(T v)
{
    insertion(root, v);
}

template <typename T>
bool AvlTree<T>::removal(AvlNode<T> *&rt, T v)
{
    if (rt == nullptr)
    {
        return false;
    }

    if (v < rt->val)
    {
        removal(rt->left, v);
    }
    else if (rt->val < v)
    {
        removal(rt->right, v);
    }
    else
    { // this is the node to be removed
        if (rt->left == nullptr and rt->right == nullptr)
        {
            delete rt;
            rt = nullptr;
            return true;
        }
        else if (rt->left == nullptr)
        {
            //    AvlNode<T> temp = *(rt->right);
            //    delete rt;
            //    rt = &temp;

            AvlNode<T> *temp = rt;
            rt = rt->right;
            delete temp;
        }
        else if (rt->right == nullptr)
        {
            //    AvlNode<T> temp = *(rt->left);
            //    delete rt;
            //   rt = &temp;

            AvlNode<T> *temp = rt;
            rt = rt->left;
            delete temp;
        }
        else
        { // two children
            AvlNode<T> *cur = rt->left;
            while (cur->right != nullptr)
            {
                cur = cur->right;
            }
            rt->val = cur->val;
            removal(rt->left, rt->val);
        }
    }

    rt->updateMetrics();
    // rotation check
    if (rt->bf > 1)
    {
        if (rt->left && rt->left->bf <= -1)
        {
            rt->left = leftRotation(rt->left);
        }
        rt = rightRotation(rt);
    }
    else if (rt->bf && rt->bf < -1)
    {
        if (rt->right->bf >= 1)
        {
            rt->right = rightRotation(rt->right);
        }
        rt = leftRotation(rt);
    }

    return false;
}

template <typename T>
void AvlTree<T>::remove(T val)
{
    removal(root, val);
};

template <typename T>
AvlNode<T> *AvlTree<T>::leftRotation(AvlNode<T> *rt)
{
    AvlNode<T> *rt_right = rt->right;
    AvlNode<T> *rt_right_left = rt_right->left;

    rt->right = rt_right_left;
    rt_right->left = rt;
    rt->updateMetrics();
    rt_right->updateMetrics();

    return rt_right;
}

template <typename T>
AvlNode<T> *AvlTree<T>::rightRotation(AvlNode<T> *rt)
{
    AvlNode<T> *rt_left = rt->left;
    AvlNode<T> *rt_left_right = rt_left->right;

    rt->left = rt_left_right;
    rt_left->right = rt;
    rt->updateMetrics();
    rt_left->updateMetrics();
    return rt_left;
}

template <typename T>
void AvlNode<T>::updateMetrics()
{
    int left_height = (left == nullptr) ? 0 : left->height;
    int right_height = (right == nullptr) ? 0 : right->height;
    height = std::max(left_height, right_height) + 1;
    bf = left_height - right_height;
}
