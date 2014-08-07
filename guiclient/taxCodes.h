/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXCODES_H
#define TAXCODES_H

#include "xwidget.h"
#include "ui_taxCodes.h"

class taxCodes : public XWidget, public Ui::taxCodes
{
    Q_OBJECT

public:
    taxCodes(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~taxCodes();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // TAXCODES_H
