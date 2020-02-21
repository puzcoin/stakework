// Copyright (c) 2011-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef STAKEWORK_QT_SENDCOINSENTRY_H
#define STAKEWORK_QT_SENDCOINSENTRY_H

#include <qt/walletmodel.h>

#include <QStackedWidget>

class WalletModel;
class PlatformStyle;

namespace Ui {
    class SendCoinsEntry;
}

/**
 * A single entry in the dialog for sending stakeworks.
 * Stacked widget, with different UIs for payment requests
 * with a strong payee identity.
 */
class SendCoinsEntry : public QStackedWidget
{
    Q_OBJECT

public:
    explicit SendCoinsEntry(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~SendCoinsEntry();

    void setModel(WalletModel *model);
    bool validate();
    SendCoinsRecipient getValue();

    /** Return whether the entry is still empty and unedited */
    bool isClear();

    void setValue(const SendCoinsRecipient &value);
    void setAddress(const QString &address);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases
     *  (issue https://bugreports.qt-project.org/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);

    void setFocus();

    void setTotalAmount(const CAmount& amount);

    CAmount totalAmount;

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void removeEntry(SendCoinsEntry *entry);
    void payAmountChanged();
    void subtractFeeFromAmountChanged();

private Q_SLOTS:
    void deleteClicked();
    void on_payTo_textChanged(const QString &address);
    void on_addressBookButton_clicked();
    void updateDisplayUnit();
    void updateAddressBook();
    void useFullAmount();
    void coinControlFeaturesChanged(bool enabled);
    void _coinControlFeaturesChanged(bool enabled);

private:
    SendCoinsRecipient recipient;
    Ui::SendCoinsEntry *ui;
    WalletModel *model;
    const PlatformStyle *platformStyle;

    bool updateLabel(const QString &address);
};

#endif // STAKEWORK_QT_SENDCOINSENTRY_H