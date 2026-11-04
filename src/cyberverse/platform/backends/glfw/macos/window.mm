/**
 * @file window.mm
 * @author simon2077s (simon2077s@gmail.com)
 * @brief macOS native window helpers
 * @version 0.1
 * @date 2026-10-21
 *
 * @copyright Copyright (c) 2026
 *
 */

#define GLFW_EXPOSE_NATIVE_COCOA 1

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>

#include <algorithm>
#include <cstring>

#include "cyberverse/platform/backends/glfw/macos/window.hpp"
#include "cyberverse/platform/backends/glfw/native.hpp"
#include "cyberverse/platform/backends/glfw/window.hpp"

static char ime_bridge_key;

@interface CVImeBridge : NSObject
@property(nonatomic, assign) void* owner;
- (instancetype)initWithOwner:(void*)owner;
@end

@implementation CVImeBridge
- (instancetype)initWithOwner:(void*)owner {
  self = [super init];
  if (self != nil) {
    _owner = owner;
  }
  return self;
}
@end

namespace cyberverse::platform::backends::glfw {

namespace {

// MoltenVK implements the Vulkan API on top of Metal. To render into a GLFW
// window we have to install a CAMetalLayer on its content view. glfwCreateWindowSurface
// does the same internally, but DiligentEngine creates the Vulkan surface inside
// its own ISwapChain implementation and cannot call the GLFW helper.
CAMetalLayer* install_metal_layer(GLFWwindow* window) noexcept {
  if (window == nullptr) {
    return nil;
  }

  NSWindow* ns_window = glfwGetCocoaWindow(window);
  if (ns_window == nil) {
    return nil;
  }

  NSView* view = [ns_window contentView];
  if (view == nil) {
    return nil;
  }

  CAMetalLayer* layer = [CAMetalLayer layer];
  [view setLayer:layer];
  [view setWantsLayer:YES];

  // Match the layer's drawable size to the backing store scale so that the
  // renderer receives retina-sized drawables.
  const CGSize view_scale = [view convertSizeToBacking:CGSizeMake(1.0, 1.0)];
  [layer setContentsScale:MIN(view_scale.width, view_scale.height)];

  return layer;
}

CAMetalLayer* ensure_metal_layer(GLFWwindow* window) noexcept {
  if (window == nullptr) {
    return nil;
  }

  NSWindow* ns_window = glfwGetCocoaWindow(window);
  if (ns_window == nil) {
    return nil;
  }

  NSView* view = [ns_window contentView];
  if (view == nil) {
    return nil;
  }

  id layer = [view layer];
  if ([layer isKindOfClass:[CAMetalLayer class]]) {
    return static_cast<CAMetalLayer*>(layer);
  }
  return install_metal_layer(window);
}

} // namespace

void* get_ns_window_view(GLFWwindow* window) noexcept {
  if (window == nullptr) {
    return nullptr;
  }

  NSWindow* ns_window = glfwGetCocoaWindow(window);
  if (ns_window == nil) {
    return nullptr;
  }

  NSView* view = [ns_window contentView];
  if (view == nil) {
    return nullptr;
  }

  ensure_metal_layer(window);
  return (__bridge void*)view;
}

void* get_metal_layer(GLFWwindow* window) noexcept { return (__bridge void*)ensure_metal_layer(window); }

cyberverse::platform::native_window_handle query_native_handle(GLFWwindow* window) noexcept {
  cyberverse::platform::native_window_handle handle;
  handle.backend = cyberverse::platform::backend_type::glfw;
  if (window == nullptr) {
    return handle;
  }

  NSWindow* ns_window = glfwGetCocoaWindow(window);
  handle.window = (__bridge void*)ns_window;
  handle.view = get_ns_window_view(window);
  handle.surface = get_metal_layer(window);
  return handle;
}

namespace {

using cyberverse::platform::backends::glfw::window;

using set_marked_text_fn = void (*)(id, SEL, id, NSRange, NSRange);
using unmark_text_fn = void (*)(id, SEL);
using insert_text_fn = void (*)(id, SEL, id, NSRange);
using first_rect_fn = NSRect (*)(id, SEL, NSRange, NSRangePointer);

set_marked_text_fn original_set_marked_text = nullptr;
unmark_text_fn original_unmark_text = nullptr;
insert_text_fn original_insert_text = nullptr;
first_rect_fn original_first_rect = nullptr;

window* owner_for_view(id view) {
  CVImeBridge* bridge = objc_getAssociatedObject(view, &ime_bridge_key);
  return bridge != nil ? static_cast<window*>(bridge.owner) : nullptr;
}

NSString* ime_string(id value) {
  if (value == nil) {
    return nil;
  }
  if ([value isKindOfClass:[NSAttributedString class]]) {
    return [static_cast<NSAttributedString*>(value) string];
  }
  return static_cast<NSString*>(value);
}

void capture_marked_text(id view, id string, NSRange selected) {
  window* owner = owner_for_view(view);
  if (owner == nullptr) {
    return;
  }
  NSString* text = ime_string(string);
  if (text == nil || [text length] == 0) {
    owner->set_ime_composition({});
    return;
  }

  NSUInteger end = NSMaxRange(selected);
  if (end > [text length]) {
    end = [text length];
  }
  NSString* prefix = [text substringToIndex:end];
  const char* utf8 = [text UTF8String];
  const char* prefix_utf8 = [prefix UTF8String];
  if (utf8 == nullptr || prefix_utf8 == nullptr) {
    owner->set_ime_composition({});
    return;
  }
  owner->set_ime_composition(cyberverse::platform::ime_state{
      .text = utf8,
      .cursor = std::strlen(prefix_utf8),
      .active = true,
  });
}

void clear_marked_text(id view) {
  if (window* owner = owner_for_view(view); owner != nullptr) {
    owner->set_ime_composition({});
  }
}

void install_method_swizzles(Class view_class) {
  const auto replace = [view_class](SEL selector, IMP replacement) -> IMP {
    Method method = class_getInstanceMethod(view_class, selector);
    if (method == nullptr) {
      return nullptr;
    }
    IMP original = method_getImplementation(method);
    method_setImplementation(method, replacement);
    return original;
  };

  original_set_marked_text = reinterpret_cast<set_marked_text_fn>(
      replace(@selector(setMarkedText:selectedRange:replacementRange:),
              reinterpret_cast<IMP>(+[](id self, SEL, id string, NSRange selected, NSRange replacement) {
                if (original_set_marked_text != nullptr) {
                  original_set_marked_text(
                      self, @selector(setMarkedText:selectedRange:replacementRange:), string, selected, replacement);
                }
                capture_marked_text(self, string, selected);
              })));
  original_unmark_text =
      reinterpret_cast<unmark_text_fn>(replace(@selector(unmarkText), reinterpret_cast<IMP>(+[](id self, SEL) {
                                                 if (original_unmark_text != nullptr) {
                                                   original_unmark_text(self, @selector(unmarkText));
                                                 }
                                                 clear_marked_text(self);
                                               })));
  original_insert_text = reinterpret_cast<insert_text_fn>(replace(
      @selector(insertText:replacementRange:), reinterpret_cast<IMP>(+[](id self, SEL, id string, NSRange replacement) {
        if (original_insert_text != nullptr) {
          original_insert_text(self, @selector(insertText:replacementRange:), string, replacement);
        }
        clear_marked_text(self);
      })));
  original_first_rect = reinterpret_cast<first_rect_fn>(replace(
      @selector(firstRectForCharacterRange:actualRange:),
      reinterpret_cast<IMP>(+[](id self, SEL, NSRange range, NSRangePointer actual) -> NSRect {
        window* owner = owner_for_view(self);
        if (owner == nullptr || original_first_rect == nullptr) {
          return original_first_rect != nullptr
                     ? original_first_rect(self, @selector(firstRectForCharacterRange:actualRange:), range, actual)
                     : NSZeroRect;
        }
        const cyberverse::platform::ime_caret caret = owner->get_ime_caret();
        if (!caret.visible) {
          return original_first_rect(self, @selector(firstRectForCharacterRange:actualRange:), range, actual);
        }
        NSView* view = static_cast<NSView*>(self);
        const CGFloat height = std::max<CGFloat>(1.0, static_cast<CGFloat>(caret.height));
        const NSRect content_rect = NSMakeRect(static_cast<CGFloat>(caret.x),
                                               [view bounds].size.height - static_cast<CGFloat>(caret.y) - height,
                                               1.0,
                                               height);
        NSRect window_rect = [view convertRect:content_rect toView:nil];
        NSWindow* ns_window = [view window];
        return ns_window != nil ? [ns_window convertRectToScreen:window_rect] : NSZeroRect;
      })));
}

} // namespace

void install_ime_bridge(GLFWwindow* glfw_window, void* owner) noexcept {
  if (glfw_window == nullptr || owner == nullptr) {
    return;
  }
  NSWindow* ns_window = glfwGetCocoaWindow(glfw_window);
  if (ns_window == nil) {
    return;
  }
  NSView* view = [ns_window contentView];
  if (view == nil) {
    return;
  }

  static dispatch_once_t once;
  dispatch_once(&once, ^{
    install_method_swizzles(object_getClass(view));
  });

  CVImeBridge* bridge = [[CVImeBridge alloc] initWithOwner:owner];
  objc_setAssociatedObject(view, &ime_bridge_key, bridge, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

} // namespace cyberverse::platform::backends::glfw
