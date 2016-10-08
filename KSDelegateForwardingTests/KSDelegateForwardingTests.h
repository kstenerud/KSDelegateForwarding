//
//  KSDelegateForwardingTests.h
//  KSDelegateForwarding
//
//  Created by Karl on 2016-10-07.
//  Copyright © 2016 Karl Stenerud. All rights reserved.
//

#import "KSDelegateForwarding.h"

@protocol SomeServiceDelegate;



/* Example service with a delegate */
@interface SomeService : NSObject

@property(nonatomic,weak,nullable) id<SomeServiceDelegate> delegate;

+ (nonnull instancetype) defaultService;

@end



/* Example delegate protocol */
@protocol SomeServiceDelegate <NSObject>

@optional

- (void) callback0Param;
- (void) callbackA:(nullable NSString*)a;
- (void) callbackA:(nullable NSString*)a b:(int)b;
- (void) callbackA:(nullable NSString*)a b:(int)b c:(double)c;
- (void) callbackA:(nullable NSString*)a b:(int)b c:(double)c d:(nullable id)d;
- (void) callbackA:(nullable NSString*)a b:(int)b c:(double)c d:(nullable id)d e:(nullable id)e;
- (void) callbackA:(nullable NSString*)a b:(int)b c:(double)c d:(nullable id)d e:(nullable id)e f:(nullable id)f;

@end



/* Forwarding delegate class declaration (we're creating a singleton variant to match the original service singleton) */
KSDF_DECLARE_DELEGATE_FORWARDING_CLASS_SINGLETON(ForwardingSomeService, SomeServiceDelegate, defaultService)
