#import <Cocoa/Cocoa.h>
#include "CEFViewHelper.hpp"

namespace Slic3r { namespace GUI { namespace CEFViewHelper {

void ConfigureParentView(void* viewHandle, bool transparent) {
    NSView* view = (__bridge NSView*)viewHandle;
    if (!view) {
        return;
    }

    [view setWantsLayer:YES];

    if (transparent) {
        view.layer.backgroundColor = [NSColor clearColor].CGColor;
        view.layer.opaque = NO;
    }
}

void DebugViewHierarchy(void* viewHandle) {
}

}}}
