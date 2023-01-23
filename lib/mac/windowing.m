#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import "../../include/main.h"

CVReturn myDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*, CVOptionFlags, CVOptionFlags*, void*);

@interface appDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

int main(int argc, char* argv[]) {
    // Code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSApp = [NSApplication sharedApplication];
	[NSApp setDelegate : [[appDelegate alloc] init]];
	[NSApp run]; //Message Loop
	[pool release];
	return 0;
}

@interface myOpenGLView : NSOpenGLView
@end

@implementation appDelegate {
	@private
	NSWindow *window;
	myOpenGLView *view;
}

-(void) applicationDidFinishLaunching : (NSNotification*)aNotification {
	//Code
	NSRect winrect = NSMakeRect(100, 100, 800, 600);
	window = [[NSWindow alloc] initWithContentRect : winrect styleMask : NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing : NSBackingStoreBuffered defer : NO];
	[window toggleFullScreen : self];
	[window setTitle : @"MacOS OpenGL Window"];
	[window center];

	view = [[myOpenGLView alloc] initWithFrame : winrect];
	[window setContentView : view];
	[window setDelegate : self];
	[window makeKeyAndOrderFront : self];
}

-(void) applicationWillTerminate : (NSNotification*)aNotification {
}

-(void) windowWillClose : (NSNotification*)aNotification {
	//Code
	[NSApp terminate : self];
}

-(void) dealloc {
	//Code
	[view release];
	[window release];
	[super dealloc];
}
@end

@implementation myOpenGLView {
	@private
	CVDisplayLinkRef displayLink;
}

-(id) initWithFrame : (NSRect)frame {
	self = [super initWithFrame : frame];
	if(self) {
		NSOpenGLPixelFormatAttribute attribs[] = {
			NSOpenGLPFAOpenGLProfile,     NSOpenGLProfileVersion4_1Core,
			NSOpenGLPFAScreenMask,          CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
			NSOpenGLPFANoRecovery,
			NSOpenGLPFAAccelerated,
			NSOpenGLPFAColorSize,               24,
			NSOpenGLPFADepthSize,              24,
			NSOpenGLPFAAlphaSize,              8,
			NSOpenGLPFADoubleBuffer,
			0
		};
		NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes : attribs] autorelease];
		NSOpenGLContext *glContext = [[[NSOpenGLContext alloc] initWithFormat : pixelFormat shareContext : nil] autorelease];
		[self setPixelFormat : pixelFormat];
		[self setOpenGLContext : glContext];
	}
	return self;
}

-(CVReturn) getFrameForTime : (const CVTimeStamp*) opTime {
	//Code
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[self drawView];
	[pool release];
	return kCVReturnSuccess;
} 

-(void) prepareOpenGL {
	//Code
	[super prepareOpenGL];
	[[self openGLContext] makeCurrentContext];

	GLint swapInterval = 1;
	[[self openGLContext] setValues : &swapInterval forParameter : NSOpenGLCPSwapInterval];
	
	setupProgram();
	init();

	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetOutputCallback(displayLink, myDisplayLinkCallback, self);
	CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
	CVDisplayLinkStart(displayLink);
}

-(void) reshape {
	//Code
	[super reshape];
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	NSRect rect = [self bounds];
	
	winSize.w = rect.size.width;
	winSize.h = rect.size.height;
		
	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(void) drawRect : (NSRect)dirtyRect {
	[self drawView];
}

-(void) drawView {
	//Code
	[[self openGLContext] makeCurrentContext];
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	
	render();

	CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(BOOL) acceptsFirstResponder {
	//Code
	[[self window] makeFirstResponder : nil];
	return YES;
}

-(void) keyDown : (NSEvent*)theEvent {
	int key = [[theEvent characters] characterAtIndex : 0];
	switch(key) {
	case 27:
		[self release];
		[NSApp terminate : [self window]];
		break;
	case 'f': case 'F':
		[[self window] toggleFullScreen : self];
		break;
	}
}

-(void) dealloc {
	[super dealloc];
}
@end

CVReturn myDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* opTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext) {
	//Code
	return [(myOpenGLView*)displayLinkContext getFrameForTime : opTime];
}
