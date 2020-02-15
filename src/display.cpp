// sherpa_41's Display module, licensed under MIT. (c) hafiz, 2018

#ifndef DISPLAY_CPP
#define DISPLAY_CPP

#include "display.h"

#include "renderer/renderer.h"

/**
 * Pure virtual dtor
 */
Display::Command::~Command() = default;

/**
 * Creates a queue of display commands to execute
 * @param root root layout node
 * @return queue of commands
 */
auto Display::Command::createQueue(const Layout::BoxPtr& root) -> Display::CommandQueue {
  CommandQueue queue;
  renderBox(root, queue);
  return queue;
}

/**
 * Creates the commands to render a box
 * @param box box to render
 * @param queue queue to add commands to
 */
void Display::Command::renderBox(const Layout::BoxPtr& box, Display::CommandQueue& queue) {
  renderBackground(box, queue);
  renderBorders(box, queue);
  // TODO: renderText

  // draw children on top of parent
  const auto children = box->getChildren();
  for (const auto& child : children) {
    renderBox(child, queue);
  }
}

/**
 * Creates the commands to render the background of a box
 * @param box box to render background of
 * @param queue queue to add commands to
 */
void Display::Command::renderBackground(const Layout::BoxPtr& box,
                                        Display::CommandQueue& queue) {
  auto colorPtr = getColor(box, "background-color", "background");
  // only render box if it actually has a background
  if (auto color = dynamic_cast<CSS::ColorValue*>(colorPtr.get())) {
    // create rectangle of padding area and background color
    queue.push(CommandPtr(new RectangleCmd(box->getDimensions().paddingArea(), *color)));
  }
}

void Display::Command::renderBorders(const Layout::BoxPtr& box,
                                     Display::CommandQueue& queue) {
  // use background if no explicit border color provided
  auto colorPtr = getColor(box, "border-color", "background-color", "background");
  auto color = dynamic_cast<CSS::ColorValue*>(colorPtr.get());
  if (color == nullptr) {
    return;  // nothing to render if no border color
  }

  const auto dims = box->getDimensions();
  const auto borderArea = dims.borderArea();

  // top border
  queue.push(
      CommandPtr(new RectangleCmd(Layout::Rectangle(borderArea.origin.x, borderArea.origin.y,
                                                    borderArea.width, dims.border.top),
                                  *color)));
  // right border
  queue.push(CommandPtr(new RectangleCmd(
      Layout::Rectangle(borderArea.origin.x + borderArea.width - dims.border.right,
                        borderArea.origin.y, dims.border.right, borderArea.height),
      *color)));
  // bottom border
  queue.push(CommandPtr(new RectangleCmd(
      Layout::Rectangle(borderArea.origin.x,
                        borderArea.origin.y + borderArea.height - dims.border.bottom,
                        borderArea.width, dims.border.bottom),
      *color)));
  // left border
  queue.push(
      CommandPtr(new RectangleCmd(Layout::Rectangle(borderArea.origin.x, borderArea.origin.y,
                                                    dims.border.left, borderArea.height),
                                  *color)));
}

/**
 * Gets the color value of a style from a box, or nullptr if no color is
 * specified for that style
 * @tparam Args variadic arguments, should be strings
 * @param box box to get style from
 * @param style style to look up
 * @param backup backup styles to look up
 * @return color value, or nullptr if it does not exist
 */
template <typename... Args>
auto Display::Command::getColor(const Layout::BoxPtr& box,
                                const std::string& style,
                                const Args&... backup) -> CSS::ValuePtr {
  if (auto sBox = dynamic_cast<Layout::StyledBox*>(box.get())) {
    auto bg = sBox->getContent().value(style, backup...);
    return bg ? bg->clone() : nullptr;
  }
  return nullptr;
}

/**
 * Command to create a rectangle of a color
 * @param rectangle rectangle to create
 * @param color color to color rectangle
 */
Display::RectangleCmd::RectangleCmd(const Layout::Rectangle& rectangle,
                                    const CSS::ColorValue& color)
    : rectangle(rectangle), color(color) {}

void Display::RectangleCmd::acceptRenderer(Renderer& renderer) const {
  renderer.render(*this);
}

/**
 * Returns encompassing rectangle
 * @return rectangle
 */
auto Display::RectangleCmd::getRectangle() const -> Layout::Rectangle {
  return rectangle;
}

/**
 * Returns color
 * @return color
 */
auto Display::RectangleCmd::getColor() const -> CSS::ColorValue {
  return color;
}

#endif
