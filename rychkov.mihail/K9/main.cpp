#include <iostream>
#include <functional>
#include <utility>

template< class T, class Cmp >
struct TriTree
{
  std::pair< T, T > data;
  TriTree< T, Cmp >* left = nullptr;
  TriTree< T, Cmp >* middle = nullptr;
  TriTree< T, Cmp >* right = nullptr;
  TriTree< T, Cmp >* parent = nullptr;
};

template< class T, class Cmp >
struct TriTreeIterator
{
  TriTree< T, Cmp >* node = nullptr;
  bool pointsToRight = false;
  TriTree< T, Cmp >* prevSaver = nullptr;
  TriTree< T, Cmp >* nextSaver = nullptr;

  bool hasNext() const noexcept
  {
    return (node != nullptr) || (nextSaver != nullptr);
  }
  bool hasPrev() const noexcept
  {
    return (node != nullptr) || (prevSaver != nullptr);
  }

  TriTreeIterator prev() const noexcept
  {
    if (node == nullptr)
    {
      return {prevSaver, true};
    }
    TriTree< T, Cmp >* current = node;
    if (pointsToRight)
    {
      if (node->middle == nullptr)
      {
        return {node, false, node};
      }
      current = node->middle;
    }
    else
    {
      if (node->left == nullptr)
      {
        TriTree< T, Cmp >* prev = node;
        current = node->parent;
        for (; (current != nullptr) && (current->left == prev); prev = current, current = current->parent)
        {}
        return {current, (current != nullptr) && (current->middle != prev), node};
      }
      current = node->left;
    }
    for (; current->right != nullptr; current = current->right)
    {}
    return {current, true, node};
  }
  TriTreeIterator next() const noexcept
  {
    if (node == nullptr)
    {
      return {nextSaver, false};
    }
    TriTree< T, Cmp >* current = node;
    if (pointsToRight)
    {
      if (node->right == nullptr)
      {
        TriTree< T, Cmp >* prev = node;
        current = node->parent;
        for (; (current != nullptr) && (current->right == prev); prev = current, current = current->parent)
        {}
        return {current, (current != nullptr) && (current->middle == prev)};
      }
      current = node->right;
    }
    else
    {
      if (node->middle == nullptr)
      {
        return {node, true};
      }
      current = node->middle;
    }
    for (; current->left != nullptr; current = current->left)
    {}
    return {current, false};
  }

  std::pair< T, T >& data()
  {
    return node->data;
  }
  T& pointed()
  {
    return pointsToRight ? node->data.second : node->data.first;
  }
};

template< class T, class Cmp >
TriTreeIterator< T, Cmp > begin(TriTree< T, Cmp >* root)
{
  if (root == nullptr)
  {
    return {};
  }
  for (; root->left != nullptr; root = root->left)
  {}
  return {root, false};
}

template< class T, class Cmp >
TriTreeIterator< T, Cmp > rbegin(TriTree< T, Cmp >* root)
{
  if (root == nullptr)
  {
    return {};
  }
  for (; root->right != nullptr; root = root->right)
  {}
  return {root, true};
}

template< class T, class Cmp1, class Cmp2 = Cmp1 >
TriTreeIterator< T, Cmp1 > lower_bound(TriTree< T, Cmp1 >* root, const T& value, Cmp2 compare = Cmp1())
{
  if (root == nullptr)
  {
    return {};
  }
  TriTreeIterator< T, Cmp1 > left = {root}, right = {};
  while (true)
  {
    if (compare(value, left.node->data.first))
    {
      right = {left.node, false};
      if (left.node->left == nullptr)
      {
        return right;
      }
      left = {left.node->left, false};
    }
    else if (compare(value, left.node->data.first))
    {
      return {left.node, false};
    }
    else if (compare(value, left.node->data.second))
    {
      right = {left.node, true};
      if (left.node->middle == nullptr)
      {
        return right;
      }
      left = {left.node->middle, false};
    }
    else if (compare(value, left.node->data.second))
    {
      return {left.node, true};
    }
    else
    {
      if (left.node->right == nullptr)
      {
        if (right.node == nullptr)
        {
          return {nullptr, false, rbegin(root).node};
        }
        return right;
      }
      left = {left.node->right, false};
    }
  }
}
template< class T, class Cmp >
TriTreeIterator< T, Cmp > upper_bound(const TriTree< T, Cmp >* root, const T& value, Cmp compare = Cmp())
{
  class less_equal
  {
    bool operator()(const T& lhs, const T& rhs)
    {
      return !compare(rhs, lhs);
    }
    Cmp compare;
  };
  return lower_bound(root, value, less_equal(std::move(compare)));
}

template< class T, class Cmp >
TriTree< T, Cmp >* insert(TriTree< T, Cmp >* root, std::pair< T, T > data)
{
  Cmp compare;
  if (compare(data.second, data.first))
  {
    std::swap(data.first, data.second);
  }
  if (root == nullptr)
  {
    return new TriTree< T, Cmp >{std::move(data)};
  }
  TriTreeIterator< T, Cmp > place = lower_bound(root, data.first);
  if (place.hasNext() && !compare(data.second, place.pointed()))
  {
    return root;
  }
  TriTree< T, Cmp >* newNode = new TriTree< T, Cmp >{std::move(data)};
  if (!place.hasNext())
  {
    place = place.prev();
    place.node->right = newNode;
    newNode->parent = place.node;
  }
  else if (place.pointsToRight)
  {
    if (place.node->middle != nullptr)
    {
      place = place.prev();
      place.node->right = newNode;
    }
    else
    {
      place.node->middle = newNode;
    }
    newNode->parent = place.node;
  }
  else
  {
    if (place.node->left != nullptr)
    {
      place = place.prev();
      place.node->right = newNode;
    }
    else
    {
      place.node->left = newNode;
    }
    newNode->parent = place.node;
  }
  return root;
}

template< class T, class Cmp >
void destroy(TriTree< T, Cmp >* root)
{
  if (root == nullptr)
  {
    return;
  }
  destroy(root->left);
  destroy(root->middle);
  destroy(root->right);
  delete root;
}

int main()
{
  TriTree< int, std::less<> >* root = nullptr;
  root = insert(root, {0, 4});
  root = insert(root, {1, 2});
  root = insert(root, {-4, -3});
  root = insert(root, {-2, -1});
  root = insert(root, {-4, -3});
  for (auto it = begin(root); it.hasNext(); it = it.next())
  {
    std::cout << it.pointed() << '\n';
  }
  destroy(root);
}
