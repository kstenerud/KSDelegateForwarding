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

/*
 These macros allow you to wrap a class that implements the delegate pattern, such that it may accept multiple delegates.
 The resulting code-generated class will wrap an instance of the original class, forwarding method calls to that object,
 as well as forwarding all delegate method calls to its own list of delegates.
 
 The new class will provide the following additional API for delegate subscriptions:
 
 - (void) subscribeDelegate:(id<DELEGATE_PROTOCOL>) delegate
 - (void) unsubscribeDelegate:(id<DELEGATE_PROTOCOL>) delegate

 Modifying the delegate property of the original object will be captured via KVO. The code-generated class will maintain
 consistent state between itself and the original object.
 */

#pragma mark - Class Declarations -

/** Declare a delegate forwarding class.
 *
 *  @param CLASS_NAME What to name the class.
 *  @param DELEGATE_PROTOCOL The delegate protocol to expect.
 */
#define KSDF_DECLARE_DELEGATE_FORWARDING_CLASS(CLASS_NAME, DELEGATE_PROTOCOL) \
          KSDFI_BEGIN_DELEGATE_FORWARDING_CLASS(CLASS_NAME, DELEGATE_PROTOCOL) \
@end

/** Declare a delegate forwarding class with a singleton access method.
 *
 *  @param CLASS_NAME What to name the class.
 *  @param DELEGATE_PROTOCOL The delegate protocol to expect.
 *  @param SINGLETON_METHOD_NAME The name of the singleton access method.
 */
#define KSDF_DECLARE_DELEGATE_FORWARDING_CLASS_SINGLETON(CLASS_NAME, DELEGATE_PROTOCOL, SINGLETON_METHOD_NAME) \
          KSDFI_BEGIN_DELEGATE_FORWARDING_CLASS(CLASS_NAME, DELEGATE_PROTOCOL) \
          + (nonnull instancetype) SINGLETON_METHOD_NAME; \
@end


#pragma mark - Internal Macros -

#define KSDFI_BEGIN_DELEGATE_FORWARDING_CLASS(CLASS_NAME, DELEGATE_PROTOCOL) \
    @interface CLASS_NAME : NSObject <DELEGATE_PROTOCOL> \
\
    - (void) subscribeDelegate:(nonnull id<DELEGATE_PROTOCOL>) delegate; \
    - (void) unsubscribeDelegate:(nonnull id<DELEGATE_PROTOCOL>) delegate;
