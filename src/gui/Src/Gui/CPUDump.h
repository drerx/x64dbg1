#ifndef CPUDUMP_H
#define CPUDUMP_H

#include "HexDump.h"
#include "GotoDialog.h"
#include "CPUDisassembly.h"

//forward declaration
class CPUMultiDump;

class CPUDump : public HexDump
{
    Q_OBJECT
public:
    explicit CPUDump(CPUDisassembly* disas, CPUMultiDump* multiDump, QWidget* parent = 0);
    void getColumnRichText(int col, dsint rva, RichTextPainter::List & richText) override;
    QString paintContent(QPainter* painter, dsint rowBase, int rowOffset, int col, int x, int y, int w, int h);
    void setupContextMenu();
    void contextMenuEvent(QContextMenuEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

signals:
    void displayReferencesWidget();

public slots:
    void memoryAccessSingleshootSlot();
    void memoryAccessRestoreSlot();
    void memoryWriteSingleshootSlot();
    void memoryWriteRestoreSlot();
    void memoryExecuteSingleshootSlot();
    void memoryExecuteRestoreSlot();
    void memoryRemoveSlot();
    void hardwareAccess1Slot();
    void hardwareAccess2Slot();
    void hardwareAccess4Slot();
    void hardwareAccess8Slot();
    void hardwareWrite1Slot();
    void hardwareWrite2Slot();
    void hardwareWrite4Slot();
    void hardwareWrite8Slot();
    void hardwareExecuteSlot();
    void hardwareRemoveSlot();

    void setLabelSlot();
    void modifyValueSlot();
    void gotoExpressionSlot();
    void gotoFileOffsetSlot();
    void gotoStartSlot();
    void gotoEndSlot();

    void hexAsciiSlot();
    void hexUnicodeSlot();
    void hexCodepageSlot();

    void textAsciiSlot();
    void textUnicodeSlot();
    void textCodepageSlot();

    void integerSignedShortSlot();
    void integerSignedLongSlot();
    void integerSignedLongLongSlot();
    void integerUnsignedShortSlot();
    void integerUnsignedLongSlot();
    void integerUnsignedLongLongSlot();
    void integerHexShortSlot();
    void integerHexLongSlot();
    void integerHexLongLongSlot();

    void floatFloatSlot();
    void floatDoubleSlot();
    void floatLongDoubleSlot();

    void addressSlot();
    void disassemblySlot();

    void selectionGet(SELECTIONDATA* selection);
    void selectionSet(const SELECTIONDATA* selection);

    void binaryEditSlot();
    void binaryFillSlot();
    void binaryCopySlot();
    void binaryPasteSlot();
    void binaryPasteIgnoreSizeSlot();
    void binarySaveToFileSlot();
    void findPattern();
    void undoSelectionSlot();
    void followStackSlot();
    void findReferencesSlot();
    void followInDisasmSlot();
    void followDataSlot();
    void followDataDumpSlot();

    void watchSlot();

    void selectionUpdatedSlot();
    void yaraSlot();
    void dataCopySlot();
    void entropySlot();
    void syncWithExpressionSlot();
    void followInDumpNSlot();

    void gotoNextSlot();
    void gotoPrevSlot();

private:
    MenuBuilder* mMenuBuilder;

    QMenu* mBreakpointMenu;

    QMenu* mMemoryAccessMenu;
    QAction* mMemoryAccessSingleshoot;
    QAction* mMemoryAccessRestore;
    QMenu* mMemoryWriteMenu;
    QAction* mMemoryWriteSingleshoot;
    QAction* mMemoryWriteRestore;
    QMenu* mMemoryExecuteMenu;
    QAction* mMemoryExecuteSingleshoot;
    QAction* mMemoryExecuteRestore;
    QAction* mMemoryRemove;
    QMenu* mHardwareAccessMenu;
    QAction* mHardwareAccess1;
    QAction* mHardwareAccess2;
    QAction* mHardwareAccess4;
#ifdef _WIN64
    QAction* mHardwareAccess8;
#endif //_WIN64
    QMenu* mHardwareWriteMenu;
    QAction* mHardwareWrite1;
    QAction* mHardwareWrite2;
    QAction* mHardwareWrite4;
#ifdef _WIN64
    QAction* mHardwareWrite8;
#endif //_WIN64
    QAction* mHardwareExecute;
    QAction* mHardwareRemove;

    QAction* mFollowStack;

    QMenu* mGotoMenu;
    QAction* mGotoExpression;
    QAction* mGotoFileOffset;
    QAction* mGotoPrevious;
    QAction* mGotoNext;
    QAction* mGotoStart;
    QAction* mGotoEnd;

    QAction* mFollowInDisasm;

    QMenu* mHexMenu;
    QAction* mHexAsciiAction;
    QAction* mHexUnicodeAction;
    QAction* mHexCodepageAction;

    QMenu* mTextMenu;
    QAction* mTextAsciiAction;
    QAction* mTextUnicodeAction;
    QAction* mTextCodepageAction;

    QMenu* mIntegerMenu;
    QAction* mIntegerSignedShortAction;
    QAction* mIntegerSignedLongAction;
    QAction* mIntegerSignedLongLongAction;
    QAction* mIntegerUnsignedShortAction;
    QAction* mIntegerUnsignedLongAction;
    QAction* mIntegerUnsignedLongLongAction;
    QAction* mIntegerHexShortAction;
    QAction* mIntegerHexLongAction;
    QAction* mIntegerHexLongLongAction;

    QMenu* mFloatMenu;
    QAction* mFloatFloatAction;
    QAction* mFloatDoubleAction;
    QAction* mFloatLongDoubleAction;

    QAction* mAddressAction;
    QAction* mDisassemblyAction;

    QAction* mSetLabelAction;
    QAction* mModifyValueAction;

    QMenu* mBinaryMenu;
    QAction* mBinaryEditAction;
    QAction* mBinaryFillAction;
    QAction* mBinaryCopyAction;
    QAction* mBinaryPasteAction;
    QAction* mBinaryPasteIgnoreSizeAction;
    QAction* mBinarySaveToFile;
    QAction* mFindPatternAction;
    QAction* mFindReferencesAction;
    QAction* mYaraAction;
    QAction* mDataCopyAction;
    QAction* mUndoSelection;
    QAction* mFollowData;
    QAction* mFollowDataDump;
    QAction* mSyncWithExpression;
    QAction* mEntropy;
    QMenu* mSpecialMenu;
    QMenu* mCustomMenu;
    QMenu* mPluginMenu;
    QMenu* mFollowInDumpMenu;
    QList<QAction*> mFollowInDumpActions;

    GotoDialog* mGoto;
    CPUDisassembly* mDisas;
    CPUMultiDump* mMultiDump;

    enum ViewEnum_t
    {
        ViewHexAscii = 0,
        ViewHexUnicode,
        ViewTextAscii,
        ViewTextUnicode,
        ViewIntegerSignedShort,
        ViewIntegerSignedLong,
        ViewIntegerSignedLongLong,
        ViewIntegerUnsignedShort,
        ViewIntegerUnsignedLong,
        ViewIntegerUnsignedLongLong,
        ViewIntegerHexShort,
        ViewIntegerHexLong,
        ViewIntegerHexLongLong,
        ViewFloatFloat,
        ViewFloatDouble,
        ViewFloatLongDouble,
        ViewAddress
    };

    void setView(ViewEnum_t view);
};

#endif // CPUDUMP_H
