//
// KSDelegateForwarding.h
//
// Created by Karl on 2016-10-04.
// Copyright © 2016 Karl Stenerud. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall remain in place
// in this source code.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#import <objc/runtime.h>

#pragma mark - Class Implementation -

/** Begin the implementation of a delegate forwarding class.
 *
 *  @param CLASS_NAME What to name the class.
 *  @param DELEGATE_PROTOCOL The delegate protocol to expect.
 *  @param FORWARDING_TYPE The type of the object to forward to.
 *  @param DELEGATE_PROPERTY The name of the property the delegate will be assigned to.
 */
#define KSDF_BEGIN_DELEGATE_FORWARDING_IMPLEMENTATION(CLASS_NAME, DELEGATE_PROTOCOL, FORWARDING_TYPE, DELEGATE_PROPERTY) \
          KSDFI_BEGIN_DELEGATE_FORWARDING_IMPLEMENTATION(CLASS_NAME, DELEGATE_PROTOCOL, FORWARDING_TYPE, DELEGATE_PROPERTY)

/** Build a method that forwards the message to all delegates.
 *
 *  Parameters must be paired, except when the method has no argument.
 *  The first parameter in the pair will be the name part of the method signature argument.
 *  The secon parameter in the pair will be the type part of the method signature argument.
 *
 *  For example:
 *  - (void) receivedMessage --> FORWARD_TO_DELEGATES(receivedMessage)
 *  - (void) sessionDidFinish:(MySession*) --> FORWARD_TO_DELEGATES(sessionDidFinish,(MySession*))
 *  - (void) session:(MySession*) didSendMessage:(NSString*) --> FORWARD_TO_DELEGATES(session,(MySession*), didSendMessage,(NSString*))
 */
#define KSDF_FORWARD_TO_DELEGATES(...) \
          KSDFI_GET_FORWARD_TO_DELEGATES(__VA_ARGS__, \
          KSDFI_FORWARD_TO_DELEGATES_6, REQUIRES_12_ARGUMENTS, \
          KSDFI_FORWARD_TO_DELEGATES_5, REQUIRES_10_ARGUMENTS, \
          KSDFI_FORWARD_TO_DELEGATES_4, REQUIRES_8_ARGUMENTS, \
          KSDFI_FORWARD_TO_DELEGATES_3, REQUIRES_6_ARGUMENTS, \
          KSDFI_FORWARD_TO_DELEGATES_2, REQUIRES_4_ARGUMENTS, \
          KSDFI_FORWARD_TO_DELEGATES_1, \
          KSDFI_FORWARD_TO_DELEGATES_0, \
          ignored)(__VA_ARGS__)

/** Add a singleton method implementation.
 *
 *  @param THIS_CLASS This class name.
 *  @param METHOD_NAME The name of the singleton access method.
 *  @param FORWARDING_OBJECT_CREATOR Code that will give access to the object to forward to.
 *
 *  For example:
 *  ADD_FORWARDING_SINGLETON_METHOD(ForwardingMySession, defaultInstance, [MySession getSingleton])
 */
#define KSDF_ADD_FORWARDING_SINGLETON_METHOD(THIS_CLASS, METHOD_NAME, FORWARDING_OBJECT_CREATOR) \
          + (instancetype) METHOD_NAME \
          { \
              static id singletonInstance = nil; \
              static dispatch_once_t onceToken; \
              dispatch_once(&onceToken, ^{ \
                  singletonInstance = [[THIS_CLASS alloc] initWithForwardingObject:FORWARDING_OBJECT_CREATOR]; \
              }); \
              return singletonInstance; \
          }

/** End the implementation of a delegate forwarding class.
 */
#define KSDF_END_DELEGATE_FORWARDING_IMPLEMENTATION() \
          @end



#pragma mark - Internal Macros -

