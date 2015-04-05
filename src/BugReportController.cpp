/*
 * BugReportController.cpp
 *
 *  Created on: 5 avr. 2015
 *      Author: pierre
 */


#include "BugReportController.hpp"



#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QRegExp>

#include <bb/cascades/AbstractPane>
#include <bb/cascades/GroupDataModel>

#include "BugDataObject.h"

#define GITHUB_URL      QString("https://api.github.com/repos/")
#define REPOSITORY      QString("amonchakai/hg10")


QMap<QString, int>  BugReportController::m_Labels;


BugReportController::BugReportController(QObject *parent) : QObject(parent), m_ListView(NULL), m_TypeIssue(0) {

    m_NetworkAccessManager = new QNetworkAccessManager(this);

    if(m_Labels.isEmpty()) {
        m_Labels["bug"] = 1;
        m_Labels["enhancement"] = 2;
        m_Labels["help wanted"] = 3;
        m_Labels["question"] = 4;
    }

}


void BugReportController::loadIssues(int typeIssues) {
//    curl -i  https://api.github.com/repos/amonchakai/hg10/issues
    m_TypeIssue = typeIssues;

    // get the list of issues
    const QUrl url(GITHUB_URL + REPOSITORY + "/issues");


    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");


    QNetworkReply* reply = m_NetworkAccessManager->get(request);
    bool ok = connect(reply, SIGNAL(finished()), this, SLOT(checkReplyIssues()));
    Q_ASSERT(ok);
    Q_UNUSED(ok);

}



void BugReportController::checkReplyIssues() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    QString response;
    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            const int available = reply->bytesAvailable();
            if (available > 0) {
                const QByteArray buffer(reply->readAll());
                response = QString::fromUtf8(buffer);
                parseIssues(response);
            }
        }

        reply->deleteLater();
    }
}


void BugReportController::parseIssues(const QString& page) {


    // ----------------------------------------------------------------------------------------------
    // Parse issues using regexp

    QRegExp regexp("\"labels_url\":"); // find the beginning of one issue

    regexp.setCaseSensitivity(Qt::CaseSensitive);
    regexp.setMinimal(true);

    m_Issues.clear();


    int pos = 0;
    int lastPos = regexp.indexIn(page, pos);

    while((pos = regexp.indexIn(page, lastPos)) != -1) {
        pos += regexp.matchedLength();
        // parse each post individually
        parseOneIssue(page.mid(lastPos, pos-lastPos));


        lastPos = pos;
    }
    parseOneIssue(page.mid(lastPos, pos-lastPos));

    updateView();
    emit completed();

}

void BugReportController::parseOneIssue(const QString& issue) {

    // ________________________________________________________
    // RegExp

    QRegExp number("\"number\":([0-9]+),");
    QRegExp title("\"title\":\"([^\"]+)\"");

    /*
     * "user": {
      "login": "amonchakai",
      "id": 6967372,
      "avatar_url": "https://avatars.githubusercontent.com/u/6967372?v=3",
     *
     */

    QRegExp user("\"user\":.*\"login\":\"([^\"]+)\".*\"avatar_url\":\"([^\"]+)\"");

    /*
    "labels": [
          {
            "url": "https://api.github.com/repos/amonchakai/Hg10/labels/bug",
            "name": "bug",
            "color": "fc2929"
          }
    */
    QRegExp label("\"labels\":.*\"name\":\"([^\"]+)\",.*\"color\":\"([^\"]+)\"");
    label.setMinimal(true);

    QRegExp state("\"state\":\"([^\"]+)\"");
    QRegExp locked("\"locked\":([^\"]+)");
    QRegExp comments("\"comments\":([0-9]+)");



    // ________________________________________________________
    // parse issue

    int pos = number.indexIn(issue);         if(pos == -1) return;
    pos += number.matchedLength();

    pos = title.indexIn(issue);
    pos += title.matchedLength();

    user.indexIn(issue, pos);
    pos += user.matchedLength();

    label.indexIn(issue, pos);
    pos += label.matchedLength();

    state.indexIn(issue, pos);
    pos += state.matchedLength();

    locked.indexIn(issue, pos);
    pos += locked.matchedLength();

    comments.indexIn(issue, pos);

    if(m_Labels[label.cap(1)] != m_TypeIssue)
        return;


    IssueListItem *item = new IssueListItem();
    item->setAuthor(user.cap(1));
    item->setAvatar(user.cap(2));
    item->setNumber(number.cap(1).toInt());
    item->setLocked(locked.cap(1) == "true");
    item->setState(state.cap(1));
    item->setTitle(title.cap(1));
    item->setComments(comments.cap(1).toInt());

    m_Issues.push_back(item);

    //qDebug() << number.cap(1) << title.cap(1) << user.cap(1) << user.cap(2) << label.cap(1) << label.cap(2) << state.cap(1) << locked.cap(1);

}

void BugReportController::updateView() {
    // ----------------------------------------------------------------------------------------------
        // get the dataModel of the listview if not already available
        using namespace bb::cascades;


        if(m_ListView == NULL) {
            qWarning() << "did not received the listview. quit.";
            return;
        }

        GroupDataModel* dataModel = dynamic_cast<GroupDataModel*>(m_ListView->dataModel());
        if (dataModel) {
            dataModel->clear();
        } else {
            qDebug() << "create new model";
            dataModel = new GroupDataModel(
                    QStringList() << "title"
                                  << "author"
                                  << "number"
                                  << "locked"
                                  << "avatar"
                                  << "comments"
                    );
            m_ListView->setDataModel(dataModel);
        }

        // ----------------------------------------------------------------------------------------------
        // push data to the view

        QList<QObject*> datas;
        for(int i = m_Issues.length()-1 ; i >= 0 ; --i) {
            datas.push_back(m_Issues.at(i));
        }

        dataModel->clear();
        dataModel->insertList(datas);
}


