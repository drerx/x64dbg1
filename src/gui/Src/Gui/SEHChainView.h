#ifndef SEHCHAINVIEW_H
#define SEHCHAINVIEW_H

#include "StdTable.h"

class SEHChainView : public StdTable
{
    Q_OBJECT
    Q_PROPERTY(int viewId MEMBER m_viewId)
public:
    explicit SEHChainView(StdTable* parent = 0);
    void setupContextMenu();
	
signals:
    void showCpu();

protected slots:
    void updateSEHChain();
    void contextMenuSlot(const QPoint pos);
    void doubleClickedSlot();
    void followAddress();
    void followHandler();

private:
    int m_viewId;
    QAction* mFollowAddress;
    QAction* mFollowHandler;
};

#endif // SEHCHAINVIEW_H
