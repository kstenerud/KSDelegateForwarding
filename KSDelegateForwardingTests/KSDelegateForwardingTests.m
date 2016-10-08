//
//  KSMultiplexedWCSessionTests.m
//  KSMultiplexedWCSessionTests
//
//  Created by Karl on 2016-10-03.
//  Copyright © 2016 Karl Stenerud. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "KSDelegateForwardingTests.h"
#import "KSDelegateForwardingImpl.h"




@implementation SomeService

- (void) callCallback0Param
{
    [self.delegate callback0Param];
}

- (void) callCallbackA:(NSString*)a
{
    [self.delegate callbackA:a];
}

- (void) callCallbackA:(NSString*)a b:(int)b
{
    [self.delegate callbackA:a b:b];
}

- (void) callCallbackA:(NSString*)a b:(int)b c:(double)c
{
    [self.delegate callbackA:a b:b c:c];
}

- (void) callCallbackA:(NSString*)a b:(int)b c:(double)c d:(id)d
{
    [self.delegate callbackA:a b:b c:c d:d];
}

- (void) callCallbackA:(NSString*)a b:(int)b c:(double)c d:(id)d e:(id)e
{
    [self.delegate callbackA:a b:b c:c d:d e:e];
}

- (void) callCallbackA:(NSString*)a b:(int)b c:(double)c d:(id)d e:(id)e f:(id)f
{
    [self.delegate callbackA:a b:b c:c d:d e:e f:f];
}

+ (instancetype) defaultService
{
    static id singletonInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        singletonInstance = [SomeService new];
    });
    return singletonInstance;
}

@end


/* Example delegate implementing all methods */
@interface SomeDelegate: NSObject <SomeServiceDelegate>

@property(nonatomic,readwrite,assign) int callback0ParamCallCount;
@property(nonatomic,readwrite,assign) int callback1ParamCallCount;
@property(nonatomic,readwrite,assign) int callback2ParamCallCount;
@property(nonatomic,readwrite,assign) int callback3ParamCallCount;
@property(nonatomic,readwrite,assign) int callback4ParamCallCount;
@property(nonatomic,readwrite,assign) int callback5ParamCallCount;
@property(nonatomic,readwrite,assign) int callback6ParamCallCount;

@end

@implementation SomeDelegate

- (void) callback0Param
{
    self.callback0ParamCallCount++;
}

- (void) callbackA:(NSString *)a
{
    self.callback1ParamCallCount++;
}

- (void) callbackA:(NSString *)a b:(int)b
{
    self.callback2ParamCallCount++;
}

- (void) callbackA:(NSString *)a b:(int)b c:(double)c
{
    self.callback3ParamCallCount++;
}

- (void) callbackA:(NSString *)a b:(int)b c:(double)c d:(id)d
{
    self.callback4ParamCallCount++;
}

- (void) callbackA:(NSString *)a b:(int)b c:(double)c d:(id)d e:(id)e
{
    self.callback5ParamCallCount++;
}

- (void) callbackA:(NSString *)a b:(int)b c:(double)c d:(id)d e:(id)e f:(id)f
{
    self.callback6ParamCallCount++;
}

@end



/* Example delegate implementing no methods */
@interface NoMethodDelegate: NSObject <SomeServiceDelegate>
@end
@implementation NoMethodDelegate
@end



/* Forwarding delegate class implementation */
KSDF_BEGIN_DELEGATE_FORWARDING_IMPLEMENTATION(ForwardingSomeService, SomeServiceDelegate, SomeService*, delegate)

/* We're creating a singleton variant to match the original service singleton */
KSDF_ADD_FORWARDING_SINGLETON_METHOD(ForwardingSomeService, defaultService, [SomeService defaultService])

KSDF_FORWARD_TO_DELEGATES(callback0Param)
KSDF_FORWARD_TO_DELEGATES(callbackA,(NSString *))
KSDF_FORWARD_TO_DELEGATES(callbackA,(NSString *), b,(int))
KSDF_FORWARD_TO_DELEGATES(callbackA,(NSString *), b,(int), c,(double))
KSDF_FORWARD_TO_DELEGATES(callbackA,(NSString *), b,(int), c,(double), d,(id))
KSDF_FORWARD_TO_DELEGATES(callbackA,(NSString *), b,(int), c,(double), d,(id), e,(id))
KSDF_FORWARD_TO_DELEGATES(callbackA,(NSString *), b,(int), c,(double), d,(id), e,(id), f,(id))

