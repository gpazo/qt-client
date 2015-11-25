/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtNetworkRequestManager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QDebug>

#define DEBUG false

xtNetworkRequestManager::xtNetworkRequestManager(const QUrl & url, QMutex &mutex, const QString &params) {
  nwam = new QNetworkAccessManager;
  _nwrep = 0;
  _response = 0;
  _url = url;
  _params = params;
  _mutex = &mutex;
  _mutex->lock();
  _loop = new QEventLoop;
  startRequest(_url);
}
void xtNetworkRequestManager::startRequest(const QUrl & url) {
    _nwrep = nwam->post(QNetworkRequest(url), _params.toUtf8());
    connect(_nwrep, SIGNAL(finished()), SLOT(requestCompleted()));
    connect(nwam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
    connect(nwam, SIGNAL(finished(QNetworkReply*)), _loop, SLOT(quit()));
    _loop->exec();
}
void xtNetworkRequestManager::requestCompleted() {
  _response = _nwrep->readAll(); //we don't really care here but store it anyways
  _nwrep->close();
  QVariant possibleRedirect = _nwrep->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if(DEBUG){
      qDebug() << "redirect=" << possibleRedirect.isValid();
      qDebug() << "replyError=" << _nwrep->errorString();
      qDebug() << "replyErrorCode=" << _nwrep->error();
  }
  if(_nwrep->error() != QNetworkReply::NoError){
      qDebug() << "network reply error on request" << _nwrep->error() << _nwrep->errorString();
      _nwrep->deleteLater();
      _mutex->unlock();
  }
  if(_nwrep->error() == QNetworkReply::NoError && !possibleRedirect.isValid()){
      //success and no redirect
      _nwrep->deleteLater();
      _mutex->unlock();
  }
  else {
      QUrl newUrl = _url.resolved(possibleRedirect.toUrl());
      _nwrep->deleteLater();
      startRequest(newUrl);
  }
}
void xtNetworkRequestManager::sslErrors(QNetworkReply*, const QList<QSslError> &errors) {
    QString errorString;
       foreach (const QSslError &error, errors) {
           if (!errorString.isEmpty())
               errorString += ", ";
           errorString += error.errorString();
       }
   qDebug() << "errorString= " << errorString;
}
QByteArray xtNetworkRequestManager::response() {
    return _response;
}
xtNetworkRequestManager::~xtNetworkRequestManager() {
    nwam->deleteLater();
    nwam = 0;
}

