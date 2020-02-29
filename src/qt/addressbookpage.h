// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_ADDRESSBOOKPAGE_H
#define BITCOIN_QT_ADDRESSBOOKPAGE_H

#include <QDialog>
#include <QTableView>
class AddressTableModel;
class PaymentCodeTableModel;
class OptionsModel;
class PlatformStyle;
class ZCoinTableModel;

namespace Ui {
    class AddressBookPage;
}

QT_BEGIN_NAMESPACE
class QItemSelection;
class QMenu;
class QModelIndex;
class QSortFilterProxyModel;
class QTableView;
QT_END_NAMESPACE

/** Widget that shows a list of sending or receiving addresses.
  */
class AddressBookPage : public QDialog
{
    Q_OBJECT

public:
    enum Tabs {
        SendingTab = 0,
        ReceivingTab = 1
    };

    enum Mode {
        ForSelection, /**< Open address book to pick address */
        ForEditing  /**< Open address book for editing */
    };

    explicit AddressBookPage(const PlatformStyle *platformStyle, Mode mode, Tabs tab, QWidget *parent);
    ~AddressBookPage();

    void setModel(AddressTableModel *model);
    void setModel(PaymentCodeTableModel *pcodeModel);
    const QString &getReturnValue() const { return returnValue; }

public Q_SLOTS:
    void done(int retval);

private:
    Ui::AddressBookPage *ui;
    AddressTableModel *model;
    PaymentCodeTableModel *pcodeModel;
    ZCoinTableModel *pActiveAddressModel;
    Mode mode;
    Tabs tab;
    int pageMode;
    QString returnValue;
    QSortFilterProxyModel *proxyModel;
    QSortFilterProxyModel *pcodeProxyModel;
    QSortFilterProxyModel *pActiveProxyModel;
    QMenu *contextMenu;
    QAction *copyAddressAction;
    QAction *deleteAction; // to be able to explicitly disable it
    QAction *editAction;
    QString newAddressToSelect;

    QTableView *pActiveTableView;
    void setActiveModel();

private Q_SLOTS:
    /** TabWidget Changed **/
    void on_tabWidget_currentChanged(int index);
    
    /** Delete currently selected address entry */
    void on_deleteAddress_clicked();
    /** Create a new address for receiving coins and / or add a new address book entry */
    void on_newAddress_clicked();
    /** Copy address of currently selected address entry to clipboard */
    void on_copyAddress_clicked();
    /** Copy label of currently selected address entry to clipboard (no button) */
    void onCopyLabelAction();
    /** Edit currently selected address entry (no button) */
    void onEditAction();
    /** Export button clicked */
    void on_exportButton_clicked();

    /** Set button states based on selected tab and selection */
    void selectionChanged();
    /** Spawn contextual menu (right mouse menu) for address book entry */
    void contextualMenu(const QPoint &point);
    /** New entry/entries were added to address table */
    void selectNewAddress(const QModelIndex &parent, int begin, int /*end*/);
    
    /** @bip47 */
    void pcodeSelectionChanged();
    void selectNewPaymentCode(const QModelIndex &parent, int begin, int /*end*/);

    void selectionChanged(const QTableView *table);
    

Q_SIGNALS:
    void sendCoins(QString addr);
};

#endif // BITCOIN_QT_ADDRESSBOOKPAGE_H
