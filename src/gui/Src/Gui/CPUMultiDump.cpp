#include "CPUMultiDump.h"
#include "Bridge.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QTabBar>

CPUMultiDump::CPUMultiDump(CPUDisassembly* disas, int nbCpuDumpTabs, QWidget* parent)
    : MHTabWidget(parent, true)
{
    mMaxCPUDumpTabs = nbCpuDumpTabs;
    mInitAllDumpTabs = false;

    for(uint i = 0; i < mMaxCPUDumpTabs; i++)
    {
        CPUDump* cpuDump = new CPUDump(disas, this);
        connect(cpuDump, SIGNAL(displayReferencesWidget()), this, SLOT(displayReferencesWidgetSlot()));
        this->addTabEx(cpuDump, QIcon(":/images/memory-map.png"), tr("Dump ") + QString::number(i + 1), QString("Dump ") + QString::number(i + 1));
    }

    mCurrentCPUDump = (CPUDump*)currentWidget();


    connect(this, SIGNAL(currentChanged(int)), this, SLOT(updateCurrentTabSlot(int)));
    connect(tabBar(), SIGNAL(OnDoubleClickTabIndex(int)), this, SLOT(openChangeTabTitleDialogSlot(int)));

    connect(Bridge::getBridge(), SIGNAL(dumpAt(dsint)), this, SLOT(printDumpAtSlot(dsint)));
    connect(Bridge::getBridge(), SIGNAL(dumpAtN(duint, int)), this, SLOT(printDumpAtNSlot(duint, int)));
    connect(Bridge::getBridge(), SIGNAL(selectionDumpGet(SELECTIONDATA*)), this, SLOT(selectionGetSlot(SELECTIONDATA*)));
    connect(Bridge::getBridge(), SIGNAL(selectionDumpSet(const SELECTIONDATA*)), this, SLOT(selectionSetSlot(const SELECTIONDATA*)));
    connect(Bridge::getBridge(), SIGNAL(dbgStateChanged(DBGSTATE)), this, SLOT(dbgStateChangedSlot(DBGSTATE)));
    connect(Bridge::getBridge(), SIGNAL(focusDump()), this, SLOT(focusCurrentDumpSlot()));

    connect(mCurrentCPUDump, SIGNAL(selectionUpdated()), mCurrentCPUDump, SLOT(selectionUpdatedSlot()));
}

CPUDump* CPUMultiDump::getCurrentCPUDump()
{
    return mCurrentCPUDump;
}

void CPUMultiDump::getTabNames(QList<QString> & names)
{
    bool addedDetachedWindows = false;
    names.clear();
    for(int i = 0; i < count(); i++)
    {
        // If empty name, then widget is detached
        if(this->tabBar()->tabText(i).length() == 0)
        {
            // If we added all the detached windows once, no need to do it again
            if(addedDetachedWindows)
                continue;

            QString windowName;
            // Loop through all detached widgets
            for(int n = 0; n < this->windows().size(); n++)
            {
                // Get the name and add it to the list
                windowName = ((MHDetachedWindow*)this->windows().at(n)->parent())->windowTitle();
                names.push_back(windowName);
            }
            addedDetachedWindows = true;
        }
        else
        {
            names.push_back(this->tabBar()->tabText(i));
        }
    }
}

int CPUMultiDump::getMaxCPUTabs()
{
    return mMaxCPUDumpTabs;
}

void CPUMultiDump::updateCurrentTabSlot(int tabIndex)
{
    mCurrentCPUDump = (CPUDump*)widget(tabIndex);
}

void CPUMultiDump::printDumpAtSlot(dsint parVa)
{
    if(mInitAllDumpTabs)
    {
        CPUDump* cpuDump = NULL;
        for(int i = 0; i < count(); i++)
        {
            cpuDump = (CPUDump*)widget(i);
            cpuDump->historyClear();
            cpuDump->addVaToHistory(parVa);
            cpuDump->printDumpAt(parVa);
        }

        mInitAllDumpTabs = false;
    }
    else
    {
        mCurrentCPUDump->printDumpAt(parVa);
        mCurrentCPUDump->addVaToHistory(parVa);
        mCurrentCPUDump->setFocus();
    }
}

void CPUMultiDump::printDumpAtNSlot(duint parVa, int index)
{
    setCurrentIndex(index);

    mCurrentCPUDump = (CPUDump*) widget(index);
    mCurrentCPUDump->printDumpAt(parVa);
    mCurrentCPUDump->addVaToHistory(parVa);
}

void CPUMultiDump::selectionGetSlot(SELECTIONDATA* selectionData)
{
    mCurrentCPUDump->selectionGet(selectionData);
}

void CPUMultiDump::selectionSetSlot(const SELECTIONDATA* selectionData)
{
    mCurrentCPUDump->selectionSet(selectionData);
}

void CPUMultiDump::dbgStateChangedSlot(DBGSTATE dbgState)
{
    if(dbgState == initialized)
    {
        mInitAllDumpTabs = true;
    }
}

void CPUMultiDump::openChangeTabTitleDialogSlot(int tabIndex)
{
    bool bUserPressedOk;
    QString sCurrentTabName = tabBar()->tabText(tabIndex);

    QString sNewTabName = QInputDialog::getText(this, tr("Change Tab %1 Name").arg(tabIndex + 1), tr("Tab Name"), QLineEdit::Normal, sCurrentTabName, &bUserPressedOk, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    if(bUserPressedOk)
    {
        if(sNewTabName.length() != 0)
            tabBar()->setTabText(tabIndex, sNewTabName);
    }
}

void CPUMultiDump::displayReferencesWidgetSlot()
{
    emit displayReferencesWidget();
}

void CPUMultiDump::focusCurrentDumpSlot()
{
    mCurrentCPUDump->setFocus();
}
