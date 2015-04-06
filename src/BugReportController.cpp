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

#include <bb/cascades/Application>
#include <bb/cascades/ThemeSupport>
#include <bb/cascades/ColorTheme>
#include <bb/cascades/Theme>


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






// ========================================================================================================
// list all the issues corresponding to the specified category




void BugReportController::listIssues(int typeIssues) {
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






// ========================================================================================================
// Render the content of one issue

void BugReportController::loadIssue(int number) {
    m_IssueDescription.clear();

    // get the list of issues
    const QUrl url(GITHUB_URL + REPOSITORY + "/issues/" + QString::number(number));


    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");


    QNetworkReply* reply = m_NetworkAccessManager->get(request);
    bool ok = connect(reply, SIGNAL(finished()), this, SLOT(checkReplyIssueDescription()));
    Q_ASSERT(ok);
    Q_UNUSED(ok);

}



void BugReportController::checkReplyIssueDescription() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    QString response;
    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            const int available = reply->bytesAvailable();
            if (available > 0) {
                const QByteArray buffer(reply->readAll());
                response = QString::fromUtf8(buffer);
                parseIssuesDescription(response);



                // get the comments. If this is the main description of the page, the url end with the issue number.
                // then we only need to request /comments.
                QString isNumber = (reply->url().toString().mid(reply->url().toString().lastIndexOf("/")+1));
                bool ok;
                isNumber.toInt(&ok);
                if(ok) {
                    QNetworkRequest request(QUrl(reply->url().toString() + "/comments"));
                    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");


                    QNetworkReply* reply = m_NetworkAccessManager->get(request);
                    bool ok = connect(reply, SIGNAL(finished()), this, SLOT(checkReplyIssueDescription()));
                    Q_ASSERT(ok);
                    Q_UNUSED(ok);
                } else {

                    // if the URL end with comments, then we can do render the page
                    initWebPage();
                }
            }
        }

        reply->deleteLater();
    }
}

void BugReportController::parseIssuesDescription(const QString &page) {
    // ----------------------------------------------------------------------------------------------
    // Parse issues using regexp

    QRegExp regexp("\"user\":"); // find the beginning of one issue or comment

    regexp.setCaseSensitivity(Qt::CaseSensitive);
    regexp.setMinimal(true);


    int pos = 0;
    int lastPos = regexp.indexIn(page, pos);

    while((pos = regexp.indexIn(page, lastPos)) != -1) {
        pos += regexp.matchedLength();
        // parse each post individually
        parseOneDescription(page.mid(lastPos, pos-lastPos));


        lastPos = pos;
    }
    parseOneDescription(page.mid(lastPos, pos-lastPos));

}

void BugReportController::parseOneDescription(const QString &description) {

    // ________________________________________________________
    // RegExp

    /*
     * "user": {
      "login": "amonchakai",
      "id": 6967372,
      "avatar_url": "https://avatars.githubusercontent.com/u/6967372?v=3",
     *
     */

    QRegExp user("\"login\":\"([^\"]+)\".*\"avatar_url\":\"([^\"]+)\"");

    /*
    "labels": [
          {
            "url": "https://api.github.com/repos/amonchakai/Hg10/labels/bug",
            "name": "bug",
            "color": "fc2929"
          }
    */
    QRegExp created_at("\"created_at\":\"([^\"]+)\"");
    QRegExp body("\"body\":\"(.*)\"(,\"closed_by\":|" + QRegExp::escape("}") + ")");
    body.setMinimal(true);


    // ________________________________________________________
    // parse issue

    int pos = user.indexIn(description);         if(pos == -1) return;
    pos += user.matchedLength();

    pos = created_at.indexIn(description);
    pos += created_at.matchedLength();

    pos = body.indexIn(description);


    // --------------------------------------------------------
    // prepare rendering of messages


    QString blackTheme = "style=\"font-size:25px; \"";
    if(bb::cascades::Application::instance()->themeSupport()->theme()->colorTheme()->style() == bb::cascades::VisualStyle::Dark) {
        blackTheme = " style=\"background:#2E2E2E; font-size:25px; \" ";
    }

    QString message = body.cap(1);
    message.replace("\\r\\n", "<br/>");
    message.replace("\\\"", "\"");


    // ----------------------------------------------------------
    // replace images
    QRegExp imageRegExp(QRegExp::escape("![") + "img.[0-9]+.[0-9]+..(https://[^" + QRegExp::escape(")") + "]+)"+QRegExp::escape(")"));
    QString cleanPost = "";
    int lastPos = 0;
    pos = 0;
    while((pos = imageRegExp.indexIn(message, pos)) != -1) {
        cleanPost += message.mid(lastPos, pos-lastPos);

        cleanPost += "<img src=\"" + imageRegExp.cap(1) + "\" />";
        pos += imageRegExp.matchedLength();
        lastPos = pos;
    }
    cleanPost += message.mid(lastPos, message.length()-lastPos);
    message = cleanPost;


    // ----------------------------------------------------------
    // rendering

    m_IssueDescription +=
    QString("<div class=\"PostHeader\" >")
                + "<div style=\"height:80%; width:auto; position:relative; top:10%; left:5px; width:100px; display: inline-block;\" ><img src=\"" + user.cap(2) + "\" style=\"height:100%; width:auto; margin-left: auto; margin-right: auto; display: block;\" /></div>"
                + "<div class=\"PostHeader-Text\">"
                    + "<div style=\"position:relative; top:-20px;\"><p " + blackTheme +">" + user.cap(1) + "</p></div>"
                    + "<div style=\"position:relative; top:-35px; font-size:small;\"><p " + blackTheme +">" + created_at.cap(1) + "</p></div>"
                + "</div>"
             + "</div><p>" + message + "</p>";


}


void BugReportController::initWebPage() {
    // ----------------------------------------------------------------------------------------------
    // get the dataModel of the listview if not already available
    using namespace bb::cascades;


    if(m_WebView == NULL) {
        qWarning() << "did not received the listview. quit.";
        return;
    }

    QFile htmlTemplateFile(QDir::currentPath() + "/app/native/assets/template.html");
    if(bb::cascades::Application::instance()->themeSupport()->theme()->colorTheme()->style() == bb::cascades::VisualStyle::Dark) {
        htmlTemplateFile.setFileName(QDir::currentPath() + "/app/native/assets/template_black.html");
    }
    QFile htmlEndTemplateFile(QDir::currentPath() + "/app/native/assets/template_end.html");
    if (htmlTemplateFile.open(QIODevice::ReadOnly) && htmlEndTemplateFile.open(QIODevice::ReadOnly)) {
        QString htmlTemplate = htmlTemplateFile.readAll();


        QString endTemplate = htmlEndTemplateFile.readAll();



        m_WebView->setHtml(htmlTemplate + m_IssueDescription + endTemplate, QUrl("local:///assets/"));


    }

}
