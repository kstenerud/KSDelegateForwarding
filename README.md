Delegate Class Forwarding
-------------------------

These macros allow you to wrap a class that implements the delegate pattern, such that it may accept multiple delegates. The resulting code-generated class will wrap an instance of the original class, forwarding method calls to that object, as well as forwarding all delegate method calls to its own list of delegates.

The new class will provide the following additional API for delegate subscriptions:

```objc
   - (void) subscribeDelegate:(id<DELEGATE_PROTOCOL>) delegate
   - (void) unsubscribeDelegate:(id<DELEGATE_PROTOCOL>) delegate
```

Modifying the delegate property of the original object will be captured via KVO. The code-generated class will maintain consistent state between itself and the original object.


Example
--------

Given the following service object and delegate protocol:

```objc
    @protocol SomeServiceDelegate;

    @interface SomeService : NSObject
    @property(nonatomic,weak,nullable) id<SomeServiceDelegate> delegate;
    + (instancetype) defaultService;
    - (void) doSomethingCool;
    @end

    @protocol SomeServiceDelegate <NSObject>
    @optional
    - (void) serviceDidSomething;
    - (void) service:(SomeService*) service didGenerateEventWithName:(NSString*) name details:(NSDictionary*) details count:(NSUInteger) count;
    @end
```

You would implement a forwarding proxy like this:

In the header:
```objc
    #import "KSDelegateForwarding.h"

    // If you want to create a singleton
    KSDF_DECLARE_DELEGATE_FORWARDING_CLASS_SINGLETON(ForwardingSomeService, SomeServiceDelegate, defaultService)
    // Otherwise
    KSDF_DECLARE_DELEGATE_FORWARDING_CLASS(ForwardingSomeService, SomeServiceDelegate)
```
In the implementation:
```objc
    #import "KSDelegateForwardingImpl.h"

    // In the implementation.
    KSDF_BEGIN_DELEGATE_FORWARDING_IMPLEMENTATION(ForwardingSomeService, SomeServiceDelegate, SomeService*, delegate)

    KSDF_FORWARD_TO_DELEGATES(serviceDidSomething)
    KSDF_FORWARD_TO_DELEGATES(service,(SomeService*), didGenerateEventWithName,(NSString*), details,(NSDictionary*), count,(NSUInteger))
    // Only if you used the singleton variant in the header
    KSDF_ADD_FORWARDING_SINGLETON_METHOD(ForwardingSomeService, defaultService, [SomeService defaultService])

    KSDF_END_DELEGATE_FORWARDING_IMPLEMENTATION()
```

You would then use it like this:

```objc
    // Access the service.
    ForwardingSomeService* service = [ForwardingSomeService defaultService];
    // OR
    ForwardingSomeService* service = [[ForwardingSomeService alloc] initWithForwardingObject:[SomeService defaultService]];

    // Add a delegate to the service.
    [service subscribeDelegate:someDelegate];

    // Remove a delegate from the service.
    [service unsubscribeDelegate:someDelegate];

    // You can still set/remove the original service delegate. Note that ForwardingSomeService will intercept via KVO,
    // add/remove the delegate in its internal list, and reset [SomeService defaultService].delegate to itself.
    [SomeService defaultService].delegate = someOtherDelegate;
    [SomeService defaultService].delegate = nil;
```

Notes
------

- All delegates are weak referenced. You can deallocate them without fear.
- You can have as many forwarding proxies as you like, even different implementations, pointing to the same service. They'll all play nice together. This means that if someone else is also using this header for their own forwarding proxy, it'll just work.
- You can implement as a singleton or not. Your choice. There are macros for either approach.

License
-------

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall remain in place
in this source code.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
