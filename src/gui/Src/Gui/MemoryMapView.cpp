#include <QFileDialog>

#include "MemoryMapView.h"
#include "Configuration.h"
#include "Bridge.h"
#include "PageMemoryRights.h"
#include "YaraRuleSelectionDialog.h"
#include "EntropyDialog.h"
#include "HexEditDialog.h"

MemoryMapView::MemoryMapView(StdTable* parent) : StdTable(parent)
{
    enableMultiSelection(false);

    int charwidth = getCharWidth();

    addColumnAt(8 + charwidth * 2 * sizeof(duint), tr("Address"), false, tr("Address")); //addr
    addColumnAt(8 + charwidth * 2 * sizeof(duint), tr("Size"), false, tr("Size")); //size
    addColumnAt(8 + charwidth * 32, tr("Info"), false, tr("Page Information")); //page information
    addColumnAt(8 + charwidth * 5, tr("Type"), false, tr("Allocation Type")); //allocation type
    addColumnAt(8 + charwidth * 11, tr("Protection"), false, tr("Current Protection")); //current protection
    addColumnAt(8 + charwidth * 8, tr("Initial"), false, tr("Allocation Protection")); //allocation protection
    addColumnAt(100, "", false);
    loadColumnFromConfig("MemoryMap");

    connect(Bridge::getBridge(), SIGNAL(updateMemory()), this, SLOT(refreshMap()));
    connect(Bridge::getBridge(), SIGNAL(dbgStateChanged(DBGSTATE)), this, SLOT(stateChangedSlot(DBGSTATE)));
    connect(this, SIGNAL(contextMenuSignal(QPoint)), this, SLOT(contextMenuSlot(QPoint)));

    setupContextMenu();
}

