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

class BugReportController : public QObject {
    Q_OBJECT;


private:

    bb::cascades::ListView          *m_ListView;


public:
    BugReportController              (QObject *parent = 0);
    virtual ~BugReportController     ()                               {};



public Q_SLOTS:

    inline void setListView          (QObject *list)                  { m_ListView = dynamic_cast<bb::cascades::ListView*>(list); };


    void loadIssues                  (int typeIssues);


Q_SIGNALS:




};




#endif /* BUGREPORTCONTROLLER_HPP_ */
