/*
 * BugReportController.hpp
 *
 *  Created on: 5 avr. 2015
 *      Author: pierre
 */

#ifndef BUGREPORTCONTROLLER_HPP_
#define BUGREPORTCONTROLLER_HPP_

#include <QtCore/QObject>
#include <bb/cascades/ListView>

class QNetworkAccessManager;
class IssueListItem;

class BugReportController : public QObject {
    Q_OBJECT;


private:

    static QMap<QString, int>        m_Labels;

    bb::cascades::ListView          *m_ListView;
    QNetworkAccessManager           *m_NetworkAccessManager;

    int                              m_TypeIssue;
    QList<IssueListItem*>            m_Issues;



    void parseIssues                 (const QString& page);
    void parseOneIssue               (const QString& issue);
    void updateView                  ();


public:
    BugReportController              (QObject *parent = 0);
    virtual ~BugReportController     ()                               {};



public Q_SLOTS:

    inline void setListView          (QObject *list)                  { m_ListView = dynamic_cast<bb::cascades::ListView*>(list); };


    void loadIssues                  (int typeIssues);



    void checkReplyIssues            ();


Q_SIGNALS:

    void completed();


};




#endif /* BUGREPORTCONTROLLER_HPP_ */
