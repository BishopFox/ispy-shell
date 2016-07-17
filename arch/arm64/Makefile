# ==========================================================================
# Build system
# ==========================================================================

# -mpreferred-stack-boundary=2 is essential in preventing gcc 4.2.x
# from aligning stack to 16 bytes. (Which is gcc's way of supporting SSE).
SDK=iphoneos
SDK_PATH=$(shell xcrun --sdk $(SDK) --show-sdk-path)
CFLAGS += $(call cc-option,-ObjC -arch arm64 -isysroot $(SDK_PATH) -I $(SDK_PATH)/usr/include -miphoneos-version-min=7.0 -fvisibility=hidden -framework Foundation -framework UIKit -framework Security)