void MemoryMapView::setupContextMenu()
{
    //Follow in Dump
    mFollowDump = new QAction(tr("&Follow in Dump"), this);
    connect(mFollowDump, SIGNAL(triggered()), this, SLOT(followDumpSlot()));

    //Follow in Disassembler
    mFollowDisassembly = new QAction(tr("Follow in &Disassembler"), this);
    mFollowDisassembly->setShortcutContext(Qt::WidgetShortcut);
    mFollowDisassembly->setShortcut(QKeySequence("enter"));
    connect(mFollowDisassembly, SIGNAL(triggered()), this, SLOT(followDisassemblerSlot()));
    connect(this, SIGNAL(enterPressedSignal()), this, SLOT(followDisassemblerSlot()));
    connect(this, SIGNAL(doubleClickedSignal()), this, SLOT(followDisassemblerSlot()));

    //Yara
    mYara = new QAction(QIcon(":/icons/images/yara.png"), "&Yara...", this);
    connect(mYara, SIGNAL(triggered()), this, SLOT(yaraSlot()));

    //Set PageMemory Rights
    mPageMemoryRights = new QAction(tr("Set Page Memory Rights"), this);
    connect(mPageMemoryRights, SIGNAL(triggered()), this, SLOT(pageMemoryRights()));

    //Switch View
    mSwitchView = new QAction(tr("&Switch View"), this);
    connect(mSwitchView, SIGNAL(triggered()), this, SLOT(switchView()));

    //Breakpoint menu
    mBreakpointMenu = new QMenu(tr("Memory &Breakpoint"), this);

    //Breakpoint->Memory Access
    mMemoryAccessMenu = new QMenu(tr("Access"), this);
    mMemoryAccessSingleshoot = new QAction(tr("&Singleshoot"), this);
    connect(mMemoryAccessSingleshoot, SIGNAL(triggered()), this, SLOT(memoryAccessSingleshootSlot()));
    mMemoryAccessMenu->addAction(mMemoryAccessSingleshoot);
    mMemoryAccessRestore = new QAction(tr("&Restore"), this);
    connect(mMemoryAccessRestore, SIGNAL(triggered()), this, SLOT(memoryAccessRestoreSlot()));
    mMemoryAccessMenu->addAction(mMemoryAccessRestore);
    mBreakpointMenu->addMenu(mMemoryAccessMenu);

    //Breakpoint->Memory Write
    mMemoryWriteMenu = new QMenu(tr("Write"), this);
    mMemoryWriteSingleshoot = new QAction(tr("&Singleshoot"), this);
    connect(mMemoryWriteSingleshoot, SIGNAL(triggered()), this, SLOT(memoryWriteSingleshootSlot()));
    mMemoryWriteMenu->addAction(mMemoryWriteSingleshoot);
    mMemoryWriteRestore = new QAction(tr("&Restore"), this);
    connect(mMemoryWriteRestore, SIGNAL(triggered()), this, SLOT(memoryWriteRestoreSlot()));
    mMemoryWriteMenu->addAction(mMemoryWriteRestore);
    mBreakpointMenu->addMenu(mMemoryWriteMenu);

    //Breakpoint->Memory Execute
    mMemoryExecuteMenu = new QMenu(tr("Execute"), this);
    mMemoryExecuteSingleshoot = new QAction(tr("&Singleshoot"), this);
    mMemoryExecuteSingleshoot->setShortcutContext(Qt::WidgetShortcut);
    connect(mMemoryExecuteSingleshoot, SIGNAL(triggered()), this, SLOT(memoryExecuteSingleshootSlot()));
    mMemoryExecuteMenu->addAction(mMemoryExecuteSingleshoot);
    mMemoryExecuteRestore = new QAction(tr("&Restore"), this);
    connect(mMemoryExecuteRestore, SIGNAL(triggered()), this, SLOT(memoryExecuteRestoreSlot()));
    mMemoryExecuteMenu->addAction(mMemoryExecuteRestore);
    mBreakpointMenu->addMenu(mMemoryExecuteMenu);

    //Breakpoint->Remove
    mMemoryRemove = new QAction(tr("&Remove"), this);
    mMemoryRemove->setShortcutContext(Qt::WidgetShortcut);
    connect(mMemoryRemove, SIGNAL(triggered()), this, SLOT(memoryRemoveSlot()));
    mBreakpointMenu->addAction(mMemoryRemove);

    //Action shortcut action that does something
    mMemoryExecuteSingleshootToggle = new QAction(this);
    mMemoryExecuteSingleshootToggle->setShortcutContext(Qt::WidgetShortcut);
    this->addAction(mMemoryExecuteSingleshootToggle);
    connect(mMemoryExecuteSingleshootToggle, SIGNAL(triggered()), this, SLOT(memoryExecuteSingleshootToggleSlot()));

    //Entropy
    mEntropy = new QAction(QIcon(":/icons/images/entropy.png"), tr("Entropy..."), this);
    connect(mEntropy, SIGNAL(triggered()), this, SLOT(entropy()));

    //Find
    mFindPattern = new QAction(QIcon(":/icons/images/search-for.png"), tr("&Find Pattern..."), this);
    this->addAction(mFindPattern);
    mFindPattern->setShortcutContext(Qt::WidgetShortcut);
    connect(mFindPattern, SIGNAL(triggered()), this, SLOT(findPatternSlot()));

    //Dump
    mDumpMemory = new QAction(tr("&Dump Memory to File"), this);
    connect(mDumpMemory, SIGNAL(triggered()), this, SLOT(dumpMemory()));

    refreshShortcutsSlot();
    connect(Config(), SIGNAL(shortcutsUpdated()), this, SLOT(refreshShortcutsSlot()));
}

void MemoryMapView::refreshShortcutsSlot()
{
    mMemoryExecuteSingleshoot->setShortcut(ConfigShortcut("ActionToggleBreakpoint"));
    mMemoryRemove->setShortcut(ConfigShortcut("ActionToggleBreakpoint"));
    mMemoryExecuteSingleshootToggle->setShortcut(ConfigShortcut("ActionToggleBreakpoint"));
    mFindPattern->setShortcut(ConfigShortcut("ActionFindPattern"));
}

