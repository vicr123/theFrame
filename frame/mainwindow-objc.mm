#include "mainwindow.h"
#include "ui_mainwindow.h"

#import <AppKit/AppKit.h>

@class MainWindowTouchBarProvider;

struct MainWindowTouchBarItem {
    MainWindowTouchBarItem() {};
    MainWindowTouchBarItem(QString identifier);
    MainWindowTouchBarItem(QString identifier, QAction* action);
    MainWindowTouchBarItem(QString identifier, QAction* action, NSImageName image);
    ~MainWindowTouchBarItem();

    void prepareTouchBarItem(MainWindowTouchBarProvider* provider);

    QString identifier;
    QAction* action;

    bool haveImage = false;;
    NSImageName image;

    NSTouchBarItem* touchBarItem;
};
typedef QSharedPointer<MainWindowTouchBarItem> TouchBarItemPtr;

@interface MainWindowTouchBarProvider: NSResponder <NSTouchBarDelegate, NSApplicationDelegate, NSWindowDelegate>

@property (strong) NSCustomTouchBarItem *firstFrameItem;
@property (strong) NSCustomTouchBarItem *lastFrameItem;
@property (strong) NSButton *firstFrameButton;
@property (strong) NSButton *lastFrameButton;

@property (strong) NSObject *qtDelegate;
@property MainWindow *mainWindow;
@property Ui::MainWindow *mainWindowUi;
@property QList<TouchBarItemPtr> touchBarActionMapping;

@end

// Create identifiers for button items.
static NSTouchBarItemIdentifier firstFrameIdentifier = @"com.vicr123.theframe.firstFrame";
static NSTouchBarItemIdentifier playIdentifier = @"com.vicr123.theframe.play";
static NSTouchBarItemIdentifier lastFrameIdentifier = @"com.vicr123.theframe.lastFrame";
static NSTouchBarItemIdentifier inPointIdentifier = @"com.vicr123.theframe.inPoint";
static NSTouchBarItemIdentifier outpointIdentifier = @"com.vicr123.theframe.outPoint";
static NSTouchBarItemIdentifier renderIdentifier = @"com.vicr123.theframe.render";
static NSTouchBarItemIdentifier timelineIdentifier = @"com.vicr123.theframe.timeline";
static NSTouchBarItemIdentifier timelineBarIdentifier = @"com.vicr123.theframe.timelineBar";

@implementation MainWindowTouchBarProvider

- (id)init: (MainWindow*)mainWin withMainWindowUi:(Ui::MainWindow*)ui {
    if (self = [super init]) {
        //Set main window UI to call touch bar handlers
        self.mainWindow = mainWin;
        self.mainWindowUi = ui;

        //Initialise the action mapping
        self.touchBarActionMapping = {
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(firstFrameIdentifier), ui->actionFirstFrame, NSImageNameTouchBarSkipToStartTemplate)),
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(playIdentifier), ui->actionPlay, NSImageNameTouchBarPlayTemplate)),
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(lastFrameIdentifier), ui->actionLastFrame, NSImageNameTouchBarSkipToEndTemplate)),
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(renderIdentifier), ui->actionRender, NSImageNameTouchBarRecordStartTemplate)),
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(inPointIdentifier), ui->actionSet_In_Point)),
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(outpointIdentifier), ui->actionSet_Out_Point)),
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(timelineIdentifier))),
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(timelineBarIdentifier)))
        };
        
        for (TouchBarItemPtr item : self.touchBarActionMapping) {
            item->prepareTouchBarItem(self);
        }
    }

    return self;
}