KSDF_END_DELEGATE_FORWARDING_IMPLEMENTATION()



/* Tests */
@interface KSMultiplexedWCSessionTests : XCTestCase
@end

@implementation KSMultiplexedWCSessionTests

- (void)testOriginalDelegateCreateForwardBefore {
    SomeService* service = [SomeService defaultService];
    __unused ForwardingSomeService* forwardingService = [ForwardingSomeService defaultService];
    SomeDelegate* delegate = [SomeDelegate new];
    service.delegate = delegate;
    [service callCallback0Param];
    
    XCTAssertEqual(delegate.callback0ParamCallCount, 1);
}

- (void)testOriginalDelegateCreateForwardAfter {
    SomeService* service = [SomeService new];
    SomeDelegate* delegate = [SomeDelegate new];
    service.delegate = delegate;
    __unused ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    [service callCallback0Param];
    
    XCTAssertEqual(delegate.callback0ParamCallCount, 1);
}

- (void)testOriginalDelegateAddRemove {
    SomeService* service = [SomeService new];
    __unused ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    service.delegate = delegate;
    service.delegate = nil;
    [service callCallback0Param];
    
    XCTAssertEqual(delegate.callback0ParamCallCount, 0);
}

- (void)testOriginalDelegateAddReplace {
    SomeService* service = [SomeService new];
    __unused ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate1 = [SomeDelegate new];
    SomeDelegate* delegate2 = [SomeDelegate new];
    service.delegate = delegate1;
    service.delegate = delegate2;
    [service callCallback0Param];
    
    XCTAssertEqual(delegate1.callback0ParamCallCount, 0);
    XCTAssertEqual(delegate2.callback0ParamCallCount, 1);
}

- (void)testMultipleDelegates {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate1 = [SomeDelegate new];
    SomeDelegate* delegate2 = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate1];
    [forwardingService subscribeDelegate:delegate2];
    [service callCallback0Param];
    
    XCTAssertEqual(delegate1.callback0ParamCallCount, 1);
    XCTAssertEqual(delegate2.callback0ParamCallCount, 1);
}

- (void)testMultipleCalls {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallback0Param];
    [service callCallback0Param];
    
    XCTAssertEqual(delegate.callback0ParamCallCount, 2);
}

- (void)testAddRemove {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [forwardingService unsubscribeDelegate:delegate];
    [service callCallback0Param];
    
    XCTAssertEqual(delegate.callback0ParamCallCount, 0);
}

- (void)test2ForwardInstances {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService1 = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    ForwardingSomeService* forwardingService2 = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate1 = [SomeDelegate new];
    SomeDelegate* delegate2 = [SomeDelegate new];
    SomeDelegate* delegate3 = [SomeDelegate new];
    SomeDelegate* delegate4 = [SomeDelegate new];
    [forwardingService1 subscribeDelegate:delegate1];
    [forwardingService2 subscribeDelegate:delegate2];
    [forwardingService1 subscribeDelegate:delegate3];
    [forwardingService2 subscribeDelegate:delegate4];
    [service callCallback0Param];
    
    XCTAssertEqual(delegate1.callback0ParamCallCount, 1);
    XCTAssertEqual(delegate2.callback0ParamCallCount, 1);
    XCTAssertEqual(delegate3.callback0ParamCallCount, 1);
    XCTAssertEqual(delegate4.callback0ParamCallCount, 1);
}