void MemoryMapView::contextMenuSlot(const QPoint & pos)
{
    if(!DbgIsDebugging())
        return;
    QMenu wMenu(this); //create context menu
    wMenu.addAction(mDumpMemory);
    wMenu.addAction(mFollowDisassembly);
    wMenu.addAction(mFollowDump);
    wMenu.addAction(mYara);
    wMenu.addAction(mEntropy);
    wMenu.addAction(mFindPattern);
    wMenu.addAction(mSwitchView);
    wMenu.addSeparator();
    wMenu.addAction(mPageMemoryRights);
    wMenu.addSeparator();
    wMenu.addMenu(mBreakpointMenu);
    QMenu wCopyMenu(tr("&Copy"), this);
    setupCopyMenu(&wCopyMenu);
    if(wCopyMenu.actions().length())
    {
        wMenu.addSeparator();
        wMenu.addMenu(&wCopyMenu);
    }

    QString wStr = getCellContent(getInitialSelection(), 0);
#ifdef _WIN64
    duint selectedAddr = wStr.toULongLong(0, 16);
#else //x86
    duint selectedAddr = wStr.toULong(0, 16);
#endif //_WIN64
    if((DbgGetBpxTypeAt(selectedAddr)&bp_memory) == bp_memory) //memory breakpoint set
    {
        mMemoryAccessMenu->menuAction()->setVisible(false);
        mMemoryWriteMenu->menuAction()->setVisible(false);
        mMemoryExecuteMenu->menuAction()->setVisible(false);
        mMemoryRemove->setVisible(true);
    }
    else //memory breakpoint not set
    {
        mMemoryAccessMenu->menuAction()->setVisible(true);
        mMemoryWriteMenu->menuAction()->setVisible(true);
        mMemoryExecuteMenu->menuAction()->setVisible(true);
        mMemoryRemove->setVisible(false);
    }

    wMenu.exec(mapToGlobal(pos)); //execute context menu
}

QString MemoryMapView::getProtectionString(DWORD Protect)
{
#define RIGHTS_STRING (sizeof("ERWCG") + 1)
    char rights[RIGHTS_STRING];

    if(!DbgFunctions()->PageRightsToString(Protect, rights))
        return "bad";

    return QString(rights);
}

QString MemoryMapView::paintContent(QPainter* painter, dsint rowBase, int rowOffset, int col, int x, int y, int w, int h)
{
    if(col == 0) //address
    {
        QString wStr = getCellContent(rowBase + rowOffset, col);
#ifdef _WIN64
        duint addr = wStr.toULongLong(0, 16);
#else //x86
        duint addr = wStr.toULong(0, 16);
#endif //_WIN64
        if((DbgGetBpxTypeAt(addr)&bp_memory) == bp_memory)
        {
            QString wStr = getCellContent(rowBase + rowOffset, col);
            QColor bpBackgroundColor = ConfigColor("MemoryMapBreakpointBackgroundColor");
            if(bpBackgroundColor.alpha())
                painter->fillRect(QRect(x, y, w - 1, h), QBrush(bpBackgroundColor));
            painter->setPen(ConfigColor("MemoryMapBreakpointColor"));
            painter->drawText(QRect(x + 4, y, getColumnWidth(col) - 4, getRowHeight()), Qt::AlignVCenter | Qt::AlignLeft, wStr);
            return "";
        }
        else if(isSelected(rowBase, rowOffset) == true)
            painter->fillRect(QRect(x, y, w, h), QBrush(selectionColor));
    }
    else if(col == 2) //info
    {
        QString wStr = StdTable::paintContent(painter, rowBase, rowOffset, col, x, y, w, h);;
        if(wStr.startsWith(" \""))
        {
            painter->setPen(ConfigColor("MemoryMapSectionTextColor"));
            painter->drawText(QRect(x + 4, y, getColumnWidth(col) - 4, getRowHeight()), Qt::AlignVCenter | Qt::AlignLeft, wStr);
            return "";
        }
    }
    else if(col == 4) //CPROT
    {
        QString wStr = StdTable::paintContent(painter, rowBase, rowOffset, col, x, y, w, h);;
        if(!ConfigBool("Engine", "ListAllPages"))
        {
            painter->setPen(ConfigColor("MemoryMapSectionTextColor"));
            painter->drawText(QRect(x + 4, y, getColumnWidth(col) - 4, getRowHeight()), Qt::AlignVCenter | Qt::AlignLeft, wStr);
            return "";
        }
    }
    return StdTable::paintContent(painter, rowBase, rowOffset, col, x, y, w, h);
}