#define KSDFI_BEGIN_DELEGATE_FORWARDING_IMPLEMENTATION(CLASS_NAME, DELEGATE_PROTOCOL, FORWARDING_TYPE, DELEGATE_PROPERTY) \
@interface CLASS_NAME () \
@property(nonatomic,readwrite,strong) FORWARDING_TYPE forwardingObject; \
@property(nonatomic,readwrite,weak) id<DELEGATE_PROTOCOL> lastDelegate; \
@end \
@implementation CLASS_NAME \
\
static NSPointerArray* ksdf_delegateOrder(id self) \
{ \
    @synchronized([self forwardingObject]) \
    { \
        return (NSPointerArray*)objc_getAssociatedObject([self forwardingObject], \
                                                        @selector(ksdf_delegateOrder##DELEGATE_PROTOCOL)); \
    } \
} \
\
static void ksdf_initDelegateOrder(id self) \
{ \
    @synchronized([self forwardingObject]) \
    { \
        if(ksdf_delegateOrder(self) == nil) \
        { \
            objc_setAssociatedObject([self forwardingObject], \
                                     @selector(ksdf_delegateOrder##DELEGATE_PROTOCOL), \
                                     [NSPointerArray weakObjectsPointerArray], \
                                     OBJC_ASSOCIATION_RETAIN); \
        } \
    } \
} \
\
static void ksdf_addDelegate(id self, id delegate) \
{ \
    @synchronized([self forwardingObject]) \
    { \
        if(delegate != nil) \
        { \
            [ksdf_delegateOrder(self) addPointer:(__bridge void*)delegate]; \
        } \
    } \
} \
\
static void ksdf_removeDelegate(id self, id delegate) \
{ \
    @synchronized([self forwardingObject]) \
    { \
        if(delegate != nil) \
        { \
            NSPointerArray* pointers = ksdf_delegateOrder(self); \
            for(NSUInteger i = 0; i < pointers.count; i++) \
            { \
                if([pointers pointerAtIndex:i] == (__bridge void*)delegate) \
                { \
                    [pointers removePointerAtIndex:i]; \
                    return; \
                } \
            } \
        } \
    } \
} \
\
static void ksdf_cleanDelegateOrderHead(id self) \
{ \
    @synchronized([self forwardingObject]) \
    { \
        NSPointerArray* delegateOrder = ksdf_delegateOrder(self); \
        while(delegateOrder.count > 0 && [delegateOrder pointerAtIndex:0] == nil) \
        { \
            [delegateOrder removePointerAtIndex:0]; \
        } \
    } \
} \
\
static BOOL ksdf_isForwardingClass(id object) \
{ \
    return [[object class] respondsToSelector:@selector(ksdf_delegateOrder##DELEGATE_PROTOCOL)]; \
} \
\
static void ksdf_callIfIAmFirst(id self, void(^block)()) \
{ \
    @synchronized([self forwardingObject]) \
    { \
        ksdf_cleanDelegateOrderHead(self); \
        NSPointerArray* delegateOrder = ksdf_delegateOrder(self); \
        id first = (id)[delegateOrder pointerAtIndex:0]; \
        void* selfPointer = (__bridge void* _Nullable)self; \
        if(!ksdf_isForwardingClass(first)) \
        { \
            for(NSUInteger i = 0; i < delegateOrder.count; i++) \
            { \
                if([delegateOrder pointerAtIndex:i] == selfPointer) \
                { \
                    [delegateOrder replacePointerAtIndex:i withPointer:[delegateOrder pointerAtIndex:0]]; \
                    [delegateOrder replacePointerAtIndex:0 withPointer:selfPointer]; \
                } \
            } \
        } \
        if([delegateOrder pointerAtIndex:0] == selfPointer) \
        { \
            block(); \
        } \
    } \
} \
\
+ (id) ksdf_delegateOrder##DELEGATE_PROTOCOL {return nil;} \
\
- (id) initWithForwardingObject:(FORWARDING_TYPE) forwardingObject \
{ \
    if((self = [super init])) \
    { \
        self.forwardingObject = forwardingObject; \
        ksdf_initDelegateOrder(self); \
        ksdf_addDelegate(self, self); \
        ksdf_callIfIAmFirst(self, ^ \
        { \
            self.lastDelegate = forwardingObject.DELEGATE_PROPERTY; \
            ksdf_addDelegate(self, self.lastDelegate); \
            forwardingObject.DELEGATE_PROPERTY = self; \
        }); \
        [self.forwardingObject addObserver:self forKeyPath:@#DELEGATE_PROPERTY options:0 context:nil]; \
    } \
    return self; \
} \
\
- (void) dealloc \
{ \
    [self.forwardingObject removeObserver:self forKeyPath:@#DELEGATE_PROPERTY]; \
    ksdf_removeDelegate(self, self); \
} \
\
- (void) subscribeDelegate:(id<DELEGATE_PROTOCOL>) delegate \
{ \
    if(delegate != self) \
    { \
        ksdf_addDelegate(self, delegate); \
    } \
} \
\
- (void) unsubscribeDelegate:(id<DELEGATE_PROTOCOL>) delegate \
{ \
    if(delegate != self) \
    { \
        ksdf_removeDelegate(self, delegate); \
    } \
} \
\
- (void) observeValueForKeyPath:(__unused NSString*)keyPath \
                       ofObject:(__unused id)object \
                         change:(__unused NSDictionary<NSString*,id>*)change \
                        context:(__unused void*)context \
{ \
    @synchronized(self.forwardingObject) \
    { \
        ksdf_callIfIAmFirst(self, ^ \
        { \
            id<DELEGATE_PROTOCOL> delegate = self.forwardingObject.DELEGATE_PROPERTY; \
            if(delegate != self) \
            { \
                [self unsubscribeDelegate:self.lastDelegate]; \
                [self subscribeDelegate:delegate]; \
                self.lastDelegate = delegate; \
                self.forwardingObject.DELEGATE_PROPERTY = self; \
            } \
        }); \
    } \
} \
\
- (NSMethodSignature*) methodSignatureForSelector:(SEL)selector \
{ \
    return [self.forwardingObject methodSignatureForSelector:selector]; \
} \
\
- (BOOL) respondsToSelector:(SEL)selector \
{ \
    return [self.forwardingObject respondsToSelector:selector]; \
} \
\
- (void) forwardInvocation:(NSInvocation*)anInvocation \
{ \
    if ([self.forwardingObject respondsToSelector:[anInvocation selector]]) \
    { \
        [anInvocation invokeWithTarget:self.forwardingObject]; \
    } \
    else \
    { \
        [super forwardInvocation:anInvocation]; \
    } \
}

#define KSDFI_FORWARD_TO_DELEGATES_IMPL(SELECTOR, METHOD_CALL) \
    ksdf_callIfIAmFirst(self, ^ \
    { \
        NSPointerArray* delegateOrder = ksdf_delegateOrder(self); \
        for(NSInteger i = 0; i < (NSInteger)delegateOrder.count; i++) \
        { \
            id delegate = [delegateOrder pointerAtIndex:(NSUInteger)i]; \
            if(delegate == nil) \
            { \
                [delegateOrder removePointerAtIndex:(NSUInteger)i]; \
                i--; \
            } \
            else if([delegate respondsToSelector:SELECTOR]) \
            { \
                METHOD_CALL; \
            } \
        } \
    }); \

#define KSDFI_FORWARD_TO_DELEGATES_0(PART1) \
- (void)PART1 \
{ \
    KSDFI_FORWARD_TO_DELEGATES_IMPL(@selector(PART1), [delegate PART1]) \
}

#define KSDFI_FORWARD_TO_DELEGATES_1(PART1,TYPE1) \
- (void)PART1:TYPE1 arg1 \
{ \
    KSDFI_FORWARD_TO_DELEGATES_IMPL(@selector(PART1:), [delegate PART1:arg1]) \
}

#define KSDFI_FORWARD_TO_DELEGATES_2(PART1,TYPE1,PART2,TYPE2) \
- (void)PART1:TYPE1 arg1 PART2:TYPE2 arg2 \
{ \
    KSDFI_FORWARD_TO_DELEGATES_IMPL(@selector(PART1:PART2:), [delegate PART1:arg1 PART2:arg2]) \
}

#define KSDFI_FORWARD_TO_DELEGATES_3(PART1,TYPE1,PART2,TYPE2,PART3,TYPE3) \
- (void)PART1:TYPE1 arg1 PART2:TYPE2 arg2 PART3:TYPE3 arg3 \
{ \
    KSDFI_FORWARD_TO_DELEGATES_IMPL(@selector(PART1:PART2:PART3:), [delegate PART1:arg1 PART2:arg2 PART3:arg3]) \
}

#define KSDFI_FORWARD_TO_DELEGATES_4(PART1,TYPE1,PART2,TYPE2,PART3,TYPE3,PART4,TYPE4) \
- (void)PART1:TYPE1 arg1 PART2:TYPE2 arg2 PART3:TYPE3 arg3 PART4:TYPE4 arg4 \
{ \
    KSDFI_FORWARD_TO_DELEGATES_IMPL(@selector(PART1:PART2:PART3:PART4:), [delegate PART1:arg1 PART2:arg2 PART3:arg3 PART4:arg4]) \
}

#define KSDFI_FORWARD_TO_DELEGATES_5(PART1,TYPE1,PART2,TYPE2,PART3,TYPE3,PART4,TYPE4,PART5,TYPE5) \
- (void)PART1:TYPE1 arg1 PART2:TYPE2 arg2 PART3:TYPE3 arg3 PART4:TYPE4 arg4 PART5:TYPE5 arg5 \
{ \
    KSDFI_FORWARD_TO_DELEGATES_IMPL(@selector(PART1:PART2:PART3:PART4:PART5:), [delegate PART1:arg1 PART2:arg2 PART3:arg3 PART4:arg4 PART5:arg5]) \
}

#define KSDFI_FORWARD_TO_DELEGATES_6(PART1,TYPE1,PART2,TYPE2,PART3,TYPE3,PART4,TYPE4,PART5,TYPE5,PART6,TYPE6) \
- (void)PART1:TYPE1 arg1 PART2:TYPE2 arg2 PART3:TYPE3 arg3 PART4:TYPE4 arg4 PART5:TYPE5 arg5 PART6:TYPE6 arg6 \
{ \
    KSDFI_FORWARD_TO_DELEGATES_IMPL(@selector(PART1:PART2:PART3:PART4:PART5:PART6:), [delegate PART1:arg1 PART2:arg2 PART3:arg3 PART4:arg4 PART5:arg5 PART6:arg6]) \
}

#define KSDFI_GET_FORWARD_TO_DELEGATES(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, NAME, ...) NAME