- (NSTouchBar *)makeTouchBar {
    // Create the touch bar with this instance as its delegate
    NSTouchBar *bar = [[NSTouchBar alloc] init];
    bar.delegate = self;

    bar.defaultItemIdentifiers = @[timelineIdentifier, firstFrameIdentifier, playIdentifier, lastFrameIdentifier];
    bar.customizationRequiredItemIdentifiers = @[firstFrameIdentifier, lastFrameIdentifier];
    bar.customizationAllowedItemIdentifiers = @[firstFrameIdentifier, lastFrameIdentifier, playIdentifier, inPointIdentifier, outpointIdentifier, renderIdentifier, timelineIdentifier, NSTouchBarItemIdentifierFlexibleSpace];
    [bar setCustomizationIdentifier:@"com.vicr123.theFrame.touchbar"];

    return bar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier {
    Q_UNUSED(touchBar);

    // Create touch bar items as NSCustomTouchBarItems which can contain any NSView.
    auto result = std::find_if(self.touchBarActionMapping.constBegin(), self.touchBarActionMapping.constEnd(), [identifier](const TouchBarItemPtr& item) {
        return [identifier isEqualToString:item->identifier.toNSString()];
    });

    if (result != self.touchBarActionMapping.constEnd()) {
        return result->data()->touchBarItem;
    }
    return nil;
}

- (void)touchBarActionClicked: (NSButton*) button {
    QString identifier = QString::fromNSString([button identifier]);
    auto result = std::find_if(self.touchBarActionMapping.constBegin(), self.touchBarActionMapping.constEnd(), [identifier](const TouchBarItemPtr& item) {
        return identifier == item->identifier;
    });

    if (result != self.touchBarActionMapping.constEnd()) {
        QAction* action = result->data()->action;
        if (action->isCheckable()) {
            action->setChecked(!action->isChecked());
        } else {
            action->trigger();
        }
    }
}

- (void)touchBarTimelineSliderChanged: (NSSliderTouchBarItem*) slider {
    self.mainWindowUi->timeline->setCurrentFrame([slider.slider doubleValue]);
    self.mainWindowUi->timeline->ensurePlayheadVisible();
}

- (void)installAsDelegateForWindow: (NSWindow *) window {
    _qtDelegate = window.delegate; // Save current delegate for forwarding
    window.delegate = self;
}

- (BOOL)respondsToSelector: (SEL) aSelector {
    // We want to forward to the qt delegate. Respond to selectors it
    // responds to in addition to selectors this instance resonds to.
    return [_qtDelegate respondsToSelector:aSelector] || [super respondsToSelector:aSelector];
}

- (void)forwardInvocation: (NSInvocation *) anInvocation {
    // Forward to the existing delegate. This function is only called for selectors
    // this instance does not responds to, which means that the Qt delegate
    // must respond to it (due to the respondsToSelector implementation above).
    [anInvocation invokeWithTarget:_qtDelegate];
}

- (NSApplicationPresentationOptions)window:(NSWindow *)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions {
    //On an unrelated note, set full screen window properties
    return (NSApplicationPresentationFullScreen | NSApplicationPresentationHideDock | NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideToolbar);
}

- (MainWindow*)getMainWindow {
    return self.mainWindow;
}

- (Ui::MainWindow*)getMainWindowUi {
    return self.mainWindowUi;
}

@end

void MainWindow::setupMacOS() {
    //Disable automatic window tabbing
    [NSWindow setAllowsAutomaticWindowTabbing:NO];

    //Install TouchBarProvider as window delegate
    NSView *view = reinterpret_cast<NSView *>(this->winId());
    MainWindowTouchBarProvider *touchBarProvider = [[MainWindowTouchBarProvider alloc] init:this withMainWindowUi:ui];
    [touchBarProvider installAsDelegateForWindow:view.window];
}

void MainWindow::updateTouchBar() {
    //Invalidate Touch Bar
    NSView *view = reinterpret_cast<NSView *>(this->winId());
    view.window.touchBar = nil;
}

MainWindowTouchBarItem::MainWindowTouchBarItem(QString identifier) {
    this->identifier = identifier;
}

MainWindowTouchBarItem::MainWindowTouchBarItem(QString identifier, QAction* action) {
    this->action = action;
    this->identifier = identifier;
}

MainWindowTouchBarItem::MainWindowTouchBarItem(QString identifier, QAction* action, NSImageName image) {
    this->action = action;
    this->identifier = identifier;
    this->image = image;
    this->haveImage = true;
}

MainWindowTouchBarItem::~MainWindowTouchBarItem()
{
    [this->touchBarItem release];
}

void MainWindowTouchBarItem::prepareTouchBarItem(MainWindowTouchBarProvider* provider) {
    if (identifier == QString::fromNSString(timelineIdentifier)) {
        NSPopoverTouchBarItem* item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];
        [item setCustomizationLabel:QApplication::translate("MainWindow", "Timeline").toNSString()];
        [item setCollapsedRepresentationLabel:QApplication::translate("MainWindow", "Timeline").toNSString()];
        [item setShowsCloseButton:YES];

        NSTouchBar* secondaryTouchBar = [[NSTouchBar alloc] init];
        secondaryTouchBar.delegate = provider;
        secondaryTouchBar.defaultItemIdentifiers = @[timelineBarIdentifier];
        [item setPressAndHoldTouchBar:secondaryTouchBar];
        [item setPopoverTouchBar:secondaryTouchBar];
        this->touchBarItem = item;
    } else if (identifier == QString::fromNSString(timelineBarIdentifier)) {
        NSSliderTouchBarItem* item = [[NSSliderTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];
        [item setCustomizationLabel:QApplication::translate("MainWindow", "Timeline").toNSString()];

        Timeline* timeline = [provider getMainWindowUi]->timeline;
        QObject::connect(timeline, &Timeline::frameCountChanged, [=](quint64 frameCount) {
            [item.slider setMaxValue:frameCount];
        });
        QObject::connect(timeline, &Timeline::currentFrameChanged, [=](quint64 currentFrame) {
            [item.slider setDoubleValue:currentFrame];
        });

        [item.slider setMinValue:0];
        [item.slider setMaxValue:timeline->frameCount()];
        [item.slider setDoubleValue:timeline->currentFrame()];
        [item setTarget:provider];
        [item setAction:@selector(touchBarTimelineSliderChanged:)];

        auto setEnabled = [=] {
            bool enabled = true;
            if ([provider getMainWindowUi]->stackedWidget->widget([provider getMainWindowUi]->stackedWidget->currentIndex()) != [provider getMainWindowUi]->mainPage) enabled = false;
            [item.slider setEnabled:enabled];
        };
        setEnabled();
        QObject::connect([provider getMainWindowUi]->stackedWidget, &tStackedWidget::currentChanged, setEnabled);

        this->touchBarItem = item;
    } else {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];
        [item setCustomizationLabel:action->text().toNSString()];

        NSButton* button = [NSButton buttonWithTitle:action->text().toNSString() target:provider action:@selector(touchBarActionClicked:)];
        [button setIdentifier:identifier.toNSString()];

        auto setState = [=] {
            bool enabled = true;
            if (!action->isEnabled()) enabled = false;
            if (action->menu() && !action->menu()->isEnabled()) enabled = false;
            if ([provider getMainWindowUi]->stackedWidget->widget([provider getMainWindowUi]->stackedWidget->currentIndex()) != [provider getMainWindowUi]->mainPage) enabled = false;
            [button setEnabled:enabled];

            [button setButtonType:action->isCheckable() ? NSPushOnPushOffButton : NSMomentaryPushInButton];
            [button setState:action->isChecked() ? NSOnState : NSOffState];
        };
        setState();

        if (haveImage) {
            [button setImage:[NSImage imageNamed:image]];
        }

        item.view = button;

        QObject::connect(action, &QAction::changed, [provider getMainWindow], setState);
        QObject::connect([provider getMainWindowUi]->stackedWidget, &tStackedWidget::currentChanged, setState);

        this->touchBarItem = item;
    }
}