void MemoryMapView::refreshMap()
{
    MEMMAP wMemMapStruct;
    int wI;

    memset(&wMemMapStruct, 0, sizeof(MEMMAP));

    DbgMemMap(&wMemMapStruct);

    setRowCount(wMemMapStruct.count);

    for(wI = 0; wI < wMemMapStruct.count; wI++)
    {
        QString wS;
        MEMORY_BASIC_INFORMATION wMbi = (wMemMapStruct.page)[wI].mbi;

        // Base address
        wS = QString("%1").arg((duint)wMbi.BaseAddress, sizeof(duint) * 2, 16, QChar('0')).toUpper();
        setCellContent(wI, 0, wS);

        // Size
        wS = QString("%1").arg((duint)wMbi.RegionSize, sizeof(duint) * 2, 16, QChar('0')).toUpper();
        setCellContent(wI, 1, wS);

        // Information
        wS = QString((wMemMapStruct.page)[wI].info);
        setCellContent(wI, 2, wS);

        // State
        switch(wMbi.State)
        {
        case MEM_FREE:
            wS = QString("FREE");
            break;
        case MEM_COMMIT:
            wS = QString("COMM");
            break;
        case MEM_RESERVE:
            wS = QString("RESV");
            break;
        default:
            wS = QString("????");
        }
        setCellContent(wI, 3, wS);

        // Type
        switch(wMbi.Type)
        {
        case MEM_IMAGE:
            wS = QString("IMG");
            break;
        case MEM_MAPPED:
            wS = QString("MAP");
            break;
        case MEM_PRIVATE:
            wS = QString("PRV");
            break;
        default:
            wS = QString("N/A");
            break;
        }
        setCellContent(wI, 3, wS);

        // current access protection
        wS = getProtectionString(wMbi.Protect);
        setCellContent(wI, 4, wS);

        // allocation protection
        wS = getProtectionString(wMbi.AllocationProtect);
        setCellContent(wI, 5, wS);

    }
    if(wMemMapStruct.page != 0)
        BridgeFree(wMemMapStruct.page);
    reloadData(); //refresh memory map
}

void MemoryMapView::stateChangedSlot(DBGSTATE state)
{
    if(state == paused)
        refreshMap();
}

void MemoryMapView::followDumpSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
    DbgCmdExecDirect(QString("dump " + addr_text).toUtf8().constData());
    emit showCpu();
}

void MemoryMapView::followDisassemblerSlot()
{
    QString commandText = QString("disasm %1").arg(getCellContent(getInitialSelection(), 0));

    // If there was no address loaded, the length
    // will only be the command length
    if(commandText.length() <= 8)
        return;

    DbgCmdExecDirect(commandText.toUtf8().constData());
    emit showCpu();
}

void MemoryMapView::yaraSlot()
{
    YaraRuleSelectionDialog yaraDialog(this);
    if(yaraDialog.exec() == QDialog::Accepted)
    {
        QString addr_text = getCellContent(getInitialSelection(), 0);
        QString size_text = getCellContent(getInitialSelection(), 1);
        DbgCmdExec(QString("yara \"%0\",%1,%2").arg(yaraDialog.getSelectedFile()).arg(addr_text).arg(size_text).toUtf8().constData());
        emit showReferences();
    }
}

void MemoryMapView::memoryAccessSingleshootSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
    DbgCmdExec(QString("bpm " + addr_text + ", 0, r").toUtf8().constData());
}

void MemoryMapView::memoryAccessRestoreSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
    DbgCmdExec(QString("bpm " + addr_text + ", 1, r").toUtf8().constData());
}

