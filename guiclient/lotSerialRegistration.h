/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#ifndef LOTSERIALREGISTRATION_H
#define LOTSERIALREGISTRATION_H

#include "guiclient.h"
#include "xdialog.h"
#include <QStringList>
#include <QSqlTableModel>
#include <QDataWidgetMapper>
#include <parameter.h>

#include "ui_lotSerialRegistration.h"

class lotSerialRegistration : public XDialog, public Ui::lotSerialRegistration
{
    Q_OBJECT

public:
    lotSerialRegistration(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~lotSerialRegistration();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sNew();
    virtual void sEdit();
    virtual void sSave();
    virtual void sUndo();
    virtual void sNewCharass();
    virtual void sEditCharass();
    virtual void sDeleteCharass();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    int               _mode;
    QString           _number;
    QSqlTableModel    _model;
    QDataWidgetMapper _mapper;
    
    enum {
      REGISTRATION_NUMBER = 0,
      TYPE                = 1,
      REGISTER_DATE       = 2,
      SOLD_DATE           = 3,
      EXPIRE_DATE         = 4,
      CRM_ACCOUNT         = 5,
      ITEM_NUMBER         = 6,
      LOTSERIAL_NUMBER    = 7,
      CONTACT_NUMBER      = 8,
      HONORIFIC           = 9,
      FIRST               = 10,
      LAST                = 11,
      JOB_TITLE           = 12,
      VOICE               = 13,
      FAX                 = 14,
      EMAIL               = 15,
      ADDRESS_CHANGE      = 16,
      ADDRESS1            = 17,
      ADDRESS2            = 18,
      ADDRESS3            = 19,
      CITY                = 20,
      STATE               = 21,
      POSTAL_CODE         = 22,
      COUNTRY             = 23,
      SALES_ORDER_NUMBER  = 24,
      SHIPMENT_NUMBER     = 25,
      NOTES               = 26};
};

#endif // LOTSERIALREGISTRATION_H
