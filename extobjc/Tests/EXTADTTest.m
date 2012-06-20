//
//  EXTADTTest.m
//  extobjc
//
//  Created by Justin Spahr-Summers on 19.06.12.
//  Released into the public domain.
//

#import "EXTADTTest.h"

ADT(Color,
    constructor(Red),
    constructor(Green),
    constructor(Blue),
    constructor(Gray, double alpha),
    constructor(Other, double r, double g, double b)
);

ADT(Multicolor,
    constructor(OneColor, ColorT c),
    constructor(TwoColor, ColorT first, ColorT second)
);

@implementation EXTADTTest

- (void)testRed {
    ColorT c = Color.Red();
    STAssertTrue(c.tag == Red, @"");
    STAssertEqualObjects(NSStringFromColorT(c), @"Red", @"");
}

- (void)testGray {
    ColorT c = Color.Gray(0.75);
    STAssertTrue(c.tag == Gray, @"");
    STAssertEqualsWithAccuracy(c.alpha, 0.75, 0.0001, @"");
    STAssertEqualObjects(NSStringFromColorT(c), @"Gray { alpha = 0.75 }", @"");
}

- (void)testOther {
    ColorT c = Color.Other(1.0, 0.5, 0.25);
    STAssertTrue(c.tag == Other, @"");
    STAssertEqualsWithAccuracy(c.r, 1.0, 0.0001, @"");
    STAssertEqualsWithAccuracy(c.g, 0.5, 0.0001, @"");
    STAssertEqualsWithAccuracy(c.b, 0.25, 0.0001, @"");
    STAssertEqualObjects(NSStringFromColorT(c), @"Other { r = 1, g = 0.5, b = 0.25 }", @"");
}

- (void)testMulticolor {
    MulticolorT c = Multicolor.TwoColor(Color.Gray(0.5), Color.Other(0.25, 0.5, 0.75));
    STAssertTrue(c.tag == TwoColor, @"");

    STAssertTrue(c.first.tag == Gray, @"");
    STAssertEqualsWithAccuracy(c.first.alpha, 0.5, 0.0001, @"");

    STAssertTrue(c.second.tag == Other, @"");
    STAssertEqualsWithAccuracy(c.second.r, 0.25, 0.0001, @"");
    STAssertEqualsWithAccuracy(c.second.g, 0.5, 0.0001, @"");
    STAssertEqualsWithAccuracy(c.second.b, 0.75, 0.0001, @"");
}

@end