void MemoryMapView::memoryWriteSingleshootSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
    DbgCmdExec(QString("bpm " + addr_text + ", 0, w").toUtf8().constData());
}

void MemoryMapView::memoryWriteRestoreSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
    DbgCmdExec(QString("bpm " + addr_text + ", 1, w").toUtf8().constData());
}

void MemoryMapView::memoryExecuteSingleshootSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
    DbgCmdExec(QString("bpm " + addr_text + ", 0, x").toUtf8().constData());
}

void MemoryMapView::memoryExecuteRestoreSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
    DbgCmdExec(QString("bpm " + addr_text + ", 1, x").toUtf8().constData());
}

void MemoryMapView::memoryRemoveSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
    DbgCmdExec(QString("bpmc " + addr_text).toUtf8().constData());
}

void MemoryMapView::memoryExecuteSingleshootToggleSlot()
{
    QString addr_text = getCellContent(getInitialSelection(), 0);
#ifdef _WIN64
    duint selectedAddr = addr_text.toULongLong(0, 16);
#else //x86
    duint selectedAddr = addr_text.toULong(0, 16);
#endif //_WIN64
    if((DbgGetBpxTypeAt(selectedAddr)&bp_memory) == bp_memory) //memory breakpoint set
        memoryRemoveSlot();
    else
        memoryExecuteSingleshootSlot();
}

void MemoryMapView::pageMemoryRights()
{
    PageMemoryRights PageMemoryRightsDialog(this);
    connect(&PageMemoryRightsDialog, SIGNAL(refreshMemoryMap()), this, SLOT(refreshMap()));
    duint addr = getCellContent(getInitialSelection(), 0).toULongLong(0, 16);
    duint size = getCellContent(getInitialSelection(), 1).toULongLong(0, 16);
    PageMemoryRightsDialog.RunAddrSize(addr, size, getCellContent(getInitialSelection(), 3));
}

void MemoryMapView::switchView()
{
    Config()->setBool("Engine", "ListAllPages", !ConfigBool("Engine", "ListAllPages"));
    Config()->writeBools();
    DbgSettingsUpdated();
    DbgFunctions()->MemUpdateMap();
    setSingleSelection(0);
    setTableOffset(0);
    stateChangedSlot(paused);
}

void MemoryMapView::entropy()
{
    duint addr = getCellContent(getInitialSelection(), 0).toULongLong(0, 16);
    duint size = getCellContent(getInitialSelection(), 1).toULongLong(0, 16);
    unsigned char* data = new unsigned char[size];
    DbgMemRead(addr, data, size);

    EntropyDialog entropyDialog(this);
    entropyDialog.setWindowTitle(QString().sprintf("Entropy (Address: %p, Size: %p)", addr, size));
    entropyDialog.show();
    entropyDialog.GraphMemory(data, size);
    entropyDialog.exec();

    delete[] data;
}

void MemoryMapView::findPatternSlot()
{
    HexEditDialog hexEdit(this);
    hexEdit.showEntireBlock(true);
    hexEdit.mHexEdit->setOverwriteMode(false);
    hexEdit.setWindowTitle("Find Pattern...");
    if(hexEdit.exec() != QDialog::Accepted)
        return;
    duint addr = getCellContent(getInitialSelection(), 0).toULongLong(0, 16);
    if(hexEdit.entireBlock())
        addr = 0;
    QString addrText = QString("%1").arg(addr, sizeof(dsint) * 2, 16, QChar('0')).toUpper();
    DbgCmdExec(QString("findmemall " + addrText + ", \"" + hexEdit.mHexEdit->pattern() + "\", &data&").toUtf8().constData());
    emit showReferences();
}

void MemoryMapView::dumpMemory()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Memory Region", QDir::currentPath(), tr("All files (*.*)"));

    if(fileName.length())
    {
        fileName = QDir::toNativeSeparators(fileName);
        QString cmd = QString("savedata ""%1"",%2,%3").arg(fileName, getCellContent(getInitialSelection(), 0),
                      getCellContent(getInitialSelection(), 1));
        DbgCmdExec(cmd.toUtf8().constData());
    }
}

