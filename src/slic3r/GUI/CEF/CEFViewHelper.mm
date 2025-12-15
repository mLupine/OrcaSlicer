#import <Cocoa/Cocoa.h>
#include "CEFViewHelper.hpp"

namespace Slic3r { namespace GUI { namespace CEFViewHelper {

void ConfigureParentView(void* viewHandle) {
    NSView* view = (__bridge NSView*)viewHandle;
    if (!view) {
        return;
    }

    [view setWantsLayer:YES];
}

void DebugViewHierarchy(void* viewHandle) {
}

}}}
