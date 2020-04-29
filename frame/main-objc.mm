#include "mainwindow.h"

#import <AppKit/AppKit.h>

@interface ApplicationDelegate: NSResponder<NSApplicationDelegate>
@property (strong) NSObject *qtDelegate;
@end

@implementation  ApplicationDelegate
- (void)installAsDelegateForApplication:(NSApplication *)app
{
    _qtDelegate = app.delegate; // Save current delegate for forwarding
    app.delegate = self;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender hasVisibleWindows:(BOOL) flag {
    if (MainWindow::openWindows.count() == 0) {
        //Open a new window
        MainWindow* w = new MainWindow();
        w->show();
    }
    return YES;
}
@end

void setupMacObjC() {
    NSApplication *sharedApp = [NSApplication sharedApplication];
    sharedApp.automaticCustomizeTouchBarMenuItemEnabled = YES;

    ApplicationDelegate *appDelegate = [[ApplicationDelegate alloc] init];
    [appDelegate installAsDelegateForApplication:sharedApp];
}