- (void)testOneForwarderDeallocated {
    SomeService* service = [SomeService new];
    SomeDelegate* delegate1 = [SomeDelegate new];
    SomeDelegate* delegate2 = [SomeDelegate new];
    SomeDelegate* delegate3 = [SomeDelegate new];
    SomeDelegate* delegate4 = [SomeDelegate new];
    ForwardingSomeService* forwardingService1 = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    @autoreleasepool
    {
        ForwardingSomeService* forwardingService2 = [[ForwardingSomeService alloc] initWithForwardingObject:service];
        [forwardingService1 subscribeDelegate:delegate1];
        [forwardingService2 subscribeDelegate:delegate2];
        [forwardingService1 subscribeDelegate:delegate3];
        [forwardingService2 subscribeDelegate:delegate4];
    }
    [service callCallback0Param];
    
    XCTAssertEqual(delegate1.callback0ParamCallCount, 1);
    XCTAssertEqual(delegate2.callback0ParamCallCount, 1);
    XCTAssertEqual(delegate3.callback0ParamCallCount, 1);
    XCTAssertEqual(delegate4.callback0ParamCallCount, 1);
}

- (void)testOnlyForwarderDeallocated {
    SomeService* service = [SomeService new];
    SomeDelegate* delegate1 = [SomeDelegate new];
    SomeDelegate* delegate2 = [SomeDelegate new];
    @autoreleasepool
    {
        ForwardingSomeService* forwardingService1 = [[ForwardingSomeService alloc] initWithForwardingObject:service];
        [forwardingService1 subscribeDelegate:delegate1];
        [forwardingService1 subscribeDelegate:delegate2];
    }
    [service callCallback0Param];
    
    XCTAssertEqual(delegate1.callback0ParamCallCount, 0);
    XCTAssertEqual(delegate2.callback0ParamCallCount, 0);
}

- (void)testOnlyForwarderDeallocatedNewAllocated {
    SomeService* service = [SomeService new];
    SomeDelegate* delegate1 = [SomeDelegate new];
    SomeDelegate* delegate2 = [SomeDelegate new];
    @autoreleasepool
    {
        ForwardingSomeService* forwardingService1 = [[ForwardingSomeService alloc] initWithForwardingObject:service];
        [forwardingService1 subscribeDelegate:delegate1];
        [forwardingService1 subscribeDelegate:delegate2];
    }
    __unused ForwardingSomeService* forwardingService2 = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    [service callCallback0Param];
    
    XCTAssertEqual(delegate1.callback0ParamCallCount, 1);
    XCTAssertEqual(delegate2.callback0ParamCallCount, 1);
}

- (void)testMethodNotImplemented {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    NoMethodDelegate* delegate = [NoMethodDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallback0Param];
    [service callCallbackA:nil];
    [service callCallbackA:nil b:0];
    [service callCallbackA:nil b:0 c:0];
    [service callCallbackA:nil b:0 c:0 d:nil];
    [service callCallbackA:nil b:0 c:0 d:nil e:nil];
    [service callCallbackA:nil b:0 c:0 d:nil e:nil f:nil];
}

- (void)testCallback0Param {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallback0Param];
    
    XCTAssertEqual(delegate.callback0ParamCallCount, 1);
}

- (void)testCallback1Param {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallbackA:@"test"];
    
    XCTAssertEqual(delegate.callback1ParamCallCount, 1);
}

- (void)testCallback2Param {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallbackA:@"test" b:10];
    
    XCTAssertEqual(delegate.callback2ParamCallCount, 1);
}

- (void)testCallback3Param {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallbackA:@"test" b:10 c:103.8];
    
    XCTAssertEqual(delegate.callback3ParamCallCount, 1);
}

- (void)testCallback4Param {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallbackA:@"test" b:10 c:103.8 d:[NSArray new]];
    
    XCTAssertEqual(delegate.callback4ParamCallCount, 1);
}

- (void)testCallback5Param {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallbackA:@"test" b:10 c:103.8 d:[NSArray new] e:[NSDictionary new]];
    
    XCTAssertEqual(delegate.callback5ParamCallCount, 1);
}

- (void)testCallback6Param {
    SomeService* service = [SomeService new];
    ForwardingSomeService* forwardingService = [[ForwardingSomeService alloc] initWithForwardingObject:service];
    SomeDelegate* delegate = [SomeDelegate new];
    [forwardingService subscribeDelegate:delegate];
    [service callCallbackA:@"test" b:10 c:103.8 d:[NSArray new] e:[NSDictionary new] f:[NSObject new]];
    
    XCTAssertEqual(delegate.callback6ParamCallCount, 1);
}

@end
