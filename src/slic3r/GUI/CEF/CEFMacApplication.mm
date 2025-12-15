#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#import <objc/message.h>
#include <include/cef_application_mac.h>

static BOOL g_handlingSendEvent = NO;

static BOOL swizzled_isHandlingSendEvent(id self, SEL _cmd) {
    return g_handlingSendEvent;
}

static void swizzled_setHandlingSendEvent(id self, SEL _cmd, BOOL handling) {
    g_handlingSendEvent = handling;
}

static void swizzled_sendEvent(id self, SEL _cmd, NSEvent* event) {
    CefScopedSendingEvent sendingEventScoper;
    ((void (*)(id, SEL, NSEvent*))objc_msgSend)(self, @selector(cef_sendEvent:), event);
}

namespace Slic3r { namespace GUI { namespace CEFMacApplication {

bool SwizzleNSApplication() {
    Class appClass = [NSApplication class];

    if (!class_conformsToProtocol(appClass, @protocol(CefAppProtocol))) {
        class_addProtocol(appClass, @protocol(CefAppProtocol));
    }

    if (!class_getInstanceMethod(appClass, @selector(isHandlingSendEvent))) {
        class_addMethod(appClass,
                       @selector(isHandlingSendEvent),
                       (IMP)swizzled_isHandlingSendEvent,
                       "B@:");
    }

    if (!class_getInstanceMethod(appClass, @selector(setHandlingSendEvent:))) {
        class_addMethod(appClass,
                       @selector(setHandlingSendEvent:),
                       (IMP)swizzled_setHandlingSendEvent,
                       "v@:B");
    }

    Method originalSendEvent = class_getInstanceMethod(appClass, @selector(sendEvent:));
    if (originalSendEvent) {
        Method swizzledSendEvent = class_getInstanceMethod(appClass, @selector(cef_sendEvent:));
        if (!swizzledSendEvent) {
            class_addMethod(appClass,
                           @selector(cef_sendEvent:),
                           (IMP)swizzled_sendEvent,
                           method_getTypeEncoding(originalSendEvent));
            swizzledSendEvent = class_getInstanceMethod(appClass, @selector(cef_sendEvent:));
        }

        if (swizzledSendEvent) {
            method_exchangeImplementations(originalSendEvent, swizzledSendEvent);
        }
    }

    return true;
}

}}}
