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
#include <bb/cascades/WebView>

class QNetworkAccessManager;
class IssueListItem;

class BugReportController : public QObject {
    Q_OBJECT;


private:

    static QMap<QString, int>        m_Labels;

    bb::cascades::ListView          *m_ListView;
    bb::cascades::WebView           *m_WebView;
    QNetworkAccessManager           *m_NetworkAccessManager;

    int                              m_TypeIssue;
    QList<IssueListItem*>            m_Issues;
    QString                          m_IssueDescription;



    void parseIssues                 (const QString& page);
    void parseOneIssue               (const QString& issue);
    void updateView                  ();

    void parseIssuesDescription      (const QString &page);
    void parseOneDescription         (const QString &description);
    void initWebPage                 ();


public:
    BugReportController              (QObject *parent = 0);
    virtual ~BugReportController     ()                               {};



public Q_SLOTS:

    inline void setListView          (QObject *list)                  { m_ListView = dynamic_cast<bb::cascades::ListView*>(list); };
    inline void setWebView           (QObject *web)                   { m_WebView = dynamic_cast<bb::cascades::WebView*>(web); };


    void listIssues                  (int typeIssues);
    void loadIssue                   (int issue);



    void checkReplyIssues            ();
    void checkReplyIssueDescription  ();

Q_SIGNALS:

    void completed();


};




#endif /* BUGREPORTCONTROLLER_HPP_ */
