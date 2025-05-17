#include <iostream>
#include <string>
#include <utility>
#include <functional>
#include <limits>

namespace rychkov
{
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
          return {node, false, nullptr, node};
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
          return {current, (current != nullptr) && (current->middle != prev), nullptr, node};
        }
        current = node->left;
      }
      for (; current->right != nullptr; current = current->right)
      {}
      return {current, true, nullptr, node};
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
          return {current, (current != nullptr) && (current->middle == prev), node};
        }
        current = node->right;
      }
      else
      {
        if (node->middle == nullptr)
        {
          return {node, true, node};
        }
        current = node->middle;
      }
      for (; current->left != nullptr; current = current->left)
      {}
      return {current, false, node};
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
  template< class T, class Cmp >
  bool equal(const TriTreeIterator< T, Cmp >& lhs, const TriTreeIterator< T, Cmp >& rhs)
  {
    return (lhs.node == rhs.node) && (lhs.pointsToRight == rhs.pointsToRight);
  }

  template< class T, class Cmp >
  TriTreeIterator< T, Cmp > lower_bound(TriTree< T, Cmp >* root, const T& value, Cmp compare = Cmp())
  {
    if (root == nullptr)
    {
      return {};
    }
    TriTreeIterator< T, Cmp > left = {root}, right = {};
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
      else if (!compare(left.node->data.first, value))
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
      else if (!compare(left.node->data.second, value))
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
  TriTreeIterator< T, Cmp > upper_bound(TriTree< T, Cmp >* root, const T& value, Cmp compare = Cmp())
  {
    if (root == nullptr)
    {
      return {};
    }
    TriTreeIterator< T, Cmp > left = {root}, right = {};
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
      else if (compare(value, left.node->data.second))
      {
        right = {left.node, true};
        if (left.node->middle == nullptr)
        {
          return right;
        }
        left = {left.node->middle, false};
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
}

int main()
{
  using node_type = rychkov::TriTree< int, std::less< int > >;
  using iterator = rychkov::TriTreeIterator< int, std::less< int > >;

  node_type* root = nullptr;
  size_t count = 0;
  if (!(std::cin >> count))
  {
    std::cerr << "failed to read input pairs count\n";
    return 1;
  }
  for (size_t i = 0; i < count; i++)
  {
    int temp1 = 0, temp2 = 0;
    if (!(std::cin >> temp1 >> temp2))
    {
      std::cerr << "failed to read pair #" << i + 1 << '\n';
      destroy(root);
      return 1;
    }
    root = rychkov::insert(root, {temp1, temp2});
  }

  std::string command;
  while (std::cin >> command)
  {
    int commandId = 0;
    if (command == "intersects")
    {
      commandId = 0;
    }
    else if (command == "covers")
    {
      commandId = 1;
    }
    else if (command == "avoids")
    {
      commandId = 2;
    }
    else
    {
      std::cerr << "unknown command\n";
      destroy(root);
      return 1;
    }

    int left = 0, right = 0;
    if (!(std::cin >> left >> right) || (left > right))
    {
      std::cout << "<INVALID COMMAND>\n";
      if (!std::cin.eof() && !std::cin.bad())
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
      }
      continue;
    }
    iterator from = rychkov::lower_bound(root, left), to = rychkov::upper_bound(root, right);
    size_t answer = 0;
    for (iterator i = from; !rychkov::equal(i, to); i = i.next())
    {
      answer++;
    }
    switch (commandId)
    {
    case 0:
      if (answer != 0)
      {
        answer += from.pointsToRight + to.pointsToRight;
      }
      answer /= 2;
      break;
    case 1:
      if (answer != 0)
      {
        answer -= from.pointsToRight + to.pointsToRight;
      }
      answer /= 2;
      break;
    case 2:
      if (answer != 0)
      {
        answer += from.pointsToRight + to.pointsToRight;
      }
      answer /= 2;
      answer = count - answer;
      break;
    }
    std::cout << answer << '\n';
  }
  destroy(root);
}
