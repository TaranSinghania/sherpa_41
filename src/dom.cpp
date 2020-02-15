// sherpa_41's DOM module, licensed under MIT. (c) hafiz, 2018

#ifndef DOM_CPP
#define DOM_CPP

#include "dom.h"

#include <iterator>
#include <sstream>

#include "visitor/visitor.h"

/**
 * Inserts an attribute
 * @param attribute attribute to add
 * @param value value of attribute
 */
void DOM::AttributeMap::insert(const std::string& attribute, const std::string& value) {
  if (find(attribute) == end()) {
    (*this)[attribute] = value;
    order.push_back(attribute);
  }
}

auto DOM::AttributeMap::print() const -> std::string {
  if (empty()) {
    return "";
  }

  auto assign = [this](const auto& attr) {
    return attr + "=\"" + const_cast<AttributeMap&>(*this)[attr] + "\"";
  };
  auto start = order.begin();
  std::string first = order.front();
  return std::accumulate(
      std::next(start), order.end(), assign(first),
      [&assign](auto acc, auto attr) { return acc + " " + assign(attr); });
}

/**
 * Creates a DOM Node
 * @param tag node tag name
 */
DOM::Node::Node(std::string tag) : tag(std::move(tag)) {}

/**
 * Pure virtual destructor prevents unanticipated instantiation
 */
DOM::Node::~Node() = default;

/**
 * Determines whether the Node is of specified type
 * @param cand tag to match
 * @return whether Node is of `cand` type
 */
auto DOM::Node::is(const std::string& cand) const -> bool {
  return tagName() == cand;
}

/**
 * Returns the tag name of the Node
 * @return Node tag
 */
auto DOM::Node::tagName() const -> std::string {
  return tag;
}

/**
 * Creates a Text Node
 * @param tag node tag name
 * @param text node content
 */
DOM::TextNode::TextNode(std::string text) : Node("TEXT NODE"), text(std::move(text)) {}

/**
 * Returns text
 * @return text
 */
auto DOM::TextNode::getText() const -> std::string {
  return text;
}

/**
 * Accepts a visitor to the node
 * @param visitor accepted visitor
 */
void DOM::TextNode::acceptVisitor(Visitor& visitor) const {
  visitor.visit(*this);
}

/**
 * Clone the Text Node to a unique pointer
 * @return cloned Node
 */
auto DOM::TextNode::clone() -> DOM::NodePtr {
  return NodePtr(new TextNode(text));
}

/**
 * Creates a Comment Node
 * @param comment node content
 */
DOM::CommentNode::CommentNode(std::string comment)
    : Node("COMMENT NODE"), comment(std::move(comment)) {}

/**
 * Returns comment
 * @return comment
 */
auto DOM::CommentNode::getComment() const -> std::string {
  return comment;
}

/**
 * Accepts a visitor to the node
 * @param visitor accepted visitor
 */
void DOM::CommentNode::acceptVisitor(Visitor& visitor) const {
  visitor.visit(*this);
}

/**
 * Clone the Comment Node to a unique pointer
 * @return cloned Node
 */
auto DOM::CommentNode::clone() -> DOM::NodePtr {
  return NodePtr(new CommentNode(comment));
}

/**
 * Creates an Element Node
 * @param tag node tag name
 * @param attributes node attributes
 * @param children children nodes
 */
DOM::ElementNode::ElementNode(std::string tag,
                              AttributeMap attributes,
                              const NodeVector& children)
    : Node(std::move(tag)), attributes(std::move(attributes)), children() {
  this->children.reserve(children.size());
  std::for_each(children.begin(), children.end(),
                [this](const auto& child) { this->children.push_back(child->clone()); });
}

/**
 * Returns pointers to children nodes
 * @return children nodes
 */
auto DOM::ElementNode::getChildren() const -> DOM::NodeVector {
  NodeVector nodes;
  std::transform(children.begin(), children.end(), std::back_inserter(nodes),
                 [](const auto& child) { return child->clone(); });
  return nodes;
}

/**
 * Returns pretty-printed attributes
 * @return attributes
 */
auto DOM::ElementNode::getAttributes() const -> std::string {
  return attributes.print();
}

/**
 * Returns id of element
 * @return id
 */
auto DOM::ElementNode::getId() const -> std::string {
  auto id = attributes.find("id");
  return id != attributes.end() ? id->second : "";
}

/**
 * Returns classes of element
 * @return classes
 */
auto DOM::ElementNode::getClasses() const -> std::vector<std::string> {
  auto classes = attributes.find("class");
  if (classes == attributes.end()) {
    return {};
  }
  std::istringstream iss(classes->second);

  return {std::istream_iterator<std::string>{iss},
          std::istream_iterator<std::string>{}};  // split classes by space
}

/**
 * Accepts a visitor to the node
 * @param visitor accepted visitor
 */
void DOM::ElementNode::acceptVisitor(Visitor& visitor) const {
  visitor.visit(*this);
}

/**
 * Clone the Element Node to a unique pointer
 * @return cloned Node
 */
auto DOM::ElementNode::clone() -> DOM::NodePtr {
  return NodePtr(new ElementNode(tagName(), attributes, children));
}

#endif
