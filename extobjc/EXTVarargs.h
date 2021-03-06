//
//  EXTVarargs.h
//  extobjc
//
//  Created by Justin Spahr-Summers on 20.06.12.
//  Copyright (C) 2012 Justin Spahr-Summers.
//  Released under the MIT license.
//

#import <Foundation/Foundation.h>
#import "EXTNil.h"
#import "metamacros.h"

/**
 * Returns the given argument list, with a type encoding as a C string inserted
 * at the beginning. The resulting argument list is suitable for a function
 * using #unpack_args.
 *
 * See #unpack_args for limitations on the argument types supported.
 */
#define pack_varargs(...) \
    /* add a C string of the variadic arguments' type encodings */ \
    [[NSString stringWithFormat: \
        @ metamacro_foreach(pack_args_format_iter,, __VA_ARGS__) \
        metamacro_foreach(pack_args_type_iter,, __VA_ARGS__) \
    ] UTF8String], \
    \
    /* then the verbatim variadic arguments */ \
    __VA_ARGS__

/**
 * Usable wherever #pack_varargs is (and where one of the two is required), this
 * macro indicates that there are no variadic arguments.
 */
#define empty_varargs ""

/**
 * Used in an argument list to a C function using #unpack_args, this makes the
 * use of variadic arguments more idiomatic. CONSTANT_ARGS is the number of
 * non-variadic arguments accepted by the function.
 *
 * Typically, this would be used in a macro that hides the function, to make it
 * easier for callers to use:
 *
 * @code
 
// for the purposes of pack_args(), this function has two non-variadic
// arguments: "value" and "str" ("types" is part of the variadic argument
// machinery, and so is not counted)
void my_variadic_function (int value, NSString *str, const char *types, ...) {
    NSArray *args = unpack_args(types);

    ...
}

// now the function can be invoked like:
// my_variadic_function(5, @"foobar", 3.14, 159, "fuzzbuzz", ...)
// 
// in other words, the "types" argument does not have to be handled by the caller
#define my_variadic_function(...) \
        my_variadic_function(pack_args(2, __VA_ARGS__))
 
 * @endcode
 *
 * @note Unlike C stdargs, variadic functions defined in this way do not need
 * any constant arguments -- they may be entirely variadic. However, every
 * function must be invoked with at least one constant or variadic argument.
 *
 * See #unpack_args for limitations on the argument types supported.
 */
#define pack_args(CONSTANT_ARGS, ...) \
    metamacro_if_eq(0, CONSTANT_ARGS) \
        (/* no constant arguments */) \
        ( \
            /* otherwise, keep the constant arguments verbatim */ \
            metamacro_take(CONSTANT_ARGS, __VA_ARGS__), \
        ) \
    \
    /* check to see if any variadic arguments were given */ \
    metamacro_if_eq(1, metamacro_argcount(metamacro_drop(CONSTANT_ARGS, __VA_ARGS__, 1))) \
        (empty_varargs) \
        (pack_varargs(metamacro_drop(CONSTANT_ARGS, __VA_ARGS__)))

/**
 * In the implementation of a variadic function, this macro will return an \c
 * NSArray containing all of the arguments passed in, automatically boxed as
 * necessary. OBJC_TYPES should be the name of the last parameter, which should
 * be a "const char *", corresponding to the type encodings generated by
 * #pack_varargs, #empty_varargs, and #pack_args.
 *
 * @code

- (NSUInteger)count_args:(const char *)types, ... {
    NSArray *args = unpack_args(types);
    return args.count;
}

- (void)doCounting {
    // "count" will be 4
    NSUInteger count = [self count_args:pack_varargs("foo", @"bar", 3.14f, 'b')];
}

 * @endcode
 *
 * See #pack_args for an example with a C function.
 *
 * @warning C arrays, structures, and unions are not supported. Unrecognized
 * pointer types will be encoded into an \c NSValue instance. \c NULL and \c nil
 * values will be represented in the array with #EXTNil.
 */
#define unpack_args(OBJC_TYPES) \
    ({  \
        va_list argList; \
        va_start(argList, OBJC_TYPES); \
        \
        const char *typeString = (OBJC_TYPES); \
        NSMutableArray *boxedArgs = [NSMutableArray array]; \
        \
        while (*typeString) { \
            while (isdigit(*typeString)) \
                ++typeString; \
            \
            if (*typeString == '\0') \
                break; \
            \
            const char *next = NSGetSizeAndAlignment(typeString, NULL, NULL); \
            size_t typeLen = next - typeString; \
            if (typeLen == 0) \
                break; \
            \
            char singleType[typeLen + 1]; \
            strncpy(singleType, typeString, typeLen); \
            singleType[typeLen] = '\0'; \
            \
            id arg = nil; \
            ext_boxVarargOfType(argList, singleType); \
            \
            if (!arg) \
                arg = [EXTNil null]; \
            \
            [boxedArgs addObject:arg]; \
            typeString = next; \
        } \
        \
        va_end(argList); \
        boxedArgs; \
    })

#define ext_boxVarargOfType(VA_LIST, OBJC_TYPE) \
    do { \
        const char *type = (OBJC_TYPE); \
        switch (*type) { \
            case 'c': { \
                char c = (char)va_arg(VA_LIST, int); \
                \
                /* special case for BOOL, which is technically a signed char */ \
                if (c == YES || c == NO) \
                    arg = [NSNumber numberWithBool:(BOOL)c]; \
                else \
                    arg = [NSString stringWithFormat:@"%c", c]; \
                \
                break; \
            } \
            \
            case 's': \
            case 'i': \
                arg = @(va_arg(VA_LIST, int)); \
                break; \
            \
            case 'C': \
            case 'I': \
            case 'S': \
                arg = @(va_arg(VA_LIST, unsigned int)); \
                break; \
            \
            case 'l': \
                arg = @(va_arg(VA_LIST, long)); \
                break; \
            \
            case 'L': \
                arg = @(va_arg(VA_LIST, unsigned long)); \
                break; \
            \
            case 'q': \
                arg = @(va_arg(VA_LIST, long long)); \
                break; \
            \
            case 'Q': \
                arg = @(va_arg(VA_LIST, unsigned long long)); \
                break; \
            \
            case 'f': \
            case 'd': \
                arg = @(va_arg(VA_LIST, double)); \
                break; \
            \
            case 'B': \
                arg = [NSNumber numberWithBool:(BOOL)va_arg(VA_LIST, int)]; \
                break; \
            \
            case '*': \
                arg = @(va_arg(VA_LIST, char *)); \
                break; \
            \
            case '@': \
            case '#': \
                arg = va_arg(VA_LIST, id); \
                break; \
            \
            case '?': \
            case '^': { \
                void *ptr = va_arg(VA_LIST, void *); \
                if (ptr) \
                    arg = [NSValue valueWithBytes:&ptr objCType:type]; \
                \
                break; \
            } \
            \
            case '[': { \
                char *next = NULL; \
                strtoul(type + 1, &next, 10); \
                \
                if (*next == 'c') { \
                    arg = @(va_arg(VA_LIST, char *)); \
                    break; \
                } \
                \
                /* else fall through */ \
            } \
            \
            case 'v': \
            default: \
                NSCAssert(NO, @"Unsupported variadic argument type \"%s\"", type); \
                arg = nil; \
        } \
    } while (0)

/*** implementation details follow ***/
#define pack_args_format_iter(INDEX, ARG) \
    "%s"

#define pack_args_type_iter(INDEX, ARG) \
    , @encode(__typeof__(ARG))
