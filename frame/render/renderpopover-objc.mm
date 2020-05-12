#include "renderpopover.h"
#include "ui_renderpopover.h"

#include <QAction>
#import <Cocoa/cocoa.h>
#import <AppKit/AppKit.h>

@class RenderPopoverTouchBarProvider;

struct RenderPopoverTouchBarItem {
    RenderPopoverTouchBarItem() {};
    RenderPopoverTouchBarItem(QString identifier);
    RenderPopoverTouchBarItem(QString identifier, QAction* action);
    RenderPopoverTouchBarItem(QString identifier, QAction* action, NSImageName image);
    ~RenderPopoverTouchBarItem();

    void prepareTouchBarItem(RenderPopoverTouchBarProvider* provider);

    QString identifier;
    QAction* action;

    bool haveImage = false;;
    NSImageName image;

    NSTouchBarItem* touchBarItem;
};
typedef QSharedPointer<RenderPopoverTouchBarItem> TouchBarItemPtr;

@interface RenderPopoverTouchBarProvider: NSResponder <NSTouchBarDelegate, NSApplicationDelegate, NSWindowDelegate>

@property (strong) NSCustomTouchBarItem *firstFrameItem;
@property (strong) NSCustomTouchBarItem *lastFrameItem;
@property (strong) NSButton *firstFrameButton;
@property (strong) NSButton *lastFrameButton;

@property (strong) NSObject *qtDelegate;
@property RenderPopover *renderPopover;
@property Ui::RenderPopover *renderPopoverUi;
@property QList<TouchBarItemPtr> touchBarActionMapping;

@end

// Create identifiers for button items.
static NSTouchBarItemIdentifier startRenderIdentifier = @"com.vicr123.theframe.render.startRender";
static NSTouchBarItemIdentifier renderLabelIdentifier = @"com.vicr123.theframe.render.renderLabel";
static NSTouchBarItemIdentifier currentPaneIdentifier = @"com.vicr123.theframe.render.currentPane";

@implementation RenderPopoverTouchBarProvider

- (id)init: (RenderPopover*)mainWin withRenderPopoverUi:(Ui::RenderPopover*)ui {
    if (self = [super init]) {
        //Set main window UI to call touch bar handlers
        self.renderPopover = mainWin;
        self.renderPopoverUi = ui;

        //Initialise the action mapping
        self.touchBarActionMapping = {
            TouchBarItemPtr(new RenderPopoverTouchBarItem(QString::fromNSString(startRenderIdentifier), ui->actionStart_Rendering)),
            TouchBarItemPtr(new RenderPopoverTouchBarItem(QString::fromNSString(currentPaneIdentifier))),
            TouchBarItemPtr(new RenderPopoverTouchBarItem(QString::fromNSString(renderLabelIdentifier)))
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

    bar.defaultItemIdentifiers = @[renderLabelIdentifier, currentPaneIdentifier, startRenderIdentifier];
    bar.customizationRequiredItemIdentifiers = @[renderLabelIdentifier, currentPaneIdentifier, startRenderIdentifier];
    bar.customizationAllowedItemIdentifiers = @[renderLabelIdentifier, currentPaneIdentifier, startRenderIdentifier];
    bar.principalItemIdentifier = startRenderIdentifier;

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
        result->data()->action->trigger();
    }
}

- (void)touchBarSegmentedControlChanged: (NSSegmentedControl*) segmentedControl {
    self.renderPopoverUi->leftList->setCurrentRow([segmentedControl selectedSegment]);
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

- (RenderPopover*)getRenderPopover {
    return self.renderPopover;
}

- (Ui::RenderPopover*)getRenderPopoverUi {
    return self.renderPopoverUi;
}

@end

void RenderPopover::setupMacOS() {
    //Disable automatic window tabbing
    [NSWindow setAllowsAutomaticWindowTabbing:NO];

    //Install TouchBarProvider as window delegate
    NSView *view = reinterpret_cast<NSView *>(this->window()->winId());
    RenderPopoverTouchBarProvider *touchBarProvider = [[RenderPopoverTouchBarProvider alloc] init:this withRenderPopoverUi:ui];
    [touchBarProvider installAsDelegateForWindow:view.window];
}

//void RenderPopover::updateTouchBar() {
    //Invalidate Touch Bar
//    NSView *view = reinterpret_cast<NSView *>(this->winId());
//    view.window.touchBar = nil;
//}

RenderPopoverTouchBarItem::RenderPopoverTouchBarItem(QString identifier) {
    this->identifier = identifier;
}

RenderPopoverTouchBarItem::RenderPopoverTouchBarItem(QString identifier, QAction* action) {
    this->action = action;
    this->identifier = identifier;
}

RenderPopoverTouchBarItem::RenderPopoverTouchBarItem(QString identifier, QAction* action, NSImageName image) {
    this->action = action;
    this->identifier = identifier;
    this->image = image;
    this->haveImage = true;
}

RenderPopoverTouchBarItem::~RenderPopoverTouchBarItem()
{
    [this->touchBarItem release];
}

void RenderPopoverTouchBarItem::prepareTouchBarItem(RenderPopoverTouchBarProvider* provider) {
    if (identifier == QString::fromNSString(currentPaneIdentifier)) {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];

        NSSegmentedControl* segmentedControl = [[NSSegmentedControl alloc] init];
        [segmentedControl setSegmentCount:[provider getRenderPopoverUi]->leftList->count()];
        for (int i = 0; i < [provider getRenderPopoverUi]->leftList->count(); i++) {
            [segmentedControl setLabel:[provider getRenderPopoverUi]->leftList->item(i)->text().toNSString() forSegment:i];
        }

        QObject::connect([provider getRenderPopoverUi]->leftList, &QListWidget::currentRowChanged, [=](int currentRow) {
            [segmentedControl setSelectedSegment:currentRow];
        });
        [segmentedControl setSelectedSegment:[provider getRenderPopoverUi]->leftList->currentRow()];

        [segmentedControl setTarget:provider];
        [segmentedControl setAction:@selector(touchBarSegmentedControlChanged:)];

        item.view = segmentedControl;
        this->touchBarItem = item;
    } else if (identifier == QString::fromNSString(renderLabelIdentifier)) {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];

        NSTextField* label = [NSTextField labelWithString:QApplication::translate("RenderPopover", "Render:").toNSString()];
        item.view = label;
        this->touchBarItem = item;
    } else {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];
        [item setCustomizationLabel:action->text().toNSString()];

        NSButton* button = [NSButton buttonWithTitle:action->text().toNSString() target:provider action:@selector(touchBarActionClicked:)];
        [button setIdentifier:identifier.toNSString()];

        if (identifier == QString::fromNSString(startRenderIdentifier)) [button setBezelColor:NSColor.systemBlueColor];

        auto setEnabled = [=] {
            bool enabled = true;
            if (!action->isEnabled()) enabled = false;
            [button setEnabled:enabled];
        };
        setEnabled();

        if (haveImage) {
            [button setImage:[NSImage imageNamed:image]];
        }

        item.view = button;

        QObject::connect(action, &QAction::changed, [provider getRenderPopover], setEnabled);

        this->touchBarItem = item;
    }
}
